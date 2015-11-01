#pragma once

#define LOCALFRAME 1
#define REMOTEFRAME 2
#define TUIO 3
#define MOUSE 4

#include "ofMain.h"
#include "ofxSyphon.h"
#include "LEDFrame.h"
#include "ofxOsc.h"
#include "ofxTuio.h"
#include "GPoint.h"
#include "ProfileStyle.h"
#include "StrokeList.h"
#include "ScrizzleStyle.h"
#include "CaligraphyStyle.h"
#include "FlowField.h"
#include "Network.h"
#include "ofxGui.h"
#include "ofxFlowTools.h"
#include "SimpleFlowField.h"

//#include "PointGroupList.h"

//#define USE_NETWORK

using namespace flowTools;

enum OutputMode {SESI, LED1, LED2, PROJECTOR, PROJECTOR_PORTRAIT};

class gFrameApp : public ofBaseApp{

public: 
    void setup();
    void update();
    void draw();
    void drawFingerPositions(int _width, int _height);
    void exit();
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void keyPressed(int key);
    void windowResized(int w, int h);

    //OSC
    void oscUpdate();
    //void oscupdate_interface();

private:
    
    // DRAWING
    StrokeList stroke_list;
    int current_style = STYLE_CALIGRAPHY;
    ofParameter<ofColor> localBrushColor;
    
    ScrizzleStyle scrizzleStyle;  
    CaligraphyStyle caligraphyStyle;
    
    //flow
    SimpleFlowField simple_flow;
    SimpleFlowField simple_flow_2;
    ofImage obstacle, mapping_aid;
    ofFbo obstacleFBO;

    //output
    bool draw_on_main_screen = true;
    bool fullscreen;
    ofFbo canvasFBO;
    //ofFbo syphonFBO;
    ofParameter<int> outputwidth = 1024;
    ofParameter<int> outputheight = 768;
    ofRectangle outputRect;
    
    ofxSyphonServer syphonMainOut;
    
    //OSC
    ofxOscReceiver receiver;
    ofParameter<int> local_osc_port;
    
    //ofParameter<string> ipad_ip;
    //ofParameter<int> ipad_port;
    //ofParameter<bool> use_ipad = true;
    //ofxOscSender sender;
    //float last_ipad_update_time = 0;

    //LEDFrame and lighting
    LEDFrame ledFrame;

    
    //TUIO support
    ofxTuioClient   tuioClient;
    ofParameter<int> tuioPort = 3334;
    void tuioAdded(ofxTuioCursor & tuioCursor);
	void tuioRemoved(ofxTuioCursor & tuioCursor);
	void tuioUpdated(ofxTuioCursor & tuioCursor);
    
    //mouse
    int current_mouse_id;
    
    // NETWORK
    Network network;
    ofParameter<string> remote_ip;
    ofParameter<int> remote_port;
    ofParameter<int> host_port;

    // GUI
    void guiSetup();
    void styleGuiSetup();
    void flowGuiSetup();
    void flow2GuiSetup();
    ofxPanel gui;
    ofxPanel style_gui;
    ofxPanel flow_gui;
    ofxPanel flow2_gui;
    bool draw_gui = true;
    
    ofParameterGroup parameters_osc;
    ofParameterGroup parameters_network;
    ofParameterGroup parameters_output;
    ofParameterGroup parameters_profile_style;
    ofParameterGroup parameters_input;
    
    ofParameter<string> gui_outputmode;
    ofParameter<string> gui_direction;
    
    ofParameterGroup parameters_brush;
    
    //eventhandlers for gui inputs
    void onSettingsSave();
    void onSettingsReload();
    void onStyleSettingsSave();
    void onStyleSettingsreload();
    void onFlowSettingsSave();
    void onFlowSettingsReload();
    void onFlow2SettingsSave();
    void onFlow2SettingsReload();
    
    // current finger positions
    ofVec2f finger_positions[20];
    ofParameterGroup parameters_finger;
    ofParameter<bool> draw_finger_positions = true;
    ofParameter<int> finger_position_size = 20;
    
    ofParameter<float> point_lifetime = 10;

    ofParameter<bool> input_mouse, input_tuio, render_stroke, render_mapping_aid;
    ofParameter<bool> stroke_to_obstacle;
    
    //set the resolution for the fluid simulation, needs app restart
    ofParameter<int> flow_scale;
    
    ofShader convert2GrayShader;
    void setupConvert2GrayShader();
    
    
};


