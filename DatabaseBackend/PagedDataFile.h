#pragma once

#include "PagedDiskFile.h"

typedef int PageAddr;

template <size_t FILE_MAXSIZE, size_t PAGE_SIZE, typename T>
class PagedDataFile : 
	public PagedDiskFile<FILE_MAXSIZE, PAGE_SIZE>
{
public:
	PagedDataFile(const char *filepath)
		: PagedDiskFile(filepath)
	{

	}

	~PagedDataFile()
	{

	}

	PageAddr WriteData(T *src)
	{
		
	}
private:
};
