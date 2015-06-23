//
//  MoldShapeObject.h
//  basicExample
//
//  Created by Daniel Windham on 6/22/15.
//
//

#ifndef __basicExample__MoldShapeObject__
#define __basicExample__MoldShapeObject__

#include <iostream>
#include "ShapeObject.h"
#include "Constants.h"
#include "ShapeIOManager.h"
#include "ofxOpenCv.h"

class MoldShapeObject : public ShapeObject {
    
#define NUM_FILTER_FRAME 6
    
public:
    MoldShapeObject();
    void setup();
    void update(float dt);
    void renderShape();
    void renderGraphics(int x, int y, int w, int h);
    void drawGuiScreen(int x, int y, int w, int h);
    void setPinHeight(unsigned char pinHeightReceive[RELIEF_SIZE_X][RELIEF_SIZE_Y]){
        mPinHeightReceive = (unsigned char *) pinHeightReceive;
    };
    //void setImageWarper(ImageWarper * pImageWarper) {mImageWarper = pImageWarper;};
    void setTableValuesForShape(ShapeIOManager *pIOManager);
    unsigned char* getPixels();
    
    int xCoordinateShift(int num);
    
    
    
    string get_shape_name() {return shape_name; };
    string shape_name = "Mold";
    
    
private:
    unsigned char *mPinHeightReceive;
   // ImageWarper * mImageWarper;
    ofxCvGrayscaleImage mOutputShapeImage, smallerImage;
    
    unsigned char* allPixels;
    int differenceHeight[RELIEF_SIZE_X][RELIEF_SIZE_Y];
    int filterFrame = NUM_FILTER_FRAME;
    
    unsigned char allPixels_store[RELIEF_SIZE][NUM_FILTER_FRAME];
    
    Boolean isTouched[RELIEF_SIZE_X][RELIEF_SIZE_Y];
};


#endif /* defined(__basicExample__MoldShapeObject__) */
