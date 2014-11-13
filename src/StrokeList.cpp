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
    
    point.setType(TYPE_LOCAL);
        
    vector<GPoint> v;
    v.push_back(point);
    strokes.push_back(v);
    
    // the current list for this strokeID is now the last one in the list
//    listForStrokeId[point.getStrokeId()] = strokes.size()-1;
    
    // send the point to the remote server
    if(enableSync){
        network->send(point);
    }
}

void StrokeList::add(GPoint point){
    bool found =false;
    
    // check if we already have this stroke id
    for(int i = 0; i< strokes.size(); i++){
        if(strokes[i][0].getStrokeId() == point.getStrokeId()){
            point.setType(TYPE_LOCAL);
            strokes[i].push_back(point);
            found = true;
            break;
        }
    }
    
    if(!found){
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
            if ((ofGetElapsedTimeMillis()) - strokes[i][j].getTimestamp() > lifetime) {
                strokes[i].erase(strokes[i].begin() + j);
            }
        }
        
        // erase the stroke if it is now empty
        if(strokes[i].size() == 0){
            strokes.erase(strokes.begin() + i);
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
//    currentListForId.clear();
//    listForStrokeId.clear();
}

//void StrokeList::getStrokesById(int id, vector<vector<GPoint> >* list){
//    for(vector<GPoint> stroke : strokes){
//        if(stroke[0].getId() == id){
//            list->push_back(stroke);
//        }
//    }
//}
