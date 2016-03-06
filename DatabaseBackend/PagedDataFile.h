#pragma once

#include <assert.h>

#include "PagedDiskFile.h"
#include "DatabaseDataType.h"
#include "BitUtil.h"

#define ULONG_LEN 32

template <
	size_t FILE_MAXSIZE,
	size_t PAGE_SIZE,
	class record_t,
	class page_t = DataPage<record_t, PAGE_SIZE>,
	uint8_t CACHESLOT_NUM = 8, uint8_t CACHESLOT_LENGTH = 1,
	PageAddr PAGEID_BITMASK = FILE_MAXSIZE / PAGE_SIZE,
	PageAddr PAGEOFFSET_BITMASK = PAGE_SIZE - 1,
	class PAGEID_MSB = SignificantBits<PAGEID_BITMASK>,
	unsigned int PAGEID_SHIFT = ULONG_LEN - PAGEID_MSB::pos,
	unsigned int MaxPageNum = FILE_MAXSIZE / PAGE_SIZE,
	size_t PageCacheSlotSize = PAGE_SIZE * CACHESLOT_LENGTH,
	size_t FreeMapPageStatSectionSize = sizeof(bool) * MaxPageNum,
	size_t PageCacheDirtysSize = sizeof(bool) * CACHESLOT_LENGTH,
	size_t PageCachePresentsSize = PageCacheDirtysSize>
class PagedDataFile : 
	public PagedDiskFile<FILE_MAXSIZE, page_t>
{
	static_assert(PAGEID_BITMASK & PAGEOFFSET_BITMASK, "Page size and file size cannot be zero.");
public:
	typedef page_t Page;
#define ID(addr) ( ( ( (addr) >> (PAGEID_SHIFT)) ) & PAGEID_BITMASK )
#define Offset(addr) ( (addr) & PAGEOFFSET_BITMASK )
	
	PagedDataFile(const char *dataFilePath, const char *freeMapFilePath, AccessMode mode)
		: PagedDiskFile(dataFilePath, freeMapFilePath, mode)
	{

	}

	~PagedDataFile()
	{

	}

	PageAddr WriteRecord(record_t *src)
	{
		PageID pid;
		Page &freePage = AllocatePageForWrite(&pid);
		RowID rid;
		try
		{
			rid = freePage.WriteRow(src);
		}
		catch (PagedDiskFileException e)
		{
			std::cout << "Page Overflow" << std::endl;
			mFreeMap.MarkFull(pid);
			freePage = AllocatePageForWrite(&pid);
			rid = freePage.WriteRow(src);
		}

		PageAddr addr = ((pid * PAGE_SIZE) << PAGEID_SHIFT) | rid;
		std::cout << "PageID = " << ID(addr) << ", PageOffset: " << Offset(addr) << std::endl;
		std::cout << freePage << std::endl;
		return addr;
	}

private:
	
};
