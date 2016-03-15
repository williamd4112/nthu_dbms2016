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
	bool file_in = false;

	if (argc >= 2) 
	{
		freopen(argv[1], "r", stdin);
		file_in = true;
	}
	if (argc >= 3)
		freopen(argv[2], "w", stdout);
	if (argc >= 4)
		freopen(argv[3], "w", stderr);

	std::stringstream input_buff;
	std::string input_str;
	bool statement_end = false;

	if(!file_in) std::cout << DB_PROMPT_PREFIX;
	while (getline(std::cin, input_str))
	{
		size_t scan_offset = 0;
		while (scan_offset < input_str.length())
		{
			size_t semicol_pos = input_str.find(";", scan_offset);

			std::string str_part = input_str.substr(scan_offset, semicol_pos - scan_offset);
			input_buff << str_part << std::endl;
			
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
		if (!file_in)  std::cout << ((statement_end) ? DB_PROMPT_PREFIX : DB_PROMPT_CONTINUE_PREFIX);
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
			
			for (int j = i - 1; j >= 0; j--)
				if (table_descs[j].attr_name == attr->getAttrName())
					throw DUPLICATE_ATTR_NAME;
		}
		db.create_table(q->getTableName().c_str(), 
			table_descs, q->attributes.size(), primary_key_index);
	}
	catch (table_exception_t e)
	{
		exception_hanlder(e);
		delete [] table_descs;
	}
}

inline static int find_attr_insert_index(table_t *table_ptr, const std::string &attr_name)
{
	for (int i = 0; i < table_ptr->attr_count(); i++)
	{
		const table_record_desc_t *desc = table_ptr->desc(i);
		assert(desc != NULL);
		if (desc->attr_name == attr_name)
			return i;
	}
	return -1;
}

inline static void handle_insert(Query *q)
{
	try {

		table_t *table_ptr = db.find_table(q->getTableName().c_str());
		if (table_ptr == NULL)
			throw TABLE_NO_SUCH_TABLE;

		int attr_num = table_ptr->attr_count();
		table_record_t record(attr_num);

		for (int i = 0; i < q->attributes.size(); i++) {
			int attrIndex = i;
			Attribute *attr = q->attributes[i];
			if (attr->getAttrName() != "")
			{
				attrIndex = find_attr_insert_index(table_ptr, attr->getAttrName());
				if (attrIndex < 0)
					throw TABLE_NO_SUCH_ATTR;
			}

			int type = attr->getAttrType();
			if (type == INT)
				record[attrIndex] = *static_cast<int*>(attr->getAttrValue());
			else if (type == VARCHAR)
				record[attrIndex] = (*static_cast<string*>(attr->getAttrValue())).c_str();
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
	case DUPLICATE_ATTR_NAME:
		std::cerr << DB_PROMPT_PREFIX << "Error: duplicate attribute name." << std::endl;
		break;
	case TABLE_NO_SUCH_TABLE:
		std::cerr << DB_PROMPT_PREFIX << "Error: no such table." << std::endl;
		break;
	case TABLE_NO_SUCH_ATTR:
		std::cerr << DB_PROMPT_PREFIX << "Error: no such attribute." << std::endl;
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