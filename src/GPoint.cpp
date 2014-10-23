//
//  GPoint.cpp
//  NetworkTest
//
//  Created by Julian Adenauer on 22.10.14.
//
//

#include "GPoint.h"


GPoint::GPoint(){
    timestamp = ofGetElapsedTimeMillis();
    point_id = instance_count;
    instance_count++;
}

GPoint::GPoint(int x, int y, int point_id, int color_r, int color_g, int color_b, int type, int stroke_id){
    loc = ofVec2f(x, y);
    this->point_id = point_id;
    this->timestamp = ofGetElapsedTimeMillis();
    this->color = ofColor(color_r, color_g, color_b);
    this->type = type;
    this->stroke_id = stroke_id;
}

void GPoint::serialize(string *s){
    s->clear();
    
    s->append(ofToString(loc.x) + "," +
              ofToString(loc.y) + "," +
              ofToString(point_id) + "," +
              ofToString(color.r) + "," +
              ofToString(color.g) + "," +
              ofToString(color.b) + "," +
              ofToString(type) + "," +
              ofToString(stroke_id)
    );
}

void GPoint::unserialize(string s){
    vector<string> list;
    
    for(int i = 0; i<8; i++){
        int pos = s.find(',');
        list.push_back(s.substr(0, pos));
        s = s.substr(pos+1);
    }
    loc = ofVec2f(ofToInt(list[0]), ofToInt(list[1]));
    this->point_id = ofToInt(list[2]);
    this->color = ofColor(ofToInt(list[3]), ofToInt(list[4]), ofToInt(list[5]));
    this->type = ofToInt(list[6]);
    this->stroke_id = ofToInt(list[7]);
}

int GPoint::instance_count;
