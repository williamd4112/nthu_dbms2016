#pragma once
#pragma intrinsic(_BitScanReverse)

#include <iostream>

enum BitException{NOSETBIT};

inline static unsigned int MsbPos(unsigned long v)
{
	unsigned long index;
	if (!_BitScanReverse(&index, v))
		return index;
	throw NOSETBIT;
}

template <unsigned int x>
struct SignificantBits {
	static const unsigned int pos = SignificantBits<x / 2>::pos + 1;
};

template <>
struct SignificantBits<0> {
	static const unsigned int pos = 0;
};