#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <climits>
#include <memory>

#include "DatabaseDataType.h"
#include "PagedDiskFilePage.h"
#include "PagedDataFile.h"
#include "PagedDiskFile.h"
#include "DataPage.h"

int main(int argc, char *argv[])
{
	try {

		PagedDataFile<FILESIZE_4GB, PAGESIZE_8K, DataRecord>
			file("db.dat", "db.fmp", AccessMode::CREATE);
		//file.DumpPageFileInfo();
		//std::cout << std::endl;

		try {
			for (int i = 0; i < 12; i++)
			{
				DataRecord r{ i, -i, "PageContent" };
				file.WriteRecord(&r);
			}
		}
		catch (PagedDiskFileException e)
		{
			std::cerr << "Failed to write: Error code " << e.type << std::endl;
		}

		//std::cout << std::endl;
		////file.DumpPageFileInfo();
		
	}
	catch (PagedDiskFileException e)
	{
		std::cerr << "Error: " << e.type << std::endl;
	}
	system("pause");
	return 0;
}