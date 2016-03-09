#include <iostream>
#include <cstdio>
#include <ctime>
#include <vector>
#include <unordered_map>
#include <map>
#include <regex>
#include <assert.h>

#define ATTR_NUM_MAX 5
#define ATTR_SIZE_MAX 40

const char *DB_PROMPT_PREFIX = "Database > ";

const char *attr_domain_t_str[] = 
{
	"INTEGER",
	"VARCHAR"
};

enum attr_domain_t
{
	INTEGER = 0,
	VARCHAR = 1
};

enum table_exception_t
{
	RECORD_INVALID_TYPE,
	DUPLICATE_RECORD,
	KEY_NOT_FOUND,
	TABLE_RECORD_DESC_INVALD_SIZE,
	TABLE_BAD_INITIAL_ARGS,
	TABLE_NO_SUCH_TABLE
};

struct attr_t
{
	union attr_value_t
	{
		int integer;
		char varchar[ATTR_SIZE_MAX];
		attr_value_t(int _val) : integer(_val){}
		attr_value_t(const char *_str) { strncpy_s(varchar, _str, ATTR_SIZE_MAX); }
		
		attr_value_t& operator=(int _val)
		{
			integer = _val;
			return *this;
		}

		attr_value_t& operator=(const char *_val)
		{
			strncpy_s(varchar, _val, ATTR_SIZE_MAX);
			return *this;
		}

		attr_value_t(){}
		~attr_value_t(){}
	};

	attr_domain_t domain;
	attr_value_t value;

	attr_t(int _val) : value(_val), domain(INTEGER){}

	attr_t(const char *_str) : value(_str), domain(VARCHAR){}

	attr_t(const attr_t& _attr) : domain(_attr.domain) 
	{
		if (_attr.domain == INTEGER) value = _attr.value.integer;
		else value = _attr.value.varchar;
	}

	attr_t() {}

	~attr_t(){}

	inline size_t size()
	{
		return (domain == INTEGER) ? sizeof(int) : strlen(value.varchar);
	}

	attr_t &operator=(const attr_t& _attr) 
	{
		domain = _attr.domain;
		if (domain == INTEGER) value = _attr.value.integer;
		else value = _attr.value.varchar;
		return *this;
	}

	attr_t &operator=(int _val) { domain = INTEGER; value = _val; return (*this); }
	attr_t &operator=(const char *_val) { domain = VARCHAR; value = _val; return (*this);}

	friend std::ostream& operator <<(std::ostream& os, attr_t &attr)
	{
		return (attr.domain == INTEGER) ? os << attr.value.integer : os << attr.value.varchar;
	}

	friend bool operator <(const attr_t &a, const attr_t &b)
	{
		assert(a.domain == b.domain);
		if (a.domain == INTEGER) return a.value.integer < b.value.integer;
		else return a.value.varchar < b.value.varchar;
	}

	inline friend bool operator==(const attr_t &a, const attr_t &b)
	{
		if (a.domain != b.domain) return false;
		return (a.domain == INTEGER) ? a.value.integer == b.value.integer : strncmp(a.value.varchar, b.value.varchar, ATTR_SIZE_MAX);
	}
};

struct table_record_desc_t
{
	char attr_name[ATTR_SIZE_MAX];
	attr_domain_t attr_domain;
	size_t attr_size;

	friend std::ostream& operator<<(std::ostream& os, table_record_desc_t& desc)
	{
		return os << "[" << desc.attr_name << ", " << attr_domain_t_str[desc.attr_domain] << ", " << desc.attr_size << "]";
	}
};

struct table_record_t
{
	unsigned int attr_num;
	attr_t attrs[ATTR_NUM_MAX];

	table_record_t(int _attr_num) : 
		attr_num(_attr_num){}

	table_record_t() : attr_num(0) {}
	
	table_record_t(const table_record_t& r) :
		attr_num(r.attr_num)
	{
		for (int i = 0; i < r.attr_num; i++)
			attrs[i] = r.attrs[i];
	}

	~table_record_t() {}

	inline void set_attr(int index, int int_val)
	{
		attrs[index] = int_val;
	}

	inline void set_attr(int index, const char *varchar_val)
	{
		attrs[index] = varchar_val;
	}

	inline attr_t &operator[](int i)
	{
		assert(i >= 0 && i < attr_num);
		return attrs[i];
	}

	friend std::ostream& operator <<(std::ostream& os, table_record_t& r)
	{
		os << "{";
		for (int i = 0; i < r.attr_num; i++)
		{
			if (i) os << ", ";
			os << r.attrs[i];
		}
		os << "}";

		return os;
	}
};

class table_t
{
public:
	std::string table_name;

	table_t(const char *_table_name, table_record_desc_t *record_descs, int record_desc_num, int _primary_key_index) :
		table_name(_table_name),
		table_record_descs(record_descs),
		table_attr_num(record_desc_num),
		primary_key_index(_primary_key_index)
	{
		if (_primary_key_index >= record_desc_num) // < 0 means -> no primary key
			throw TABLE_BAD_INITIAL_ARGS;
		for (int i = 0; i < record_desc_num; i++)
		{
			if (record_descs[i].attr_size > ATTR_SIZE_MAX || record_descs[i].attr_size <= 0)
				throw TABLE_RECORD_DESC_INVALD_SIZE;
		}
	}

