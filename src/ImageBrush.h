//
//  ImageBrush.h
//  gFrame
//
//  Created by Robert Albert on 24.08.15.
//
//

#ifndef __gFrame__ImageBrush__
#define __gFrame__ImageBrush__
#include <iostream>

#include "ofMain.h"
#include "GPoint.h"

#ifndef Included_BaseStyle_H
#define Included_BaseStyle_H
#include "BaseStyle.h"
#endif

class ImageBrush : public BaseStyle {
public:
    ImageBrush();
    ofParameterGroup parameters;
    void render(vector<GPoint> &points, int width, int height);
    void renderFBO();
    
    void clear();

private:
    ofParameter<int> min_brush_width;
    ofParameter<int> max_brush_width;

    ofImage brush_image;
    
    ofFbo fbo;
    
    int max_distance;
    
    int brush_width_start, brush_width_end;
    map<int,int> width_start;
};
#endif /* defined(__gFrame__ImageBrush__) */
