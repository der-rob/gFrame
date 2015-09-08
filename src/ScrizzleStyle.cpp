//
//  ScrizzleStyle.cpp
//  brushtest_02
//
//  Created by Robert Albert on 24.10.14.
//
//

#include "ScrizzleStyle.h"

ScrizzleStyle::ScrizzleStyle() {
    parameters.setName("wild parameters");
    
    parameters.add(new_point_distance.set("new point distance",20,10,100));
    parameters.add(amplitude.set("amplitude",8.0,0.0,20));
    parameters.add(wavelength.set("wavelength", 4.0, 1.0, 10.0));
    parameters.add(fadeouttime.set("fadeout time",10.0,2.0,60.0));
    parameters.add(fadeduration.set("fade duration", 5.0, 0.0, 60));
    parameters.add(nervosity.set("nervousity",1.0,0.5,20.0));
    parameters.add(mainLineThickness.set("main line thickness", 4.0, 1.0, 10.0));
    parameters.add(byLineThicknes.set("by line thickness", 0.5, 0.1, 5.0));
}

void ScrizzleStyle::render(vector<GPoint> &points) {
    //amplitude
    amplitude1 = 0.5 * amplitude;
    amplitude2 = amplitude;
    amplitude3 = 1.5 * amplitude;
    
    //line thickness
    linewidth1 = mainLineThickness;
    linewidth2 = byLineThicknes * mainLineThickness;
    linewidth3 = byLineThicknes * mainLineThickness;
    
    //length
    length1 = 0.8 * wavelength;
    length2 = wavelength;
    length3 = 1.2 * wavelength;
    
    //speed
    speed1 = nervosity; speed2 = 0.6 * nervosity; speed3 = 0.4 * nervosity;
    
    //fadeouttime
    endFadeAge = fadeouttime*1000;
    startFadeAge = (fadeouttime - fadeduration)*1000;
    
    glDisable(GL_DEPTH_TEST);
    GPoint *last_used_point;
    last_used_point = &points[0];
    setColor(points[0].getColor());
    
    interpolator.clear();
    glLineWidth(linewidth1);
    ofSetColor(color1);
    glBegin(GL_LINE_STRIP);
    for (int i = 1 ; i < points.size(); i++) {
        //check distance to last used point
        if(last_used_point->getLocation().distance(points[i].getLocation()) < getNewPointDistance() && i!=1)
            continue;
        else last_used_point = &points[i];
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()) - points[i].getTimestamp();
        double alpha;
        //check if point is too old
        if (age > endFadeAge) {
            points.erase(points.begin()+i);
            continue;
        }
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
        float width = sin(length1*(counter*speed1 + points[i].getTimestamp()/10.0)) * amplitude1;
        ofVec2f newPoint = this_point + normal*width;
        ofVec3f newPointWithAlpha = ofVec3f(newPoint.x, newPoint.y, alpha);
        interpolator.push_back(newPointWithAlpha);
    }
    
    float spacing = 1.0/(points.size()*isteps);
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
    glLineWidth(linewidth2);
    ofSetColor(color2);
    glBegin(GL_LINE_STRIP);
    for (int i = 1 ; i < points.size(); i++) {
        //check distance to last used point
        if(last_used_point->getLocation().distance(points[i].getLocation()) < getNewPointDistance() && i!=1)
            continue;
        else last_used_point = &points[i];
        
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()) - points[i].getTimestamp();
        double alpha;
        //check if point is too old
        if (age > endFadeAge) {
            points.erase(points.begin()+i);
            continue;
        }
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
    
    spacing = 1.0/(points.size()*isteps);
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
    glLineWidth(linewidth3);
    ofSetColor(color3);
    glBegin(GL_LINE_STRIP);
    for (int i = 1 ; i < points.size(); i++) {
        //check distance to last used point
        if(last_used_point->getLocation().distance(points[i].getLocation()) < getNewPointDistance() && i!=1)
            continue;
        else last_used_point = &points[i];
        
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()) - points[i].getTimestamp();
        double alpha;
        //check if point is too old
        if (age > endFadeAge) {
            points.erase(points.begin()+i);
            continue;
        }
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
    
    spacing = 1.0/(points.size()*isteps);
    for (float f = 0; f <= 1; f+=spacing) {
        ofVec3f v_with_alpha = interpolator.sampleAt(f);
        ofVec2f v = ofVec2f(v_with_alpha.x, v_with_alpha.y);
        color3.a = v_with_alpha.z;
        ofSetColor(color3);
        glVertex2f(v.x, v.y);
    }
    glEnd();
    glEnable(GL_DEPTH_TEST);
    float last_time = ofGetElapsedTimef();
    counter=last_time;
}

