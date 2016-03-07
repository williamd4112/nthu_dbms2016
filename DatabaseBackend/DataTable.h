#pragma once

#include <iostream>
#include <cstring>
#include <cstdio>
#include <sstream>

#include "DatabaseDataType.h"
#include "PagedDataFile.h"

#define TABLE_DISKSIZE (sizeof(DataTable<FILESIZE_MAX, PAGE_SIZE>) - sizeof(DataFile<FILESIZE_MAX, PAGE_SIZE>*))

template <
	size_t FILESIZE_MAX,
	size_t PAGE_SIZE>
class DataTable
{
#define BIT_DEFINE 0x1
public:
	DataTable() : mHasInit(false) {}

	~DataTable()
	{
		if (mHasInit) 
		{
			mDataFile->Close();
			mHeader.WriteBackAt(mTblFile, 0);
			fclose(mTblFile);
		}
		std::cout << "Table delete" << std::endl;
	}

	inline void AddRecord(byte *src)
	{
		assert(mHasInit);
		mDataFile->WriteRecordHeap(src);
	}

	inline byte *FindRecord(const byte *src)
	{
		assert(mHasInit);
		return mDataFile->FindRecordHeap(src);
	}

	inline void DumpInfo()
	{
		assert(mHasInit);
		for (int i = 0; i < mHeader.maxAttrNum; i++)
		{
			if (mHeader.attrStatMap.Test(i, BIT_DEFINE)) {
				std::cout << mHeader.attributes[i] << std::endl;
			}
		}
	}

	inline void ShowAll()
	{
		assert(mHasInit);
		for (int i = 0; i <= mDataFile->MaxPageID(); i++)
		{
			RowDataPage<PAGE_SIZE> *page = mDataFile->ReadPage(i);
			assert(page != NULL);
			std::cout << "Page [ " << i << " ]" << std::endl;
			for (int j = 0; j < page->MaxRowCount(); j++)
			{
				const byte *row = page->ReadRow(j);
				if (row != NULL)
				{
					translate((const byte*)row);
				}
			}
			std::cout << std::endl;
		}
	}

	inline void InitTable(const char *tableName, Attribute *attrs, int attrNum)
	{
		mTblFile = fopen(getFilePathWithExt(tableName, ".tbl").c_str(), "wb+");
		if (mTblFile == NULL)
		{
			throw TABLE_CREATE_ERROR;
		}

		mHeader.InitHeader(tableName, attrNum, attrs);
		mDataFile = new DataFile<FILESIZE_MAX, PAGE_SIZE>(
			getFilePathWithExt(tableName, ".dat").c_str(),
			getFilePathWithExt(tableName, ".fmp").c_str(),
			AccessMode::CREATE,
			mHeader.rowSize);
		mHasInit = true;
	}
	
	inline void LoadTable(const char *tableName)
	{
		mTblFile = fopen(getFilePathWithExt(tableName, ".tbl").c_str(), "rb+");
		if (mTblFil == nullptr)
			throw TABLE_LOAD_ERROR;
		mHeader.ReadFrom(mTblFile, 0);
		mDataFile = new DataFile<FILESIZE_MAX, PAGE_SIZE>(
			getFilePathWithExt(tableName, ".dat").c_str(),
			getFilePathWithExt(tableName, ".fmp").c_str(),
			AccessMode::LOAD,
			mHeader.rowSize);
		mHasInit = true;
	}
private:
	bool mHasInit;
	FILE *mTblFile;

	TableHeader mHeader;
	DataFile<FILESIZE_MAX, PAGE_SIZE> *mDataFile;

	inline void translate(const byte *src)
	{
		int integer;
		char varchar[40];
		for (int i = 0; i < mHeader.maxAttrNum; i++)
		{
			if (mHeader.attrStatMap.Test(i, BIT_DEFINE))
			{
				if (i != 0) std::cout << ", ";
				switch (mHeader.attributes[i].type)
				{
				case INTEGER:
					std::cout << *(int*)(src + mHeader.attributes[i].offset);
					break;
				case VARCHAR:
					std::cout << (char*)(src + mHeader.attributes[i].offset);
					break;
				default:
					std::cout << "_unknown";
					break;
				}
			}
		}
		std::cout << std::endl;
	}

	inline std::string getFilePathWithExt(const char *prefix, const char *ext)
	{
		return (prefix + std::string(ext));
	}

	inline void WriteBackHeader(FILE *file)
	{
		fwrite(&mHeader, sizeof(TableHeader), 1, file);
	}

	inline void ReadHeader(FILE *file)
	{
		fread(&mHeader, sizeof(TableHeader), 1, file);
	}
};