// EquirectangularToHead.cpp

#include "EquirectangularToHead.h"
#include "../arffHelper/ArffUtil.h"

#include <iostream>
#include <memory>

using namespace std;

EquirectangularToHead::EquirectangularToHead(Arff *arff) : m_pArff(arff) 
{
}

EquirectangularToHead::~EquirectangularToHead()
{
}

unique_ptr<Arff> EquirectangularToHead::Convert()
{
    const char *c_horHeadAttName = "x_head";
    const char *c_verHeadAttName = "y_head";

    unique_ptr<Arff> convertedArff(new Arff());
    convertedArff->SetWidthPx(m_pArff->WidthPx());
    convertedArff->SetHeightPx(m_pArff->HeightPx());
    convertedArff->SetRelation(m_pArff->GetRelation());
    convertedArff->AddColumn("time", "integer");
    convertedArff->AddColumn("x", "numeric");
    convertedArff->AddColumn("y", "numeric");
    convertedArff->AddColumn("confidence", "numeric");

    int xInd, yInd, timeInd, confInd;
    bool res = ArffUtil::GetTXYCindex(m_pArff, timeInd, xInd, yInd, confInd);

    int xHeadInd, yHeadInd;
    res &= m_pArff->GetAttIndex(c_horHeadAttName, xHeadInd);
    res &= m_pArff->GetAttIndex(c_verHeadAttName, yHeadInd);

    if (!res)
    {
        cout << "ERROR: could not find time, or x, or y, or confidence, or " << c_horHeadAttName << ", or " << c_verHeadAttName << " in the provided ARFF file." << endl;
        exit(-1);
    }

    for (auto entry : *m_pArff)
    {
        vector<double> row;
        row.push_back(entry[timeInd]);
        row.push_back(entry[xHeadInd]);
        row.push_back(entry[yHeadInd]);
        row.push_back(entry[confInd]);

        convertedArff->AddRow(row);
    }

    return move(convertedArff);
}
