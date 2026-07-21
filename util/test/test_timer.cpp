/**
 * @file test_timer.cpp
 *
 */

#include <rusefi/timer.h>
#include <rusefi/rusefi_time_math.h>

#include <gtest/gtest.h>

// see mock/lib-time-mocks.cpp
void setTimeNowNt(efitick_t nt);

TEST(util, timerResetStampedInTheFuture) {
	Timer timer;

	efitick_t nowNt = USF2NT((efitick_t)10'000'000); // 10 seconds of uptime
	setTimeNowNt(nowNt);

	// sanity: reset stamped 100us in the past, nothing has elapsed yet
	timer.reset(nowNt - USF2NT((efitick_t)100));
	EXPECT_FALSE(timer.hasElapsedMs(250));

	// ISR stamps the reset 100us AFTER the checking thread sampled "now"
	timer.reset(nowNt + USF2NT((efitick_t)100));

	// getElapsedNt() has a negative-delta guard and correctly clamps to zero
	EXPECT_EQ(0u, timer.getElapsedNt(nowNt));

	// hasElapsedUs() must not wrap the negative delta into "elapsed": a freshly-reset
	// timer has not elapsed, no matter which side of the reset "now" was sampled on
	EXPECT_FALSE(timer.hasElapsedMs(250));

	setTimeNowNt(0);
}
