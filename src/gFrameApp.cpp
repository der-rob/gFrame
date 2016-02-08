#include "gFrameApp.h"

//#define USE_NETWORK

//--------------------------------------------------------------
void gFrameApp::setup()
{
    //just set up the openFrameworks stuff
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(ofColor::black);
    
    //LEDFrame setup
    ledFrame.setup();
    
    //GUI setup
    guiSetup();
    styleGuiSetup();
    
    //eventhamdlers for gui
    ofAddListener(gui.loadPressedE, this, &gFrameApp::onSettingsReload);
    ofAddListener(gui.savePressedE, this, &gFrameApp::onSettingsSave);
    ofAddListener(style_gui.loadPressedE, this, &gFrameApp::onStyleSettingsreload);
    ofAddListener(style_gui.savePressedE, this, &gFrameApp::onStyleSettingsSave);
    
    //load settings and stylesettings
    gui.loadFromFile("settings.xml");
    style_gui.loadFromFile("stylesettings.xml");
    
    //dimensions for final output
    //after settings are loaded from file
    ofSetWindowShape(outputwidth, outputheight);
    outputRect = ofRectangle(0,0,outputwidth, outputheight);
    
    //set positions for style and normal gui
    style_gui.setPosition(ofGetWidth() - 2*style_gui.getWidth() - 20, 10);
    gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
    
    //some more listeners for the gui
    stroke_to_obstacle.addListener(this, &gFrameApp::onStrokeToObstacleChanged);
    vertical_obstacle.addListener(this, &gFrameApp::onVerticalObstacleChanged);
    
    //flowfield & gui
    obstacle.load("obstacle/obstacle_fullheight.jpg");
    mapping_aid.load("obstacle/obstacle_mapping_aid.jpg");
    
    simple_flow.setup(outputRect.width/2, outputRect.height/2, flow_scale);
    flowGuiSetup();
    flow_gui.loadFromFile("demo_4.xml");
    ofAddListener(flow_gui.loadPressedE, this, &gFrameApp::onFlowSettingsReload);
    ofAddListener(flow_gui.savePressedE, this, &gFrameApp::onFlowSettingsSave);
    simple_flow.use_seperate_fluid_color.addListener(this, &gFrameApp::onSepFluidColorChanged);
    
    simple_flow_2.setup(outputRect.width/2, outputRect.height/2, flow_scale);
    flow2GuiSetup();
    flow2_gui.loadFromFile("demo_4_back.xml");
    ofAddListener(flow2_gui.loadPressedE, this, &gFrameApp::onFlow2SettingsReload);
    ofAddListener(flow2_gui.savePressedE, this, &gFrameApp::onFlow2SettingsSave);
    fluidColor = localBrushColor;
    
    //need to call this here, otherwise would get a BAD ACCESS FAULT
    gui.draw();
    style_gui.draw();
    
    //Syphon stuff
    syphonMainOut.setName("gFrame Main Out");
    
    //TUIO setup
    tuioClient.start(tuioPort);
    ofAddListener(tuioClient.cursorAdded,this,&gFrameApp::tuioAdded);
	ofAddListener(tuioClient.cursorUpdated,this,&gFrameApp::tuioUpdated);
    ofAddListener(tuioClient.cursorRemoved,this,&gFrameApp::tuioRemoved);
    
    //OSC
    receiver.setup(local_osc_port);

    // NETWORK
    //network.setup(host_port, remote_ip, remote_port);
    //stroke_list.setupSync(&network);

    canvasFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 8);
    canvasFBO.begin(); ofClear(0); canvasFBO.end();
    
    obstacleFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 8);
    obstacleFBO.begin(); ofClear(0); obstacleFBO.end();

    // initialize finger positions
    for(ofVec2f finger : finger_positions){
        finger = ofVec2f(0,0);
    }
    
    render_mapping_aid = false;
    
    //load the LUTfile
    loadLUTFile("LUT/lutxml.xml");
    
    //shader for converting the obstaclefbo to bw image for the obstacle buffer
    setupConvert2BWShader();
    
    vector<ofVideoDevice> devices = vidGrabber.listDevices();
    
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(30);
    vidGrabber.initGrabber(320, 240);
    videoInverted.allocate(320, 240, OF_PIXELS_RGB);
    videoTexture.allocate(videoInverted);
}

