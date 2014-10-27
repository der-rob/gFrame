//
//  ProfileStyle.cpp
//  StyleTest
//
//  Created by Julian Adenauer on 14.10.14.
//
//

#include "ProfileStyle.h"

ProfileStyle::ProfileStyle(){
    fill = true;
    
    // initialize the all_points vector with an empty vector
    vector<GPoint> p;
    points.push_back(p);
    
    stroke_id = 0;
    
}

void ProfileStyle::update(){
    
//    // remove points that are too old
//    for(int all = 0; all<points.size(); all++){
//        bool found = true;
//        if(points[all].size() > 0){
//            while(found){
////                ofLog() << ofGetElapsedTimeMillis() - points[all].back().lifetime;
//                if(ofGetElapsedTimeMillis() - points[all].back().lifetime > 5000){
//                    points[all].pop_back();
////                    ofLog() << "deleted";
//                }
//                else found = false;
//            }
//        }
//    }
//    int size = points.back().size();
//    while(size == 0){
//        points.pop_back();
//        size = points.back().size();
//    }
//    
//    if(points.size() == 0){
//    vector<gPoint> p;
//    points.push_back(p);
//    }
}

void ProfileStyle::draw(){
    int depth = 30;
    int width = 30;
    
    for(int all = 0; all<points.size(); all++){
        ofMesh mesh;
        float zIndex = 0;
        int angle = 0;
        
        if(points[all].size() > 2){
            mesh.setMode(OF_PRIMITIVE_TRIANGLES);
            
            zIndex = (float) points[all][0].getTimestamp() - (ofGetElapsedTimeMillis() / 10);
            
            // add the first points
            ofVec3f last_1 = ofVec3f(points[all][0].getLocation().x, points[all][0].getLocation().y - 5, zIndex/3);
            ofVec3f last_2 = ofVec3f(points[all][0].getLocation().x, points[all][0].getLocation().y + 5, zIndex/3);
            ofVec3f last_3 = ofVec3f(points[all][0].getLocation().x, points[all][0].getLocation().y + 5, zIndex/3 +depth);
            ofVec3f last_4 = ofVec3f(points[all][0].getLocation().x, points[all][0].getLocation().y - 5, zIndex/3 +depth);
            
            
            // 1) GENERATE 3D POINTS OUT OF 2D LINES
            for(int cur=1; cur < points[all].size(); cur++){
                zIndex = (float) points[all][cur].getTimestamp() - (ofGetElapsedTimeMillis() / 10);
                
                ofVec2f lastPoint = points[all][cur-1].getLocation();
                ofVec2f currentPoint = points[all][cur].getLocation();
                float dx = lastPoint.x - currentPoint.x;
                float dy = lastPoint.y - currentPoint.y;
                
                ofVec2f v1, v2;
                v1 = ofVec2f(-dy, dx);
                v1.scale(width);
                v2 = ofVec2f(dy, -dx);
                v2.scale(width);
                
                // calculate normals
                ofVec3f middlePoint = points[all][cur].getLocation();
                middlePoint.z = depth/2 + zIndex/3;
                
                ofVec3f lastMiddlePoint = points[all][cur-1].getLocation();
                lastMiddlePoint.z = depth/2 + (zIndex+1)/3;
                
                ofVec3f point_1 = ofVec3f(v1 + currentPoint); point_1.z = zIndex/3;
                ofVec3f point_2 = ofVec3f(v2 + currentPoint); point_2.z = zIndex/3;
                ofVec3f point_3 = ofVec3f(v2 + currentPoint); point_3.z = depth + zIndex/3;
                ofVec3f point_4 = ofVec3f(v1 + currentPoint); point_4.z = depth + zIndex/3;
                
                point_1.rotate(angle, middlePoint, (middlePoint - lastMiddlePoint));
                point_2.rotate(angle, middlePoint, (middlePoint - lastMiddlePoint));
                point_3.rotate(angle, middlePoint, (middlePoint - lastMiddlePoint));
                point_4.rotate(angle, middlePoint, (middlePoint - lastMiddlePoint));
                
                // calculate normals
                ofVec3f normalFront = ((((point_2 - point_1) / 2) + point_1) - middlePoint).normalize();
                ofVec3f normalTop = ((((point_4 - point_1) / 2) + point_1) - middlePoint).normalize();
                ofVec3f normalBack = ((((point_3 - point_4) / 2) + point_4) - middlePoint).normalize();
                ofVec3f normalBottom = ((((point_2 - point_3) / 2) + point_3) - middlePoint).normalize();
                
                // front
                mesh.addVertex(last_1);
                mesh.addNormal(normalFront);
                mesh.addVertex(last_2);
                mesh.addNormal(normalFront);
                mesh.addVertex(point_1);
                mesh.addNormal(normalFront);
                
                mesh.addVertex(point_1);
                mesh.addNormal(normalFront);
                mesh.addVertex(point_2);
                mesh.addNormal(normalFront);
                mesh.addVertex(last_2);
                mesh.addNormal(normalFront);
                
                // top
                mesh.addVertex(last_1);
                mesh.addNormal(normalTop);
                mesh.addVertex(point_4);
                mesh.addNormal(normalTop);
                mesh.addVertex(point_1);
                mesh.addNormal(normalTop);
                
                mesh.addVertex(last_1);
                mesh.addNormal(normalTop);
                mesh.addVertex(last_4);
                mesh.addNormal(normalTop);
                mesh.addVertex(point_4);
                mesh.addNormal(normalTop);
                
                // back
                mesh.addVertex(last_4);
                mesh.addNormal(normalBack);
                mesh.addVertex(last_3);
                mesh.addNormal(normalBack);
                mesh.addVertex(point_4);
                mesh.addNormal(normalBack);
                
                mesh.addVertex(last_3);
                mesh.addNormal(normalBack);
                mesh.addVertex(point_3);
                mesh.addNormal(normalBack);
                mesh.addVertex(point_4);
                mesh.addNormal(normalBack);
                
                // bottom
                mesh.addVertex(last_2);
                mesh.addNormal(normalBottom);
                mesh.addVertex(last_3);
                mesh.addNormal(normalBottom);
                mesh.addVertex(point_3);
                mesh.addNormal(normalBottom);
                
                mesh.addVertex(last_2);
                mesh.addNormal(normalBottom);
                mesh.addVertex(point_2);
                mesh.addNormal(normalBottom);
                mesh.addVertex(point_3);
                mesh.addNormal(normalBottom);
                
                
                last_1 = point_1;
                last_2 = point_2;
                last_3 = point_3;
                last_4 = point_4;
                
                zIndex++;
                angle+=5;
                if(angle > 359) angle = 0;
            }

            ofSetColor(255,15,150);

            if(fill){
                mesh.draw();
            }
            else {
                mesh.drawWireframe();
            }
        }
    }
}

