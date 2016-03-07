#pragma once

#include <iostream>
#include <cstring>
#include <memory>

#include "DatabaseDataType.h"

template <
	size_t FILE_MAXSIZE,
	size_t PAGE_SIZE, 
	unsigned int CACHESLOT_NUM = 32, unsigned int CACHESLOT_LENGTH = 8,
	unsigned int MAXPAGE_NUM = FILE_MAXSIZE / PAGE_SIZE>
class DataFile
{
public:
	struct DatFreeMap 
	{
#define BIT_VALID 0x1
#define BIT_FULL 0x2
		// In-disk
		unsigned int stat[MAXPAGE_NUM];
		unsigned int count[MAXPAGE_NUM];
		PageRowUsedExtent pageRowUseds[MAXPAGE_NUM];

		inline void MarkValid(int i)
		{
			stat[i] |= BIT_VALID;
		}

		inline void AllocateRowUsedExtent()
		{

		}

		inline void ReadFrom(FILE *file, size_t offset)
		{
			fseek(file, offset, SEEK_SET);
			fread(this, sizeof(DatFreeMap), 1, file);
		}

		inline void WriteBackAt(FILE *file, size_t offset)
		{
			fseek(file, offset, SEEK_SET);
			fwrite(this, sizeof(DatFreeMap), 1, file);
		}
	};

	DataFile(const char *datFilePath, 
		const char *dfmpFilePath,
		size_t rowsize,
		AccessMode mode) : 
		mMaxRowNum(PAGE_SIZE / rowsize)
	{
		if (mode == AccessMode::CREATE)
		{
			mDatFile = fopen(datFilePath, "wb+");
			mDfmpFile = fopen(dfmpFilePath, "wb+");
			if(mDfmpFile == NULL || mDfmpFile == NULL)
				throw PagedDiskFileException::CREATE_ERROR;
		}
		else if(mode == AccessMode::LOAD)
		{
			mDatFile = fopen(datFilePath, "rb+");
			mDfmpFile = fopen(dfmpFilePath, "rb+");
			if (mDfmpFile == NULL || mDfmpFile == NULL)
				throw PagedDiskFileException::FILE_NOSUCHFILE;
		}
	}

	~DataFile()
	{
		for (int i = 0; i < CACHESLOT_NUM; i++)
			mPageCaches[i].Flush(mDatFile);
		mDatFreemap.WriteBackAt(mDfmpFile, 0);
		fclose(mDatFile);
		fclose(mDfmpFile);
	}
private:
	FILE *mDatFile;
	FILE *mDfmpFile;
	PageCacheSlot<DataPage<PAGE_SIZE>, CACHESLOT_LENGTH> mPageCaches[CACHESLOT_NUM];
	DatFreeMap mDatFreemap;
	unsigned int mMaxRowNum;
};