//--------------------------------------------------------------
void gFrameApp::exit(){
    ledFrame.disconnect();
    //network.disconnect();

}

//--------------------------------------------------------------
void gFrameApp::update()
{
    ///experimental
    vidGrabber.update();
    if(vidGrabber.isFrameNew()){
        ofPixels & pixels = vidGrabber.getPixels();
        for(int i = 0; i < pixels.size(); i += pixels.getNumChannels()){
            int r = pixels[i];
            int g = pixels[i+1];
            int b = pixels[i+2];
            
            if ((r+g+b) / 3 > 128) {
                videoInverted[i] = 255;
                videoInverted[i+1] = 255;
                videoInverted[i+2] = 255;
            } else {
                videoInverted[i] = 0;
                videoInverted[i+1] = 0;
                videoInverted[i+2] = 0;
            }
        }
        videoTexture.loadData(videoInverted);
    }
    ///experimental
    
    stroke_list.update();
    
    oscUpdate();
    
    tuioClient.getMessage();
    
    simple_flow.update();
    simple_flow_2.update();
    simple_flow.color = fluidColor;
    
    // DMX UPDATE
    if (ledFrame.getEnabled()) {
        ledFrame.updateLevel();
        ledFrame.setColor(localBrushColor);
        ledFrame.update();
    }
    
    canvasFBO.begin();
    ofBackground(0);
    
    //generate obstacle on the fly
    //the stroke is drawn to an extra fbo
    //a simple shader converts the drawing into a black/white image which is then
    //feed into the obstacle buffers of the two fluids or only one fluid
    
    if (stroke_to_obstacle)
    {
//        obstacleFBO.begin();
//        
//        convert2GrayShader.begin();
//        
//        ofClear(0);
//        for(vector<GPoint> stroke : *stroke_list.getAllStrokes()){
//            caligraphyStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
//        }
//        convert2GrayShader.end();
//        
//        obstacleFBO.end();
//
//        simple_flow.addObstacle(obstacleFBO.getTexture());
//        //simple_flow_2.addObstacle(obstacleFBO.getTextureReference());
        
        //video to obstacle
        obstacleFBO.begin();
        //obstacle_video.draw(0,0, outputRect.width, outputRect.height);
        
        videoTexture.draw(0,0, outputRect.width, outputRect.height);
        
        obstacleFBO.end();
        simple_flow.addObstacle(obstacleFBO.getTexture());
        
    }
    
    //the white background fluid
    simple_flow_2.draw();
    
    if (stroke_first)
    {
        ofBlendMode(OF_BLENDMODE_SCREEN);
        //the colored foreground fluid
        simple_flow.draw();
    }
    
    if (render_stroke) {
        for(vector<GPoint> stroke : *stroke_list.getAllStrokes())
        {
            //always use claigraphy style
            caligraphyStyle.render(stroke, (int)outputRect.width, (int)outputRect.height);
        }
    }

    if (!stroke_first)
    {
        //the colored foreground fluid
        simple_flow.draw();
    }
    
    if(draw_finger_positions){
        drawFingerPositions((int)outputRect.width, (int)outputRect.height);
    }

    if (render_mapping_aid)
    {
        ofPushStyle();
        ofSetColor(255,255);
        mapping_aid.draw(0,0,outputRect.width, outputRect.height);
        ofPopStyle();
    }

    canvasFBO.end();
    
    // lifetime
    stroke_list.setLifetime(point_lifetime * 1000);
}

