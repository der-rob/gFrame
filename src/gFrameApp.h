#pragma once

#define LOCALFRAME 1
#define REMOTEFRAME 2
#define TUIO 3
#define MOUSE 4

#include "ofMain.h"
#include "ofxSyphon.h"
#include "ofxDmx.h"
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


#define STYLE_PROFILE 0
#define STYLE_SCRIZZLE 1
#define STYLE_CALIGRAPHY 2

//#define USE_NETWORK

using namespace flowTools;

enum OutputMode {SESI, LED1, LED2, PROJECTOR, PROJECTOR_PORTRAIT};
enum Orientation {PORTRAIT, LANDSCAPE};

class gFrameApp : public ofBaseApp{

public: 
    void setup();
    void update();
    void draw();
    void drawFingerPositions(int _width, int _height);
    void exit();
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void keyPressed(int key);
    void windowResized(int w, int h);
    ofVec2f				lastMouse;

    
    // DMX
    void updateLEDpulsing();
    void setLEDColor(ofColor ledColor);
    void start_pulsing();
    void stop_pulsing();

    //OSC
    void oscUpdate();
    void oscupdate_interface();

private:
    
    // DRAWING
    StrokeList stroke_list;
    int current_style = STYLE_CALIGRAPHY;
    ofParameter<ofColor> localBrushColor;
    ofParameter<int> newPointDistance;
    
    ProfileStyle profileStyle;
    ofParameter<int> style_profile_depth = 10;
    ofParameter<int> style_profile_width = 10;
    ofParameter<int> style_profile_zspeed = 1;
    ofParameter<int> style_profile_twist = 5;

    
    ScrizzleStyle scrizzleStyle;
    ofParameterGroup wild_parameters;
    ofParameter<float> W_amplitude;
    ofParameter<float> W_wavelength;
    ofParameter<float> W_nervosity;
    ofParameter<float> W_mainLine_thickness;
    ofParameter<float> W_byLine_thicknes;
    ofParameter<float> W_fadeout_time;
    ofParameter<float> W_fadeduration;
    
    CaligraphyStyle caligraphyStyle;
    ofParameterGroup caligraphy_parameters;
    ofParameter<int> C_width_min;
    ofParameter<int> C_width_max;
    ofParameter<float> C_fadeout_time;
    ofParameter<float> C_fadeduration;
    
    FlowField flowField;
    ofLight light;

    //output
    bool draw_on_main_screen = true;
    bool fullscreen;
    ofFbo syphonFBO, canvasFBO;
    ofParameter<int> outputwidth = 1024;
    ofParameter<int> outputheight = 768;
    ofRectangle outputRect;
    OutputMode outputmode = PROJECTOR;
    Orientation orientation = LANDSCAPE;
    
    ofxSyphonServer syphonMainOut;

    ofImage mCanvas;
    
    //OSC
    ofParameter<string> ipad_ip;
    ofParameter<int> ipad_port;
    ofParameter<int> local_osc_port;
    ofParameter<bool> use_ipad = true;
    ofxOscReceiver receiver;
    ofxOscSender sender;
    float last_ipad_update_time = 0;
    
    //DMX
    ofParameterGroup dmx_settings;
    ofParameter<bool> dmx_on = true;
    ofxDmx dmx;
    ofColor LEDstripColor;
    ofParameter<float> upper_pulsing_limit, lower_pulsing_limit;
    ofParameter<int> LED_pulsing_time;
    ofParameter<float> LED_level;
    ofParameter<float> LED_brightness;
    ofParameter<float> LED_frequency;
    
    bool LED_pulsing;
    float last_points_time;
        
    //TUIO support
    ofxTuioClient   tuioClient;
    ofParameter<int> tuioPort = 3333;
//    ofxPQLabs pqlabsframe;
    void tuioAdded(ofxTuioCursor & tuioCursor);
	void tuioRemoved(ofxTuioCursor & tuioCursor);
	void tuioUpdated(ofxTuioCursor & tuioCursor);
//    void onTouchPoint(TouchPointEvent &event);
    
    // NETWORK
    Network network;
    ofParameter<string> remote_ip;
    ofParameter<int> remote_port;
    ofParameter<int> host_port;

    // GUI
    void guiSetup();
    void styleGuiSetup();
    void flowGuiSetup();
    ofxPanel gui;
    ofxPanel style_gui;
    ofxPanel flow_gui;
    ofxPanel flow_gui_2;
    bool draw_gui = true;
    bool draw_flow_gui = true;
    
    ofParameterGroup parameters;
    ofParameterGroup style_settings;
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

    ofParameter<bool> input_mouse, input_pqlabs, input_tuio;
    
    //stencil
    ofParameter<string> mStencilText;
    ofTrueTypeFont stencilFont;
    ofFbo stencilFBO;
    void changeStencilText(string _newStencilText);
    
    
    
    //helpers
    string toUpperCase ( string str )
    {
        string strUpper = "";
        
        for( int i=0; i<str.length(); i++ )
        {
            strUpper += toupper( str[ i ] );
        }
        
        return strUpper;
    }
    
    string toLowerCase ( string str )
    {
        string strLower = "";
        
        for( int i=0; i<str.length(); i++ )
        {  
            strLower += tolower( str[ i ] );  
        }  
        
        return strLower;  
    }
    
};


