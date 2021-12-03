/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <random>

#include "adaptivecpu/Model.h"
#include "mocks.h"

using testing::_;
using testing::ByMove;
using testing::Return;
using std::chrono_literals::operator""ns;

namespace aidl {
namespace google {
namespace hardware {
namespace power {
namespace impl {
namespace pixel {

TEST(ModelTest, ModelInput_SetCpuFreqiencies) {
    const ModelInput expected{
            .cpuPolicyAverageFrequencyHz = {100, 101, 102},
    };
    ModelInput actual;
    ASSERT_TRUE(actual.SetCpuFreqiencies({
            {.policyId = 0, .averageFrequencyHz = 100},
            {.policyId = 4, .averageFrequencyHz = 101},
            {.policyId = 6, .averageFrequencyHz = 102},
    }));
    ASSERT_EQ(actual, expected);
}

TEST(ModelTest, ModelInput_SetCpuFreqiencies_failsWithOutOfOrderFrquencies) {
    ASSERT_FALSE(ModelInput().SetCpuFreqiencies({
            {.policyId = 0, .averageFrequencyHz = 100},
            {.policyId = 6, .averageFrequencyHz = 102},
            {.policyId = 4, .averageFrequencyHz = 101},
    }));
}

TEST(ModelTest, RunModel_randomInputs) {
    std::default_random_engine generator;
    std::uniform_real_distribution<double> frequencyDistribution(0, 1e6);
    std::uniform_real_distribution<double> idleTimesDistribution(0, 1);
    std::uniform_int_distribution<uint32_t> frameTimeDistribution(1, 100);
    std::uniform_int_distribution<uint16_t> numRenderedFramesDistribution(1, 20);
    std::uniform_int_distribution<uint32_t> throttleDecisionDistribution(0, 3);

    const auto randomModelInput = [&]() {
        return ModelInput{
                .cpuPolicyAverageFrequencyHz = {frequencyDistribution(generator),
                                                frequencyDistribution(generator),
                                                frequencyDistribution(generator)},
                .cpuCoreIdleTimesPercentage =
                        {idleTimesDistribution(generator), idleTimesDistribution(generator),
                         idleTimesDistribution(generator), idleTimesDistribution(generator),
                         idleTimesDistribution(generator), idleTimesDistribution(generator),
                         idleTimesDistribution(generator), idleTimesDistribution(generator)},
                .workDurationFeatures =
                        {.averageDuration =
                                 std::chrono::nanoseconds(frameTimeDistribution(generator)),
                         .maxDuration = std::chrono::nanoseconds(frameTimeDistribution(generator)),
                         .numMissedDeadlines = numRenderedFramesDistribution(generator),
                         .numDurations = numRenderedFramesDistribution(generator)},
                .previousThrottleDecision =
                        static_cast<ThrottleDecision>(throttleDecisionDistribution(generator)),
        };
    };

    for (int i = 0; i < 10; i++) {
        std::deque<ModelInput> modelInputs{randomModelInput(), randomModelInput(),
                                           randomModelInput()};
        RunModel(modelInputs);
    }
}

}  // namespace pixel
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace google
}  // namespace aidl
