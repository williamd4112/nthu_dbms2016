#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <climits>
#include <memory>

#include "DatabaseDataType.h"
#include "PagedDiskFilePage.h"
#include "PagedDiskFile.h"
#include "DataTable.h"
#include "Database.h"

struct Record {
	int id;
	char name[20];
	char address[30];
	char phone[10];
};

//DataTable<FILESZE_4GB, PAGESIZE_8K> *t;

Attribute attrs[] = {
	Attribute("Student ID", AttributeType::INTEGER, 0, 4),
	Attribute("Student Name", AttributeType::VARCHAR, 4, 20),
	Attribute("Address", AttributeType::VARCHAR, 24, 30),
	Attribute("Phone", AttributeType::VARCHAR, 54, 10)
};

int main(int argc, char *argv[])
{
	Database<FILESZE_4GB, PAGESIZE_8K> db("Database.db");

	std::string cmd, arg1;
	while (std::cin >> cmd)
	{
		if (cmd == "CREATE")
		{
			std::cin >> arg1;
			db.CreateTable(arg1.c_str(), attrs, 4);
		}
		else if (cmd == "EXIT")
			break;
	}

	//db.CreateTable("MyDb", attrs, 4);
	//t = new DataTable<FILESZE_4GB, PAGESIZE_8K>(4, "db.dat", "db.fmp", AccessMode::CREATE, 64);
	//t->SetAttribute(0, "Student ID", AttributeType::INTEGER, 0, 4);
	//t->SetAttribute(1, "Student Name", AttributeType::VARCHAR, 4, 20);
	//t->SetAttribute(2, "Address", AttributeType::VARCHAR, 24, 30);
	//t->SetAttribute(3, "Phone", AttributeType::VARCHAR, 54, 10);
	//
	//Record record{ 0, "StudentName" , "St.102 Rd.Brook", "1234"};
	//clock_t tStart = clock();
	//for (int j = 0; j < 1000; j++)
	//{
	//	record.id = j;
	//	t->AddRecord((byte*)&record);
	//}
	//printf("Time taken: %.4fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
	//system("pause");

	//std::cout << "------------------" << std::endl;
	//t->DumpInfo();
	//std::cout << "------------------" << std::endl;
	//t->ShowAll();

	//delete t;
	system("pause");

	return 0;
}