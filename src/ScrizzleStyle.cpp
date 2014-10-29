//
//  ScrizzleStyle.cpp
//  brushtest_02
//
//  Created by Robert Albert on 24.10.14.
//
//

#include "ScrizzleStyle.h"

ScrizzleStyle::ScrizzleStyle() {
}

void ScrizzleStyle::render(vector<GPoint> &points) {

    GPoint *last_used_point;
    last_used_point = &points[0];
    
    interpolator.clear();
    glLineWidth(3.0);
    ofSetColor(color1);
    glBegin(GL_LINE_STRIP);
    for (int i = 1 ; i < points.size(); i++) {
        //check distance to last used point
        if(last_used_point->getLocation().distance(points[i].getLocation()) < new_point_distance && i!=1)
            continue;
        else last_used_point = &points[i];
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()/10) - points[i].getTimestamp();
        double alpha;
        if (age < startFadeAge)
            alpha = 255;
        else
            alpha = 255 - ofMap(age,startFadeAge,endFadeAge,0,255);
        //get directtion
        ofVec2f this_point = points[i].getLocation();
        ofVec2f last_point = points[i-1].getLocation();
        ofVec2f dir = (this_point-last_point);
        ofVec2f unitDir = dir.normalize();
        //get normal
        ofVec2f normal = unitDir.getRotated(-90);
        //get width
        float width = sin(length1*(counter*speed1 + points[i].getTimestamp())) * amplitude1;
        ofVec2f newPoint = this_point + normal*width;
        ofVec3f newPointWithAlpha = ofVec3f(newPoint.x, newPoint.y, alpha);
        interpolator.push_back(newPointWithAlpha);
    }
    
    
    float spacing = 1.0/(points.size()*isteps1);
    for (float f = 0; f <= 1; f+=spacing) {
        ofVec3f v_with_alpha = interpolator.sampleAt(f);
        ofVec2f v = ofVec2f(v_with_alpha.x, v_with_alpha.y);
        color1.a = v_with_alpha.z;
        ofSetColor(color1);
        glVertex2f(v.x, v.y);
    }
    glEnd();
    
    //second run through all points in this stroke
    interpolator.clear();
    last_used_point = &points[0];
    glLineWidth(2.0);
    ofSetColor(color2);
    glBegin(GL_LINE_STRIP);
    for (int i = 1 ; i < points.size(); i++) {
        //check distance to last used point
        if(last_used_point->getLocation().distance(points[i].getLocation()) < new_point_distance && i!=1)
            continue;
        else last_used_point = &points[i];
        
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()/10) - points[i].getTimestamp();
        double alpha;
        if (age < startFadeAge)
            alpha = 255;
        else
            alpha = 255 - ofMap(age,startFadeAge,endFadeAge,0,255);
        
        //get directtion
        ofVec2f this_point = points[i].getLocation();
        ofVec2f last_point = points[i-1].getLocation();
        ofVec2f dir = (this_point-last_point);
        ofVec2f unitDir = dir.normalize();
        //get normal
        ofVec2f normal = unitDir.getRotated(90);
        //get width
        float width = sin(length2*(counter*speed2 + points[i].getTimestamp())) * amplitude2;
        ofVec2f newPoint = this_point + normal*width;
        ofVec3f newPointWithAlpha = ofVec3f(newPoint.x, newPoint.y, alpha);
        interpolator.push_back(newPointWithAlpha);
    }
    //setcolor after calculating age
    ofSetColor(color2);
    
    spacing = 1.0/(points.size()*isteps2);
    for (float f = 0; f <= 1; f+=spacing) {
        ofVec3f v_with_alpha = interpolator.sampleAt(f);
        ofVec2f v = ofVec2f(v_with_alpha.x, v_with_alpha.y);
        color2.a = v_with_alpha.z;
        ofSetColor(color2);
        glVertex2f(v.x, v.y);
    }
    glEnd();
    
    //third run through all points in this stroke
    last_used_point = &points[0];
    interpolator.clear();
    glLineWidth(1.0);
    ofSetColor(color3);
    glBegin(GL_LINE_STRIP);
    for (int i = 1 ; i < points.size(); i++) {
        //check distance to last used point
        if(last_used_point->getLocation().distance(points[i].getLocation()) < new_point_distance && i!=1)
            continue;
        else last_used_point = &points[i];
        
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()/10) - points[i].getTimestamp();
        double alpha;
        if (age < startFadeAge)
            alpha = 255;
        else
            alpha = 255 - ofMap(age,startFadeAge,endFadeAge,0,255);
        
        //get directtion
        ofVec2f this_point = points[i].getLocation();
        ofVec2f last_point = points[i-1].getLocation();
        ofVec2f dir = (this_point-last_point);
        ofVec2f unitDir = dir.normalize();
        
        //get normal
        ofVec2f normal = unitDir.getRotated(-90);
        
        //draw to both sides
        float width = sin(length3*(counter*speed3 + points[i].getTimestamp())) * amplitude3;
        ofVec2f newPoint = this_point + normal*width;
        ofVec3f newPointWithAlpha = ofVec3f(newPoint.x, newPoint.y, alpha);
        interpolator.push_back(newPointWithAlpha);
    }
    
    //setcolor after calculating age
    ofSetColor(color3);
    
    spacing = 1.0/(points.size()*isteps3);
    for (float f = 0; f <= 1; f+=spacing) {
        ofVec3f v_with_alpha = interpolator.sampleAt(f);
        ofVec2f v = ofVec2f(v_with_alpha.x, v_with_alpha.y);
        color3.a = v_with_alpha.z;
        ofSetColor(color3);
        glVertex2f(v.x, v.y);
    }
    glEnd();
    
    float last_time = ofGetElapsedTimef();
    counter=last_time;
}