void ProfileStyle::render(vector<GPoint>& points){
    int depth = 30;
    int width = 30;
    
    GPoint* last_used_point;
    
    list<int> ids;
    
    if(points.size() > 2){
        ofMesh mesh;
        float zIndex = 0;
        int angle = 0;
        
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        zIndex = (float) points[0].getTimestamp() - (ofGetElapsedTimeMillis() / 10);
        
        // add the first points
        ofVec3f last_1 = ofVec3f(points[0].getLocation().x, points[0].getLocation().y - 5, zIndex/3);
        ofVec3f last_2 = ofVec3f(points[0].getLocation().x, points[0].getLocation().y + 5, zIndex/3);
        ofVec3f last_3 = ofVec3f(points[0].getLocation().x, points[0].getLocation().y + 5, zIndex/3 +depth);
        ofVec3f last_4 = ofVec3f(points[0].getLocation().x, points[0].getLocation().y - 5, zIndex/3 +depth);
        
        last_used_point = &points[0];

        for(int all = 1; all<points.size(); all++){
            
            // only use points that are far enough apart, but always use the first two (otherwise this migh break something)
            if(last_used_point->getLocation().distance(points[all].getLocation()) < 20 && all!=1) continue;
            else last_used_point = &points[all];
            
            // 1) GENERATE 3D POINTS OUT OF 2D LINES
            zIndex = (float) points[all].getTimestamp() - (ofGetElapsedTimeMillis() / 10);
            
            ofVec2f lastPoint = points[all-1].getLocation();
            ofVec2f currentPoint = points[all].getLocation();
            float dx = lastPoint.x - currentPoint.x;
            float dy = lastPoint.y - currentPoint.y;
            
            ofVec2f v1, v2;
            v1 = ofVec2f(-dy, dx);
            v1.scale(width);
            v2 = ofVec2f(dy, -dx);
            v2.scale(width);
            
            // calculate normals
            ofVec3f middlePoint = currentPoint;
            middlePoint.z = depth/2 + zIndex/3;
            
            ofVec3f lastMiddlePoint = lastPoint;
            lastMiddlePoint.z = depth/2 + (zIndex+1)/3;
            
            ofVec3f point_1 = ofVec3f(v1 + currentPoint); point_1.z = zIndex/3;
            ofVec3f point_2 = ofVec3f(v2 + currentPoint); point_2.z = zIndex/3;
            ofVec3f point_3 = ofVec3f(v2 + currentPoint); point_3.z = depth + zIndex/3;
            ofVec3f point_4 = ofVec3f(v1 + currentPoint); point_4.z = depth + zIndex/3;
            
            point_1.rotate(angle, middlePoint, (middlePoint - lastMiddlePoint));
            point_2.rotate(angle, middlePoint, (middlePoint - lastMiddlePoint));
            point_3.rotate(angle, middlePoint, (middlePoint - lastMiddlePoint));
            point_4.rotate(angle, middlePoint, (middlePoint - lastMiddlePoint));
            
            // calculate normals
            ofVec3f normalFront = ((((point_2 - point_1) / 2) + point_1) - middlePoint).normalize();
            ofVec3f normalTop = ((((point_4 - point_1) / 2) + point_1) - middlePoint).normalize();
            ofVec3f normalBack = ((((point_3 - point_4) / 2) + point_4) - middlePoint).normalize();
            ofVec3f normalBottom = ((((point_2 - point_3) / 2) + point_3) - middlePoint).normalize();
            
            // front
            mesh.addVertex(last_1);
            mesh.addNormal(normalFront);
            mesh.addVertex(last_2);
            mesh.addNormal(normalFront);
            mesh.addVertex(point_1);
            mesh.addNormal(normalFront);
            
            mesh.addVertex(point_1);
            mesh.addNormal(normalFront);
            mesh.addVertex(point_2);
            mesh.addNormal(normalFront);
            mesh.addVertex(last_2);
            mesh.addNormal(normalFront);
            
            // top
            mesh.addVertex(last_1);
            mesh.addNormal(normalTop);
            mesh.addVertex(point_4);
            mesh.addNormal(normalTop);
            mesh.addVertex(point_1);
            mesh.addNormal(normalTop);
            
            mesh.addVertex(last_1);
            mesh.addNormal(normalTop);
            mesh.addVertex(last_4);
            mesh.addNormal(normalTop);
            mesh.addVertex(point_4);
            mesh.addNormal(normalTop);
            
            // back
            mesh.addVertex(last_4);
            mesh.addNormal(normalBack);
            mesh.addVertex(last_3);
            mesh.addNormal(normalBack);
            mesh.addVertex(point_4);
            mesh.addNormal(normalBack);
            
            mesh.addVertex(last_3);
            mesh.addNormal(normalBack);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBack);
            mesh.addVertex(point_4);
            mesh.addNormal(normalBack);
            
            // bottom
            mesh.addVertex(last_2);
            mesh.addNormal(normalBottom);
            mesh.addVertex(last_3);
            mesh.addNormal(normalBottom);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBottom);
            
            mesh.addVertex(last_2);
            mesh.addNormal(normalBottom);
            mesh.addVertex(point_2);
            mesh.addNormal(normalBottom);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBottom);
            
            
            last_1 = point_1;
            last_2 = point_2;
            last_3 = point_3;
            last_4 = point_4;
            
            zIndex++;
            angle+=5;
            if(angle > 359) angle = 0;
        }
            
        ofSetColor(255,15,150);
        
        if(fill){
            mesh.draw();
        }
        else {
            mesh.drawWireframe();
        }
    }
}

void ProfileStyle::dragging(int x, int y){
    if(lastPoint.distance(ofVec3f(x, y)) > 20){
        GPoint p;        
        p.setLocation(ofVec2f(x, y));
        p.setStrokeId(stroke_id);
        points.back().push_back(p);
        lastPoint = ofPoint(x, y);
    }
}

void ProfileStyle::released(int x, int y){
    if(points.back().size() > 0){
        vector<GPoint> p;
        points.push_back(p);
        stroke_id++;
    }
}

void ProfileStyle::addPoint(GPoint p){
    // if the stroke id is different from the current one, start a new list
    if(stroke_id != p.getStrokeId() && points.back().size() > 0){
        vector<GPoint> pointVector;
        points.push_back(pointVector);
        points.back().push_back(p);
        stroke_id = p.getStrokeId();
    }
    // otherwise just append it
    else{
        points.back().push_back(p);
    }
}

void ProfileStyle::setFill(bool fill){
    this->fill = fill;
}

void ProfileStyle::toggleFill(){
    fill = !fill;
}

void ProfileStyle::clear(){
    points.clear();
    vector<GPoint> p;
    points.push_back(p);
}
