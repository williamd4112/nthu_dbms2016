#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <climits>
#include <memory>

#include "PagedDiskFilePage.h"
#include "PagedDiskFile.h"
#include "DataPage.h"

struct DataRecord
{
	static const int c;
	int id;
	const char *name;

	DataRecord(int id, const char *name) : id(id), name(name)
	{
	}

	friend std::ostream& operator <<(std::ostream& os, DataRecord &r)
	{
		return os << "{" << "id = " << r.id << ", " << "name = " << r.name << "}";
	}
};

#define BUFFSIZE 8 << 10
void *buffer[BUFFSIZE];

void testIo()
{
	FILE *ifp = fopen("largefile", "rb");
	FILE *ofp = fopen("outputfile", "wb");
	if (ifp != NULL && ofp != NULL)
	{
		size_t readByte;
		while ((readByte = fread(buffer, BUFFSIZE, 1, ifp)) > 0)
		{
			size_t writeByte = fwrite(buffer, BUFFSIZE, 1, ofp);
			if (writeByte <= 0)
			{
				std::cerr << "Error: error write." << std::endl;

			}
		}
	}
}

int main(int argc, char *argv[])
{


	return 0;
}