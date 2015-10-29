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
#include "Poco/RegularExpression.h"

#define STYLE_FINGER 0
#define STYLE_SCRIZZLE 1
#define STYLE_CALIGRAPHY 2
#define STYLE_WAVER 3
#define STYLE_IMAGE 4
#define STYLE_STENCIL 5

using Poco::RegularExpression;

class GPoint {
    
public:
    GPoint();
    GPoint(int x, int y, int point_id, int color_r, int color_g, int color_b, int type, int stroke_id);
    void serialize(string* s);
    bool unserialize(string s);
    
    // setters
    void setLocation(ofVec2f loc){this->loc = loc;}
    void setId(int id){this->finger_id = id;}
    void setColor(ofColor c){ color = c; }
    void setType(int type){this->type = type;}
    void setStrokeId(int stroke_id){this->stroke_id = stroke_id;}
    void setStyle(int style){ this->style_id = style; }
    void setPointGroup (int _group) {this->point_group = _group;}
    void setSizeScale(float _scale) {this->size_scale = _scale;}
    
    // getters
    ofVec2f getLocation(){return loc;}
    int getId(){return finger_id;}
    unsigned long getTimestamp(){return timestamp;}
    ofColor getColor(){return color;}
    int getType(){return type;}
    long getStrokeId(){return stroke_id;}
    int getStyle(){return style_id;}
    int getPointGroup() {return this->point_group;}
    float getSizeScale() {return size_scale;}
    
private:
    ofVec2f loc;
    int finger_id;
    unsigned long timestamp;
    ofColor color;
    int type;
    long stroke_id;
    int style_id;
    int point_group;
    float size_scale;
    
    static int instance_count;
};

#endif /* defined(__NetworkTest__GPoint__) */
