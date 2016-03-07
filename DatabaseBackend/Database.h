#pragma once

#include <iostream>
#include <unordered_map>

#include "DatabaseDataType.h"
#include "DataTable.h"

#define TABLE_NUM_MAX 10

template<size_t FILESIZE_MAX, size_t PAGE_SIZE>
class Database 
{
	typedef DataTable<FILESIZE_MAX, PAGE_SIZE> Table;
public:
	Database(const char *dbFilePath)
	{
		std::cout << "Database initializing..." << std::endl;
		mDatabaseFile = fopen(dbFilePath, "r+");
		if (mDatabaseFile == NULL)
		{
			mDatabaseFile = fopen(dbFilePath, "w+");
			if (mDatabaseFile == NULL)
				throw DBFILE_OPEN_ERROR;
			std::cout << "Database has been created !" << std::endl;
		}
		else 
		{	
			Table *table;
			char tableName[TABLE_NAME_LEN_MAX + 1];
			while (fscanf(mDatabaseFile, "%s", tableName) != EOF);
			{
				std::cout << "Load table " << tableName << std::endl;
				table = new Table;
				assert(table != nullptr);
				mTables.insert(std::pair<std::string, Table*>(std::string(tableName), table));
			}
		}
		std::cout << "Database is online" << std::endl;
	}

	~Database()
	{
		for (std::unordered_map<std::string, Table*>::iterator it = mTables.begin();
			it != mTables.end();
			it++)
		{
			fprintf(mDatabaseFile, "%s\n", it->first.c_str());
			delete (*it).second;
		}
		fclose(mDatabaseFile);
	}

	inline void CreateTable(const char *tableName, Attribute *attrs, int attrNum)
	{
		Table *table = new Table;
		assert(table != nullptr);
		
		table->InitTable(tableName, attrs, attrNum);
		mTables.insert(std::pair<std::string, Table*>(tableName, table));

		std::cout << "Table " << tableName << " created." << std::endl;
		table->DumpInfo();
		std::cout << std::endl;
	}

	inline void InsertRecord(const char *tableName, const byte *src, size_t size)
	{

	}

	inline void ShowTableInfo(const char *tableName)
	{
		
	}
private:
	FILE *mDatabaseFile;
	std::unordered_map<std::string, Table*> mTables;
};
