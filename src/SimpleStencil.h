//
//  SimpleStencil.h
//  2DStencil
//
//  Created by Robert Albert on 08.09.15.
//
//

#ifndef ___DStencil__SimpleStencil__
#define ___DStencil__SimpleStencil__

#include <stdio.h>
#include "ofMain.h"
#include <algorithm>

struct StencilPrint {
    ofImage image;
    float timeStamp;
    float alpha;
    string text;
};

enum StencilSize { MEDIUM, LARGE, XTRALARGE };

class SimpleStencil {
    
public:
    void setup(int _width, int _height, string _fontName = "StencilStd.otf", float _fadingTime = 2.0, float _fadeOutAfterSeconds = 10.0);
    
    void updateText(string _text);
    void updateStencil(int x, int y);
    void draw();
    
    void addNewPrint();
    void drawLastStencilPrints();
    void clearFBOs();
    
    void setPlaceMode(bool _placeMode) { placeMode = _placeMode; }
    bool getPlaceMode() {return placeMode;}
    
    void setStencilSize(StencilSize _size) {size = _size; cout << size << endl;}
    void setInvert(bool _invert) {invert = _invert; cout << "invert: " << invert << endl;}
    
    void setFadingTime(float _fadingTime) {fadingTime = _fadingTime;}
    void setFadeOutAfterSeconds(float _fadeOutAfterSeconds) {fadeOutAfterSeconds = _fadeOutAfterSeconds;}
    
    void startCanvas();
    void finishCanvas();
    
    //for debugging
    ofTexture &getStencilTex() {return stencilFBO.getTextureReference();}
    ofTexture &getCanvasTex() {return canvasFBO.getTextureReference();}
    ofTexture &getComposedTex() {return composedFBO.getTextureReference();}
    
private:
    //creating the Stencil itself
    ofFbo stencilFBO;
    ofTrueTypeFont font;
    ofPoint center;
    ofRectangle bounds;
    ofShader cutoutShader;
    string text;
    string fontName;
    bool invert;
    bool old_invert;
    
    StencilSize size;

    bool placeMode;
    
    vector<StencilPrint> oldStencilPrints;
    ofShader fadeoutShader;
    string oldText;
    float fadeOutAfterSeconds;
    float fadingTime;
    
    ofFbo canvasFBO;
    ofFbo composedFBO;
    int width, height;
};

#endif /* defined(___DStencil__SimpleStencil__) */


//pointer to fbo object
//one fbo for foreground, one for background
//if new stencil is triggered all the sofar draw stuff is stored to background
// other possibillity stack of textures: each one holds the draw stuff of one stencil "round"