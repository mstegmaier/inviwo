/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/propertybasedtesting/algorithm/histogramtesting.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>

#include <filesystem>

namespace inviwo {

class TestResult;

class TestProperty {
protected:
	const auto& options() {
		const static std::vector<OptionPropertyOption<int>> options {
				{"EQUAL",			"EQUAL",			0},
				{"NOT_EQUAL",		"NOT_EQUAL",		1},
				{"LESS",			"LESS",				2},
				{"LESS_EQUAL",		"LESS_EQUAL",		3},
				{"GREATER",			"GREATER",			4},
				{"GREATER_EQUAL",	"GREATER_EQUAL",	5},
				{"ANY",				"ANY",				6},
				{"NOT_COMPARABLE",	"NOT_COMPARABLE",	7}
			};
		return options;
	}
public:
	virtual void withSubProperties(std::function<void(Property*)>) const = 0;

	virtual std::string getValueString(std::shared_ptr<TestResult>) const = 0;

	virtual std::optional<util::PropertyEffect> getPropertyEffect(
			std::shared_ptr<TestResult>, 
			std::shared_ptr<TestResult>) const = 0;

	virtual std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>) const = 0;

	virtual std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>,
			std::shared_ptr<TestResult>) const = 0;

	virtual void setToDefault() const = 0;
	virtual void storeDefault() = 0;
	virtual std::vector<std::shared_ptr<PropertyAssignment>> generateAssignments() = 0;
	virtual Property* getProperty() const = 0;
	virtual ~TestProperty() = default;
};

std::optional<std::shared_ptr<TestProperty>> testableProperty(Property* prop);

class TestPropertyComposite : public TestProperty {
	CompositeProperty* property;
	std::vector<std::shared_ptr<TestProperty>> subProperties;
	std::vector<BoolCompositeProperty*> compProps;
public:
	void withSubProperties(std::function<void(Property*)> f) const override {
		for(const auto& x : compProps)
			f(x);
	}

	std::string getValueString(std::shared_ptr<TestResult> testResult) const override {
		std::stringstream str;
		size_t n = 0;
		str << "(";
		for(const auto& subProp : subProperties)
			str << (n++ > 0 ? ", " : "") << subProp->getValueString(testResult);
		str << ")";
		return str.str();
	}

