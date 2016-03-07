#pragma once
#include<iostream>
#include<string>
using namespace std;

class Attribute {
public:
	Attribute();
	void setAttrName(string&);
	string& getAttrName();
	
	void setAttrValue(string);
	void setAttrValue(int);

	void* getAttrValue();

	void setAttrType(int);
	int getAttrType();

	void setAttrLength(int);
	int getAttrLength();

	void setPrimaryKey(bool);
	bool getPrimaryKey();
private:
	void *attrValue;
	int  attrIValue;
	string attrSValue;
	string attributeName;
	int attributeType;//1 int ;2 varchar
	int attributeLength;
	bool primaryKey;
};


