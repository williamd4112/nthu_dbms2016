#pragma once

#include <memory>

#define ATTR_NUM_MAX 5
#define ATTR_SIZE_MAX 40
#define NO_PRIMARY_KEY -1
#define MAX_NO_ATTRIBUTE 1000

const char *DB_PROMPT_PREFIX = "Database > ";
const char *DB_PROMPT_CONTINUE_PREFIX = "         > ";

const char *attr_domain_t_str[] =
{
	"INTEGER_DOMAIN",
	"VARCHAR_DOMAIN"
};

const char *action_str[] =
{
	"UNDEFINED ACTION", // Empty string for fitting CREATE = 1, INSERT = 2	
	"CREATE",
	"INSERT"
};

enum attr_domain_t
{
	INTEGER_DOMAIN = 0,
	VARCHAR_DOMAIN = 1,
	UNDEFINED_DOMAIN = 2
};

enum table_exception_t
{
	RECORD_INVALID_TYPE,
	RECORD_ATTR_NUM_INCONSISTENT,
	RECORD_DOMAIN_INCONSISTENT,
	RECORD_PRIMARY_KEY_NULL,
	RECORD_SIZE_INCONSISTENT,
	DUPLICATE_RECORD,
	DUPLICATE_KEY,
	DUPLICATE_ATTR_NAME,
	KEY_NOT_FOUND,
	TABLE_RECORD_DESC_INVALD_SIZE,
	TABLE_BAD_INITIAL_ARGS,
	TABLE_NO_SUCH_TABLE,
	TABLE_NO_SUCH_ATTR,
	ATTRTYPE_TO_DOMAIN_INVALID_TYPE,
	INSERTION_NO_TYPE,
	DESC_TOO_MANY_ATTR,
	ATTR_SIZE_TOO_LARGE
};

struct attr_t
{
	union attr_value_t
	{
		int integer;
		char *varchar;
		attr_value_t(int _val) : integer(_val) {}
		attr_value_t(const char *_str) { varchar = _strdup(_str); }
		attr_value_t() {}
		~attr_value_t() {}

		attr_value_t& operator=(int _val)
		{
			integer = _val;
			return *this;
		}

		attr_value_t& operator=(const char *_val)
		{
			varchar = _strdup(_val);
			return *this;
		}
	};

public:
	attr_t(int _val) : value(_val), domain(INTEGER_DOMAIN) {}

	attr_t(const char *_str) : value(_str), domain(VARCHAR_DOMAIN) {}

	attr_t(const attr_t& _attr) : domain(_attr.domain)
	{
		if (_attr.domain == INTEGER_DOMAIN) value = _attr.value.integer;
		else if (_attr.domain == VARCHAR_DOMAIN) value = _attr.value.varchar;
	}

	attr_t() : domain(UNDEFINED_DOMAIN) {}

	~attr_t() {}

	inline size_t size()
	{
		return (domain == INTEGER_DOMAIN) ? sizeof(int) :
			(domain == VARCHAR_DOMAIN) ? strlen(value.varchar) :
			0;
	}

	inline attr_domain_t Domain() const { return domain; }
	inline int Int() const { return value.integer; }
	inline const char *Varchar() const { return value.varchar; }

	attr_t &operator=(const attr_t& _attr)
	{
		domain = _attr.domain;
		if (domain == INTEGER_DOMAIN) value = _attr.value.integer;
		else if (_attr.domain == VARCHAR_DOMAIN) value = _attr.value.varchar;
		return *this;
	}

	attr_t &operator=(int _val) { domain = INTEGER_DOMAIN; value = _val; return (*this); }
	attr_t &operator=(const char *_val) 
	{ 
		if (domain == VARCHAR_DOMAIN) 
			delete value.varchar; 
		domain = VARCHAR_DOMAIN; 
		value = _val; return (*this); 
	}

	friend std::ostream& operator <<(std::ostream& os, attr_t &attr)
	{
		switch (attr.domain)
		{
		case INTEGER_DOMAIN:
			return os << attr.value.integer;
		case VARCHAR_DOMAIN:
			return os << attr.value.varchar;
		default:
			return os << "null";
		}
	}

	friend bool operator <(const attr_t &a, const attr_t &b)
	{
		assert(a.Domain() == b.Domain());
		if (a.Domain() == INTEGER_DOMAIN) return a.Int() < b.Int();
		else if(a.Domain() == VARCHAR_DOMAIN) return strncmp(a.Varchar(), b.Varchar(), ATTR_NUM_MAX) < 0;
		else return false;
	}

	inline friend bool operator==(const attr_t &a, const attr_t &b)
	{
		if (a.Domain() != b.Domain()) return false;
		switch (a.domain)
		{
		case INTEGER_DOMAIN:
			return a.value.integer == a.value.integer;
		case VARCHAR_DOMAIN:
			return strncmp(a.value.varchar, b.value.varchar, ATTR_SIZE_MAX);
		default: // NULL TYPE
			return true;
		}
	}
private:
	attr_domain_t domain;
	attr_value_t value;
};

struct table_record_desc_t
{
	std::string attr_name;
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
		attr_num(_attr_num) {}

	table_record_t() : attr_num(0) {}

	table_record_t(const table_record_t& r) :
		attr_num(r.attr_num)
	{
		for (int i = 0; i < r.attr_num; i++)
			attrs[i] = r.attrs[i];
	}

	~table_record_t() {}

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
};

class table_t
{
public:
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

	~table_t() {}

