#include "gFrameApp.h"

//--------------------------------------------------------------
void gFrameApp::setup(){
    //just set up the openFrameworks stuff
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(82,70,86);
    
    //Syphon stuff
    syphonMainOut.setName("gFrame Main Out");

    touchFrame.connect("127.0.0.1");
    ofAddListener(touchFrame.touchEventDispatcher, this, &gFrameApp::onTouchPoint);
    
    //ensure that points_f is not empty but filled with empty vectors
    for (int i =0; i<12;i++)
    {
        vector<ofVec3f> vec;
        points_f.push_back(vec);
    }

}

//--------------------------------------------------------------
void gFrameApp::update(){

}

//--------------------------------------------------------------
void gFrameApp::draw(){
    //draw mouse
    for(unsigned int i = 1; i < points_m.size(); i++)
    {
        ofCircle(points_m[i].x, points_m[i].y, 10);
        ofLine(points_m[i-1].x, points_m[i-1].y, points_m[i].x, points_m[i].y);
    }
    
    
    //draw local frame
    for(unsigned int i = 0; i < points_f.size(); i++)
    {
        for (int j = 1;j< points_f[i].size(); j++)
        {
            ofCircle(points_f[i][j].x, points_f[i][j].y, 10);
            ofLine(points_f[i][j-1].x, points_f[i][j-1].y, points_f[i][j].x, points_f[i][j].y);
        }
    }
    
    //syphon
    syphonMainOut.publishScreen();
}

//--------------------------------------------------------------
void gFrameApp::keyPressed(int key){

}

//--------------------------------------------------------------
void gFrameApp::keyReleased(int key){

}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y){
    ofVec3f mousePoint(x,y,0);
    points_m.push_back(mousePoint);

}

//--------------------------------------------------------------
void gFrameApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void gFrameApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void gFrameApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void gFrameApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void gFrameApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void gFrameApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void gFrameApp::onTouchPoint(TouchPointEvent &event) {
    ofVec3f framePoint(event.touchPoint.x,event.touchPoint.y,0);
    
    //sort points by touch id
    int id = event.touchPoint.id;
    points_f[id].push_back(framePoint);
}