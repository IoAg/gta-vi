// ArffOps.h

#ifndef __ARFFOPS_H__
#define __ARFFOPS_H__

#include "Arff.h"

#include <vector>

class ArffOps
{
public:
    static void MajorityVote(Arff *pArff, const char *attributeName, vector<unsigned int> attIds);
    ///< Add a columns at the end with the provided name and populates it
    ///< based on the majority vote of the provided columns (time, x, y excluded)
    ///< if they exist. Attribute values of 0 do not contribute in the vote.

    static void MajorityVote(Arff *pArff, const char *attributeName);
    ///< Overload function. Returns vote considering all columns.

    static void MajorityVote(Arff *pArff, const char *attributeName, unsigned int startId, unsigned int endId);
    ///< Overload function. Returns vote for given range.

};

#endif /*__ARFFOPS_H__*/
