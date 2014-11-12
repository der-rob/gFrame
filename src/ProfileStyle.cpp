//
//  ProfileStyle.cpp
//  StyleTest
//
//  Created by Julian Adenauer on 14.10.14.
//
//

#include "ProfileStyle.h"

void ProfileStyle::render(vector<GPoint>& points){
    
    GPoint* last_used_point;
    
    list<int> ids;
    
    if(points.size() > 2){
        ofMesh mesh;
        float zIndex = 0;
        int angle = 0;
        
        mesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        zIndex = ((float) points[0].getTimestamp() - ofGetElapsedTimeMillis())/(10.0/z_speed);
        
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
            zIndex = ((float) points[all].getTimestamp() - ofGetElapsedTimeMillis())/(10.0/z_speed);
            
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
            mesh.addColor(colorLast);
            mesh.addVertex(point_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorCurrent);
            mesh.addVertex(point_1);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast);
            
            mesh.addVertex(last_1);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast);
            mesh.addVertex(last_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorLast);
            mesh.addVertex(point_4);
            mesh.addNormal(normalTop);
            mesh.addColor(colorCurrent);
            
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
            mesh.addColor(colorLast);
            mesh.addVertex(last_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorLast);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent);
            
            mesh.addVertex(last_2);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorLast);
            mesh.addVertex(point_2);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent);
            mesh.addVertex(point_3);
            mesh.addNormal(normalBottom);
            mesh.addColor(colorCurrent);
            
            
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
}

void ProfileStyle::setFill(bool fill){
    this->fill = fill;
}

void ProfileStyle::toggleFill(){
    fill = !fill;
}