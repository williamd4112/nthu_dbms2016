#pragma once

#include <cstdio>
#include <unordered_map>
#include <memory>
#include <queue>
#include "DataPage.h"

#define FILESIZE_4GB (4294967295)

#define INVALID_PAGEID 0xffffffff

/**
 * @typedef	int PageID
 *
 * @brief	Page number in file
 */
typedef unsigned int PageID;

/**
 * @typedef	int PageAddr
 *
 * @brief	|-----------------------|------------------|
 * 					Page ID				Page Offset
 */

typedef unsigned int PageAddr;

enum AccessMode {LOAD, CREATE};

/**
 * @class	PagedDiskFile
 *
 * @brief	In-Memory:
 * 			File pointer
 * 			Page Cache
 * 			
 * 			In-Disk:
 * 			.fmp: free map file
 * 			
 * 			page_t need:
 * 			1. Full():  
 * 			2. WriteBackAt(FILE *file, size_t offset):  
 * 			3. ReadFrom(FILE *file, size_t offset:  
 * 			4. sizeof(page_t) == PAGE_SIZE
 *
 * @author	User
 * @date	2016/3/5
 *
 * @tparam	FILE_MAXSIZE			  	Must be power of 2
 * @tparam	PAGE_SIZE				  	Must be power of 2
 * @tparam	CACHESLOT_NUM			  	Must be power of 2
 * @tparam	CACHESLOT_LENGTH		  	Must be power of 2
 * @tparam	PAGEID_BITMASK			  	(DONT 'T CHANGE)
 * @tparam	PAGEOFFSET_BITMASK		  	(DONT 'T CHANGE)
 * @tparam	MaxPageNum				  	(DONT 'T CHANGE)
 * @tparam	PageCacheSlotSize		  	(DONT 'T CHANGE)
 * @tparam	FreeMapPageStatSectionSize	(DONT 'T CHANGE)
 * @tparam	PageCacheDirtysSize		  	(DONT 'T CHANGE)
 * @tparam	PageCachePresentsSize	  	(DONT 'T CHANGE)
 */

template <
	size_t FILE_MAXSIZE, class page_t,
	size_t PAGE_SIZE = sizeof(page_t),
	uint8_t CACHESLOT_NUM = 8, uint8_t CACHESLOT_LENGTH = 1,
	PageAddr PAGEID_BITMASK = FILE_MAXSIZE / PAGE_SIZE,
	PageAddr PAGEOFFSET_BITMASK = PAGE_SIZE - 1,
	unsigned int MaxPageNum = FILE_MAXSIZE / PAGE_SIZE,
	size_t PageCacheSlotSize = PAGE_SIZE * CACHESLOT_LENGTH,
	size_t FreeMapPageStatSectionSize = sizeof(bool) * MaxPageNum,
	size_t PageCacheDirtysSize = sizeof(bool) * CACHESLOT_LENGTH,
	size_t PageCachePresentsSize = PageCacheDirtysSize,
	size_t PageCacheIdsSize = sizeof(bool) * CACHESLOT_LENGTH>
