#pragma once

#include <iostream>
#include <assert.h>

#include "DatabaseDataType.h"
#include "PagedDiskFilePage.h"
#include "PagedDiskFileException.h"

#define PAGESIZE_8K 8192

typedef int RowID;

template <size_t PAGE_SIZE>
class alignas(PAGE_SIZE) DataPage : 
	public PagedDiskFilePage<PAGE_SIZE>
{
public:
	DataPage() :
		PagedDiskFilePage<PAGE_SIZE>(){}
	~DataPage(){}

	inline void WriteRow(size_t pageOffset, void *src, size_t size)
	{
		memcpy(rows + pageOffset, src, size);
	}

	inline const byte *ReadRow(size_t pageOffset, void *dst, size_t size)
	{
		memcpy(dst, rows + pageOffset, size);
	}
private:
	byte rows[PAGE_SIZE];
};

