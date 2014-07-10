#pragma once

#include "ofMain.h"
#include "ofxPQLabs.h"
#include "ofxSyphon.h"
#include "ofxDmx.h"
#include "ofxOsc.h"

class gFrameApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
        void exit();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    //frame and point data
    ofxPQLabs touchFrame;
    void onTouchPoint(TouchPointEvent &event);

    //vectors containing point data
    //mouse for easier testing
    vector<ofVec3f> points_m;
    //local frame
    vector< vector<ofVec3f> > points_f;
    
    //DMX
    ofxDmx dmx;
    ofColor colorFromPoint(ofVec3f thePoint);
    void setLEDColor(ofColor ledColor);
    ofColor LEDstripColor;
    
    //OSC
    ofxOscReceiver receiver;
    void oscUpdate();
    
    //Syphon output
    ofxSyphonServer syphonMainOut;
    
    //drawing parameter
    ofParameterGroup localDrawingParameters;
    ofParameter<int> localPenWidth;
    ofParameter<ofColor> localPenColor;
};
