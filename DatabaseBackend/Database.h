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
				table = new Table;
				assert(table != nullptr);
				std::pair<std::unordered_map<std::string, Table*>::iterator, bool> result = 
					mTables.insert(std::pair<std::string, Table*>(std::string(tableName), table));
				if (result.second)
				{
					table->LoadTable(tableName);
					std::cout << "Load table " << tableName << std::endl;
				}
				else 
				{
					std::cout << "Duplicate Table" << tableName << std::endl;
					delete table;
				}
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

	inline void CreateTable(const char *tableName, Relation *attrs, int attrNum)
	{
		Table *table = new Table;
		assert(table != nullptr);
		
		std::pair<std::unordered_map<std::string, Table*>::iterator, bool> result = 
			mTables.insert(std::pair<std::string, Table*>(tableName, table));
		if (result.second)
		{
			std::cout << "Table " << tableName << " created." << std::endl;
			table->InitTable(tableName, attrs, attrNum);
			table->DumpInfo();
		}
		else
		{
			std::cout << "Table " << tableName << " exist." << std::endl;
			delete table;
		}
		std::cout << std::endl;
	}

	inline void InsertRecord(const char *tableName, byte *src)
	{
		std::unordered_map<std::string, Table*>::iterator it =
			mTables.find(tableName);
		if (it != mTables.end())
		{
			Table *table = it->second;
			assert(table != nullptr);

			try 
			{
				table->AddRecord(src);
				std::cout << "Insertion success: " << std::endl;
				table->ShowAll();
			}
			catch (FileException e)
			{
				std::cerr << "Insertion failed: out of pages" << std::endl;
			}
		}
		else 
		{
			std::cerr << "Table doesn't exist." << std::endl;
		}
	}

	inline void ShowTableInfo(const char *tableName)
	{
		std::unordered_map<std::string, Table*>::iterator it =
			mTables.find(tableName);
		if (it != mTables.end())
		{
			Table *table = it->second;
			assert(table != nullptr);

			table->ShowAll();
		}
		else 
		{
			std::cerr << "Table doesn't exist." << std::endl;
		}
	}
private:
	FILE *mDatabaseFile;
	std::unordered_map<std::string, Table*> mTables;
};
