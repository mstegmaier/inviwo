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
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

#include <inviwo/core/ports/imageport.h>

namespace inviwo {

/** \docpage{org.inviwo.Histogram, Histogram}
 * ![](org.inviwo.Histogram.png?classIdentifier=org.inviwo.Histogram)
 * Explanation of how to use the processor.
 *
 * ### Inports
 *   * __<Inport1>__ <description>.
 *
 * ### Outports
 *   * __<Outport1>__ <description>.
 *
 * ### Properties
 *   * __<Prop1>__ <description>.
 *   * __<Prop2>__ <description>
 */
class IVW_MODULE_PROPERTYBASEDTESTING_API Histogram : public Processor {
public:
    Histogram(InviwoApplication* app);
    virtual ~Histogram() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
	enum TestingState {
		NONE,
		GATHERING
	};
	TestingState testingState;

	InviwoApplication* const app_;

	ImageInport inport_;
	
	ButtonProperty startButton_;
	ButtonProperty collectButton_;

	std::vector<IntMinMaxProperty*> props_;
	using ValueMap = std::unordered_map<const IntMinMaxProperty*, IntMinMaxProperty::range_type>;
	ValueMap defaultValues;
	void resetAllProps() {
		for(auto prop : props_) {
			if(defaultValues.find(prop) == defaultValues.end())
				std::cerr << "resetAllProps() : " << prop << std::endl;
			prop->set(defaultValues.at(prop));
		}
	}

	// Testing stuff
	void initTesting();

	using PropertyAssignment = std::pair<IntMinMaxProperty*, IntMinMaxProperty::range_type>;
	using Test = std::vector<PropertyAssignment>;
	class TestResult {
	private:
		const ValueMap values;
		const ValueMap &defaultValues;
	public:
		const IntMinMaxProperty::range_type& getValue(const IntMinMaxProperty* prop) const {
			auto it = values.find(prop);
			if(it != values.end())
				return it->second;
			return defaultValues.at(prop);
		}
		const Test test;
		const size_t backgroundPixels;
		TestResult(const ValueMap& defaultValues, const Test& t, size_t val) : 
			values([&t]() {
					ValueMap res;
					for(const auto& [prop,v] : t)
						res[prop] = v;
					return res;
				}()),
			defaultValues(defaultValues),
			test(t),
			backgroundPixels(val) { }
	};

	bool testIsSetUp(const Test& test);
	void setupTest(const Test& test);

	std::vector<Test> generateTests(IntMinMaxProperty* p1, IntMinMaxProperty* p2);

	std::queue<Test> remainingTests;
	std::vector<std::shared_ptr<TestResult>> testResults;
	void checkTestResults();
};

}  // namespace inviwo