	std::optional<util::PropertyEffect> getPropertyEffect(
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const override {
		std::vector<util::PropertyEffect> subEffects;
	
		std::optional<util::PropertyEffect> res = {util::PropertyEffect::ANY};
		for(const auto& subProp : subProperties) {
			if(!res)
				break;
			auto tmp = subProp->getPropertyEffect(newTestResult, oldTestResult);
			if(!tmp) {
				res = std::nullopt;
				break;
			}
			res = util::combine(*res, *tmp);
		}
		return res;
	}

	std::ostream& ostr(std::ostream& out,
			std::shared_ptr<TestResult> testResult) const override {
		out << "(";
		size_t n = 0;
		for(const auto& subProp : subProperties) {
			if(n++ > 0) out << ", ";
			subProp->ostr(out, testResult);
		}
		return out << ")";
	}

	std::ostream& ostr(std::ostream& out,
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const override {
		out << getProperty()->getDisplayName() << ":" << std::endl;
		for(const auto& subProp : subProperties)
			subProp->ostr(out << "\t", newTestResult, oldTestResult) << std::endl;
		return out;
	}

	TestPropertyComposite(CompositeProperty* original)
			: TestProperty()
			, property(original) {
		for(Property* prop : original->getProperties())
			if(auto p = testableProperty(prop); p != std::nullopt) {
				subProperties.emplace_back(*p);

				auto propComp = new BoolCompositeProperty(
						(*p)->getProperty()->getIdentifier() + "Comp",
						(*p)->getProperty()->getDisplayName(),
						false);
				propComp->setCollapsed(true);
				(*p)->withSubProperties([&propComp](auto opt){ propComp->addProperty(opt); });

				compProps.emplace_back(propComp);
			}
	}
	~TestPropertyComposite() = default;
	Property* getProperty() const override {
		return property;//getTypedProperty();
	}
	void setToDefault() const override {
		for(const auto& subProp : subProperties)
			subProp->setToDefault();
	}

	template<typename T>
	std::optional<typename T::value_type> getValue(const T* prop) const;

	void storeDefault() {
		for(const auto& subProp : subProperties)
			subProp->storeDefault();
	}
	std::vector<std::shared_ptr<PropertyAssignment>> generateAssignments() {
		std::vector<std::shared_ptr<PropertyAssignment>> res;
		for(const auto& subProp : subProperties) {
			auto tmp = subProp->generateAssignments();
			res.insert(res.end(), tmp.begin(), tmp.end());
		}
		return res;
	}
};

template<typename T>
class TestPropertyTyped : public TestProperty {
	using val_type = typename T::value_type;
	static constexpr size_t numComponents = DataFormat<val_type>::components();

	T* typedProperty;
	val_type defaultValue;
	std::array<OptionPropertyInt*, numComponents> effectOption;
public:
	void withSubProperties(std::function<void(Property*)> f) const override {
		for(auto& x : effectOption)
			f(x);
	}

	std::string getValueString(std::shared_ptr<TestResult>) const override;

	std::optional<util::PropertyEffect> getPropertyEffect(
			std::shared_ptr<TestResult> newTestResult,
			std::shared_ptr<TestResult> oldTestResult) const override;

	std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>) const override;

	std::ostream& ostr(std::ostream&,
			std::shared_ptr<TestResult>,
			std::shared_ptr<TestResult>) const override;

	TestPropertyTyped(T* original)
		: TestProperty()
		, typedProperty(original)
		, defaultValue(original->get())
		, effectOption([this,original](){
				std::array<OptionPropertyInt*, numComponents> res;
				for(size_t i = 0; i < numComponents; i++) {
					std::string identifier = original->getIdentifier() + "_selector_" + std::to_string(i);
					res[i] = new OptionPropertyInt(
						identifier,
						"Comparator for increasing values",
						options(),
						options().size() - 1);
				}
				return res;
			}()){
	}
	~TestPropertyTyped() = default;
	T* getTypedProperty() const {
		return typedProperty;
	}
	Property* getProperty() const override {
		return getTypedProperty();
	}
	void setToDefault() const override {
		typedProperty->set(defaultValue);
	}
	const val_type& getDefaultValue() {
		return defaultValue;
	}
	void storeDefault() {
		defaultValue = typedProperty->get();
	}
	std::vector<std::shared_ptr<PropertyAssignment>> generateAssignments() {
		static const GenerateAssignments<T> tmp;
		return tmp(typedProperty);
	}
};

class TestResult {
	private:
		const std::vector<std::shared_ptr<TestProperty>>& defaultValues;
		const Test test;
		const size_t pixels;
		const std::filesystem::path imgPath;
	public:
		size_t getNumberOfPixels() {
			return pixels;
		}
		const std::filesystem::path& getImagePath() {
			return imgPath;
		}

		template<typename T>
		typename T::value_type getValue(const T* prop) const;

		TestResult(const std::vector<std::shared_ptr<TestProperty>>& defaultValues
				, const Test& t
				, size_t val
				, const std::filesystem::path& imgPath)
			  : defaultValues(defaultValues)
			  , test(t)
			  , pixels(val)
			  , imgPath(imgPath) {
			  }
};
		
template<typename T>
std::optional<typename T::value_type> TestPropertyComposite::getValue(const T* prop) const {
	for(auto subProp : subProperties) {
		if(auto p = std::dynamic_pointer_cast<TestPropertyTyped<T>>(subProp); p != nullptr) {
			if(p->getProperty() == prop)
				return p->getDefaultValue();
		} else if(auto p = std::dynamic_pointer_cast<TestPropertyComposite>(subProp); p != nullptr) {
			if(auto res = p->getValue(prop); res != std::nullopt)
				return res;
		}
	}
	return std::nullopt;
}
template<typename T>
typename T::value_type TestResult::getValue(const T* prop) const {
	for(const auto& t : test)
		if(auto p = std::dynamic_pointer_cast<PropertyAssignmentTyped<typename T::value_type>>(t);
				p && reinterpret_cast<T*>(p->getProperty()) == prop)
			return p->getValue();

	for(auto def : defaultValues) {
		if(auto p = std::dynamic_pointer_cast<TestPropertyTyped<T>>(def); p != nullptr) {
			if(p->getProperty() == prop) return p->getDefaultValue();
		} else if(auto p = std::dynamic_pointer_cast<TestPropertyComposite>(def); p != nullptr) {
			if(auto res = p->getValue(prop); res != std::nullopt)
				return *res;
		}
	}
	std::cerr << "could not get value for " << prop << " " << typeid(T).name() << std::endl;
	exit(1);
}

using PropertyTypes = std::tuple<OrdinalProperty<int>, OrdinalProperty<float>, OrdinalProperty<double>, IntMinMaxProperty>;

using TestingError = std::tuple<std::shared_ptr<TestResult>, std::shared_ptr<TestResult>, util::PropertyEffect, size_t, size_t>;

} // namespace inviwo