	~table_t(){}

	inline void insert_record(table_record_t &record)
	{
		if (!validate(record)) 
			throw RECORD_INVALID_TYPE;
		if (primary_key_index < 0 && isduplicate(record))
			throw DUPLICATE_RECORD;

		auto result = 
			table_index.insert(std::pair<attr_t, int>(record[primary_key_index], table_records.size()));
		
		if (result.second) 
			table_records.push_back(record);
		else 
			throw DUPLICATE_RECORD;
	}

	inline table_record_t &find_record(attr_t &key)
	{
		auto result = table_index.find(key);
		if (result != table_index.end())
		{
			return table_records[result->second];
		}
		throw KEY_NOT_FOUND;
	}

	inline friend bool operator==(const table_record_t &a, const table_record_t &b)
	{
		if (a.attr_num != b.attr_num) return false;
		for (int i = 0; i < a.attr_num; i++)
		{
			if (!(a.attrs[i] == b.attrs[i])) 
				return false;
		}
		return true;
	}

	inline void show_all()
	{
		std::cout << "Table: " << table_name << std::endl;
		for (std::vector<table_record_t>::iterator it = table_records.begin();
			it != table_records.end();
			it++)
		{
			std::cout << *it << std::endl;
		}
	}

private:
	int table_attr_num;
	int primary_key_index;
	table_record_desc_t *table_record_descs;
	std::vector<table_record_t> table_records;
	std::map<attr_t, int> table_index;

	inline bool validate(table_record_t &record)
	{
		if (record.attr_num != table_attr_num) 
			return false;

		for (int i = 0; i < table_attr_num; i++)
		{
			if (table_record_descs[i].attr_domain != record.attrs[i].domain ||
				table_record_descs[i].attr_size < record.attrs[i].size())
				return false;
		}
		return true;
	}

	inline bool isduplicate(table_record_t &record)
	{
		std::vector<table_record_t>::iterator it = std::find(table_records.begin(),
			table_records.end(), record);
		return (it != table_records.end());
	}
};

class database_t
{
	typedef std::unordered_map<std::string, table_t*> hashtable;
public:
	database_t()
	{}
	
	~database_t()
	{
		for (hashtable::iterator it = tables.begin(); it != tables.end(); it++)
		{
			delete it->second;
		}
	}

	inline void create_table(const char *_table_name,
		table_record_desc_t *record_descs, int _record_desc_num, int _primary_key_index)
	{
		table_t *new_table = new table_t(_table_name, record_descs, _record_desc_num, _primary_key_index);
		
		auto result = tables.insert(std::pair<std::string, table_t*>(_table_name, new_table));
		if (result.second)
		{
			std::cout << "Database > Table \"" << _table_name << "\" has been created." << std::endl;
			for (int i = 0; i < _record_desc_num; i++)
			{
				std::cout << "         > " << record_descs[i] << std::endl;
			}
		}
		else
		{
			std::cerr << DB_PROMPT_PREFIX << "error, " << _table_name << " has been existed." << std::endl;
		}
	}

	inline void insert_record(const char *_tablename, table_record_t &_record)
	{
		auto result = tables.find(_tablename);
		if (result != tables.end())
		{
			table_t *table_ptr = result->second;
			assert(table_ptr != nullptr);

			try
			{
				table_ptr->insert_record(_record);
				std::cout << DB_PROMPT_PREFIX << _record << " has been inserted." << std::endl;
			}
			catch (table_exception_t e)
			{
				std::cerr << DB_PROMPT_PREFIX << _record << " has already existed." << std::endl;
			}
		}
		else
		{
			std::cerr << DB_PROMPT_PREFIX << "table " << _tablename << " doesn't exist." << std::endl;
		}
	}
private:
	hashtable tables;
};

inline static void exception_hanlder(table_exception_t e)
{
	switch (e)
	{
	case DUPLICATE_RECORD:
		std::cerr << "Error: duplicate key." << std::endl;
		break;
	case KEY_NOT_FOUND:
		std::cerr << "Error: key not found." << std::endl;
		break;
	case RECORD_INVALID_TYPE:
		std::cerr << "Error: invalid data type." << std::endl;
		break;
	default:
		break;
	}
}

static database_t db;

int main(int argc, char *argv[])
{	
	table_record_desc_t descs[] = {
		{"ID", INTEGER, 4},
		{"NAME", VARCHAR, 40},
		{"ADDRESS", VARCHAR, 20}
	};

	db.create_table("mydb", descs, 3, 0);

	table_record_t buff(3);
	buff.set_attr(0, 0);
	buff.set_attr(1, "Williamd");
	buff.set_attr(2, "Road St.");

	for (int i = 0; i < 10; i++)
	{
		buff.set_attr(0, 1);
		try
		{
			db.insert_record("mydb", buff);
		}
		catch (table_exception_t e)
		{
			exception_hanlder(e);
		}
	}
	system("pause");

	return 0;
}