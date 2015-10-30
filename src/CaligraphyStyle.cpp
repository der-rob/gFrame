//
//  CaligraphyStyle.cpp
//  StyleTest
//
//  Created by Julian Adenauer on 14.10.14.
//
//

#include "CaligraphyStyle.h"


CaligraphyStyle::CaligraphyStyle(){
    fill = true;
    parameters.setName("caligraphy parameters");
    parameters.add(new_point_distance.set("new point distance",5,1,50));
    parameters.add(width_min.set("width min", 1, 0, 20));
    parameters.add(width_max.set("width max", 20, 1, 60));
    parameters.add(fadeouttime.set("fadeout time",10.0,2.0,60.0));
    parameters.add(fadeduration.set("fade duration", 5.0, 0.0, 60));
}

void CaligraphyStyle::render(vector<GPoint> &points, int width, int height){
    if(points.size() > 1){
        
        endFadeAge = fadeouttime*1000;
        startFadeAge = fadeouttime*1000 - fadeduration*1000;
        
        interpolator1.clear(); interpolator2.clear();
        
        ofMesh m;
        m.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for(int i = 1; i < points.size(); i++){
            // sort out the points that are faded out
            if(ofGetElapsedTimeMillis() - points[i].getTimestamp() > endFadeAge){
                points.erase(points.begin()+i);
                continue;
            }
            
            ofPoint currentPoint = points[i].getLocation();
            currentPoint.x *= width;
            currentPoint.y *= height;
            
            ofPoint lastPoint = points[i-1].getLocation();;
            lastPoint.x *= width;
            lastPoint.y *= height;
            
            if (currentPoint.distance(lastPoint) < getNewPointDistance())
                continue;
            
            float dx = lastPoint.x - currentPoint.x;
            float dy = lastPoint.y - currentPoint.y;
            
            ofPoint v1, v2;
            
//            v1 = ofPoint(-dy, dx);
//            v1.scale(ofMap(v1.length(), 0, 100, width_max, width_min, true));
//            v1 += currentPoint;
//            
//            v2 = ofPoint(dy, -dx);
//            v2.scale(ofMap(v2.length(), 0, 100, width_max, width_min, true));
//            v2+= currentPoint;

            v1 =  ofPoint(-10,10);
            v1.scale(ofMap(v1.length(), 0, 100, width_max, width_min, true));
            v1 += currentPoint;
            
            v2 =  ofPoint(10,-10);
            v2.scale(ofMap(v2.length(), 0, 100, width_max, width_min, true));
            v2 += currentPoint;
            
            interpolator1.push_back(v1);
            interpolator2.push_back(v2);
        }
        
        float step = 1.0/(points.size()*5.0);
        for(float f=0; f<1; f+=step){
            ofColor c = points[(int) ((f/step)/5.0)].getColor();
            c.a = 255 - ofMap(ofGetElapsedTimeMillis() - points[(int) ((f/step)/5.0)].getTimestamp(), startFadeAge, endFadeAge, 0, 255, true);
            m.addVertex(interpolator1.sampleAt(f));
            m.addColor(c);
            m.addVertex(interpolator2.sampleAt(f));
            m.addColor(c);
        }
        
        m.draw();
    }
}

void CaligraphyStyle::setFill(bool fill){
    this->fill = fill;
}

void CaligraphyStyle::toggleFill(){
    fill = !fill;
}