class alignas(PAGE_SIZE) PagedDiskFile
{
public:
	typedef page_t Page;
	typedef Page& PageRef;
	typedef Page* PagePtr;

#define PageOffset(id) ((id * PAGE_SIZE))
#define SetCache(cacheID, pageID, present, dirty) { dirtys[ (cacheID) ] = (dirty); \
													ids[ (cacheID) ] = (pageID); \
													presents[ (cacheID) ] = (present); }\
	
	enum PageCacheException {MISS};

	struct PageCacheSlot
	{
	public:
		bool dirtys[CACHESLOT_LENGTH];
		bool presents[CACHESLOT_LENGTH];
		PageID ids[CACHESLOT_LENGTH];
		Page *pages;

		PageCacheSlot() : 
			dirtys{0}, presents{0}, 
			idsEnd(&ids[CACHESLOT_LENGTH - 1])
		{
			pages = new Page[CACHESLOT_LENGTH];
		}

		~PageCacheSlot()
		{
			delete[] pages;
		}

		inline Page &AllocateCachePage(PageID pid, FILE *flushFile)
		{
			for (int i = 0; i < CACHESLOT_LENGTH; i++)
			{
				if (!presents[i] || (!dirtys[i]))
				{
					SetCache(i, pid, true, true);
					return pages[i];
				}
			}
	
			WriteBackCache(flushFile);
			resetCacheStat();
			SetCache(0, pid, true, true);

			return pages[0];

		}

		inline Page& LoadPageToCache(PageID id, FILE *file, bool write)
		{
			fseek(file, PageOffset(id), SEEK_SET);
			for (int i = 0; i < CACHESLOT_LENGTH; i++)
			{
				if (!presents[i])
				{
					fread(&pages[i], PAGE_SIZE, 1, file);
					SetCache(i, id, true, write);
					printf("Laod Page %d to Cache [%d]\n",id, i);
					return pages[i];
				}
			}

			WriteBackCache(file); // Flush dirty page (present but clean page only need reset)
			resetCacheStat();
			fread(&pages[0], PAGE_SIZE, 1, file);
			SetCache(0, id, true, write);

			return pages[0];
		}

		inline void WriteBackCache(FILE *flushFile)
		{
			for (int i = 0; i < CACHESLOT_LENGTH; i++)
			{
				if (dirtys[i] && presents[i])
				{
					printf("Flush Page %d at %d\n",ids[i], ids[i] * PAGE_SIZE);
					pages[i].WriteBackAt(flushFile, PageOffset(ids[i]));
				}
			}
		}
		
		inline Page &operator [](PageID id)
		{
			for (int i = 0; i < CACHESLOT_LENGTH; i++)
				if (presents[i] && ids[i] == id)
				{
					printf("Cache hit Page %d at Cell %d\n",id ,i);
					return pages[i];
				}
			printf("Cache miss (%d)\n",id);
			throw PageCacheException::MISS;
		}
	private:
		PageID *idsEnd;
	
		inline void resetCacheStat()
		{
			memset(dirtys, 0, PageCacheDirtysSize);
			memset(presents, 0, PageCachePresentsSize);
			memset(ids, 0, PageCacheIdsSize);
		}
	};

	struct PageFreeMap  {
		int count;
		bool pageFull[MaxPageNum];

		PageFreeMap() : count(0), pageFull{ 0 } {}

		inline void MarkFull(PageID id) { pageFull[id] = true; }
		
		inline void WriteBackAt(FILE *file, size_t offset)
		{
			fseek(file, offset, SEEK_SET);
			fwrite(this, sizeof(PageFreeMap), 1, file);
		}

		inline void ReadFrom(FILE *file, size_t offset)
		{
			fseek(file, offset, SEEK_SET);
			fread(this, sizeof(PageFreeMap), 1, file);
		}
	};

	PagedDiskFile(const char *dataFilePath, const char *freeMapFilePath, AccessMode mode) :
		mFreeMap(), mPageCache()
	{
		switch (mode)
		{
		case LOAD:
			mDataFile = fopen(dataFilePath, "rb+");
			if (mDataFile == NULL)
				throw PagedDiskFileException(PagedDiskFileException::FILE_NOSUCHFILE);
			
			mFreeMapFile = fopen(freeMapFilePath, "rb+");
			if (mFreeMapFile == NULL)
				throw PagedDiskFileException(PagedDiskFileException::FILE_NOSUCHFILE);
			mFreeMap.ReadFrom(mFreeMapFile, 0);
			break;
		case CREATE:
			mDataFile = fopen(dataFilePath, "wb+");
			if(mDataFile == NULL)
				throw PagedDiskFileException(PagedDiskFileException::CREATE_ERROR);

			mFreeMapFile = fopen(freeMapFilePath, "wb+");
			if (mFreeMapFile == NULL)
				throw PagedDiskFileException(PagedDiskFileException::CREATE_ERROR);
			break;
		default:
			break;
		}
	}

	~PagedDiskFile()
	{
		mFreeMap.WriteBackAt(mFreeMapFile, 0);
		for (int i = 0; i < CACHESLOT_NUM; i++)
			mPageCache[i].WriteBackCache(mDataFile);
		fclose(mDataFile);
		fclose(mFreeMapFile);
	}

	Page &ReadPage(PageID id)
	{
		PageID cacheID = id % CACHESLOT_NUM;
		try {
			return mPageCache[cacheID][id];
		}
		catch (PageCacheException e)
		{
			printf("Handle miss at CacheSlot %d\n",cacheID);
			return mPageCache[cacheID].LoadPageToCache(id, mDataFile, false);
		}
	}

	void DumpPageFileInfo()
	{
		printf("# Page: %d\n", (FILE_MAXSIZE / PAGE_SIZE));
		printf("PageID Bit Mask: 0x%X\n", PAGEID_BITMASK);
		printf("Page Offset Bit Mask: 0x%X\n", PAGEOFFSET_BITMASK);

		for (int i = 0; i < mFreeMap.count; i++)
		{
			printf("---------------------------------Page %d----------------------------------\n", i);
			printf("ID = %d\tFull = %d\n",i,mFreeMap.pageFull[i]);
			std::cout << ReadPage(i) << std::endl;
			printf("---------------------------------  END  ----------------------------------\n");
		}

	}

protected:

	/** In-Memory **/
	FILE *mDataFile;
	FILE *mFreeMapFile;
	PageCacheSlot mPageCache[CACHESLOT_NUM];

	/** In-Disk **/
	PageFreeMap mFreeMap;

	Page &AllocatePageForWrite(PageID *id)
	{
		PageID cacheID;
		try {
			// Find free page with scanning from last page id 
			for (*id = mFreeMap.count; *id < MaxPageNum; *id++)
			{
				if (!mFreeMap.pageFull[*id])
				{
					cacheID = (*id) % CACHESLOT_NUM;
					return mPageCache[cacheID][*id];
				}
			}
		}
		catch (PageCacheException e)
		{
			if (*id <= mFreeMap.count)
				return mPageCache[cacheID].LoadPageToCache(*id, mDataFile, true);
			else
				return mPageCache[cacheID].AllocateCachePage(*id, mDataFile);
				
		}
		throw PagedDiskFileException::PAGE_NOFREEPAGE;
	}

};