//--------------------------------------------------------------
void gFrameApp::draw(){
    
    ofBackground(0);
    ofSetColor(255,255);

    syphonMainOut.publishTexture(&canvasFBO.getTexture());
    
    //switching of the main screen might improve the performance
    if (draw_on_main_screen)
    {
        canvasFBO.draw(0,0,outputRect.width, outputRect.height);
    }

    //gui output here
    if(draw_gui){
        gui.draw();
        style_gui.draw();
        flow_gui.draw();
        flow2_gui.draw();
        ofSetColor(255,255);
        ofDrawBitmapString("fps: " + ofToString(ofGetFrameRate(), 2), ofGetWidth()-120, ofGetHeight()-10 );
    }
    
    //for debugging/testing, render the obstacle fbo to screen
//    obstacleFBO.draw(ofGetWidth() - ofGetWidth()/10 - 10, ofGetHeight() - ofGetHeight()/10 -10, ofGetWidth()/10, ofGetHeight()/10);
    
    //render camera input which is load to obstcacle buffer
    //videoTexture.draw(20,ofGetHeight() - 20 -240);
}

//--------------------------------------------------------------
void gFrameApp::drawFingerPositions(int _width, int _height)
{
    ofPushStyle();
    ofDisableDepthTest(); // disable depth test so the alpha blending works properly
    ofDisableLighting();
    
    ofColor outerColor = localBrushColor;
    outerColor.set(localBrushColor.get().r, localBrushColor.get().g, localBrushColor.get().b, 30);
    
    ofSetColor(localBrushColor);
    int i=0;
    for(ofVec2f finger : finger_positions){
        int px = finger.x * _width;
        int py = finger.y * _height;
        if(!(px == 0 && py == 0)){
            float incr = (float) ((2 * PI) / 32);
            
            glBegin(GL_TRIANGLE_FAN);
            ofSetColor(localBrushColor);
            glVertex2f(px, py);
            
            ofSetColor(outerColor);
            
            for(int i = 0; i < 32; i++){
                float angle = incr * i;
                float x = ((float) cos(angle) * finger_position_size) + px;
                float y = ((float) sin(angle) * finger_position_size) + py;
                glVertex2f(x, y);
            }
            
            glVertex2f(finger_position_size + px, py);
            glEnd();
#if DEBUG
            // draw finger id for debugging
            ofSetColor(255, 255, 255, 255);
            ofDrawBitmapString(ofToString(i), finger.x-5, finger.y+5);
#endif
        }
        i++;
    }
    ofPopStyle();
}

//--------------------------------------------------------------
void gFrameApp::keyPressed(int key)
{
    if (key == 'r') {
        //localBrushColor = ofColor(255,0,0,255);
        setColor(0,use_lut);
    }
    else if (key == 'b') {
        //localBrushColor = ofColor(0,0,255,255);
        setColor(4, use_lut);
    }
    else if (key == 'g') {
        //localBrushColor = ofColor(0,255,0,255);
        setColor(3, use_lut);
    }
    else if(key == 'c') {
        stroke_list.clear();
    }
    else if (key == 'p')
        draw_finger_positions = !draw_finger_positions;
    else if(key == 'h')
        draw_gui = !draw_gui;
    else if (key == 'd')
        ledFrame.toggleEnabled();
    else if(key == 'y')
        draw_on_main_screen = !draw_on_main_screen;
    else if (key == 'x') {
        fullscreen = !fullscreen;
        ofSetFullscreen(fullscreen);
        style_gui.setPosition(ofGetWidth() - 2*style_gui.getWidth() - 20, 10);
        gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
        outputRect.width = ofGetWidth();
        outputRect.height = ofGetHeight();
        canvasFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 16);
        obstacleFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 16);
        //syphonFBO.allocate(outputRect.width, outputRect.height, GL_RGBA, 16);
        cout << outputRect.width << " x " << outputRect.height << endl;
    }
    
    else if (key == 'm') {
        draw_on_main_screen = !draw_on_main_screen;
    }
    else if (key == 'o') {
        render_stroke = !render_stroke;
    }
    else if (key == 'q')
    {
        render_mapping_aid = !render_mapping_aid;
    }
}

