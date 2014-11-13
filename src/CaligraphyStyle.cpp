////
////  CaligraphyStyle.cpp
////  StyleTest
////
////  Created by Julian Adenauer on 14.10.14.
////
////
//
//#include "CaligraphyStyle.h"
//
//
//CaligraphyStyle::CaligraphyStyle(){
//    fill = true;
//}
//
//void CaligraphyStyle::render(vector<GPoint>& points){
////    for(int i = 1; i < currentPoints.size(); i++)
////    {
////        ofSetColor(0,0,255);
////        float speed = sqrt(pow(currentPoints[i].loc.x - currentPoints[i-1].loc.x, 2) + pow(currentPoints[i-1].loc.y - currentPoints[i].loc.y, 2));
////        
////        float dx = currentPoints[i-1].loc.x - currentPoints[i].loc.x;
////        float dy = currentPoints[i-1].loc.y - currentPoints[i].loc.y;
////        
////        //        ofLine(-dy+currentPoints[i].loc.x, dx+ currentPoints[i].loc.y, dy+ currentPoints[i].loc.x, -dx+currentPoints[i].loc.y);
////        
////        //        ofCircle(currentPoints[i].loc.x, currentPoints[i].loc.y, speed);
////        
////        ofLine(currentPoints[i-1].loc.x, currentPoints[i-1].loc.y, currentPoints[i].loc.x, currentPoints[i].loc.y);
////        //        ofLine(outline1[i-1].loc.x, outline1[i-1].loc.y, outline1[i].loc.x, outline1[i].loc.y);
////        //        ofLine(outline2[i-1].loc.x, outline2[i-1].loc.y, outline2[i].loc.x, outline2[i].loc.y);
////    }
//    
////    ofNoFill();
////    
////    ofSetPolyMode(OF_POLY_WINDING_ODD);
////    ofSetColor(255, 0, 0);
////    ofBeginShape();
////    if(outline1.size() > 1) ofVertex(outline1[0].loc.x, outline1[0].loc.y);
////    for(int i = 1; i < currentPoints.size(); i++) {
////        ofCurveVertex(outline1[i].loc.x, outline1[i].loc.y);
////    }
////    ofEndShape();
////    
////    ofBeginShape();
////    ofSetColor(255, 0, 0);
////    if(outline1.size() > 1) ofVertex(outline2[0].loc.x, outline2[0].loc.y);
////    for(int i = 1; i < currentPoints.size(); i++) {
////        ofCurveVertex(outline2[i].loc.x, outline2[i].loc.y);
////    }
////    ofEndShape();
//    
//    // draw old elements
//    
//
//    
//    ofSetColor(100,0,100);
////    for(int all = 0; all<all_points.size(); all++){
////        for(int cur=1; cur < all_points[all].size();cur++){
////            float dx = all_points[all][cur-1].loc.x - all_points[all][cur].loc.x;
////            float dy = all_points[all][cur-1].loc.y - all_points[all][cur].loc.y;
////            ofLine(all_points[all][cur-1].loc.x, all_points[all][cur-1].loc.y, all_points[all][cur].loc.x, all_points[all][cur].loc.y);
////        }
////    }
//    
//    // render filled outlines
//    ofSetPolyMode(OF_POLY_WINDING_POSITIVE);
//    if(points.size() > 1){
//            if(fill){
//                ofFill();
//            }
//            else{
//                ofNoFill();
//            }
//        
//            ofBeginShape();
//            
//            // add startpoint
//            ofVertex(points[0].getLocation().x, points[0].getLocation().y);
//        
//        // GENERATE OUTLINES
//        
//            GPoint lastPoint = all_points.back()[all_points.back().size() - 1];
//        float dx = lastPoint.getLocation().x - x;
//        float dy = lastPoint.getLocation().y - y;
//        
//        ofVec2f v1, v2;
//        v1 = ofVec2f(-dy, dx);
//        v1.scale(ofMap(v1.length(), 0, 200, 20, 1));
//        v2 = ofVec2f(dy, -dx);
//        v2.scale(ofMap(v2.length(), 0, 200, 30, 1));
//        
//        GPoint p1, p2;
//        p1.getLocation() = v1 + ofVec2f(x, y);
//        p2.getLocation() = v2 + ofVec2f(x, y);
//        all_outlines1.back().push_back(p1);
//        all_outlines2.back().push_back(p2);
//        
//        
//        // OLD CODE
//            
//            for(int cur=1; cur < all_outlines1[all].size()-1;cur++){
//                ofCurveVertex(all_outlines1[all][cur].getLocation().x, all_outlines1[all][cur].getLocation().y);
//            }
//            
//            ofVertex(all_points[all][all_points[all].size()-1].getLocation().x, all_points[all][all_points[all].size()-1].getLocation().y);
//            
//            // draw the second outline backwards
//            for(int cur=all_outlines2[all].size()-1; cur >= 0;cur--){
//                ofCurveVertex(all_outlines2[all][cur].getLocation().x, all_outlines2[all][cur].getLocation().y);
//            }
//            
//            ofEndShape();
//        
//            ofNoFill();
//        }
//    }
//}
//
//void CaligraphyStyle::dragging(int x, int y){
//    // calculate the points of the outlines
//    // get normals as described here: http://stackoverflow.com/questions/1243614/how-do-i-calculate-the-normal-vector-of-a-line-segment
//    // not the first element in the line
//    if(all_points.back().size() > 0){
//        
//    }
//    else {
//        GPoint p1, p2;
//        p1.getLocation() = ofVec2f(x, y);
//        p2.getLocation() = ofVec2f(x, y);
//        all_outlines1.back().push_back(p1);
//        all_outlines2.back().push_back(p2);
//    }
//    
//    GPoint the_point;
//    the_point.setLocation(ofVec2f(x,y));
////    the_point.point_id = 0;
//    //    the_point.color = localPenColor;
//    //    the_point.type = MOUSE;
//    //    the_point.lifetime = timeTolive;
//    all_points.back().push_back(the_point);
//
//}
//
//void CaligraphyStyle::setFill(bool fill){
//    this->fill = fill;
//}
//
//void CaligraphyStyle::toggleFill(){
//    fill = !fill;
//}