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
#include "BPTree.h"

struct Record {
	int id;
	char name[20];
	char address[30];
	char phone[10];
};

//DataTable<FILESZE_4GB, PAGESIZE_8K> *t;

Relation attrs[] = {
	Relation("Student ID", DomainType::INTEGER_DOMAIN, 0, 4),
	Relation("Student Name", DomainType::VARCHAR, 4, 20),
	Relation("Address", DomainType::VARCHAR, 24, 30),
	Relation("Phone", DomainType::VARCHAR, 54, 10)
};

struct Pair {
	int key;
	char name[10];
};

int main(int argc, char *argv[])
{
	int result;
	Pair p1{ 1, "abc" };
	Pair p2{ 2, "abc" };
	Pair p3{ 3, "abc" };
	Pair p4{ 4, "abc" };
	BPTree<int, PAGESIZE_8K> tree(3);
	BPTreeLeafNode<int, PAGESIZE_8K> node(tree, INTEGER_DOMAIN, 4);

	clock_t tStart = clock();
	for (int i = 0; i < 10; i++)
	{
		//p1.key = rand() % 10;
		try
		{
			if (node.InsertKey(&(i)))
			{
				std::cout << "Overflow" << std::endl;
				printf("Time taken: %.4fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
				system("pause");
				break;
			}
		}
		catch (TableException e)
		{
			std::cerr << "Duplicate key" << std::endl;
		}
	}
	//BPTreeNode<int, PAGESIZE_8K> *sibling = node.Split();
	int k = 12;
	node.SetValue(&k, 22);
	node.DumpAllKeys();
	//sibling->DumpAllKeys();
	//for (int i = 0; i < 1000; i++)
	//{
	//	clock_t tStart = clock();

	//	printf("Time taken: %.4fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
	//}
	//	printf("%d\n",result);
	system("pause");
	return 0;
}

//int main(int argc, char *argv[])
//{
//	Database<FILESZE_4GB, PAGESIZE_8K> db("Database.db");
//	Record r{ 1, "Col2", "Column3", "FinalCol"};
//	std::string cmd, arg1;
//	while (std::cin >> cmd)
//	{
//		if (cmd == "CREATE")
//		{
//			std::cin >> arg1;
//
//			db.CreateTable(arg1.c_str(), attrs, 4);
//		}
//		else if (cmd == "INSERT")
//		{
//			std::cin >> arg1;
//			db.InsertRecord(arg1.c_str(), (byte*)&r);
//			
//		}
//		else if (cmd == "SHOW")
//		{
//			std::cin >> arg1;
//			db.ShowTableInfo(arg1.c_str());
//		}
//		else if (cmd == "EXIT")
//			break;
//	}
//
//	//db.CreateTable("MyDb", attrs, 4);
//	//t = new DataTable<FILESZE_4GB, PAGESIZE_8K>(4, "db.dat", "db.fmp", AccessMode::CREATE, 64);
//	//t->SetAttribute(0, "Student ID", AttributeType::INTEGER, 0, 4);
//	//t->SetAttribute(1, "Student Name", AttributeType::VARCHAR, 4, 20);
//	//t->SetAttribute(2, "Address", AttributeType::VARCHAR, 24, 30);
//	//t->SetAttribute(3, "Phone", AttributeType::VARCHAR, 54, 10);
//	//
//	//Record record{ 0, "StudentName" , "St.102 Rd.Brook", "1234"};
//	//clock_t tStart = clock();
//	//for (int j = 0; j < 1000; j++)
//	//{
//	//	record.id = j;
//	//	t->AddRecord((byte*)&record);
//	//}
//	//printf("Time taken: %.4fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
//	//system("pause");
//
//	//std::cout << "------------------" << std::endl;
//	//t->DumpInfo();
//	//std::cout << "------------------" << std::endl;
//	//t->ShowAll();
//
//	//delete t;
//	system("pause");
//
//	return 0;
//}