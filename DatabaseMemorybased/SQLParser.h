#pragma once

#include <iostream>
#include <queue> 
#include <regex> 
#include "Query.h"
#define INT 0
#define VARCHAR 1
#define CREATE 1
#define INSERT 2
#define CommaStr "(\\s*,\\s*)+(?=([^\']*\'[^\']*\')*[^\']*$)"
#define SemicolonStr "(\\s*;\\s*)+(?=([^\']*\'[^\']*\')*[^\']*$)"

#define reg_createStr "\\s*create\\s+table\\s+(\\w+)\\s*\\(([\\w\\(\\)\\s]+(,[\\w\\)\\(\\s]+)*)\\)\\s*"
#define reg_attrStr "\\s*(\\w+)\\s+(\\w+)\\s*(\\(\\s*([0-9]+)\\s*\\))?\\s*(primary\\s+key)?\\s*$"

#define reg_insertStr "\\s*insert\\s+into\\s+(\\w+)\\s*([\\s\\S]+)\\s*"
#define reg_insertTypeStr "\\s*(\\((\\s*\\w+\\s*(,\\s*\\w+\\s*)*)\\))?\\s*values\\s*\\((\\s*('[\\s\\S]+'|[\\d]+)\\s*(\\s*,\\s*('[\\s\\S]+'|[\\d]+))*\\s*)\\)\\s*"
#define reg_attrValueStr "\\s*('([\\s\\S]+)'|([\\d]+))\\s*"


using namespace std;

class SQLParser {
public:
	queue<Query*> queryQueue;
	void parse(string&);
	Query* insert(smatch&);
	Query* createTable(smatch&);
	bool caseInsensitiveStrcmp(const string&, const string&);
	void split(string&,const string&, deque<string>&);

};
