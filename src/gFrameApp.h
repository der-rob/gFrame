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
#include "ProfileStyle.h"
#include "StrokeList.h"


#define STYLE_PROFILE 0


class gFrameApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();
    void mouseMoved(int x, int y);
    void keyPressed(int key);
    
    void dmxUpdate();
    
    //frame and point data
    void onTouchPoint(TouchPointEvent &event);
    
    // DMX
    void setLEDColor(ofColor ledColor);
    void start_pulsing();
    void stop_pulsing();

    //OSC
    void oscUpdate();
    void oscupdate_interface();
   

private:
    
    // DRAWING
    StrokeList stroke_list;
    int current_style = STYLE_PROFILE;
    ProfileStyle profileStyle;
    
    ofLight light;

    //Syphon output
    ofxSyphonServer syphonMainOut;
    ofTexture texScreen;
    
    //drawing parameter
    ofParameterGroup localDrawingParameters;
    ofParameter<int> localPenWidth;
    ofParameter<ofColor> localPenColor;
    
    //OSC
    ofxOscReceiver receiver;
    ofxOscSender sender;
    float last_ipad_update_time = 0;
    
    //DMX
    ofxDmx dmx;
    ofColor LEDstripColor;
    float upper_pulsing_limit, lower_pulsing_limit;
    bool LED_pulsing;
    int LED_pulsing_time;
    float LED_level;
    float last_points_time;
    
    //TUIO support
    ofxTuioClient   tuioClient;
    void	tuioAdded(ofxTuioCursor & tuioCursor);
	void	tuioRemoved(ofxTuioCursor & tuioCursor);
	void	tuioUpdated(ofxTuioCursor & tuioCursor);
    
    // FRAME
    ofxPQLabs touchFrame;
    
};


