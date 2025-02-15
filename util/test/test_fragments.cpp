#include <rusefi/fragments.h>
#include <rusefi/arrays.h>

#include <gtest/gtest.h>

#include <cstring>

struct obj1 {
	const uint8_t x[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
};

static_assert(sizeof(obj1) == 10);

struct obj2 {
	const uint8_t x[5] = { 11, 12, 13, 14, 15 };
};

static_assert(sizeof(obj2) == 5);

obj1 buffer10;
obj2 buffer5;

template<>
const obj1* getLiveData(size_t) {
	return &buffer10;
}

template<>
const obj2* getLiveData(size_t idx) {
	if (idx == 1) {
		return nullptr;
	}

	return &buffer5;
}

static FragmentEntry fragmentBuffer[] = {
	decl_frag<obj1>{},
	decl_frag<obj2, 0>{},
	decl_frag<obj2, 1>{},	// this one will be null
	decl_frag<obj1>{},
};

static uint8_t buffer[120];
static void resetBuffer() {
	memset(buffer, 0xFF, sizeof(buffer));
}

TEST(Util_Fragments, fragments) {
	static_assert(4 == efi::size(fragmentBuffer));

	FragmentList fragments{ fragmentBuffer, efi::size(fragmentBuffer) };

	resetBuffer();

	{
		// Exactly copy both buffers
		uint8_t expected[] = { 
			1, 2, 3, 4, 5, 6, 7, 8, 9, 10,	// buffer10
			11, 12, 13, 14, 15,				// buffer5
			0, 0, 0, 0, 0,					// null obj2
			1, 2, 3, 4, 5, 6, 7, 8, 9, 10	// buffer10 again
		};
		copyRange(buffer, fragments, 0, 10 + 5 + 5 + 10);
		EXPECT_TRUE( 0 == std::memcmp(buffer, expected, sizeof(expected)));
	}

	resetBuffer();

	{
		// Check overlap between first and second fragments
		uint8_t expected[] = {9, 10, 11, 12, 13};
		copyRange(buffer, fragments, 8, 5);
		EXPECT_TRUE( 0 == std::memcmp(buffer, expected, sizeof(expected)));
	}

	resetBuffer();

	{
		// Check partial of only second fragment
		uint8_t expected[] = {13, 14, 15};
		copyRange(buffer, fragments, 12, 3);
		EXPECT_TRUE( 0 == std::memcmp(buffer, expected, sizeof(expected)));
	}

	resetBuffer();

	{
		// Check overlap between second fragment and null fragment
		uint8_t expected[] = {15, 0, 0};
		copyRange(buffer, fragments, 14, 3);
		EXPECT_TRUE( 0 == std::memcmp(buffer, expected, sizeof(expected)));
	}

	resetBuffer();

	{
		// Check overlap between null fragment and 4th fragment
		uint8_t expected[] = {0, 1, 2};
		copyRange(buffer, fragments, 19, 3);
		EXPECT_TRUE( 0 == std::memcmp(buffer, expected, sizeof(expected)));
	}

	resetBuffer();

	{
		// Check overlap between last fragment and off-end of buffer (off-buffer should give 0s)
		uint8_t expected[] = {9, 10, 0, 0};
		copyRange(buffer, fragments, 28, 4);
		EXPECT_TRUE( 0 == std::memcmp(buffer, expected, sizeof(expected)));
	}

	resetBuffer();

	{
		// Check way off the end of the buffer
		uint8_t expected[] = {0, 0, 0};
		copyRange(buffer, fragments, 1000, 3);
		EXPECT_TRUE( 0 == std::memcmp(buffer, expected, sizeof(expected)));
	}
}