//--------------------------------------------------------------
void gFrameApp::mouseMoved(int x, int y)
{
}

//--------------------------------------------------------------
void gFrameApp::mousePressed(int button, int x, int y) {
    if(input_mouse && button == OF_MOUSE_BUTTON_1){
        
        //rescale mouse position
        ofVec2f mouse;
        mouse.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
        
        //size scale animation
        //brushSizeScale[12].createAnimation(0, 1, 4, ZERO);
        
        GPoint the_point;
        //the_point.setSizeScale(brushSizeScale[12].getValue());
        the_point.setLocation(ofVec2f(mouse.x,mouse.y));
        the_point.setId(0);
        the_point.setStrokeId(current_mouse_id);
        the_point.setColor(localBrushColor);
        the_point.setStyle(current_style);
        stroke_list.addToNewStroke(the_point);
        
        ledFrame.stopPulsing();
        ledFrame.updateLastPointsTime();
        
        //flowfield
        simple_flow.inputUpdate(x, y);
        simple_flow_2.inputUpdate(x, y);
    }
}

//--------------------------------------------------------------
void gFrameApp::mouseDragged(int x, int y, int button)
{
    if(input_mouse && button == OF_MOUSE_BUTTON_1) {
        
        //rescale mouse position
        ofVec2f mouse;
        mouse.set(x / (float)ofGetWindowWidth(), y / (float)ofGetWindowHeight());
        
        GPoint the_point;
        the_point.setLocation(ofVec2f(mouse.x,mouse.y));
        the_point.setId(0);
        the_point.setStrokeId(current_mouse_id);
        the_point.setColor(localBrushColor);
        the_point.setStyle(current_style);
        stroke_list.add(the_point);
        
        ledFrame.stopPulsing();
        ledFrame.updateLastPointsTime();
        
        //flowfield
        simple_flow.inputUpdate(x, y);
        simple_flow_2.inputUpdate(x, y);
    }
}

//--------------------------------------------------------------
void gFrameApp::mouseReleased(int x, int y, int button) {
    if (button == OF_MOUSE_BUTTON_1) {
        current_mouse_id++;
    }
}

//--------------------------------------------------------------
void gFrameApp::windowResized(int w, int h)
{
    outputRect.width = w;
    outputRect.height = h;
}

