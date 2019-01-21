// ArffBase.h

#ifndef __ARFFBASE_H__
#define __ARFFBASE_H__

#include "AttributeTypes.h"

#include <memory>
#include <vector>
#include <iterator>

using namespace std;

typedef vector<vector<double>> arffData;

class ArffBase
{
public:
    ArffBase();

    ArffBase(const char* filename);

    ~ArffBase();

    bool Load(const char *filename);

    void Save(const char *filename);

    void AddRow(vector<double> row);
    ///< Adds the provided row at the end of the data

    void AddColumn(string attName, string type);
    ///< Adds a new column at the end and initializes the new column data to 0. The string type
    ///< is either "integer", "numeric" or nominal. For nominal attributes provide
    ///< the possible values within curly braces (ex. "{val1,val2}").

    vector<double>& operator[](const int index);
    ///< Returns an entry (a row) of the ARFF data.

    void SetRelation(string relation);

    string GetRelation();

    bool GetMetadata(const string& key, string& value);

    bool SetMetadata(const string &key, const string &value);
    ///< This function returns always true.

    bool GetAttMapping(const int &attIndex, vector<string>& values);

    bool GetAttMapping(const string &attName, vector<string>& values);
    ///< The values are the strings of the mapping to the doubles
    ///< in the data.

    bool GetAttIndex(const string &attName, int &index) const;

    void Size(int &rows, int &columns) const;
    ///< \p rows represents number of data rows in the ARFF file and
    ///< \p columns represents the number of attributes.

    arffData::const_iterator cbegin() const;

    arffData::const_iterator cend() const;

    arffData::iterator begin();

    arffData::iterator end();
private:
    void ProcessArffLine(string& line);
    void ProcessMetadata(vector<string>& metaLine);
    void ProcessAttribute(vector<string>& attLine);
    void ProcessComment(string& comLine);
    void ProcessData(string& dataLine);

    unique_ptr<AttributeType> AttributeFactory(string attName, string type);

    bool CompareStrings(string first, string second) const;
    ///< Case insensitive string comparison.


    vector<unique_ptr<AttributeType>>   m_vpAttributes;
    map<string, string>                 m_mMetadata;
    vector<string>                      m_vComments;

    arffData                            m_data;
    string                              m_relation;

    bool                                m_dataReached;
};

#endif /*__ARFFBASE_H__*/
