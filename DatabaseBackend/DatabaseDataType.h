#pragma once

#include <iostream>

struct DataRecord
{
	int pid;
	int rid;
	char name[768];

	friend std::ostream& operator <<(std::ostream& os, DataRecord &r)
	{
		return os << "{" << "pid = " << r.pid << ", " << "rid = " << r.rid << ", " << "name = " << r.name << "}";
	}
};