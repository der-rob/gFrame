//
//  PointList.cpp
//  gFrame
//
//  Created by Julian Adenauer on 24.10.14.
//
//

#include "StrokeList.h"

void StrokeList::setupSync(Network* network){
    this->network = network;
    enableSync = true;
}

void StrokeList::addToNewStroke(GPoint point){
    
    // assign stroke id
    point.setStrokeId(stroke_count);
    point.setType(TYPE_LOCAL);
    
    // increase the stroke count and reset it when it is too big
    stroke_count++;
    if(stroke_count == 999) stroke_count = 0;
    
    vector<GPoint> v;
    v.insert(v.begin(), point);
    strokes.insert(strokes.begin(), v);
    
    // the current stroke for this ID is now the last one in the list
    currentListForId[point.getId()] = strokes.size()-1;
    
    // send the point to the remote server
    if(enableSync){
        network->send(point);
    }
}

void StrokeList::add(GPoint point){
    // check if we already have a stroke with this id in the list
    if(currentListForId.count(point.getId()) == 1){
        // get the stroke id from the last one on the same list
        point.setStrokeId(strokes[currentListForId[point.getId()]].back().getStrokeId());
        point.setType(TYPE_LOCAL);
//        strokes[currentListForId[point.getId()]].push_back(point);
        strokes[currentListForId[point.getId()]].insert(strokes[currentListForId[point.getId()]].begin(), point);
    }
    // otherwise treat the point like a new stroke
    else{
        addToNewStroke(point);
    }
    
    // send the point to the remote server
    if(enableSync){
        network->send(point);
    }
}

void StrokeList::update(){
    
    // remove points that have exceeded their lifetime
    for(int i = 0; i < strokes.size(); i++)
    {
        for(int j = 0; j < strokes[i].size(); j++){
            if ((ofGetElapsedTimeMillis()/10.0) - strokes[i][j].getTimestamp() > lifetime) {
//                strokes[i].erase(strokes[i].begin() + j);
                strokes[i].pop_back();
            }
        }
        
        // erase the stroke if it is empty
        if(strokes[i].size() == 0){
            strokes.erase(strokes.begin() + i);
            
            // if this vector was the current one for one of the ids, remove it from the currentListForId
            for(int m = 0; m<currentListForId.size(); m++){
                if (currentListForId[m] == i) {
                    currentListForId.erase(currentListForId[m]);
                }
                // decrease the following ones by one
                else if (currentListForId[m] > i){
                    currentListForId[m]-=1;
                }
            }
        }
    }
    
    // get new points from the remote frame
    if(enableSync){
        while(network->getReceiveQueueLength() > 0){
            GPoint p = network->popPoint();
            p.setType(TYPE_REMOTE);
            
            // check if we already have the stroke id of this point in the list
            int found = -1;
            for(int i = 0; i < strokes.size(); i++){
                if(strokes[i][0].getStrokeId() == p.getStrokeId() && strokes[i][0].getType() == TYPE_REMOTE){
                    found = i;
                    break;
                }
            }
            
            if(found != -1){
                strokes[found].push_back(p);
            }
            // otherwise make a new list
            else {
                vector<GPoint> v;
                v.push_back(p);
                strokes.push_back(v);
            }
        }
    }
}

void StrokeList::clear(){
    strokes.clear();
    currentListForId.clear();
}

//void StrokeList::getStrokesById(int id, vector<vector<GPoint> >* list){
//    for(vector<GPoint> stroke : strokes){
//        if(stroke[0].getId() == id){
//            list->push_back(stroke);
//        }
//    }
//}
