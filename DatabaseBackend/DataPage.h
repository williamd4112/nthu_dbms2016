#pragma once

#include <iostream>
#include <assert.h>

#include "PagedDiskFilePage.h"
#include "PagedDiskFileException.h"

#define PAGESIZE_8K 8192
#define PAGEHEADER_SIZE (2 * sizeof(int))

typedef int RowID;

template <class T, size_t PAGE_SIZE, 
	unsigned int MaxEntryCount = (PAGE_SIZE - PAGEHEADER_SIZE) / (sizeof(T) + sizeof(bool))>
class alignas(PAGE_SIZE) DataPage : public PagedDiskFilePage<PAGE_SIZE>
{
public:
	DataPage() : PagedDiskFilePage<PAGE_SIZE>(PAGETYPE_ROW), 
		mEntryCount(0) 
	{
		assert(sizeof(DataPage < T, PAGE_SIZE>) == PAGE_SIZE);
		if (MaxEntryCount <= 0)
			throw PagedDiskFileException(PagedDiskFileException::OUT_OF_PAGE);
		memset(mRowUse, 0x0, sizeof(bool) * MaxEntryCount);
	}

	~DataPage()
	{
	
	}

	/**
	 * @fn	int DataPage::Count() const
	 *
	 * @brief	Gets the count.
	 *
	 * @author	Williamd
	 * @date	2016/3/4
	 *
	 * @return	Number of row in this page
	 */

	int Count() const { return mEntryCount; }

	/**
	 * @fn	bool DataPage::InUse(RowID id)
	 *
	 * @brief	In use.
	 *
	 * @author	User
	 * @date	2016/3/4
	 *
	 * @param	id	The identifier.
	 *
	 * @return	true if it is using, false if it fails.
	 */

	bool InUse(RowID id) { return mRowUse[id]; }

	bool Full() { return mEntryCount == MaxEntryCount; }

	/**
	 * @fn	size_t DataPage::WriteRow(T *src, size_t size)
	 *
	 * @brief	Writes a row.
	 * @todo	Try to improve twice data copy
	 *
	 * @author	User
	 * @date	2016/3/4
	 *
	 * @param	src   	The T to process.
	 * @param	size	The size.
	 *
	 * @return	offset from object start (so that we can pass offset to access)
	 */
	RowID WriteRow(T *src)
	{
		if (mEntryCount >= MaxEntryCount)
			throw PagedDiskFileException(PagedDiskFileException::WRITE_OUT_OF_SPACE);

		RowID id = mEntryCount;
		while (mRowUse[id]) 
		{
			id++;
			id %= MaxEntryCount;
			if (id == mEntryCount)
				return -1;
		}

		mRows[id] = *src;
		mRowUse[id] = true;
		mEntryCount++;

		return id;
	}

	/**
	 * @fn	T DataPage::*GetRow(RowID id)
	 *
	 * @brief	Gets a row.
	 *
	 * @author	Williamd
	 * @date	2016/3/4
	 *
	 * @exception	PagedDiskFileException::INDEX_OUT_OF_BOUND	Thrown when an index out of bound
	 * 															error condition occurs.
	 *
	 * @param	id	The identifier.
	 *
	 * @return	null if it fails, else the row.
	 */

	T &operator [](RowID id)
	{
		if (id < 0 || id >= MaxEntryCount)
			throw PagedDiskFileException::INDEX_OUT_OF_BOUND;
		if (!mRowUse[id])
			throw PagedDiskFileException::PAGE_NOT_USING;
		return mRows[id];
	}

	friend std::ostream& operator <<(std::ostream& os, DataPage<T, PAGE_SIZE>& p)
	{
		os << "ObjectSize: " << sizeof(DataPage<T, PAGE_SIZE>) << std::endl
			<< "PageSize: " << p.Size() << std::endl
			<< "PageType: " << GetPageTypeStr(p.PageType()) << std::endl
			<< "PageRowSize: " << sizeof(T) << std::endl
			<< "MaxEntryNum: " << MaxEntryCount << std::endl
			<< "EntryCount: " << p.Count() << std::endl;
		for (int i = 0; i < p.Count(); i++)
		{
			os << "[" << i << "]: " << p[i] << std::endl;
		}
		return os;
	}
private:
	int mEntryCount;

	bool mRowUse[MaxEntryCount];
	T mRows[MaxEntryCount];
};

