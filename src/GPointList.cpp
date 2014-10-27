//
//  PointList.cpp
//  gFrame
//
//  Created by Julian Adenauer on 24.10.14.
//
//

#include "GPointList.h"


void GPointList::addToNewStroke(GPoint point){
    vector<GPoint> v;
    v.push_back(point);
    strokes.push_back(v);
    
    // the current stroke for this ID is now the last one in the list
    currentStrokeForId[point.getId()] = strokes.size()-1;
}

void GPointList::add(GPoint point){
    // check if we already have a stroke with this id in the list
    if(currentStrokeForId.count(point.getId()) == 1){
        ofLog() << currentStrokeForId[point.getId()];
        strokes[currentStrokeForId[point.getId()]].push_back(point);
    }
    // otherwise treat the point like a new stroke
    else{
        addToNewStroke(point);
    }
}

//void GPointList::getStrokesById(int id, vector<vector<GPoint> >* list){
//    for(vector<GPoint> stroke : strokes){
//        if(stroke[0].getId() == id){
//            list->push_back(stroke);
//        }
//    }
//}
