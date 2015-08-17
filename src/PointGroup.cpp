//
//  PointGroup.cpp
//  PointGroupTest
//
//  Created by Robert Albert on 03.06.15.
//
//

#include "PointGroup.h"

PointGroup::PointGroup() {
    mTimestamp = ofGetElapsedTimeMillis();
    mStyleID = 0;
    mGroupID = instanceCount;
    instanceCount++;
    mStyleID = STYLE_FINGER;
}
void PointGroup::removePoint(GPoint _point) {
    for (int i = 0; i < mPoints.size(); i++)
    {
        if (mPoints[i].getStrokeId() == _point.getStrokeId()) {
            mPoints.erase(mPoints.begin()+i);
            break;
        }
    }
    //center neu berechnen
    calculateCenter();
}

//----------------------------------
void PointGroup::calculateCenter() {
    float x = 0;
    float y = 0;
    
    for (GPoint _point : mPoints)
    {
        ofVec2f loc = _point.getLocation();
        x = loc.x + x;
        y = loc.y + y;
    }
    x /= mPoints.size();
    y /= mPoints.size();
    
    mCenter.set(x,y);
}

//----------------------------------
float PointGroup::getDistanceToCenter(ofVec2f _loc) {
    return (_loc.distance(mCenter));
}

//----------------------------------
void PointGroup::addPoint(GPoint _point) {
    mPoints.push_back(_point);
    calculateCenter();
}

//--------------------------------------------------------------
bool PointGroup::checkForStrokeID(int _strokeID){
    for (GPoint _point : mPoints)
    {
        if (_point.getStrokeId() == _strokeID)
            return true;
    }
    return false;
}
int PointGroup::instanceCount;