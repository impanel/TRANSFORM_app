//
//  TouchShapeObject.cpp
//  basicExample
//
//  Created by Ken Nakagaki on 6/16/15.
//
//

#include "TouchShapeObject.h"


TouchShapeObject::TouchShapeObject()
{
    mOutputShapeImage.allocate(RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
    allPixels = new unsigned char[RELIEF_SIZE];
};

void TouchShapeObject::setup()
{
    
}

//----------------------------------------------------

void TouchShapeObject::update(float dt)
{
    unsigned char * pixels;

    //ofxCvGrayscaleImage smallerImage;
    smallerImage.allocate(RELIEF_PHYSICAL_SIZE_X, RELIEF_PHYSICAL_SIZE_Y);
    
    int x = RELIEF_SIZE_X / 2;
    int y = RELIEF_SIZE_Y / 2;
    int lineSize = sizeof(char) * RELIEF_SIZE_Y;
    unsigned char controlPinHeight = mPinHeightReceive[x * lineSize + y];
    cout << "height: "<< (int) controlPinHeight << endl;

    // depress pin heights depending on their distance from the control pin
    for (int x = 0; x < RELIEF_PHYSICAL_SIZE_X; x++) {
        for(int y = 0; y < RELIEF_PHYSICAL_SIZE_Y; y++){
            int distance = ofDist(RELIEF_PHYSICAL_SIZE_X/2, RELIEF_PHYSICAL_SIZE_Y/2, x, y);
            distance = min(15, distance);
            int outputHeight = ofMap(distance, 0, 15, (int) controlPinHeight, 230);
            allPixels[RELIEF_PHYSICAL_SIZE_X * y + x] = outputHeight;
        }
    }

    // but force the control pin back up
    allPixels[RELIEF_PHYSICAL_SIZE_X * RELIEF_PHYSICAL_SIZE_Y/2 + RELIEF_PHYSICAL_SIZE_X/2] = 230;
}

//----------------------------------------------------

void TouchShapeObject::renderShape()
{
    mOutputShapeImage.draw(0,0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
}

//----------------------------------------------------

void TouchShapeObject::renderGraphics(int x, int y, int w, int h)
{
    mOutputShapeImage.draw(0,0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
}

//----------------------------------------------------

void TouchShapeObject::drawGuiScreen(int x, int y, int w, int h)
{
    
}

//----------------------------------------------------

void TouchShapeObject::setTableValuesForShape(ShapeIOManager *pIOManager)
{
    pIOManager->set_max_speed(200);
    pIOManager->set_gain_p(1.5f);
    pIOManager->set_gain_i(0.045f);
    pIOManager->set_max_i(25);
    pIOManager->set_deadzone(2);
};

//----------------------------------------------------

unsigned char* TouchShapeObject::getPixels()
{
    return allPixels;
}