// ArffOps.cpp

#include "ArffOps.h"
#include "ArffUtil.h"

#include <vector>
#include <algorithm>
#include <cassert>

#define UNUSED(x) ((void)(x))
#define ATT_TYPE "INTEGER"
#define CONF_THRESHOLD 0.5

using namespace std;

/*static*/ void ArffOps::MajorityVote(Arff *pArff, const char *attributeName, vector<unsigned int> attIds)
{
    int rows, columns;
    pArff->Size(rows, columns);

    // if empty or no attributes provided return
    if (rows == 0)
        return;

    // check if attribute ids are within limits
    for (size_t i=0; i<attIds.size(); i++)
        assert(attIds[i] < (unsigned int)columns);

    // if no attribute before majority vote, add attribute and return
    pArff->AddColumn(attributeName, ATT_TYPE);

    // vector to hold the attributes
    vector<double> attributes;

    int maxLabel, maxCount;
    int curLabel, curCount;

    int confIndex;
    bool res = pArff->GetAttIndex("confidence", confIndex);
    UNUSED(res);

    for (int i=0; i<rows; i++)
    {
        attributes.clear();
        for (size_t j=0; j<attIds.size(); j++)
            attributes.push_back((*pArff)[i][attIds[j]]);
        
        sort(attributes.begin(), attributes.end());
        // push back 0 in order to get an attribute change
        attributes.push_back(0);

        maxLabel = 0;
        maxCount = -1;
        curLabel = attributes[0];
        curCount = 0;

        for (size_t j=0; j<attributes.size(); j++)
        {
            // attribute = curLabel increase counter
            if (attributes[j] == curLabel)
            {
                curCount++;
            }
            else
            {
                if (curCount > maxCount && curLabel >= 0){
                    maxLabel = curLabel;
                    maxCount = curCount;
                }
                curLabel = attributes[j];
                curCount = 1;
            }
        }

        //check if confidence for current sample is above threshold
        double confThreshold = CONF_THRESHOLD;
        if ((*pArff)[i][confIndex] > confThreshold)
        {
            // set label of newly added column to the mode of attributes if greater than 0
            pArff->ChangeData(i, columns, maxLabel);
        }
        else
        {
            // set label to noise -> 4
            pArff->ChangeData(i, columns, 4);
        }
    }
}

/*static*/ void ArffOps::MajorityVote(Arff *pArff, const char *attributeName)
{
    int rows, columns;
    pArff->Size(rows, columns);

    // if empty return
    if (rows == 0)
        return;

    int t,x,y,c;
    bool res = ArffUtil::GetTXYCindex(pArff, t, x, y, c);
    UNUSED(res);
    vector<int> ind = {t,x,y,c}; 

    vector<unsigned int> attIds;
    for (unsigned int i=0; i<(unsigned int)columns; i++)
    {
        vector<string> values;
        pArff->GetAttMapping(i, values);
        if (find(ind.begin(), ind.end(), i) == ind.end() && !values.empty())
            attIds.push_back(i);
    }

    ArffOps::MajorityVote(pArff, attributeName, attIds);
}

/*static*/ void ArffOps::MajorityVote(Arff *pArff, const char *attributeName, unsigned int startId, unsigned int endId)
{
    vector<unsigned int> attIds;
    for (size_t i=startId; i<=endId; i++)
        attIds.push_back(i);

    ArffOps::MajorityVote(pArff, attributeName, attIds);
}
