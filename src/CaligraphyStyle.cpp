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
    parameters.add(fadeduration.set("fade duration", 0.04, 0.0, 2.0));
    parameters.add(tip_angle.set("tip angle",60.0,0.0,180.0));
    parameters.add(zSpeed.set("z-speed",0.0,0.0,1.0));
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
            if(ofGetElapsedTimeMillis() - points[points.size()-1].getTimestamp() > endFadeAge){
                points.clear();
                continue;
            }
            
            float z = ((float) points[0].getTimestamp() - ofGetElapsedTimeMillis())/(100.0/zSpeed);
            
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
            
            //cout << dx << ":" << dy << endl;
            ofPoint v1, v2, v1h, v2h;
            
//            v1 = ofPoint(-dy, dx);
//            v1.scale(ofMap(v1.length(), 0, 100, width_max, width_min, true));
//            v1 += currentPoint;
//            
//            v2 = ofPoint(dy, -dx);
//            v2.scale(ofMap(v2.length(), 0, 100, width_max, width_min, true));
//            v2+= currentPoint;
            
            v1h = ofPoint(-dy, dx); //need these for calculation of the brushwidth according to the point distance
            v2h = ofPoint(dy, -dx);
            
            
            float start_scale = ofMap(i, 3, 1, 1,10, true);
            float end_scale = ofMap(i,points.size(), points.size()-3, 10,1, true);
            float tempx = cos(tip_angle*PI/180);//*start_scale*end_scale;//*distance;
            float tempy = sin(tip_angle*PI/180);//*start_scale*end_scale;//*distance;
            
            
            v1 =  ofPoint(-tempx,tempy,z);
            v1.scale(ofMap(v1h.length(), 0, 100, width_max, width_min, true)*1/(start_scale*end_scale));
            v1 += currentPoint;
            
            v2 =  ofPoint(tempx,-tempy,z);
            v2.scale(ofMap(v2h.length(), 0, 100, width_max, width_min, true)*1/(start_scale*end_scale));
            v2 += currentPoint;
            
            interpolator1.push_back(v1);
            interpolator2.push_back(v2);
        }
        
        float step = 1.0/(points.size()*5.0);
        for(float f=0; f<1; f+=step) {
            ofColor c = points[(int) ((f/step)/5.0)].getColor();
            //fading out from the end
            c.a = c.a - ofMap(ofGetElapsedTimeMillis() - points[(int) ((f/step)/5.0)].getTimestamp(), startFadeAge, endFadeAge, 0, c.a, true);
//            c.r = c.r - ofMap(ofGetElapsedTimeMillis() - points[points.size()-1].getTimestamp(), startFadeAge, endFadeAge, 255, c.r, true);
//            c.g = c.g - ofMap(ofGetElapsedTimeMillis() - points[points.size()-1].getTimestamp(), startFadeAge, endFadeAge, 255, c.g, true);
//            c.b = c.b - ofMap(ofGetElapsedTimeMillis() - points[points.size()-1].getTimestamp(), startFadeAge, endFadeAge, 255, c.b, true);
//            c.r = c.a - ofMap(ofGetElapsedTimeMillis() - points[points.size()-1].getTimestamp(), startFadeAge, endFadeAge, 255, c.a, true);
//            
            //cout << (int)c.r << ":" << (int)c.g << ":"  << (int)c.b << ":" << (int)c.a << endl;
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