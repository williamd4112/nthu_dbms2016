#include <string>
#include "Query.h"

using namespace std;
Query::Query() {
	int action=NULL;//1 CREATE TABLE; 2 INSERT
}
Query::~Query()
{
	//attributes.swap()
	attributes.clear();
}
void Query::setTableName(string name) {
	tableName = name;
}
string Query::getTableName() {
	return tableName;
}
void Query::setAction(int inputAction) {
	action = inputAction;
}
int Query::getAction() {
	return action;
}

void Query::printQuery()
{
	cout <<"table:"<< tableName << endl;
	cout <<"action:"<< action << endl;
	for (int i = 0; i < attributes.size(); i++) {
		cout <<"attrName:" <<attributes.at(i)->getAttrName()<<endl;
		if (action == 2) {
			if (attributes.at(i)->getAttrType() == 1)
				cout << "attrValue:" << *static_cast<int*>(attributes.at(i)->getAttrValue()) << endl;
			else
				cout << "attrValue:" << *static_cast<string*>(attributes.at(i)->getAttrValue()) << endl;
		}
		
		cout << "attrType:"<<attributes.at(i)->getAttrType() << endl;
		cout << "attrLength:"<<attributes.at(i)->getAttrLength() << endl;
		cout << "PrimaryKey:"<<attributes.at(i)->getPrimaryKey() << endl;
		cout << endl;
	}
	
}

