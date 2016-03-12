#include <iostream>
#include <sstream>
#include <cstdio>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <map>
#include <regex>
#include <assert.h>

#include "data_type.h"
#include "SQLParser.h"

static database_t db;

inline static void parse(std::string &input_str);
inline static void handle_create(Query *q);
inline static void handle_insert(Query *q);
inline static attr_domain_t attr_type_to_domain(int t);
inline static void exception_hanlder(table_exception_t e);

int main(int argc, char *argv[])
{	
	std::stringstream input_buff;
	std::string input_str;
	bool statement_end = false;
	
	std::cout << DB_PROMPT_PREFIX;
	while (getline(std::cin, input_str))
	{
		size_t scan_offset = 0;
		while (scan_offset < input_str.length())
		{
			size_t semicol_pos = input_str.find(";", scan_offset);

			std::string str_part = input_str.substr(scan_offset, semicol_pos - scan_offset);
			input_buff << str_part;
			
			if (semicol_pos == std::string::npos) 
				break; // no ';'
			else 
			{
				scan_offset = semicol_pos + 1;
				parse(input_buff.str());
				input_buff.str("");
				input_buff.clear();
				statement_end = true;
			}
		}
		std::cout << ((statement_end) ? DB_PROMPT_PREFIX : DB_PROMPT_CONTINUE_PREFIX);
		statement_end = false;
	}
	return 0;
}

static void parse(std::string &input_str)
{
	SQLParser *parser = new SQLParser();
	parser->parse(input_str);
	while (!parser->queryQueue.empty())
	{
		Query *q = parser->queryQueue.front();
		switch (q->getAction())
		{
		case CREATE:
			handle_create(q);
			break;
		case INSERT:
			handle_insert(q);
			break;
		default:
			std::cerr << DB_PROMPT_PREFIX << "Undefined operation" << std::endl;
			break;
		}
		parser->queryQueue.pop();
	}
	delete parser;
}

inline static void handle_create(Query *q)
{
	table_record_desc_t *table_descs = new table_record_desc_t[q->attributes.size()];
	int primary_key_index = NO_PRIMARY_KEY;

	try {
		for (int i = 0; i < q->attributes.size(); i++) {
			Attribute *attr = q->attributes[i];
			table_descs[i].attr_name = attr->getAttrName();
			table_descs[i].attr_domain = attr_type_to_domain(attr->getAttrType());
			table_descs[i].attr_size = attr->getAttrLength();
			if (attr->getPrimaryKey())
				primary_key_index = i;
		}
		db.create_table(q->getTableName().c_str(), 
			table_descs, q->attributes.size(), primary_key_index);
	}
	catch (table_exception_t e)
	{
		exception_hanlder(e);
		delete table_descs;
	}
}

inline static void handle_insert(Query *q)
{
	table_record_t record(q->attributes.size());
	try {
		for (int i = 0; i < q->attributes.size(); i++) {
			Attribute *attr = q->attributes[i];
			int type = attr->getAttrType();
			if (type == INT)
				record[i] = *static_cast<int*>(attr->getAttrValue());
			else if (type == VARCHAR)
				record[i] = *static_cast<string*>(attr->getAttrValue());
			else
				throw UNDEFINED_TYPE;
		}
		db.insert_record(q->getTableName().c_str(), record);
		db.show(q->getTableName().c_str());
	}
	catch (table_exception_t e)
	{
		exception_hanlder(e);
	}
}

inline static attr_domain_t attr_type_to_domain(int t)
{
	switch (t)
	{
	case INT:
		return INTEGER_DOMAIN;
	case VARCHAR:
		return VARCHAR_DOMAIN;
	default:
		throw ATTRTYPE_TO_DOMAIN_INVALID_TYPE;
	}
}

inline static void exception_hanlder(table_exception_t e)
{
	switch (e)
	{
	case DUPLICATE_RECORD:
		std::cerr << DB_PROMPT_PREFIX << "Error: duplicate key." << std::endl;
		break;
	case KEY_NOT_FOUND:
		std::cerr << DB_PROMPT_PREFIX << "Error: key not found." << std::endl;
		break;
	case RECORD_INVALID_TYPE:
		std::cerr << DB_PROMPT_PREFIX << "Error: invalid data type." << std::endl;
		break;
	case ATTRTYPE_TO_DOMAIN_INVALID_TYPE:
		std::cerr << DB_PROMPT_PREFIX << "Error: undefined type conversion." << std::endl;
		break;
	case UNDEFINED_TYPE:
		std::cerr << DB_PROMPT_PREFIX << "Error: undefined attribute type" << std::endl;
		break;
	default:
		std::cerr << DB_PROMPT_PREFIX << "Error: unhandled exception." << std::endl;
		break;
	}
}