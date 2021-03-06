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
    finger_id = instance_count;
    instance_count++;
    color = ofColor(255,255,255,255);
}

GPoint::GPoint(int x, int y, int finger_id, int color_r, int color_g, int color_b, int color_a, int type, int stroke_id){
    loc = ofVec2f(x, y);
    this->finger_id = finger_id;
    this->timestamp = ofGetElapsedTimeMillis();
    this->color = ofColor(color_r, color_g, color_b, color_a);
    this->type = type;
    this->stroke_id = stroke_id;
}

void GPoint::serialize(string *s){
    s->clear();
    
    s->append(ofToString(loc.x, 4) + "," +
              ofToString(loc.y, 4) + "," +
              ofToString((int) finger_id) + "," +
              ofToString((int) color.r) + "," +
              ofToString((int) color.g) + "," +
              ofToString((int) color.b) + "," +
              ofToString((int) type) + "," +
              ofToString((int) style_id) + "," +
              ofToString((int) stroke_id)
              );
    
}

bool GPoint::unserialize(string s){
    vector<string> list;
    
//    RegularExpression regEx("(-?[0-9]{1,4},){2}([0-9]{1,3},){6}([0-9]{1,10})"); // accept negative values on the first two arguments (position) and long numbers on the last (strokeid)
    RegularExpression regEx("(-?[0-1].[0-9]{0,4},){2}([0-9]{1,3},){6}([0-9]{1,10})"); // NEW: Deal with float values for x&y
    RegularExpression::Match match;
    int found = regEx.match(s, match);
    
    if(match.offset == 0 && match.length == s.length()){
        for(int i = 0; i<9; i++){
            int pos = s.find(',');
            list.push_back(s.substr(0, pos));
            s = s.substr(pos+1);
        }
        loc = ofVec2f(ofToFloat(list[0]), ofToFloat(list[1]));
        this->finger_id = ofToInt(list[2]);
        this->color = ofColor(ofToInt(list[3]), ofToInt(list[4]), ofToInt(list[5]));
        this->type = ofToInt(list[6]);
        this->style_id = ofToInt(list[7]);
        this->stroke_id = ofToInt(list[8]);
        return true;
    }
    else {
        return false;
    }
}

int GPoint::instance_count;
