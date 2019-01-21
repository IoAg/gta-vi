// AttributeTypes.cpp

#include "AttributeTypes.h"

#include <iomanip>
#include <sstream>
#include <algorithm>
#include <clocale>

using namespace std;

// CLASS ATTRIBUTE_TYPE
AttributeType::~AttributeType()
{
}

void AttributeType::PrintDescription(ofstream &ofs)
{
    ofs << "@ATTRIBUTE " << m_attName << " ";
}

vector<string> AttributeType::GetMapping()
{
    return vector<string>(0);
}

string AttributeType::GetName()
{
    return m_attName;
}

// CLASS ATTRIBUTE_TYPE_INT
AttributeTypeInt::AttributeTypeInt(string attName) : AttributeType(attName)
{
}

AttributeTypeInt::~AttributeTypeInt()
{
}

double AttributeTypeInt::operator()(string attValue)
{
    return stoi(attValue);
}

void AttributeTypeInt::Print(ofstream& ofs, double const &attValue)
{
    ofs << (int) attValue;
}

void AttributeTypeInt::PrintDescription(ofstream& ofs)
{
    AttributeType::PrintDescription(ofs);
    ofs << "INTEGER\n";
}

// CLASS ATTRIBUTE_TYPE_NUM
AttributeTypeNum::AttributeTypeNum(string attName, int precision) : AttributeType(attName), m_precision(precision)
{
    // *** Use the following locale in order to get "." as decimal-point separator
    std::setlocale(LC_NUMERIC, "C");
}

AttributeTypeNum::~AttributeTypeNum()
{
}

double AttributeTypeNum::operator()(string attValue)
{
    return stod(attValue);
}

void AttributeTypeNum::Print(ofstream& ofs, double const &attValue)
{
    ofs << fixed << setprecision(m_precision) << attValue;
}

void AttributeTypeNum::PrintDescription(ofstream& ofs)
{
    AttributeType::PrintDescription(ofs);
    ofs << "NUMERIC\n";
}

// CLASS ATTRIBUTE_TYPE_NOM
AttributeTypeNom::AttributeTypeNom(string attName, string attDescription) : AttributeType(attName)
{
    // find content between curly braces
    size_t start = attDescription.find("{");
    size_t end = attDescription.find("}");

    if (start == string::npos || end == string::npos)
        throw range_error("Could not find one or both curly braces");

    attDescription = attDescription.substr(start+1, end-start-1);
    
    stringstream ss(attDescription);
    int id = 0;
    while (ss)
    {
        string value;
        if (!getline(ss, value, ','))
            break;
        m_nominalMap[value] = id;
        id++;
    }

    //create inverse map
    for (auto p: m_nominalMap)
        m_inversedMap[p.second] = p.first;
}

AttributeTypeNom::~AttributeTypeNom()
{
}

double AttributeTypeNom::operator()(string attValue)
{
    return m_nominalMap[attValue];
}

void AttributeTypeNom::Print(ofstream& ofs, double const &attValue)
{
    ofs << m_inversedMap[attValue];
}

void AttributeTypeNom::PrintDescription(ofstream& ofs)
{
    AttributeType::PrintDescription(ofs);
    ofs << "{";
    std::map<string,double>::iterator it;
    unsigned int i;
    for (i=0; i<m_inversedMap.size()-1; i++)
        ofs << m_inversedMap[i] << ',';
    ofs << m_inversedMap[i] <<  "}" << endl;
}

vector<string> AttributeTypeNom::GetMapping()
{
    vector<string> res;
    for (unsigned int i=0; i<m_inversedMap.size(); i++)
        res.push_back(m_inversedMap[i]);

    return res;
}
