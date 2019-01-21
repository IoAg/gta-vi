// EquirectangularToHead.h
// Transfers the x_head, y_head coordinates to the x,y

#ifndef __EQUIRECTANGULARTOHEAD_H__
#define __EQUIRECTANGULARTOHEAD_H__

#include "../arffHelper/Arff.h"

class EquirectangularToHead
{
public:
    EquirectangularToHead(Arff *pArff);

    ~EquirectangularToHead();

    unique_ptr<Arff> Convert();
    ///< Populates the x,y attributes of the returned ARFF with the x_head, 
    ///< y_head of the input ARFF.

private:
    Arff    *m_pArff;
};

#endif /*__EQUIRECTANGULARTOHEAD_H__*/
