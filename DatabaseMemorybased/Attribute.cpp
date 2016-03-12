#include "Attribute.h"
using namespace std;

Attribute::Attribute() : 
	attributeType(0), attributeLength(0), primaryKey(false){

}
void Attribute::setAttrName(string &name)
{
	attributeName =name;
}

string & Attribute::getAttrName()
{
	return attributeName;
}

void Attribute::setAttrValue(string &value)
{
	attrSValue = value;
	attrValue = &attrSValue;
}
void Attribute::setAttrValue(int value)
{
	attrIValue = value;
	attrValue = &attrIValue;
}
void* Attribute::getAttrValue()
{
	return attrValue;
}
void Attribute::setAttrType(int type)
{
	attributeType = type;
}

int Attribute::getAttrType()
{
	return attributeType;
}

void Attribute::setAttrLength(int length)
{
	attributeLength = length;
}

int Attribute::getAttrLength()
{
	return attributeLength;
}

void Attribute::setPrimaryKey(bool PK)
{
	primaryKey = PK;
}

bool Attribute::getPrimaryKey()
{
	return primaryKey;
}
