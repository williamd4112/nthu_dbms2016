#pragma once
#include <iostream>
#include <string>
#include <deque>
#include"Attribute.h"
using namespace std;
class Query {
public:
	deque<Attribute*> attributes;
	Query();
	~Query();
	void setTableName(string);
	string getTableName();
	void setAction(int);
	int getAction();
	void printQuery();
private:
	int action;//1 CREATE TABLE; 2 INSERT
	string tableName;

};