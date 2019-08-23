// ArffUtil.cpp

#include "ArffUtil.h"

#include <algorithm>

/*static*/ unsigned long int ArffUtil::FindPosition(const Arff *pArff, const int &attIndex, const double &value)
{
    arffData::const_iterator pos = lower_bound(pArff->cbegin(), pArff->cend(), value, [&attIndex](const vector<double>& a, const long double& b) { return a.at(attIndex) < b;});

    // if the provided time is higher than all times in the data, pos points to
    // the first element after the vector. Thus outside of its range. It is
    // changed to last valid point
    unsigned int res = pos - pArff->cbegin();
    
    if (pos == pArff->cend())
        return res-1;
    else
        return res;
}

/*static*/ bool ArffUtil::GetTXYCindex(const Arff *pArff, int &timeInd, int &xInd, int &yInd, int &confInd)
{
    bool res = true;

    res &= pArff->GetAttIndex("time", timeInd);
    res &= pArff->GetAttIndex("x", xInd);
    res &= pArff->GetAttIndex("y", yInd);
    res &= pArff->GetAttIndex("confidence", confInd);

    return res;
}

/*static*/ double ArffUtil::GetSamplingPeriod(const Arff *pArff)
{
    int timeInd;
    pArff->GetAttIndex("time", timeInd);
    int rows, columns;
    pArff->Size(rows, columns);

    double startTime = pArff->cbegin()[0][timeInd];
    double endTime = pArff->cend()[-1][timeInd];
    double dur = endTime - startTime;

    return dur / rows;
}
