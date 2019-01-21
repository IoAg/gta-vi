// AttributeTypes.h

#ifndef __ATTRIBUTETYPES_H__
#define __ATTRIBUTETYPES_H__

#include <string>
#include <ostream>
#include <fstream>
#include <map>
#include <vector>

using namespace std;

class AttributeType
{
public:
    AttributeType(string attName) : m_attName(attName) {}
    
    virtual ~AttributeType() = 0;

    virtual double operator()(string attValue) = 0;
    ///< Functor to convert the string values of the attribute in the file
    ///< to double representation in the data.

    virtual void Print(ofstream& ofs, double const &attValue) = 0;
    ///< Writes data correctly formatted to the provided output file stream.

    virtual void PrintDescription(ofstream &ofs);
    ///< Writes the description of the attributes as it should be in the ARFF file.

    virtual vector<string> GetMapping();
    ///< Returns the mapping of the attribute values to the one
    ///< provided in the definition of the attribute.

    string GetName();

private:
    string m_attName;
};

// Class representing Integer attribute in ARFF
class AttributeTypeInt : public AttributeType
{
public:
    AttributeTypeInt(string attName);
    
    ~AttributeTypeInt();

    double operator()(string attValue);
    ///< The returned double can correctly represent an integer up the
    ///< range of its fraction term with 0 exponent.

    void Print(ofstream& ofs, double const &attValue);
    
    void PrintDescription(ofstream &ofs);
};

// Class representing Numeric attribute in ARFF
class AttributeTypeNum : public AttributeType
{
public:
    AttributeTypeNum(string attName, int precision = 2);
    
    ~AttributeTypeNum();

    double operator()(string attValue);
    ///< Just return a double.

    void Print(ofstream& ofs, double const &attValue);

    void PrintDescription(ofstream &ofs);
private:
    int m_precision;
};

// Class representing Nominal attribute in ARFF
class AttributeTypeNom : public AttributeType
{
public:
    AttributeTypeNom(string attName, string attDescription);
    ///< The input string is the line describing the attribute. It processes 
    ///< the content within the curly braces.
    
    ~AttributeTypeNom();

    double operator()(string attValue);
    ///< Returns the position of the string the list of nominal values of teh attribute.
    ///< It works quivalently to an enumeration.

    void Print(ofstream& ofs, double const &attValue);

    void PrintDescription(ofstream &ofs);

    vector<string> GetMapping();
    ///< Returns the mapping of nominal values to double.
private:
    map<string,double> m_nominalMap;
    map<double,string> m_inversedMap;
};

#endif /*__ATTRIBUTETYPES_H__*/
