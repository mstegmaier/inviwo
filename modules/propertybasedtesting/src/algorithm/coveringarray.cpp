#include <inviwo/propertybasedtesting/algorithm/coveringarray.h>

namespace inviwo {

namespace util {

// 2-coverage, randomized discrete SLJ strategy
std::vector<Test> coveringArray(const Test& init, const std::vector<std::vector< std::shared_ptr<PropertyAssignment> >>& vars) {
	srand(42); // deterministic for regression testing

	std::cerr << "coveringArray: vars.size() = " << vars.size() << std::endl;
	assert(vars.size() > 0);

	// special case
	if(vars.size() == 1) {
		std::vector<Test> res(vars[0].size(), init);
		for(size_t i = 0; i < vars[0].size(); i++) {
			res[i].emplace_back(vars[0][i]);
		}
		std::cerr << "special case : res.size() = " << res.size() << std::endl;
		return res;
	}

	const size_t v = [&](){
			size_t res = 0;
			for(const auto& v : vars) res = std::max(res, v.size());
			return res;
		}();

	std::unordered_set<size_t> uncovered;
	std::map< std::array<size_t,4>, size_t > idx;
	for(size_t i = 1; i < vars.size(); i++) {
		for(size_t j = 0; j < i; j++) {
			for(size_t ii = 0; ii < vars[i].size(); ii++) {
				for(size_t ji = 0; ji < vars[j].size(); ji++) {
					uncovered.insert(idx.size());
					idx[{i,j,ii,ji}] = idx.size();
				}
			}
		}
	}

	std::vector<std::vector<size_t>> coveringArray;
	while(!uncovered.empty()) {
		size_t expectedCoverage = (uncovered.size() + (v*v-1)) / (v*v); // expectedCoverage > 0
		size_t coverage;
		std::vector<size_t> row(vars.size());
		do {
			for(size_t i = 0; i < row.size(); i++)
				row[i] = rand() % vars[i].size();
			coverage = 0; // number of uncovered interactions
			for(size_t i = 1; i < vars.size(); i++) {
				for(size_t j = 0; j < i; j++) {
					size_t id = idx[{i,j,row[i],row[j]}];
					coverage += uncovered.count(id);
				}
			}
		} while(coverage < expectedCoverage);
		for(size_t i = 1; i < vars.size(); i++) {
			for(size_t j = 0; j < i; j++) {
				size_t id = idx[{i,j,row[i],row[j]}];
				uncovered.erase(id);
			}
		}
		coveringArray.emplace_back(row);
	}

	// contruct result
	std::vector<Test> res(coveringArray.size(), init);
	for(size_t c = 0; c < coveringArray.size(); c++) {
		for(size_t i = 0; i < vars.size(); i++) {
			res[c].emplace_back( vars[i][coveringArray[c][i]] );
		}
	}

	size_t naive = 0;
	for(size_t i = 1; i < vars.size(); i++)
		for(size_t j = 0; j < i; j++)
			naive += vars[i].size() * vars[j].size();
	std::cerr << "size reduction: " << naive << " => " << res.size() << std::endl;

	return res;
}

std::vector<Test> optCoveringArray(const Test& init,
		const std::vector<
				std::pair<
					std::function<std::optional<util::PropertyEffect>(
						const std::shared_ptr<PropertyAssignment>& oldVal,
						const std::shared_ptr<PropertyAssignment>& newVal)>,
					std::vector< std::shared_ptr<PropertyAssignment> >
				>
			>& vars) {
	srand(42); // deterministic for regression testing
	
	std::cerr << "optCoveringArray: vars.size() = " << vars.size() << std::endl;
	for(const auto&[cmp,var] : vars) {
		for(const auto& as : var) {
			as->print(std::cerr << "\t");
			std::cerr << std::endl;
		}
		std::cerr << std::endl;
	}
	assert(vars.size() > 0);

	using TestConf = std::map<size_t, size_t>; // {var, var_idx}, vars[var][var_idx]
	std::vector<TestConf> unused;
	
	// init
	for(size_t var = 0; var < vars.size(); var++)
		for(size_t i = 0; i < vars[var].second.size(); i++)
			for(size_t var2 = 0; var2 < var; var2++)
				for(size_t i2 = 0; i2 < vars[var2].second.size(); i2++)
					unused.emplace_back(std::map<size_t,size_t>{{{var,i}, {var2,i2}}});

	std::vector<TestConf> finished;

	const std::function<bool(const TestConf&, const std::vector<TestConf::value_type>&)> comparable = 
		[&](const auto& ref, const auto& vs) {
			std::optional<util::PropertyEffect> res = {util::PropertyEffect::ANY};
			for(const auto&[var,i] : vs) {
				const auto& j = ref.at(var);
				const auto& expect = vars[var].first( vars[var].second[i], vars[var].second[j] );
				if(!expect)
					return false;
				res = util::combine(*expect, *res);
				if(!res)
					return false;
			}
			return true;
		};

	const std::function<size_t(const TestConf&, const TestConf&)> cmp =
		[&](const auto& gen, const auto& ref) {
			std::vector<TestConf::value_type> vars(gen.begin(), gen.end());
			for(const auto&[var,i] : ref)
				if(gen.count(var) == 0)
					vars.emplace_back(var,i);
			
			size_t res = 0;
			for(const auto& fin : finished)
				res += comparable(fin, vars) ? 1 : 0;
			return res;
		};

	const std::function<void(TestConf&, const TestConf&)> comb =
		[&](auto& gen, const auto& ref) {
			for(const auto&[i,v] : ref)
				gen.emplace(i,v);
		};

	while(!unused.empty()) {
		TestConf gen{{unused.back()}};
		unused.pop_back();
		while(gen.size() < vars.size()) {
			// find opt
			int opt = -1, idx;
			bool is_unused;

			for(size_t i = 0; i < unused.size(); i++) {
				const int val = cmp(gen, unused[i]);
				if(val > opt)
					opt = val, idx = i, is_unused = true;
			}
			for(size_t i = 0; i < finished.size(); i++) {
				const int val = cmp(gen, finished[i]);
				if(val > opt)
					opt = val, idx = i, is_unused = false;
			}

			assert(opt != -1);

			if(is_unused) {
				comb(gen, unused[idx]);
				swap(unused[idx], unused.back());
				unused.pop_back();
			} else {
				comb(gen, finished[idx]);
			}
		}
		finished.emplace_back(gen);
	}
	
	// build tests
	std::vector<Test> res;
	for(const auto& test : finished) {
		Test tmp = init;
		for(const auto&[var,i] : test)
			tmp.emplace_back(vars[var].second[i]);
		res.emplace_back(tmp);
	}
	return res;
}

} // namespace util

} // namespace inviwo
