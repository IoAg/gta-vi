// ArffBase.cpp

#include "ArffBase.h"

#include <fstream>
#include <iterator>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <iostream>

using namespace std;

ArffBase::ArffBase() : m_dataReached(false)
{
}

ArffBase::ArffBase(const char* filename) : m_dataReached(false)
{
    Load(filename);
}

ArffBase::~ArffBase()
{
}

bool ArffBase::Load(const char* filename)
{
    ifstream ifs;

    ifs.open(filename, ifstream::in);

    if (ifs.fail())
    {
        cerr << "Could not open provided ARFF file: " << filename << endl;
        return false;
    }

    string line;
    while (ifs)
    {
        if(!getline(ifs, line))
            break;
        ProcessArffLine(line);
    }

    ifs.close();
    return true;
}

void ArffBase::Save(const char* filename)
{
    ofstream ofs;
    ofs.open(filename, ofstream::out);

    ofs << "@RELATION " << m_relation << endl << endl;

    for (auto p: m_mMetadata)
        ofs << "\%@METADATA " << p.first << " " << p.second << endl;
    ofs << endl;

    for (unsigned int i=0; i<m_vpAttributes.size(); i++)
        (*m_vpAttributes[i]).PrintDescription(ofs);
    ofs << endl;

    for (auto p:m_vComments)
        ofs << p << endl;
    ofs << endl;

    ofs << "@DATA" << endl;
    for (auto lineEntry:m_data)
    {
        unsigned int i;
        for (i=0; i<lineEntry.size()-1; i++)
        {
            (*m_vpAttributes.at(i)).Print(ofs, lineEntry[i]);
            ofs << ",";
        }
        (*m_vpAttributes.at(i)).Print(ofs, lineEntry[i]);
        ofs << endl;
    }


    ofs.close();
}

void ArffBase::AddRow(vector<double> row)
{
    assert(row.size() == m_vpAttributes.size());
    m_data.push_back(row);
}

void ArffBase::AddColumn(string attName, string type)
{
    // remove spaces from type
    type.erase(remove(type.begin(), type.end(), ' '), type.end());

    m_vpAttributes.push_back(AttributeFactory(attName, type));

    for (auto &row:m_data)
        row.push_back(0.0);
}

vector<double>& ArffBase::operator[](const int index)
{
    // range checked indexing
    return m_data.at(index);
}

void ArffBase::SetRelation(string relation)
{
    m_relation = relation;
}

string ArffBase::GetRelation() const
{
    return m_relation;
}

bool ArffBase::GetMetadata(const string& key, string& value) const
{
    map<string,string>::const_iterator it = m_mMetadata.find(key);
    if (it != m_mMetadata.end())
    {
        value = it->second;
        return true;
    }

    value = "";
    return false;
}

bool ArffBase::SetMetadata(const string &key, const string &value)
{
    m_mMetadata[key] = value;

    return true;
}

bool ArffBase::GetAttMapping(const int &attIndex, vector<string>& values) const
{
    values = m_vpAttributes.at(attIndex)->GetMapping();
    if (values.size() == 0)
        return false;

    return true;
}

bool ArffBase::GetAttMapping(const string &attName, vector<string>& values) const
{
    int index = 0;

    if (!GetAttIndex(attName, index))
        return false;

    return GetAttMapping(index, values);
}

bool ArffBase::GetAttIndex(const string &attName, int &index) const
{
    for (index=0; index<(int)m_vpAttributes.size(); index++)
    {
        string curAttName = m_vpAttributes[index]->GetName();
        if (CompareStrings(attName, curAttName))
            break;
    }

    if (index == (int)m_vpAttributes.size())
        return false;

    return true;
}

void ArffBase::Size(int &rows, int &columns) const
{
    rows = m_data.size();
    columns = m_vpAttributes.size();
}

arffData::const_iterator ArffBase::cbegin() const
{
    return m_data.cbegin();
}

arffData::const_iterator ArffBase::cend() const
{
    return m_data.cend();
}

arffData::iterator ArffBase::begin()
{
    return m_data.begin();
}

arffData::iterator ArffBase::end()
{
    return m_data.end();
}

// PRIVATE:

void ArffBase::ProcessArffLine(string& line)
{
    if (line.size() == 0)
        return;

    // break string on white space
    stringstream ss(line);
    istream_iterator<string> begin(ss);
    istream_iterator<string> end;
    vector<string> lineWords(begin, end);

    // find what type of line it is
    if (m_dataReached)
        ProcessData(line);
    else if (lineWords.size() > 0 && CompareStrings(lineWords[0], "@ATTRIBUTE"))
        ProcessAttribute(lineWords);
    else if (lineWords.size() > 0 && CompareStrings(lineWords[0], "\%@METADATA"))
        ProcessMetadata(lineWords);
    else if (lineWords.size() > 1 && CompareStrings(lineWords[0], "@RELATION"))
        m_relation = lineWords[1];
    else if (line[0] == '%')
        ProcessComment(line);
    else if (lineWords.size() > 0 && CompareStrings(lineWords[0], "@DATA"))
        m_dataReached = true;

}

void ArffBase::ProcessMetadata(vector<string>& metaLine)
{
    m_mMetadata[metaLine[1]] = metaLine[2];
}

void ArffBase::ProcessAttribute(vector<string>& attLine)
{
    // Attribute with fewer than 3 string parts is not possible
    if (attLine.size() < 3)
        return;

    string attName = attLine[1];

    string type;
    if (attLine[2].find('{') != string::npos)
    {
        for (auto s:attLine)
            type += s;
    }
    else
        type = attLine[2];

    m_vpAttributes.push_back(AttributeFactory(attName, type));
}

unique_ptr<AttributeType> ArffBase::AttributeFactory(string attName, string type)
{
    unique_ptr<AttributeType> res;

    if (CompareStrings(type, "INTEGER"))
        res.reset(new AttributeTypeInt(attName));
    else if (CompareStrings(type, "NUMERIC"))
        res.reset(new AttributeTypeNum(attName));
    else
        res.reset(new AttributeTypeNom(attName, type));

    return move(res);
}

void ArffBase::ProcessComment(string& comLine)
{
    m_vComments.push_back(comLine);
}

void ArffBase::ProcessData(string& dataLine)
{
    // remove comments
    size_t pos = dataLine.find("\%");
    if (pos != string::npos)
        dataLine = dataLine.substr(0,pos);

    stringstream ss(dataLine);
    int id = 0;
    vector<double> dataEntry;
    while (ss)
    {
        string value;
        if (!getline(ss, value, ','))
            break;

        // Convert value based on the attribute
        double singleEntry = (*m_vpAttributes.at(id))(value);
        dataEntry.push_back(singleEntry);
        id++;
    }
    if (id == 0)
        return;

    assert(dataEntry.size() == m_vpAttributes.size()); 
    m_data.push_back(dataEntry);
}

bool ArffBase::CompareStrings(string first, string second) const
{
    transform(first.begin(), first.end(), first.begin(), ::toupper);
    transform(second.begin(), second.end(), second.begin(), ::toupper);

    return first == second;
}
