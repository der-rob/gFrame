//
//  PointList.cpp
//  gFrame
//
//  Created by Julian Adenauer on 24.10.14.
//
//

#include "StrokeList.h"


void StrokeList::addToNewStroke(GPoint point){
    
    // assign stroke id
    point.setStrokeId(stroke_count);
    
    // increase the stroke count and reset it when it is too big 
    stroke_count++;
    if(stroke_count == 999) stroke_count = 0;
    
    vector<GPoint> v;
    v.push_back(point);
    strokes.push_back(v);
    
    // the current stroke for this ID is now the last one in the list
    currentStrokeForId[point.getId()] = strokes.size()-1;
}

void StrokeList::add(GPoint point){
    // check if we already have a stroke with this id in the list
    if(currentStrokeForId.count(point.getId()) == 1){
        // get the stroke id from the last one on the same list
        point.setStrokeId(strokes[currentStrokeForId[point.getId()]].back().getStrokeId());
        
        ofLog() << currentStrokeForId[point.getId()];
        strokes[currentStrokeForId[point.getId()]].push_back(point);
    }
    // otherwise treat the point like a new stroke
    else{
        addToNewStroke(point);
    }
}

void StrokeList::update(){

    // remove points that have exceeded their lifetime
    for(int i = 0; i < strokes.size(); i++)
    {
        for(int j = 0; j < strokes[i].size(); j++){
            if ((ofGetElapsedTimeMillis()/10) - strokes[i][j].getTimestamp() > lifetime) {
                strokes[i].erase(strokes[i].begin() + j);
            }
        }
        
        // erase the stroke if it is empty
        if(strokes[i].size() == 0){
            strokes.erase(strokes.begin() + i);
            
            // if this vector was the current one for one of the ids, remove it from the currentStrokeForId
            for(int m = 0; m<currentStrokeForId.size(); m++){
                if (currentStrokeForId[m] == i) {
                    currentStrokeForId.erase(currentStrokeForId[m]);
                }
                // decrease the following ones by one
                else if (currentStrokeForId[m] > i){
                    currentStrokeForId[m]-=1;
                }
            }
        }
    }
}

//void StrokeList::getStrokesById(int id, vector<vector<GPoint> >* list){
//    for(vector<GPoint> stroke : strokes){
//        if(stroke[0].getId() == id){
//            list->push_back(stroke);
//        }
//    }
//}
