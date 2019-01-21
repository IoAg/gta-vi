// ArffUtil.h

#ifndef __ARFFUTIL_H__
#define __ARFFUTIL_H__

#include "Arff.h"

class ArffUtil
{
public:
    static unsigned long int FindPosition(const Arff *pArff, const int &attIndex, const double &value);
    ///< This function searches in a sorted column of the Arff class and returns
    ///< the position of the first element that is bigger than the provided value.

    static bool GetTXYCindex(const Arff *pArff, int &timeInd, int &xInd, int &yInd, int &confInd);
    ///< This is a helper function to get time, x, y, confidence indices in Arff data.
};

#endif /*__ARFFUTIL_H__*/