	const inline std::string &name() { return table_name; }
	const inline int pk_index() const { return primary_key_index; }
	const inline int attr_count() { return table_attr_num; }
	const inline table_record_desc_t *desc(int i)
	{
		if (i < 0 || i >= table_attr_num) return NULL;
		else return &table_record_descs[i];
	}

	inline void insert_record(table_record_t &record)
	{
		validate(record);

		if (primary_key_index < 0 && isduplicate(record)) // check dupilicate if no PK
			throw DUPLICATE_RECORD;
		if (primary_key_index >= 0) // check dupilicate if has pk
		{
			auto result =
				table_index.insert(std::pair<attr_t, int>(record[primary_key_index], table_records.size()));
			if (!result.second)
				throw DUPLICATE_KEY;
		}
		table_records.push_back(record);
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

	friend std::ostream& operator<<(std::ostream& os, table_t &table)
	{
		os << "Table: " << table.table_name << " Count: " << table.table_records.size() << std::endl;
		for (std::vector<table_record_t>::iterator it = table.table_records.begin();
			it != table.table_records.end();
			it++)
		{
			os  << *it << std::endl;
		}
		os << std::endl;
		return os;
	}
private:
	std::string table_name;
	int table_attr_num;
	int primary_key_index;
	table_record_desc_t *table_record_descs;
	std::vector<table_record_t> table_records;
	std::map<attr_t, int> table_index;

	inline void validate(table_record_t &record)
	{
		if (record.attr_num != table_attr_num)
			throw RECORD_ATTR_NUM_INCONSISTENT;

		for (int i = 0; i < table_attr_num; i++)
		{
			bool domainSame = (table_record_descs[i].attr_domain == record.attrs[i].Domain() ||
				(record.attrs[i].Domain() == UNDEFINED_DOMAIN && i != primary_key_index));
			bool sizeAccept = record.attrs[i].size() <= table_record_descs[i].attr_size;
			if(i == primary_key_index && record.attrs[i].Domain() == UNDEFINED_DOMAIN)
				throw RECORD_PRIMARY_KEY_NULL;
			if (!domainSame) throw RECORD_DOMAIN_INCONSISTENT;
			if (!sizeAccept) throw RECORD_SIZE_INCONSISTENT;
		}
	}

	inline bool isduplicate(table_record_t &record)
	{
		return !(std::find(table_records.begin(), table_records.end(), record) == table_records.end());
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

	inline table_t *find_table(const char *tablename)
	{
		hashtable::iterator it = tables.find(tablename);
		if (it != tables.end())
			return it->second;
		else
			return NULL;
	}

	inline void create_table(const char *_table_name,
		table_record_desc_t *record_descs, int _record_desc_num, int _primary_key_index)
	{
		if (_record_desc_num > ATTR_NUM_MAX)
		{
			std::cerr << DB_PROMPT_PREFIX << " error, too many attributes." << std::endl;
			return;
		}

		for (int i = 0; i < _record_desc_num; i++)
		{
			if (record_descs[i].attr_size > ATTR_SIZE_MAX)
			{
				std::cerr << DB_PROMPT_PREFIX << " error, too large column." << std::endl;
				return;
			}
		}

		table_t *new_table = new table_t(_table_name, record_descs, _record_desc_num, _primary_key_index);

		auto result = tables.insert(std::pair<std::string, table_t*>(_table_name, new_table));
		if (result.second)
		{
			std::cout << DB_PROMPT_PREFIX << "Table \"" 
				<< _table_name << "\" has been created." << std::endl;
			for (int i = 0; i < _record_desc_num; i++)
			{
				std::cout << DB_PROMPT_CONTINUE_PREFIX 
					<< record_descs[i];
				if (i == _primary_key_index)
					std::cout << " (Primary Key)";
				std::cout << std::endl;
			}
		}
		else
		{
			std::cerr << DB_PROMPT_PREFIX 
				<< "error, " << _table_name << " has been existed." << std::endl;
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
				std::cout << DB_PROMPT_PREFIX << _record << " has been inserted to " << _tablename << std::endl;
			}
			catch (table_exception_t e)
			{
				switch (e)
				{
				case DUPLICATE_KEY:
					std::cerr << DB_PROMPT_PREFIX << _record[table_ptr->pk_index()] << " key has already existed." << std::endl;
					break;
				case DUPLICATE_RECORD:
					std::cerr << DB_PROMPT_PREFIX << _record << " has already existed." << std::endl;
					break;
				case RECORD_ATTR_NUM_INCONSISTENT:
					std::cerr << DB_PROMPT_PREFIX << _record << " attribute number is wrong." << std::endl;
					break;
				case RECORD_DOMAIN_INCONSISTENT:
					std::cerr << DB_PROMPT_PREFIX << _record << " domain is wrong." << std::endl;
					break;
				case RECORD_PRIMARY_KEY_NULL:
					std::cerr << DB_PROMPT_PREFIX << _record << " primary key cannot be null." << std::endl;
					break;
				case RECORD_SIZE_INCONSISTENT:
					std::cerr << DB_PROMPT_PREFIX << _record << " size is too large." << std::endl;
					break;
				default:
					std::cerr << DB_PROMPT_PREFIX << "unhandled exception " << e << std::endl;
					break;
				}
			}
		}
		else
		{
			std::cerr << DB_PROMPT_PREFIX << "table " << _tablename << " doesn't exist." << std::endl;
		}
	}

	inline void show(const char *tablename)
	{
		auto result = tables.find(tablename);
		if (result != tables.end())
		{
			std::cout << *(result->second) << std::endl;
		}
		else
		{
			std::cerr << DB_PROMPT_PREFIX << "table " << tablename << " doesn't exist." << std::endl;
		}
	}
private:
	hashtable tables;
};