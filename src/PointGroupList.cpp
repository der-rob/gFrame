//
//  PointGroupList.cpp
//  PointGroupTest
//
//  Created by Robert Albert on 03.06.15.
//
//  Manages all pointgroups, and also decides with style each group has

#include "PointGroupList.h"


//------------------------------
void PointGroupList::add(GPoint _point) {
    ofVec2f loc = _point.getLocation();
    bool found = false;
    for (int i = 0; i<mGroupList.size(); i++)
    {
        //check if point is near one group
        if (mGroupList[i].getDistanceToCenter(loc) < 0.3)
        {
            //check if point is already in one group (via stroke ID)
            vector<GPoint> *points = mGroupList[i].getPoints();
            for (int j=0; j<points->size(); j++)
            {
                if (_point.getStrokeId() == points->at(j).getStrokeId())
                {
                    points->at(j) = _point; //alter punkt wird geupdated
                    
                    found = true;
                    mGroupList[i].calculateCenter();
                    return;
                }
            }
            
            //ToDo: Punkt war bisher nicht in der Gruppe -> prüfen ob Punkt zur Gruppe gehören sollte, Zeit betrachten
            if (_point.getTimestamp() - mGroupList[i].getTimestamp() < 500 )
            {
                mGroupList[i].addPoint(_point);
                if (mGroupList[i].getPoints()->size() == 1)
                    mGroupList[i].setStyle(STYLE_FINGER);
                else if (mGroupList[i].getPoints()->size() == 2)
                {
                    mGroupList[i].setStyle(STYLE_SCRIZZLE);
                    enablePlaceMode = true;
                    //ofNotifyEvent(E_enablePlaceMode, enablePlaceMode, this); //this event will be cause by osc message later
                }
                else if (mGroupList[i].getPoints()->size() == 3)
                    mGroupList[i].setStyle(STYLE_CALIGRAPHY);
                else if (mGroupList[i].getPoints()->size() == 5)
                    mGroupList[i].setStyle(STYLE_WAVER);
                
                found = true; //needs to be more specific later
                mGroupList[i].calculateCenter();
            }
        }
    }
    
    if (!found) //point belongs to new group/is single waving point
    {
        addToNewGroup(_point);
    }
}

//------------------------------
void PointGroupList::addToNewGroup(GPoint _point) {
    PointGroup newGroup;
    newGroup.addPoint(_point);
    mGroupList.push_back(newGroup);
}

//------------------------------
void PointGroupList::removePoint(GPoint _point) {
    
    for (int i = 0; i < mGroupList.size(); i++)
    {
        mGroupList[i].removePoint(_point);
        
        //remove group when emtpy
        if(mGroupList[i].getPointCount() <= 0)
        {
            //needed for stencil placement
            if (mGroupList[i].getStyle() == STYLE_WAVER) {
                enablePlaceMode = false;
                ofNotifyEvent(E_enablePlaceMode, enablePlaceMode,this);
            }
            
            mGroupList.erase(mGroupList.begin() + i);
        }
    }
}

//--------------------------------------------------------------
PointGroup *PointGroupList::getGroup(int _strokeID) {
    for(int i = 0; i < mGroupList.size(); i++) {
        if (mGroupList[i].checkForStrokeID(_strokeID))
            return &mGroupList[i];
    }
}
