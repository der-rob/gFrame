//
//  ImageBrush.cpp
//  gFrame
//
//  Created by Robert Albert on 24.08.15.
//
//

#include "ImageBrush.h"

ImageBrush::ImageBrush() {
    parameters.setName("ImageBrush parameters");
    parameters.add(min_brush_width.set("min width",15,1,100));
    parameters.add(max_brush_width.set("max width",50,10,100));
    
    // the textures color must be white, otherwise it will not work correctly with ofSetColor in the draw step
    brush_image.loadImage("test_brush_inverted.png");
    brush_image.setAnchorPercent(0.5f,0.5f);
    
    fbo.allocate(ofGetWidth(), ofGetHeight(),4);
    fbo.begin(); ofClear(0); fbo.end();
    
    max_distance = 10;
    brush_width_start = 0;
}

void ImageBrush::render(vector<GPoint> &_points, int _width, int _height)
{
    if (_points.size() <2)
    {
        width_start.insert(pair<int,int>(_points[0].getStrokeId() ,0));
        return;
    }
    else if (_points.size() > 2)
    {
        while (_points.size() > 2) {
            _points.erase(_points.begin());
        }
    }
    
    if (width_start.find(_points[0].getStrokeId()) == width_start.end())
    {
        width_start.insert(pair<int,int>(_points[0].getStrokeId() ,0));
    }
    
    // since the fbo is not cleared after every frame, we only need to draw the last added point and its predecessor
    int i = _points.size()-1;
    vector<ofVec2f> points_to_draw;
    
    //get directtion
    ofVec2f this_point = ofVec2f(_points[i].getLocation().x*_width,_points[i].getLocation().y*_height);
    ofVec2f last_point = ofVec2f(_points[i-1].getLocation().x*_width,_points[i-1].getLocation().y*_height);
    
    ofVec2f dir = this_point-last_point;
    
    //fill points to draw array
    if (dir.length() > max_distance) {
        ofVec2f unit_dir = dir.normalized();
        int steps = dir.length() / max_distance;
        float step = dir.length() / steps;
        
        for (float j = 0; j <= dir.length(); j+= step)
        {
            ofVec2f new_point = last_point + j * unit_dir;
            points_to_draw.push_back(new_point);
        }
    } else {
        points_to_draw.push_back(this_point);
    }
    
    float angle = ofVec2f(0,1.0f).angle(dir)+180.0;
    
    fbo.begin();
    ofEnableAlphaBlending();
    
    brush_width_end = ofMap(dir.length(), 0,100, max_brush_width*_points[i].getSizeScale(), min_brush_width, true);
    try {
        brush_width_start = width_start.at(_points[0].getStrokeId());
    }
    catch (exception e) {
        cout << ofGetElapsedTimef() << " SID: " << _points[0].getStrokeId() << " i: " << i << endl;
    }
    
    for (int k=0;k<points_to_draw.size();k++)
    {
        ofPushMatrix();
        int brush_width = ofMap(k,0,points_to_draw.size(), brush_width_start,brush_width_end,true);
        //        int brush_width = ofMap(k,0,points_to_draw.size(), 10,70,true);
        ofTranslate(points_to_draw[k]);
        glRotated(angle, 0.0, 0.0, 1.0);
        ofSetColor(255,200,0);
        ofSetColor(_points[0].getColor());
        brush_image.draw(0,0, brush_width, brush_width);
        //        ofCircle(0,0,brush_width/2);
        ofPopMatrix();
    }
    points_to_draw.clear();
    
    fbo.end();
    
    brush_width_start = brush_width_end;
    try {
    width_start.at(_points[0].getStrokeId()) = brush_width_end;
    }
    catch (exception e) {
        cout << "2 " << e.what() << endl;
    }
    _points.clear();
  
    
    // the fbo is always affected by the current draw color
    ofSetColor(255);
    fbo.draw(0,0);
    
//    non fbo version
//    for (int i =1; i<_points.size();i++)
//    {
//        //get directtion
//        ofVec2f this_point = ofVec2f(_points[i].getLocation().x*_width,_points[i].getLocation().y*_height);
//        ofVec2f last_point = ofVec2f(_points[i-1].getLocation().x*_width,_points[i-1].getLocation().y*_height);
//        
//        ofVec2f dir = this_point-last_point;
//        
//        float angle = ofVec2f(0,1.0f).angle(dir)+180.0;
//        
//        int brush_width = ofMap(dir.length(), 0,100, max_brush_width, min_brush_width,true);
//        
//        ofPushMatrix();
//        ofTranslate(this_point);
//        glRotated(angle, 0.0, 0.0, 1.0);
//        brush_image.draw(0,0, brush_width, brush_width);
//        ofPopMatrix();
//    }
}

void ImageBrush::clear() {
    fbo.begin(); ofClear(0); fbo.end();
}