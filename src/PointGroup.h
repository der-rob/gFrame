//
//  PointGroup.h
//  PointGroupTest
//
//  Created by Robert Albert on 03.06.15.
//
//

#ifndef __PointGroupTest__PointGroup__
#define __PointGroupTest__PointGroup__

#include <stdio.h>
#include "GPoint.h"

class PointGroup {
public:
    PointGroup();
    float getDistanceToCenter(ofVec2f _loc);
    vector<GPoint> *getPoints() {return &mPoints;}

    void addPoint(GPoint _point);
    void removePoint(GPoint _point);
    ofVec2f getCenter() { return mCenter; }
    int getPointCount() { return mPoints.size();}
    unsigned long getTimestamp() {return mTimestamp;}
    void calculateCenter();
    
    void setStyle(int _styleID) {this->mStyleID = _styleID; }
    int getStyle() { return mStyleID; }
    int getGroupID() { return mGroupID;}
    bool checkForStrokeID(int _strokeID);
    
private:
    vector<GPoint> mPoints;
    ofVec2f mCenter;
    int mStyleID;
    int mGroupID;
    unsigned long mTimestamp;
    
    bool isEmpty() {return (mPoints.size()==0);}
    static int instanceCount;
    
    
};
#endif /* defined(__PointGroupTest__PointGroup__) */
