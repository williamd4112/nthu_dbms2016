#pragma once

#include <cstdio>
#include <iostream>
#include <assert.h>

#define UNDEFINED_TYPE 0x0

#define PAGETYPE_ROW 0x1
#define PAGESTAT_FULL 0x80000000

typedef int PageType;

template <size_t PAGE_SIZE>
class alignas(PAGE_SIZE) PagedDiskFilePage
{
public:
	static const char *GetPageTypeStr(PageType t)
	{
		switch (t)
		{
		case PAGETYPE_ROW:
			return "Type-Row";
		default:
			return "Type-Unknown";
		}
	}
public:
	PagedDiskFilePage(int pageType = UNDEFINED_TYPE) : mPageType(pageType) {}
	
	~PagedDiskFilePage() {}

	int Size() { return PAGE_SIZE; }
	int PageType() { return mPageType; }

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

	friend std::ostream& operator <<(std::ostream& os, PagedDiskFilePage<PAGE_SIZE>& p)
	{
		return os << "ObjectSize: " << sizeof(PagedDiskFilePage<PAGE_SIZE>) << std::endl
			<< "PageSize: " << p.Size() << std::endl
			<< "PageType: " << p.PageType();
	}

private:
	int mPageType;
};