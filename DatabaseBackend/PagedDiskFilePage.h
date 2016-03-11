#pragma once

#include <cstdio>
#include <iostream>
#include <assert.h>

#define PAGESIZE_8K 8192

typedef unsigned short RowID;

template <size_t PAGE_SIZE>
class alignas(PAGE_SIZE) PagedDiskFilePage
{
public:
	PagedDiskFilePage(){}
	
	~PagedDiskFilePage() {}

	size_t Size() { return PAGE_SIZE; }

	void WriteBackAt(FILE *file, size_t offset)
	{
		assert(file != NULL);
		fseek(file, offset, SEEK_SET);
		fwrite(this, PAGE_SIZE, 1, file);
	}

	size_t ReadFrom(FILE *file, size_t offset)
	{
		assert(file != NULL);
		fseek(file, offset, SEEK_SET);
		return fread(this, PAGE_SIZE, 1, file);
	}
};

template <size_t PAGE_SIZE, 
	size_t ROWSIZE_MIN = 16,
	size_t HEADER_SIZE = sizeof(size_t) + sizeof(unsigned short) + sizeof(RowID) + sizeof(unsigned short),
	unsigned int FREEPATTERN_NUM_BIT = sizeof(byte) * 8,
	unsigned int ROWNUM_MAX = (PAGE_SIZE - HEADER_SIZE) / ((1 / (FREEPATTERN_NUM_BIT)) + ROWSIZE_MIN),
	size_t FREESPACE_SIZE = (PAGE_SIZE - HEADER_SIZE - sizeof(Bitmap<ROWNUM_MAX>))>
class alignas(PAGE_SIZE) RowDataPage
	: public PagedDiskFilePage<PAGE_SIZE>
{
public:
#define RowOffset(i, rowSize) ( (i) * (rowSize))
	RowDataPage(size_t rowSize) :
		PagedDiskFilePage(), 
		mRowSize(rowSize),
		mRowCount(0),
		mMaxRowCount(FREESPACE_SIZE / rowSize),
		mLastScanRowID(0),
		mDatas{0}
	{
		static_assert(sizeof(RowDataPage<PAGE_SIZE, ROWSIZE_MIN>) == PAGE_SIZE, 
			"Error: Page size must be equal to object size.");
		if (rowSize < ROWSIZE_MIN)
			rowSize = ROWSIZE_MIN;
	}
	
	~RowDataPage()
	{

	}

	inline bool Full() { return mRowCount >= mMaxRowCount; }

	inline int MaxRowCount() { return mMaxRowCount; }

	inline int RowCount() { return mRowCount; }

	inline void WriteRowHeap(void *src)
	{
		RowID i = mLastScanRowID;
		do 
		{
			i %= mMaxRowCount;
			if (!mFreeMap.Test(i))
			{
				memcpy(mDatas + i * mRowSize, src, mRowSize);
				mFreeMap.Set(i);
				mRowCount++;
				mLastScanRowID = i;
				return;
			}
			i++;
		} while (i != mLastScanRowID);
		throw NO_FREE_DATAROW;
	}

	inline const byte *ReadRow(RowID id)
	{
		assert(id >= 0 && id < mMaxRowCount);

		return (mFreeMap.Test(id)) ? mDatas + id * mRowSize : NULL;
	}

	inline byte *FindRow(const byte *src)
	{
		byte *row;
		for (int i = 0; i < mMaxRowCount; i++)
		{
			std::cout << "Test at " << i << std::endl;
			row = mDatas + RowOffset(i, mRowSize);
			if (mFreeMap.Test(i) && memcmp(src, row, mRowSize) == 0)
			{
				std::cout << "Find at " << i << std::endl;
				return row;
			}
		}
		return NULL;
	}

	inline void Reset()
	{
		mLastScanRowID = 0;
		mRowCount = 0;
		mFreeMap.Reset();
		memset(mDatas, 0, FREESPACE_SIZE);
	}

	void DumpInfo()
	{
		std::cout << "Global Page Properties: " << std::endl
			<< "Max # Row: " << ROWNUM_MAX << std::endl
			<< "Min Row-Size: " << ROWSIZE_MIN << std::endl
			<< "Header Size: " << HEADER_SIZE << std::endl
			<< "FreeMap Size: " << sizeof(Bitmap<ROWNUM_MAX>) << std::endl
			<< "FreeSpace Size: " << FREESPACE_SIZE << std::endl << std::endl;
		std::cout << "Variable Page Properties: " << std::endl
			<< "Row-Size: " << mRowSize << std::endl
			<< "Max-Row-Count: " << mMaxRowCount << std::endl
			<< "Row-Count: " << mRowCount << std::endl
			<< "Last-Scan: " << mLastScanRowID << std::endl;
		mFreeMap.DumpInfo();
	}
private:
	size_t mRowSize;
	RowID mLastScanRowID;
	unsigned short mMaxRowCount;
	unsigned short mRowCount;

	Bitmap<ROWNUM_MAX> mFreeMap;
	byte mDatas[FREESPACE_SIZE];
};

template <size_t PAGE_SIZE>
class alignas(PAGE_SIZE) BTreeInnerNodePage
	: public PagedDiskFilePage<PAGE_SIZE>
{
public:
private:
};

template <size_t PAGE_SIZE>
class alignas(PAGE_SIZE)BTreeLeafNodePage
	: public PagedDiskFilePage<PAGE_SIZE>
{
public:
private:
};