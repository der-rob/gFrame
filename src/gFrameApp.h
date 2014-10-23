#pragma once

#define LOCALFRAME 1
#define REMOTEFRAME 2
#define TUIO 3
#define MOUSE 4

#include "ofMain.h"
#include "ofxPQLabs.h"
#include "ofxSyphon.h"
#include "ofxDmx.h"
#include "ofxOsc.h"
#include "ofxTuio.h"
#include "GPoint.h"

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
    //vector containing ALL points
    vector <GPoint>  all_points;
    float timeToDie;
    
    //TUIO support
    ofxTuioClient   tuioClient;
    void	tuioAdded(ofxTuioCursor & tuioCursor);
	void	tuioRemoved(ofxTuioCursor & tuioCursor);
	void	tuioUpdated(ofxTuioCursor & tuioCursor);
    
    //DMX
    ofxDmx dmx;
    ofColor LEDstripColor;
    float upper_pulsing_limit, lower_pulsing_limit;
    void setLEDColor(ofColor ledColor);
    void start_pulsing();
    void stop_pulsing();
    
    bool LED_pulsing;
    int LED_pulsing_time;
    float LED_level;
    float last_points_time;
    
    //OSC
    ofxOscReceiver receiver;
    ofxOscSender sender;
    void oscUpdate();
    void oscupdate_interface();
    float last_ipad_update_time = 0;
    
    //Syphon output
    ofxSyphonServer syphonMainOut;
    
    //drawing parameter
    ofParameterGroup localDrawingParameters;
    ofParameter<int> localPenWidth;
    ofParameter<ofColor> localPenColor;
};


