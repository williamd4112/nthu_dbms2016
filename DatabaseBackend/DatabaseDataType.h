#pragma once

#include <iostream>
#include <iomanip> 
#include <assert.h>

typedef unsigned char byte;
typedef unsigned int PageID;

#define FILESZE_4GB 0xffffffff

#define ATTR_NUM_MAX 5
#define ATTR_NAME_SIZE_MAX 32
#define ATTR_VALUE_SIZE_MAX 41
#define VARCHAR_LEN_MAX ATTR_VALUE_SIZE_MAX
#define TABLE_NAME_LEN_MAX 40

#define WRITE true
#define READ false

#define BIT_VALID 0x1
#define BIT_FULL 0x2

enum AccessMode 
{
	LOAD,
	CREATE
};

enum CacheException
{
	MISS
};

enum PageException
{
	NO_FREE_DATAROW,
	NO_SUCH_DATAROW
};

enum FileException
{
	CREATE_ERROR,
	LOAD_ERROR
};

enum DomainType {
	INTEGER,
	VARCHAR
};

enum TableException
{
	TABLE_CREATE_ERROR,
	TABLE_LOAD_ERROR,
	ATTRIBUTE_SIZE_TOOLARGE,
	OUT_OF_MEMORY_FOR_NEWTABLE
};

enum DatabaseException
{
	DBFILE_OPEN_ERROR
};

struct Relation 
{
	char name[ATTR_NAME_SIZE_MAX];
	DomainType type;
	size_t offset;
	size_t size;

	Relation(){}
	Relation(const char *_name, DomainType _type, size_t _offset, size_t _size)
		: type(_type), offset(_offset), size(_size)
	{
		strncpy(name, _name, ATTR_NAME_SIZE_MAX);
	}
	~Relation() {}

	friend std::ostream& operator <<(std::ostream& os, const Relation& attr)
	{
		return os << "Name = " << attr.name << ", "
			<< "Type = " << AttributeTypeToString(attr.type) << ", "
			<< "Offset = " << attr.offset << ", "
			<< "Size = " << attr.size;
	}

	static const char *AttributeTypeToString(DomainType type)
	{
		switch (type)
		{
		case INTEGER:
			return "INTEGER";
		case VARCHAR:
			return "VARCHAR";
		default:
			return "Unknown-Type";
		}
	}
};

template <class page_t, unsigned int CACHE_NUM = 8, 
	size_t PAGE_SIZE = sizeof(page_t),
	size_t CACHEDIRTY_SIZE = sizeof(bool) * CACHE_NUM,
	size_t CACHEPRESENT_SIZE = sizeof(bool) * CACHE_NUM,
	size_t CACHEIDS_SIZE = sizeof(PageID) * CACHE_NUM>
struct PageCacheSlot
{
public:
#define SetCache(cid, pid, present, dirty) ids[(cid)] = (pid);\
										   presents[(cid)] = (present);\
										   dirtys[(cid)] = (dirty); \

#define LoadPageFromDisk(f, cid, id, ofs) fseek((f), (id) * PAGE_SIZE, SEEK_SET + (ofs)); \
									 fread(pages[(cid)], PAGE_SIZE, 1, (f))


	PageCacheSlot(size_t rowSize) : 
		dirtys{ 0 }, presents{ 0 }, ids{ 0 }
	{
		for (int i = 0; i < CACHE_NUM; i++)
			pages[i] = new page_t(rowSize);
	}

	~PageCacheSlot() 
	{
		for (int i = 0; i < CACHE_NUM; i++)
			delete pages[i];
	}

	inline bool Dirty(int i) { return dirtys[i]; }
	inline bool Present(int i) { return presents[i]; }

	inline page_t *Read(int id)
	{
		for (int i = 0; i < CACHE_NUM; i++)
		{
			if (presents[i] && ids[i] == id)
			{
				return pages[i];
			}
		}
		throw CacheException::MISS;
	}

	inline page_t *AllocateNewPage(FILE *pageFile, PageID id, size_t offset)
	{
		for (int i = 0; i < CACHE_NUM; i++)
		{
			if (!dirtys[i])
			{
				SetCache(i, id, true, true);

				// If this cell is not dirty, but present, clear the page
				if (presents[i])
				{
					pages[i]->Reset();
				}

				return pages[i];
			}
		}

		Flush(pageFile, offset);
		SetCache(0, id, true, true);
		if (presents[0])
		{
			pages[0]->Reset();
		}

		return pages[0];
	}

	inline page_t *Load(FILE *pageFile, PageID id, size_t offset, bool writeFlag)
	{
		for (int i = 0; i < CACHE_NUM; i++)
		{
			if (!dirtys[i])
			{
				LoadPageFromDisk(pageFile, i, id, offset);
				SetCache(i, id, true, writeFlag);
				return pages[i];
			}
		}

		Flush(pageFile, 0);
		SetCache(0, id, true, writeFlag);
		LoadPageFromDisk(pageFile, 0, id, offset);
		return pages[0];
	}
	
