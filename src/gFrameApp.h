#pragma once

#include "ofMain.h"
#include "ofxPQLabs.h"

class gFrameApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofxPQLabs touchFrame;
    //vectors containing point data
    //mouse
    vector<ofVec3f> points_m;
    //local frame
    vector< vector<ofVec3f> > points_f;
    
    void onTouchPoint(TouchPointEvent &event);

};