void ScrizzleStyle::render(vector<GPoint> &points, int _width, int _height) {
    //point location is normalizid from 0 to 1
    
    //amplitude
    amplitude1 = 0.5 * amplitude;
    amplitude2 = amplitude;
    amplitude3 = 1.5 * amplitude;
    
    //line thickness
    linewidth1 = mainLineThickness;
    linewidth2 = byLineThicknes * mainLineThickness;
    linewidth3 = byLineThicknes * mainLineThickness;
    
    //length
    length1 = 0.8 * wavelength;
    length2 = wavelength;
    length3 = 1.2 * wavelength;
    
    //speed
    speed1 = nervosity; speed2 = 0.6 * nervosity; speed3 = 0.4 * nervosity;
    
    //fadeouttime
    endFadeAge = fadeouttime*1000;
    startFadeAge = (fadeouttime - fadeduration)*1000;
    
    
    glDisable(GL_DEPTH_TEST);
    GPoint *last_used_point;
    last_used_point = &points[0];
    setColor(points[0].getColor());
    
    interpolator.clear();
    glLineWidth(linewidth1);
    ofSetColor(color1);
    glBegin(GL_LINE_STRIP);
    for (int i = 1 ; i < points.size(); i++) {
        
        //check distance to last used point
        int last_used_x = last_used_point->getLocation().x*_width;
        int last_used_y = last_used_point->getLocation().y*_height;
        int cur_x = points[i].getLocation().x * _width;
        int cur_y = points[i].getLocation().y * _height;
        
        if ((ofVec2f(last_used_x,last_used_y).distance(ofVec2f(cur_x,cur_y))) < getNewPointDistance() && i!=1)
            continue;
        else last_used_point = &points[i];
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()) - points[i].getTimestamp();
        double alpha;
        //check if point is too old
        if (age > endFadeAge) {
            points.erase(points.begin()+i);
            continue;
        }
        if (age < startFadeAge)
            alpha = 255;
        else
            alpha = 255 - ofMap(age,startFadeAge,endFadeAge,0,255);
        //get directtion
        ofVec2f this_point = ofVec2f(points[i].getLocation().x*_width,points[i].getLocation().y*_height);
        ofVec2f last_point = ofVec2f(points[i-1].getLocation().x*_width,points[i-1].getLocation().y*_height);
        ofVec2f dir = (this_point-last_point);
        ofVec2f unitDir = dir.normalize();
        //get normal
        ofVec2f normal = unitDir.getRotated(-90);
        //get width
        float width = sin(length1*(counter*speed1 + points[i].getTimestamp()/10.0)) * amplitude1;
        ofVec2f newPoint = this_point + normal*width;
        ofVec3f newPointWithAlpha = ofVec3f(newPoint.x, newPoint.y, alpha);
        interpolator.push_back(newPointWithAlpha);
    }
    
    float spacing = 1.0/(points.size()*isteps);
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
    for (int i = 1 ; i < points.size(); i++) {
        //check distance to last used point
        //check distance to last used point
        int last_used_x = last_used_point->getLocation().x*_width;
        int last_used_y = last_used_point->getLocation().y*_height;
        int cur_x = points[i].getLocation().x * _width;
        int cur_y = points[i].getLocation().y * _height;

        if ((ofVec2f(last_used_x,last_used_y).distance(ofVec2f(cur_x,cur_y))) < getNewPointDistance() && i!=1)
            continue;
        else last_used_point = &points[i];
 
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()) - points[i].getTimestamp();
        double alpha;
        //check if point is too old
        if (age > endFadeAge) {
            points.erase(points.begin()+i);
            continue;
        }
        if (age < startFadeAge)
            alpha = 255;
        else
            alpha = 255 - ofMap(age,startFadeAge,endFadeAge,0,255);
        
        //get directtion
        ofVec2f this_point = ofVec2f(points[i].getLocation().x*_width,points[i].getLocation().y*_height);
        ofVec2f last_point = ofVec2f(points[i-1].getLocation().x*_width,points[i-1].getLocation().y*_height);
        ///
        ofSetColor(0,255,0,150);
        ofCircle(last_point, 20);
        ofSetColor(255,0,0);
        ofCircle(this_point, 10);
        ///
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
    
    glLineWidth(linewidth2);
    ofSetColor(color2);
    glBegin(GL_LINE_STRIP);
    spacing = 1.0/(points.size()*isteps);
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
    glLineWidth(linewidth3);
    ofSetColor(color3);
    glBegin(GL_LINE_STRIP);
    for (int i = 1 ; i < points.size(); i++) {

        //check distance to last used point
        int last_used_x = last_used_point->getLocation().x*_width;
        int last_used_y = last_used_point->getLocation().y*_height;
        int cur_x = points[i].getLocation().x * _width;
        int cur_y = points[i].getLocation().y * _height;
        
        if ((ofVec2f(last_used_x,last_used_y).distance(ofVec2f(cur_x,cur_y))) < getNewPointDistance() && i!=1)
            continue;
        else last_used_point = &points[i];
        
        //calculate color to fade out when exeeding age
        double age = (ofGetElapsedTimeMillis()) - points[i].getTimestamp();
        double alpha;
        //check if point is too old
        if (age > endFadeAge) {
            points.erase(points.begin()+i);
            continue;
        }
        if (age < startFadeAge)
            alpha = 255;
        else
            alpha = 255 - ofMap(age,startFadeAge,endFadeAge,0,255);
        
        //get directtion
        ofVec2f this_point = ofVec2f(points[i].getLocation().x*_width,points[i].getLocation().y*_height);
        ofVec2f last_point = ofVec2f(points[i-1].getLocation().x*_width,points[i-1].getLocation().y*_height);
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
    
    spacing = 1.0/(points.size()*isteps);
    for (float f = 0; f <= 1; f+=spacing) {
        ofVec3f v_with_alpha = interpolator.sampleAt(f);
        ofVec2f v = ofVec2f(v_with_alpha.x, v_with_alpha.y);
        color3.a = v_with_alpha.z;
        ofSetColor(color3);
        glVertex2f(v.x, v.y);
    }
    glEnd();
    
    glEnable(GL_DEPTH_TEST);
    float last_time = ofGetElapsedTimef();
    counter=last_time;
}

void ScrizzleStyle::setColor(ofColor _color)
{
    float hue = _color.getHue();
    float saturation = _color.getSaturation();
    float brightness = _color.getBrightness();
    color1.setHsb(hue, saturation, brightness);
    color2.setHsb(hue + 15.0, saturation - 10.0, brightness - 20.0);
    color3.setHsb(hue - 15.0, saturation + 10.0, brightness - 40.0);
}