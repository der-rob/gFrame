//
//  ProfileStyle.cpp
//  StyleTest
//
//  Created by Julian Adenauer on 14.10.14.
//
//

#include "ProfileStyle.h"

void ProfileStyle::render(vector<GPoint>& points){
    
    ofPushStyle();

    ofEnableLighting();

    // SETUP OPENGL
    ofEnableDepthTest(); // IMPORTANT!!!
    
    GPoint* last_used_point;
    
    if(points.size() > 1){
        ofMesh mesh;
        float zIndex = 0;
        int angle = 0;
        
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        zIndex = ((float) points[0].getTimestamp() - ofGetElapsedTimeMillis())/(100.0/z_speed);
        
        // add the first points
        ofVec3f last_1 = ofVec3f(points[0].getLocation().x, points[0].getLocation().y - 5, zIndex/3);
        ofVec3f last_2 = ofVec3f(points[0].getLocation().x, points[0].getLocation().y + 5, zIndex/3);
        ofVec3f last_3 = ofVec3f(points[0].getLocation().x, points[0].getLocation().y + 5, zIndex/3 +depth);
        ofVec3f last_4 = ofVec3f(points[0].getLocation().x, points[0].getLocation().y - 5, zIndex/3 +depth);
        
        last_used_point = &points[0];

        for(int all = 1; all<points.size(); all++){
            
            // only use points that are far enough apart, but always use the first two (otherwise this migh break something)
            if(last_used_point->getLocation().distance(points[all].getLocation()) < getNewPointDistance() && all!=1) continue;
            else last_used_point = &points[all];
            
            // 1) GENERATE 3D POINTS OUT OF 2D LINES
            zIndex = ((float) points[all].getTimestamp() - ofGetElapsedTimeMillis())/(100.0/z_speed);
            
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
            
            ofFloatColor colorLast = ofFloatColor(
                                                  ofMap(points[all-1].getColor().r, 0, 255, 0.0, 1.0),
                                                  ofMap(points[all-1].getColor().g, 0, 255, 0.0, 1.0),
                                                  ofMap(points[all-1].getColor().b, 0, 255, 0.0, 1.0)
                                                  );
            
            ofFloatColor colorCurrent = ofFloatColor(
                                                     ofMap(points[all].getColor().r, 0, 255, 0.0, 1.0),
                                                     ofMap(points[all].getColor().g, 0, 255, 0.0, 1.0),
                                                     ofMap(points[all].getColor().b, 0, 255, 0.0, 1.0)
                                                     );
            
            ofColor colorCurrent_top = points[all].getColor();
            colorCurrent_top.setBrightness(colorCurrent_top.getBrightness()/2);
            ofColor colorLast_top = points[all-1].getColor();
            colorLast_top.setBrightness(colorLast_top.getBrightness()/2);
            ofFloatColor colorCurrent_topF = ofFloatColor(
                                                          ofMap(colorCurrent_top.r, 0, 255, 0.0, 1.0),
                                                          ofMap(colorCurrent_top.g, 0, 255, 0.0, 1.0),
                                                          ofMap(colorCurrent_top.b, 0, 255, 0.0, 1.0)
                                                        );
            ofFloatColor colorLast_topF = ofFloatColor(
                                                          ofMap(colorLast_top.r, 0, 255, 0.0, 1.0),
                                                          ofMap(colorLast_top.g, 0, 255, 0.0, 1.0),
                                                          ofMap(colorLast_top.b, 0, 255, 0.0, 1.0)
                                                          );
            
            ofColor colorCurrent_bottom = points[all].getColor();
            colorCurrent_bottom.setSaturation(colorCurrent_bottom.getSaturation()/2);
            ofColor colorLast_bottom = points[all-1].getColor();
            colorLast_bottom.setSaturation(colorLast_bottom.getSaturation()/2);
            ofFloatColor colorCurrent_bottomF = ofFloatColor(
                                                          ofMap(colorCurrent_bottom.r, 0, 255, 0.0, 1.0),
                                                          ofMap(colorCurrent_bottom.g, 0, 255, 0.0, 1.0),
                                                          ofMap(colorCurrent_bottom.b, 0, 255, 0.0, 1.0)
                                                          );
            ofFloatColor colorLast_bottomF = ofFloatColor(
                                                       ofMap(colorLast_bottom.r, 0, 255, 0.0, 1.0),
                                                       ofMap(colorLast_bottom.g, 0, 255, 0.0, 1.0),
                                                       ofMap(colorLast_bottom.b, 0, 255, 0.0, 1.0)
                                                       );
            
            // front
            mesh.addVertex(last_1);
            mesh.addNormal(normalFront);
            mesh.addColor(colorLast);
            mesh.addVertex(last_2);
            mesh.addNormal(normalFront);
            mesh.addColor(colorLast);
            mesh.addVertex(point_1);
            mesh.addNormal(normalFront);
            mesh.addColor(colorCurrent);
            
            mesh.addVertex(point_1);
            mesh.addNormal(normalFront);
            mesh.addColor(colorCurrent);
            mesh.addVertex(point_2);
            mesh.addNormal(normalFront);
            mesh.addColor(colorCurrent);
            mesh.addVertex(last_2);
            mesh.addNormal(normalFront);
            mesh.addColor(colorLast);
            
            // top
            mesh.addVertex(last_1);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast_topF);
            mesh.addVertex(point_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorCurrent_topF);
            mesh.addVertex(point_1);
            mesh.addNormal(normalTop);
            mesh.addColor(colorCurrent_top);
            
            mesh.addVertex(last_1);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast_topF);
            mesh.addVertex(last_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast_topF);
            mesh.addVertex(point_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorCurrent_topF);
            
            // back
            mesh.addVertex(last_4);
            mesh.addNormal(normalBack);
            mesh.addColor(colorLast);
            mesh.addVertex(last_3);
            mesh.addNormal(normalBack);
            mesh.addColor(colorLast);
            mesh.addVertex(point_4);
            mesh.addNormal(normalBack);
            mesh.addColor(colorCurrent);
            
            mesh.addVertex(last_3);
            mesh.addNormal(normalBack);
            mesh.addColor(colorLast);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBack);
            mesh.addColor(colorCurrent);
            mesh.addVertex(point_4);
            mesh.addNormal(normalBack);
            mesh.addColor(colorCurrent);
            
            // bottom
            mesh.addVertex(last_2);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorLast_bottomF);
            mesh.addVertex(last_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorLast_bottomF);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent_bottomF);
            
            mesh.addVertex(last_2);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorLast_bottomF);
            mesh.addVertex(point_2);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent_bottomF);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent_bottomF);
            
            
            last_1 = point_1;
            last_2 = point_2;
            last_3 = point_3;
            last_4 = point_4;
            
            zIndex++;
            angle += twist;
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
    ofPopStyle();
}
//------------------------------------
void ProfileStyle::render(vector<GPoint>& points, int _width, int _height){
    
    ofPushStyle();
    
    ofEnableLighting();
    
    // SETUP OPENGL
    ofEnableDepthTest(); // IMPORTANT!!!
    
    GPoint* last_used_point;
    
    if(points.size() > 1){
        ofMesh mesh;
        float zIndex = 0;
        int angle = 0;
        
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        zIndex = ((float) points[0].getTimestamp() - ofGetElapsedTimeMillis())/(100.0/z_speed);
        
        // add the first points
        int px = points[0].getLocation().x * _width;
        int py = points[0].getLocation().y * _height;
        ofVec3f last_1 = ofVec3f(px, py - 5, zIndex/3);
        ofVec3f last_2 = ofVec3f(px, py + 5, zIndex/3);
        ofVec3f last_3 = ofVec3f(px, py + 5, zIndex/3 +depth);
        ofVec3f last_4 = ofVec3f(px, py - 5, zIndex/3 +depth);
        
        last_used_point = &points[0];
        
        for(int all = 1; all<points.size(); all++){
            
            // only use points that are far enough apart, but always use the first two (otherwise this migh break something)
            int last_used_x = last_used_point->getLocation().x*_width;
            int last_used_y = last_used_point->getLocation().y*_height;
            int cur_x = points[all].getLocation().x * _width;
            int cur_y = points[all].getLocation().y * _height;
            
            if ((ofVec2f(last_used_x,last_used_y).distance(ofVec2f(cur_x,cur_y))) < getNewPointDistance() && all!=1)
                continue;
            else last_used_point = &points[all];
            
            // 1) GENERATE 3D POINTS OUT OF 2D LINES
            zIndex = ((float) points[all].getTimestamp() - ofGetElapsedTimeMillis())/(100.0/z_speed);
            
            ofVec2f lastPoint = ofVec2f(points[all-1].getLocation().x * _width, points[all-1].getLocation().y * _height);
            ofVec2f currentPoint = ofVec2f(points[all].getLocation().x * _width, points[all].getLocation().y * _height);
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
            
            ofFloatColor colorLast = ofFloatColor(
                                                  ofMap(points[all-1].getColor().r, 0, 255, 0.0, 1.0),
                                                  ofMap(points[all-1].getColor().g, 0, 255, 0.0, 1.0),
                                                  ofMap(points[all-1].getColor().b, 0, 255, 0.0, 1.0)
                                                  );
            
            ofFloatColor colorCurrent = ofFloatColor(
                                                     ofMap(points[all].getColor().r, 0, 255, 0.0, 1.0),
                                                     ofMap(points[all].getColor().g, 0, 255, 0.0, 1.0),
                                                     ofMap(points[all].getColor().b, 0, 255, 0.0, 1.0)
                                                     );
            
            ofColor colorCurrent_top = points[all].getColor();
            colorCurrent_top.setBrightness(colorCurrent_top.getBrightness()/2);
            ofColor colorLast_top = points[all-1].getColor();
            colorLast_top.setBrightness(colorLast_top.getBrightness()/2);
            ofFloatColor colorCurrent_topF = ofFloatColor(
                                                          ofMap(colorCurrent_top.r, 0, 255, 0.0, 1.0),
                                                          ofMap(colorCurrent_top.g, 0, 255, 0.0, 1.0),
                                                          ofMap(colorCurrent_top.b, 0, 255, 0.0, 1.0)
                                                          );
            ofFloatColor colorLast_topF = ofFloatColor(
                                                       ofMap(colorLast_top.r, 0, 255, 0.0, 1.0),
                                                       ofMap(colorLast_top.g, 0, 255, 0.0, 1.0),
                                                       ofMap(colorLast_top.b, 0, 255, 0.0, 1.0)
                                                       );
            
            ofColor colorCurrent_bottom = points[all].getColor();
            colorCurrent_bottom.setSaturation(colorCurrent_bottom.getSaturation()/2);
            ofColor colorLast_bottom = points[all-1].getColor();
            colorLast_bottom.setSaturation(colorLast_bottom.getSaturation()/2);
            ofFloatColor colorCurrent_bottomF = ofFloatColor(
                                                             ofMap(colorCurrent_bottom.r, 0, 255, 0.0, 1.0),
                                                             ofMap(colorCurrent_bottom.g, 0, 255, 0.0, 1.0),
                                                             ofMap(colorCurrent_bottom.b, 0, 255, 0.0, 1.0)
                                                             );
            ofFloatColor colorLast_bottomF = ofFloatColor(
                                                          ofMap(colorLast_bottom.r, 0, 255, 0.0, 1.0),
                                                          ofMap(colorLast_bottom.g, 0, 255, 0.0, 1.0),
                                                          ofMap(colorLast_bottom.b, 0, 255, 0.0, 1.0)
                                                          );
            
            // front
            mesh.addVertex(last_1);
            mesh.addNormal(normalFront);
            mesh.addColor(colorLast);
            mesh.addVertex(last_2);
            mesh.addNormal(normalFront);
            mesh.addColor(colorLast);
            mesh.addVertex(point_1);
            mesh.addNormal(normalFront);
            mesh.addColor(colorCurrent);
            
            mesh.addVertex(point_1);
            mesh.addNormal(normalFront);
            mesh.addColor(colorCurrent);
            mesh.addVertex(point_2);
            mesh.addNormal(normalFront);
            mesh.addColor(colorCurrent);
            mesh.addVertex(last_2);
            mesh.addNormal(normalFront);
            mesh.addColor(colorLast);
            
            // top
            mesh.addVertex(last_1);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast_topF);
            mesh.addVertex(point_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorCurrent_topF);
            mesh.addVertex(point_1);
            mesh.addNormal(normalTop);
            mesh.addColor(colorCurrent_top);
            
            mesh.addVertex(last_1);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast_topF);
            mesh.addVertex(last_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast_topF);
            mesh.addVertex(point_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorCurrent_topF);
            
            // back
            mesh.addVertex(last_4);
            mesh.addNormal(normalBack);
            mesh.addColor(colorLast);
            mesh.addVertex(last_3);
            mesh.addNormal(normalBack);
            mesh.addColor(colorLast);
            mesh.addVertex(point_4);
            mesh.addNormal(normalBack);
            mesh.addColor(colorCurrent);
            
            mesh.addVertex(last_3);
            mesh.addNormal(normalBack);
            mesh.addColor(colorLast);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBack);
            mesh.addColor(colorCurrent);
            mesh.addVertex(point_4);
            mesh.addNormal(normalBack);
            mesh.addColor(colorCurrent);
            
            // bottom
            mesh.addVertex(last_2);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorLast_bottomF);
            mesh.addVertex(last_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorLast_bottomF);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent_bottomF);
            
            mesh.addVertex(last_2);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorLast_bottomF);
            mesh.addVertex(point_2);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent_bottomF);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent_bottomF);
            
            
            last_1 = point_1;
            last_2 = point_2;
            last_3 = point_3;
            last_4 = point_4;
            
            zIndex++;
            angle += twist;
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
    ofPopStyle();
}


void ProfileStyle::setFill(bool fill){
    this->fill = fill;
}

void ProfileStyle::toggleFill(){
    fill = !fill;
}