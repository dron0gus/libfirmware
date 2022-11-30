/*
 * FragmentEntry.cpp
 *
 *  Created on: Jan 5, 2022
 * @author Andrey Belomutskiy, (c) 2012-2022
 */

#include <rusefi/fragments.h>

#include <rusefi/math.h>
#include <cstring>

void copyRange(uint8_t* destination, FragmentList src, size_t skip, size_t size) {
	size_t fragmentIndex = 0;

	// Find which fragment to start - skip any full fragments smaller than `skip` parameter
	while (fragmentIndex < src.count && skip > src.fragments[fragmentIndex].size) {
		skip -= src.fragments[fragmentIndex].size;
		fragmentIndex++;
	}

	size_t destinationIndex = 0;

	while (size > 0) {
		if (fragmentIndex >= src.count) {
			// somehow we are past the end of fragments - fill with zeros
			memset(destination + destinationIndex, 0, size);
			return;
		}

		int copyNowSize = minI(size, src.fragments[fragmentIndex].size - skip);
		const uint8_t* fromBase = src.fragments[fragmentIndex].get();
		if (!fromBase) {
			// we have no buffer for this fragment - fill with zeroes
			memset(destination + destinationIndex, 0, copyNowSize);
		} else {
			memcpy(destination + destinationIndex, fromBase + skip, copyNowSize);
		}
		destinationIndex += copyNowSize;
		skip = 0;
		size -= copyNowSize;
		fragmentIndex++;
	}
}