	inline void Flush(FILE *pagefile, size_t offset)
	{
		for (int i = 0; i < CACHE_NUM; i++)
		{
			// Only flush dirty page
			if (presents[i] && dirtys[i])
			{
				fseek(pagefile, ids[i] * PAGE_SIZE, SEEK_SET + offset);
				fwrite(pages[i], PAGE_SIZE, 1, pagefile);
				presents[i] = dirtys[i] = false;
				ids[i] = 0;
				//std::cout << "Page " << ids[i] << std::endl;
				//pages[i]->DumpInfo();
				pages[i]->Reset();
				//std::cout << "Flush page " << ids[i] << " at " << ids[i] * PAGE_SIZE << std::endl;
			}
		}
	}
private:
	bool dirtys[CACHE_NUM];
	bool presents[CACHE_NUM];
	PageID ids[CACHE_NUM];
	page_t *pages[CACHE_NUM];
};

template <unsigned int BIT_NUM,
		  unsigned int PARTITION_NUM = BIT_NUM / 8 + 1>
struct Bitmap
{
	Bitmap() : bits{ 0 } {}
	~Bitmap() {}

	inline void Set(int i)
	{
		unsigned int part = i >> 3;
		bits[part] = (bits[part]) | (1 << (i % 8));
	}

	inline bool Test(int i)
	{
		return (bits[i >> 3]) & (1 << (i % 8));
	}

	inline void Reset()
	{
		memset(bits, 0, sizeof(unsigned char) * PARTITION_NUM);
	}

	inline void Dump()
	{
		for (int i = 0; i < PARTITION_NUM; i++)
		{
			for (int j = 0; j < 8 && j < BIT_NUM; j++)
			{
				std::cout << ((bits[i] & (1 << j)) ? 1 : 0);
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	inline void DumpInfo()
	{
		std::cout << "# BitNum: " << BIT_NUM << std::endl;
		std::cout << "# PartNum: " << PARTITION_NUM << std::endl;
	}
private:
	unsigned char bits[PARTITION_NUM];
};

template <unsigned int BYTE_NUM>
struct Bytemap
{
public:
	Bytemap() : bytes{ 0 } {}
	~Bytemap(){}

	inline bool Test(int i, byte mask)
	{
		return (bytes[i] & mask);
	}

	inline void Or(int i, byte mask)
	{
		bytes[i] |= mask;
	}

	inline void And(int i, byte mask)
	{
		bytes[i] &= mask;
	}

	inline void Clear()
	{
		memset(bytes, 0, sizeof(byte) * (BYTE_NUM + 1));
	}

	byte operator [] (int i)
	{
		assert(i >= 0 && i < BYTE_NUM + 1);
		return bytes[i];
	}

	inline void ReadFrom(FILE *file, size_t offset)
	{
		fseek(file, offset, SEEK_SET);
		fread(this, sizeof(Bytemap<BYTE_NUM>), 1, file);
	}

	inline void WriteBackAt(FILE *file, size_t offset)
	{
		fseek(file, offset, SEEK_SET);
		fwrite(this, sizeof(Bytemap<BYTE_NUM>), 1, file);
	}
private:
	byte bytes[BYTE_NUM + 1];
};

template <unsigned int PAGENUM_MAX>
struct DataFileFreeMap
{
	unsigned int maxPageID;
	Bytemap<PAGENUM_MAX> byteMap;

	DataFileFreeMap() : maxPageID(0)
	{

	}

	~DataFileFreeMap()
	{

	}

	inline void WriteBackAt(FILE *file, size_t offset)
	{
		fseek(file, offset, SEEK_SET);
		fwrite(this, sizeof(DataFileFreeMap<PAGENUM_MAX>), 1, file);
	}

	inline void ReadFrom(FILE *file, size_t offset)
	{
		fseek(file, offset, SEEK_SET);
		fread(this, sizeof(DataFileFreeMap<PAGENUM_MAX>), 1, file);
		std::cout << "ReadFrom Disk: " <<
			"MaxPageID: " << maxPageID << std::endl;
		for (unsigned int i = 0; i <= maxPageID; i++)
		{
			if (byteMap.Test(i, BIT_VALID)) 
			{
				std::cout << "Page [" << i << "] = ";
				printf("0x%x\n",byteMap[i]);
			}
		}
	}
};

struct TableHeader
{
	char name[40];
	int maxAttrNum;
	size_t rowSize;
	Bytemap<ATTR_NUM_MAX> attrStatMap;
	Relation attributes[ATTR_NUM_MAX];

	TableHeader() :
		maxAttrNum(0), rowSize(0) 
	{

	}

	~TableHeader()
	{

	}

	inline void InitHeader(const char *_name, int _maxAttrNum, Relation *_attributes)
	{
		strncpy(name, _name, TABLE_NAME_LEN_MAX);
		maxAttrNum = _maxAttrNum;
		
		rowSize = 0;
		for (int i = 0; i < _maxAttrNum; i++)
		{
			// Invalid attribute schema
			if (_attributes[i].size > ATTR_VALUE_SIZE_MAX)
			{
				attrStatMap.Clear();
				throw ATTRIBUTE_SIZE_TOOLARGE;
			}
			attributes[i] = _attributes[i];
			attrStatMap.Or(i, BIT_VALID);
			rowSize += attributes[i].size;
		}
	}

	inline void ReadFrom(FILE *file, size_t offset)
	{
		fseek(file, offset, SEEK_SET);
		fwrite(this, sizeof(TableHeader), 1, file);
	}

	inline void WriteBackAt(FILE *file, size_t offset)
	{
		fseek(file, offset, SEEK_SET);
		fwrite(this, sizeof(TableHeader), 1, file);
	}
};
