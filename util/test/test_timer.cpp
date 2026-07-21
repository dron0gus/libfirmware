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

	// hasElapsedUs() lacks that guard: static_cast<uint32_t>(negative delta) wraps to
	// ~4.29e9 ticks, so a freshly-reset timer reports ANY threshold as elapsed.
	// This assertion documents the DEFECT: the fix must flip it to EXPECT_FALSE.
	EXPECT_TRUE(timer.hasElapsedMs(250)) << "negative-delta guard is in place, flip this assertion to EXPECT_FALSE";

	setTimeNowNt(0);
}
