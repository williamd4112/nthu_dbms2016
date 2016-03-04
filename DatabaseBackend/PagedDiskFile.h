#pragma once

#include <cstdio>
#include <unordered_map>
#include <memory>
#include <queue>
#include "DataPage.h"

#define FILESIZE_4GB (4294967295)

/**
	This class is consist of two disk file.
	(1). (.idx | .dat) file : index file or data file
	(2). .fpm file : free page map, this file contain
*/
typedef int PageID;

enum AccessMode {LOAD, CREATE};

template <size_t FILE_MAXSIZE, size_t PAGE_SIZE>
class alignas(PAGE_SIZE) PagedDiskFile
{
public:
	// TODO: PageCache should be fast when searching
	typedef std::unordered_map<PageID, PagedDiskFilePage<PAGE_SIZE>*> PageCache;
	typedef PagedDiskFilePage<PAGE_SIZE> Page;
	typedef Page& PageRef;
	typedef Page* PagePtr;
#define lookUpCache(id) ((mModifiedPageCache.find(id))->second)
#define isCacheMiss(cacheRef) ((cacheRef != mReferencedPageCache.end())) 

	PagedDiskFile(const char *dataFilePath, const char *freeMapFilePath, AccessMode mode) :
		mMaxAllocatedPageID(-1)
	{
		errno_t err;
		switch (mode)
		{
		case LOAD:
			err = fopen_s(&dataFilePath, "rb+");
			if (!err) throw PagedDiskFileException(PagedDiskFileException::FILE_NOSUCHFILE);
			err = fopen_s(&freeMapFilePath, "rb+");
			if (!err) throw PagedDiskFileException(PagedDiskFileException::FILE_NOSUCHFILE);
			break;
		case CREATE:
			err = fopen_s(&dataFilePath, "wb+");
			if(!err) throw PagedDiskFileException(PagedDiskFileException::CREATE_ERROR);
			err = fopen_s(&freeFilePathPath, "wb+");
			if (!err) throw PagedDiskFileException(PagedDiskFileException::CREATE_ERROR);
			break;
		default:
			break;
		}
	}

	PagedDiskFile() : mMaxAllocatedPageID(-1) {}

	~PagedDiskFile()
	{
		WriteBackCache();
		fclose(mDataFile);
		fclose(mFreemapFile);
	}

	void WriteBackCache()
	{
		for (PageCache::iterator it = mReferencedPageCache.begin();
			it != mReferencedPageCache.end();
			it++)
		{
			PagedDiskFilePage<PAGE_SIZE> *page = it->second;
			page->WriteBackAt(mDataFile, it->first * PAGE_SIZE);
			delete page;
		}
	}
private:
	PagedDiskFilePage<PAGE_SIZE> &getPage(PageID pageID)
	{
		// Check cache (if !NULL, cache hit)
		PagePtr page = lookUpCache(pageID);
		
		// Cache miss
		if (page == NULL)
		{
			page = loadPage(pageID);
			mReferencedPageCache.insert(std::pair < PageID, PagedDiskFilePage<PAGE_SIZE>*(pageID, page));
		}

		return page;
	}

	PagedDiskFilePage<PAGE_SIZE> *loadPage(PageID pageID)
	{
		if(pageID >= mMaxAllocatedPageID)
			throw PagedDiskFileException(PagedDiskFileException::LOAD_OUT_OF_BOUND);

		PagedDiskFilePage<PAGE_SIZE> *allocatedPage = new PagedDiskFilePage<PAGE_SIZE>;
		if (allocatedPage == NULL)
		{
			throw PagedDiskFileException(PagedDiskFileException::OUT_OF_MEMORY);
		}

		size_t numReadByte = allocatedPage->ReadFrom(mDataFile, pageID * PAGE_SIZE);
		if (numReadByte <= 0)
		{
			throw PagedDiskFileException(PagedDiskFileException::LOAD_READBYTE_ERROR);
		}

		return allocatedPage;
	}

	PageID getAvailablePageID()
	{
		PageID pageID;

		// Check free map first
		if (!mFreePageHeap.empty())
		{

		}
		else
		{
			pageID = ++mMaxAllocatedPageID;
		}
		return pageID;
	}

	/** In-Memory **/
	FILE *mDataFile, *mFreemapFile;
	PageCache mReferencedPageCache;

	/** In-Disk **/
	/*	In-FreeMapFile */
	std::priority_queue<PageID> mFreePageHeap;
	int mMaxAllocatedPageID;
};