//--------------------------------------------------------------
void gFrameApp::tuioAdded(ofxTuioCursor &cursor)
{
    if(input_tuio){
        GPoint the_point;
        float x,y;
        x = cursor.getX();
        y = cursor.getY();

        the_point.setLocation(ofVec2f(x, y));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.addToNewStroke(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
        //flowfield
        simple_flow.inputUpdate(x, y, cursor.getFingerId());
        simple_flow_2.inputUpdate(x, y, cursor.getFingerId());
        
        ledFrame.stopPulsing();
        ledFrame.updateLastPointsTime();
    }
}

//--------------------------------------------------------------
void gFrameApp::tuioUpdated(ofxTuioCursor &cursor)
{
    if(input_tuio)
    {
        GPoint the_point;
        float x,y;
        x = cursor.getX();
        y = cursor.getY();
        
        the_point.setLocation(ofVec2f(x, y));
        the_point.setId(cursor.getFingerId());
        the_point.setStrokeId(cursor.getSessionId());
        the_point.setColor(localBrushColor);
        the_point.setType(TUIO);
        the_point.setStyle(current_style);
        stroke_list.add(the_point);
        
        finger_positions[cursor.getFingerId()] = ofVec2f(x, y);
        //flowfield
        simple_flow.inputUpdate(x, y, cursor.getFingerId());
        simple_flow_2.inputUpdate(x, y, cursor.getFingerId());
        
        ledFrame.stopPulsing();
        ledFrame.updateLastPointsTime();
    }
}

//--------------------------------------------------------------
void gFrameApp::tuioRemoved(ofxTuioCursor & cursor)
{
    if(input_tuio)
    {
        finger_positions[cursor.getFingerId()] = ofVec2f(0, 0);
    }
}

//--------------------------------------------------------------
void gFrameApp::oscUpdate()
{
    while (receiver.hasWaitingMessages())
    {
        ofxOscMessage m;
        receiver.getNextMessage(m);
        cout << m.getAddress() << " " << m.getArgAsString(0) << endl;
       
        //old color changing via given colors
        /*
        //style color
        if (m.getAddress() == "/col_red") setColor(0, use_lut);
        else if (m.getAddress() == "/col_pink") setColor(1, use_lut);
        else if (m.getAddress() == "/col_yellow") setColor(2, use_lut);
        else if (m.getAddress() == "/col_green") setColor(3, use_lut);
        else if (m.getAddress() == "/col_blue") setColor(4, use_lut);
        else if (m.getAddress() == "/clearcanvas") stroke_list.clear();
         */
        //new colorchange via color picker
        if (m.getAddress() == "/color/color") {
            ofColor _col;
            _col.r = m.getArgAsChar(0);
            _col.g = m.getArgAsChar(1);
            _col.b = m.getArgAsChar(2);
            _col.a = m.getArgAsChar(3);
            
            localBrushColor = _col;
            fluidColor = _col;
            
        }
        //brush style
        else if (m.getAddress() == "/style_wild") current_style = STYLE_SCRIZZLE;
        else if (m.getAddress() == "/style_cali") current_style = STYLE_CALIGRAPHY;
    }
}

//--------------------------------------------------------------
void gFrameApp::guiSetup() {
    //GUI Setup
    gui.setup();
    gui.setPosition(ofGetWidth() - gui.getWidth() - 10, 10);
    gui.setName("general settings");
    
    ///output settings
    parameters_output.setName("output settings");
    outputwidth.setName("width");
    parameters_output.add(outputwidth);
    outputheight.setName("height");
    parameters_output.add(outputheight);
    parameters_output.add(flow_scale.set("Flow Scale", 4, 1, 128));
    parameters_output.add(render_mapping_aid.set("show mapping aid", false));
    
    ///OSC
    parameters_osc.setName("osc");
    local_osc_port.setName("local OSC port");
    parameters_osc.add(local_osc_port);
    
    ///network
//    parameters_network.setName("network");
//    host_port.setName("host port");
//    remote_ip.setName("remote ip");
//    remote_port.setName("remote port");
//    
//    parameters_network.add(host_port);
//    parameters_network.add(remote_ip);
//    parameters_network.add(remote_port);

    ///general brush settings
    parameters_brush.setName("brush settings");
    parameters_brush.add(point_lifetime.set("point lifetime", 10, 1, 100));
    parameters_brush.add(localBrushColor.set("brush color", ofColor(255, 255, 255, 255), ofColor(0,0,0,0), ofColor(255,255,255,255)));
    parameters_brush.add(render_stroke.set("render stroke", true));
    parameters_brush.add(stroke_to_obstacle.set("stroke to obstacle", false));
    parameters_brush.add(vertical_obstacle.set("vertical obstacle", false));
    parameters_brush.add(use_lut.set("use lut", true));
    parameters_brush.add((stroke_first.set("stroke first", true)));
    
    // finger positions
    parameters_finger.setName("finger positions");
    parameters_finger.add(draw_finger_positions.set("draw finger positions", true));
    parameters_finger.add(finger_position_size.set("finger circle radius", 30, 2, 100));
    
    // input settings
    parameters_input.setName("input selection");
    parameters_input.add(tuioPort.set("TUIO port", 3333));
    parameters_input.add(input_mouse.set("mouse", false));
    parameters_input.add(input_tuio.set("tuio", true));
    
    //add the subgroups to the gui
    gui.add(parameters_finger);
    gui.add(parameters_brush);
    gui.add(parameters_output);
    gui.add(ledFrame.parameters);
//    gui.add(parameters_network);
    gui.add(parameters_osc);
    gui.add(parameters_input);
    
    //minimize some of the gui elements
    vector<string> controlNames = gui.getControlNames();
    for (int i=0; i < controlNames.size(); i++) {
        string the_name = controlNames[i];
        if (the_name == "output settings" ||
            the_name == "dmx settings" ||
            the_name == "network" ||
            the_name == "osc" ||
            the_name == "input selection")
        {
            ofxGuiGroup *the_group = &gui.getGroup(the_name);
            the_group->minimize();
        }
    }
}

//--------------------------------------------------------------
void gFrameApp::styleGuiSetup() {
    //more specific style settings
    style_gui.setup();
    style_gui.setName("style settings");
    style_gui.setPosition(ofGetWidth() - 2*style_gui.getWidth() - 20, 10);
    
    style_gui.add(caligraphyStyle.parameters);
    
    style_gui.minimizeAll();
}

//--------------------------------------------------------------
void gFrameApp::onSettingsReload() {
    gui.loadFromFile("settings.xml");
    //network
    network.disconnect();
    network.setup(host_port, remote_ip, remote_port);
    //osc
    receiver.setup(local_osc_port);
    //tuio
    tuioClient.start(tuioPort);
    //clean stroklist
    stroke_list.clear();
    
    cout << "New settings loaded" << endl;
}

//--------------------------------------------------------------
void gFrameApp::onSettingsSave() {
    gui.saveToFile("settings.xml");
}

//--------------------------------------------------------------
void gFrameApp::onStyleSettingsreload() {
    style_gui.loadFromFile("stylesettings.xml");
    ofLog() << "style settings reloaded";
}

//--------------------------------------------------------------
void gFrameApp::onStyleSettingsSave() {
    style_gui.saveToFile("stylesettings.xml");
    ofLog() << "style settings saved";
}

//--------------------------------------------------------------
void gFrameApp::flowGuiSetup() {
    flow_gui.setup();
    flow_gui.setName("foreground fluid");
    flow_gui.setPosition(10, 10);
    flow_gui.add(simple_flow.fluid.parameters);
    flow_gui.add(simple_flow.options);
    flow_gui.minimizeAll();
}

//--------------------------------------------------------------
void gFrameApp::onFlowSettingsSave() {
    ofFileDialogResult save_result = ofSystemSaveDialog("NewFlowSettings.xml", "save new flow settings");
    string new_filename = save_result.getPath();
    cout << new_filename << endl;
    if (new_filename != "")
        flow_gui.saveToFile(new_filename);
}

//--------------------------------------------------------------
void gFrameApp::onFlowSettingsReload() {
    ofFileDialogResult load_result = ofSystemLoadDialog();
    string load_filename = load_result.getPath();
    if (load_filename != "")
        flow_gui.loadFromFile(load_filename);
}

//--------------------------------------------------------------
void gFrameApp::flow2GuiSetup() {
    flow2_gui.setup();
    flow2_gui.setName("background fluid");
    flow2_gui.setPosition(220, 10);
    flow2_gui.add(simple_flow_2.fluid.parameters);
    flow2_gui.add(simple_flow_2.options);
    flow2_gui.minimizeAll();
}

//--------------------------------------------------------------
void gFrameApp::onFlow2SettingsSave() {
    ofFileDialogResult save_result = ofSystemSaveDialog("NewFlowSettings.xml", "save new flow settings");
    string new_filename = save_result.getPath();
    cout << new_filename << endl;
    if (new_filename != "")
        flow2_gui.saveToFile(new_filename);
}

//--------------------------------------------------------------
void gFrameApp::onFlow2SettingsReload() {
    ofFileDialogResult load_result = ofSystemLoadDialog();
    string load_filename = load_result.getPath();
    if (load_filename != "")
        flow2_gui.loadFromFile(load_filename);
}

//--------------------------------------------------------------
void gFrameApp::setupConvert2BWShader() {
    convert2BWShader.load("shader/convert2bw.vert", "shader/convert2bw.frag");
}

//--------------------------------------------------------------
void gFrameApp::loadLUTFile(string filename) {
    ofxXmlSettings LUTxml;
    LUTxml.load(filename);
    
    for (int i = 1; i<=5;i++)
    {
        string pairName = "colorPair_" + ofToString(i);
        LUTxml.pushTag(pairName);
        
        ofParameter<ofColor> color1;
        color1.setName("brush");
        LUTxml.deserialize(color1);
        
        ofParameter<ofColor> color2;
        color2.setName("fluid");
        LUTxml.deserialize(color2);
        
        LUTxml.popTag();
        
        colorLUT[i-1][0] = color1;
        colorLUT[i-1][1] = color2;
    }
    
    for (int i = 0; i < 4;i++)
        {
            cout << (int) colorLUT[i][0].r << ":"<< (int)colorLUT[i][0].g << ":"<< (int)colorLUT[i][0].b << ":"<< (int)colorLUT[i][0].a << "|";
            cout << (int) colorLUT[i][1].r << ":"<< (int)colorLUT[i][1].g << ":"<< (int)colorLUT[i][1].b << ":"<< (int)colorLUT[i][1].a << endl;
            
        }
}

//--------------------------------------------------------------
//void gFrameApp::initLUTFile() {
//    ofxXmlSettings LUTxml;
//    for (int i = 1; i<=5;i++)
//    {
//        string pairName = "colorPair_" + ofToString(i);
//        
//        LUTxml.addTag(pairName);
//        LUTxml.pushTag(pairName);
//        ofParameter<ofColor> color1 = ofColor::seaGreen;
//        color1.setName("brush");
//        LUTxml.serialize(color1);
//        ofParameter<ofColor> color2 = ofColor::aquamarine;
//        color2.setName("fluid");
//        LUTxml.serialize(color2);
//        LUTxml.popTag();
//    }
//    LUTxml.save("lutxml.xml");
//}

//--------------------------------------------------------------
void gFrameApp::setColor(int colorLUTIndex, bool use_lut)
{
    localBrushColor = colorLUT[colorLUTIndex][0];
    
    if (use_lut)
        fluidColor = colorLUT[colorLUTIndex][1];
    else
        fluidColor = colorLUT[colorLUTIndex][0];
}

//--------------------------------------------------------------
void gFrameApp::onVerticalObstacleChanged(bool &state) {
    if (state)
    {
        //deactivate stroke as obstacle
        stroke_to_obstacle = false;
        //update obstacle buffer to image based obstacle
        simple_flow.addObstacle(obstacle.getTexture());
        simple_flow_2.addObstacle(obstacle.getTexture());
    } else {
        if (!stroke_to_obstacle)
        {
            simple_flow.resetObstacle();
            simple_flow_2.resetObstacle();
        }
    }
}
 
//--------------------------------------------------------------
void gFrameApp::onStrokeToObstacleChanged(bool &state) {
    if (state)
    {
        //deactivate vertical obstacle (the image based one)
        vertical_obstacle = false;
        
        //update obstacle buffer to stroke as obstacle
        obstacleFBO.begin(); ofClear(0); obstacleFBO.end();
        simple_flow.addObstacle(obstacleFBO.getTexture());
        simple_flow_2.resetObstacle();
    } else {
        if (!vertical_obstacle)
        {
            simple_flow.resetObstacle();
            simple_flow_2.resetObstacle();
        }
    }
}

//--------------------------------------------------------------
void gFrameApp::onSepFluidColorChanged(bool &state) {
    if (!state)
        fluidColor = localBrushColor;
}