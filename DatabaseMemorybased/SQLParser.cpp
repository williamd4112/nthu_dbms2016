#include <string>
#include "SQLParser.h"
#include <regex>
#include <deque>

using namespace std;

void SQLParser::parse(string& input) {
	
	regex reg_create(reg_createStr, regex_constants::icase);
	regex reg_insert(reg_insertStr, regex_constants::icase);
	smatch sm_cre,sm_ins;
	deque<string> splittoken;

	//split
	split(input, SemicolonStr, splittoken);
	while (!splittoken.empty()) {
		string token = splittoken.front();
		
		//create table
		if (regex_match(token, sm_cre, reg_create)) {
			Query* q= createTable(sm_cre);
			if (q != NULL) {
				queryQueue.push(q);
				//q->printQuery();
			}	
		}
		//insert into
		else if (regex_match(token, sm_ins, reg_insert)) {
			Query * q = insert(sm_ins);
			if (q != NULL) {
				queryQueue.push(q);
				//q->printQuery();
			}
		}
		else {
			cerr << "Syntax error" << endl;
		}
		splittoken.pop_front();
	}
	splittoken.clear();
}

Query * SQLParser::insert(smatch &sm_ins)
{
	Query *q = new Query();
	//set table name
	string tablename = sm_ins[1];
	string leftover=sm_ins[2].str();
	q->setTableName(tablename);
	q->setAction(INSERT);
	regex reg_bracket(reg_insertTypeStr, regex_constants::icase);
	smatch sm_bra,sm_value;
	if (regex_match(leftover, sm_bra, reg_bracket)) {
		deque<Attribute*> temp_attr;
		deque<string> valueStr_deque;
		deque<string> attrStr_deque;
		//set value
		string Svalues = sm_bra[4].str();
		split(Svalues, CommaStr, valueStr_deque);
		for (int i = 0; i < valueStr_deque.size(); i++) {
			if (regex_match(valueStr_deque.at(i), sm_value, regex(reg_attrValueStr))) {
				Attribute* temp = new Attribute();
				if (sm_value[2].str() != "") {
					temp->setAttrType(VARCHAR);
					temp->setAttrValue(sm_value[2].str());
				}
				else if(sm_value[3].str()!=""){
					string::size_type sz;
					temp->setAttrType(INT);
					temp->setAttrValue(stoi(sm_value[3].str(), &sz));
				}
				temp_attr.push_back(temp);
			}
		}
		//set attribute
		if (sm_bra[2].str() != "") {
			string Sattr = sm_bra[2].str();
			split(Sattr, "\\s*,\\s*", attrStr_deque);
			if (attrStr_deque.size() != valueStr_deque.size()) {
				cerr << "insert Number doesn't match" << endl;
				valueStr_deque.clear();
				attrStr_deque.clear();
				temp_attr.clear();
				delete(q);
				return NULL;
			}
			else{
				int asize = attrStr_deque.size();
				for (int i = 0; i < asize; i++) {
					
					temp_attr.at(i)->setAttrName(attrStr_deque.front());
					attrStr_deque.pop_front();
				}
			}
		}
		q->attributes = temp_attr;
	}
	else {
		cerr << "Syntax error" << endl;
		delete(q);
		return NULL;
	}
	return q;
}

Query* SQLParser::createTable(smatch &sm_cre) {
	Query *q = new Query();
	//set table name
	string tablename = sm_cre[1];
	q->setTableName(tablename);
	q->setAction(CREATE);
	//split attribute
	deque<string> attribute;
	split(sm_cre[2].str(), ",", attribute);
	//attribute syntax check
	regex reg_attr(reg_attrStr, regex_constants::icase);
	smatch sm_attr;
	while (!attribute.empty()) {
		Attribute *temp = new Attribute();
		string attr = attribute.front();
		if (regex_match(attr, sm_attr, reg_attr)) {
			//set attribute name
			temp->setAttrName(sm_attr[1].str());
			//set data type
			if (caseInsensitiveStrcmp(sm_attr[2].str(), "int")) {
				temp->setAttrType(INT);
				temp->setAttrLength(4);
			}
			else if (caseInsensitiveStrcmp(sm_attr[2].str(), "varchar")) {
				temp->setAttrType(VARCHAR);
				string::size_type sz;
				temp->setAttrLength(stoi(sm_attr[4].str(), &sz));
			}
			else {
				cerr << "unknown data type" << endl;
				delete(temp);
				attribute.clear();
				delete(q);
				return NULL;
			}
			//primary key
			if (caseInsensitiveStrcmp(sm_attr[5].str(), "primary key"))
				temp->setPrimaryKey(true);
			q->attributes.push_back(temp);
		}
		else {
			cerr << "Syntax error" << endl;
			delete(temp);
			attribute.clear();
			delete(q);
			return NULL;
		}
		attribute.pop_front();
	}
	attribute.clear();
	return q;
}
void SQLParser::split(string &input,const string &reg, deque<string>&token) {
	regex reg_token(reg);
	smatch sm;
	while (regex_search(input, sm, reg_token)) {
		string s = input.substr(0, sm.position());
		token.push_back(s);
		input = sm.suffix().str();

	}
	if(input!="")
		token.push_back(input);
}
bool SQLParser::caseInsensitiveStrcmp(const string& str1, const string& str2) {
	string str1Cpy(str1);
	string str2Cpy(str2);
	transform(str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), tolower);
	transform(str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), tolower);
	return (str1Cpy == str2Cpy);
}

