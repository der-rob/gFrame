//
//  GPoint.h
//  NetworkTest
//
//  Created by Julian Adenauer on 22.10.14.
//
//

#ifndef __NetworkTest__GPoint__
#define __NetworkTest__GPoint__

#include <iostream>
#include "ofMain.h"

class GPoint {
    
public:
    GPoint();
    GPoint(int x, int y, int point_id, int color_r, int color_g, int color_b, int type, int stroke_id);
    void serialize(string* s);
    void unserialize(string s);
    
    // setters
    void setLocation(ofVec2f loc){this->loc = loc;}
    void setColor(ofColor c){ color = c; }
    void setType(int type){this->type = type;}
    void setStrokeId(int stroke_id){this->stroke_id = stroke_id;}
    
    
    // getters
    ofVec2f getLocation(){return loc;}
    int getId(){return point_id;}
    unsigned long getTimestamp(){return timestamp;}
    ofColor* getColor(){return &color;}
    int getType(){return type;}
    int getStrokeId(){return stroke_id;}
    
private:
    ofVec2f loc;
    int point_id;
    unsigned long timestamp;
    ofColor color;
    int type;
    int stroke_id;
    
    static int instance_count;

    
};

#endif /* defined(__NetworkTest__GPoint__) */