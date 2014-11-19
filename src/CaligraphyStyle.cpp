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
}

void CaligraphyStyle::render(vector<GPoint> &points, int width, int height){
    if(points.size() > 1){
        
        interpolator1.clear(); interpolator2.clear();
        
        ofMesh m;
        m.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        
        for(int cur = 1;cur < points.size(); cur++){
            ofPoint currentPoint = points[cur].getLocation();
            currentPoint.x *= width;
            currentPoint.y *= height;
            
            ofPoint lastPoint = points[cur-1].getLocation();;
            lastPoint.x *= width;
            lastPoint.y *= height;
            
            float dx = lastPoint.x - currentPoint.x;
            float dy = lastPoint.y - currentPoint.y;
            
            ofPoint v1, v2;
            
            v1 = ofPoint(-dy, dx);
            v1.scale(ofMap(v1.length(), 0, 200, stroke_width, 2, true));
            v1 += currentPoint;
            
            v2 = ofPoint(dy, -dx);
            v2.scale(ofMap(v2.length(), 0, 200, stroke_width, 2, true));
            v2+= currentPoint;
            
            interpolator1.push_back(v1);
            interpolator2.push_back(v2);
        }
        
        float step = 1.0/(points.size()*10.0);
        for(float f=0; f<1; f+=step){
            m.addVertex(interpolator1.sampleAt(f));
            m.addColor(points[(int) ((f/step)/10.0)].getColor());
            m.addVertex(interpolator2.sampleAt(f));
            m.addColor(points[(int) ((f/step)/10.0)].getColor());
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