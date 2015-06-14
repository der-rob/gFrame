//
//  PointGroupList.h
//  PointGroupTest
//
//  Created by Robert Albert on 03.06.15.
//
//

#ifndef __PointGroupTest__PointGroupList__
#define __PointGroupTest__PointGroupList__

#include <stdio.h>
#include "PointGroup.h"

class PointGroupList {
private:
    vector<PointGroup> mGroupList;
public:
    void add(GPoint _point);
    void addToNewGroup(GPoint _point);
    void removePoint(GPoint _point);
    bool isEmpty() {return (mGroupList.size() == 0);}
    vector<PointGroup> *getPointGroupList() {return &mGroupList;}
    
    ofEvent<bool> E_enablePlaceMode;
    ofEvent<void> waverLeftEvent;
    
    bool enablePlaceMode;
    PointGroup *getGroup(int _strokeID);
};

#endif /* defined(__PointGroupTest__PointGroupList__) */
