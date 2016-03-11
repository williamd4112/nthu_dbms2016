#pragma once

#include <cstdio>
#include <iostream>
#include <assert.h>

#include "DatabaseDataType.h"
#include "PagedDiskFilePage.h"

template<size_t FILESIZE_MAX, size_t PAGE_SIZE, 
		unsigned int CACHESLOT_NUM = 8,
		unsigned int CACHESLOT_DEPTH = 1,
		unsigned int PAGENUM_MAX = FILESIZE_MAX / PAGE_SIZE>
class DataFile 
{
public:
	typedef RowDataPage<PAGE_SIZE> Page;

	DataFile(const char *datFilePath, const char *fmpFilePath, AccessMode mode, size_t rowSize)
		: mRowSize(rowSize)
	{
		mDatFile = fopen(datFilePath, (mode == AccessMode::CREATE) ? "wb+" : "rb+");
		if (mDatFile == NULL)
			throw (mode == AccessMode::CREATE) ? CREATE_ERROR : LOAD_ERROR;
		mFmpFile = fopen(fmpFilePath, (mode == AccessMode::CREATE) ? "wb+" : "rb+");
		if (mDatFile == NULL)
			throw (mode == AccessMode::CREATE) ? CREATE_ERROR : LOAD_ERROR;
	
		mDataFileFreeMap = new DataFileFreeMap<PAGENUM_MAX>;
		for (int i = 0; i < CACHESLOT_NUM; i++)
			mPageCache[i] = new PageCacheSlot<RowDataPage<PAGE_SIZE>>(rowSize);

		if (mode == AccessMode::LOAD && mDataFileFreeMap != NULL)
		{
			mDataFileFreeMap->ReadFrom(mFmpFile, 0);
		}
	}
	
	~DataFile()
	{

	}

	inline int MaxPageID() { return mDataFileFreeMap->maxPageID; }

	inline void WriteRecordHeap(void *src)
	{
		int cacheID;
		for (int i = mDataFileFreeMap->maxPageID; i < PAGENUM_MAX; i++)
		{
			cacheID = i % CACHESLOT_NUM;
			if (!mDataFileFreeMap->byteMap.Test(i, BIT_FULL))
			{
				RowDataPage<PAGE_SIZE> *page;
				try 
				{
					if (mDataFileFreeMap->byteMap.Test(i, BIT_VALID))
					{
						page = mPageCache[cacheID]->Read(i);
					}
					else 
					{
						page = mPageCache[cacheID]->AllocateNewPage(mDatFile, i, 0);
						mDataFileFreeMap->byteMap.Or(i, BIT_VALID);
					}
					page->WriteRowHeap(src);
					return;
				}
				catch (CacheException e)
				{
					page = mPageCache[cacheID]->Load(mDatFile, i, 0, WRITE);
					page->WriteRowHeap(src);
					return;
				}
				catch (PageException e)
				{
					page = NULL;
				}
				
				if (page != NULL && page->Full())
				{
					mDataFileFreeMap->byteMap.Or(i, BIT_FULL);
					mDataFileFreeMap->maxPageID++;
				}
			}
		}
		throw WRITE_OUT_OF_PAGE;
	}
	
	inline Page *ReadPage(PageID id)
	{
		unsigned int cacheID = id % CACHESLOT_NUM;
		try 
		{
			return mPageCache[cacheID]->Read(id);
		}
		catch (CacheException e) 
		{
			return mPageCache[cacheID]->Load(mDatFile, id, 0, READ);
		}
	}

	inline byte *FindRecordHeap(const byte *src)
	{
		// Unordered pages can only use linear search
		for (int i = 0; i < mDataFileFreeMap->maxPageID; i++)
		{
			byte *row;
			Page *page = ReadPage(i);
			if ((row = page->FindRow(src)) != NULL)
			{
				std::cout << "Record found " << std::endl;
				return row;
			}
		}
		std::cout << "Record not found " << std::endl;
		return NULL;
	}

	inline void Close()
	{
		mDataFileFreeMap->WriteBackAt(mFmpFile, 0);
		for (int i = 0; i < CACHESLOT_NUM; i++)
		{
			std::cout << "---- CacheSlot " << i << " flushing -----" << std::endl;
			mPageCache[i]->Flush(mDatFile, 0);
			delete mPageCache[i];
			std::cout << "---- CacheSlot " << i << " flushed -----" << std::endl;
		}
		fclose(mDatFile);
		fclose(mFmpFile);
	}

	void DumpInfo()
	{
		std::cout << "Size of FreeMap: " << sizeof(DataFileFreeMap<PAGENUM_MAX>) << std::endl
				  << "Size of Cache: " << sizeof(PageCacheSlot<RowDataPage<PAGE_SIZE> >) * CACHESLOT_NUM << std::endl;
	}

private:
	FILE *mDatFile;
	FILE *mFmpFile;
	size_t mRowSize;

	DataFileFreeMap<PAGENUM_MAX> *mDataFileFreeMap;
	PageCacheSlot<RowDataPage<PAGE_SIZE>> *mPageCache[CACHESLOT_NUM];
};