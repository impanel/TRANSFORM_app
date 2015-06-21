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
    //mKinectHeightImage.allocate(KINECT_X, KINECT_Y);
    mOutputShapeImage.allocate(RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
    allPixels = new unsigned char[RELIEF_SIZE];
    
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            differenceHeight[i][j] = 0;
        }
    }
    
}

void TouchShapeObject::setup()
{
    
}

//----------------------------------------------------

void TouchShapeObject::update(float dt)
{
    unsigned char * pixels;
    
    int lineSize = sizeof(char) * RELIEF_SIZE_Y;
    int x = RELIEF_SIZE_X / 2;
    int y = RELIEF_SIZE_Y / 2;
    unsigned char h = mPinHeightReceive[x * lineSize + y];
    
    int alX;
    if (x<16) {
        alX = PINBLOCK_0_X_OFFSET +x ;
    } else if (x<32){
        alX = PINBLOCK_1_X_OFFSET +x -16;
    } else {
        alX = PINBLOCK_2_X_OFFSET +x -32;
    }
    
    
    for (int i = 0; i < RELIEF_PHYSICAL_SIZE_X; i++) {
        for(int j = 0; j < RELIEF_PHYSICAL_SIZE_Y; j++){

            int d = ofDist(alX, y, i, j);
            if(d>15){ d = 15; };
                int dHeight = ofMap(d, 0, 15, (int)h, 210);
            allPixels[RELIEF_PHYSICAL_SIZE_X* j+ i] =  dHeight;
            
        }
    }
    
    
    allPixels[RELIEF_PHYSICAL_SIZE_X*y+alX]=210;
    
    
    // caliculate difference between send and receive value
    
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            int alX = i;
            if (i<16) {
                alX = PINBLOCK_0_X_OFFSET +i ;
            } else if (i<32){
                alX = PINBLOCK_1_X_OFFSET +i -16;
            } else {
                alX = PINBLOCK_2_X_OFFSET +i -32;
            }
            int output = int(allPixels[RELIEF_PHYSICAL_SIZE_X*j+alX]);
            int input = int(mPinHeightReceive[i * lineSize + j]);
            
            differenceHeight[i][j] =  output - input;
             if(i == RELIEF_SIZE_X/2-1 && j == RELIEF_SIZE_Y/2-1){
                cout << "diff :" << differenceHeight[i][j] << ", output :" << output << ", input :" << input << endl;
            }
            
        }
    }
    
    
    
}

//----------------------------------------------------

void TouchShapeObject::renderShape()
{
    mOutputShapeImage.draw(0,0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
    //smallerImage.draw(0, 0, RELIEF_PHYSICAL_SIZE_X, RELIEF_PHYSICAL_SIZE_Y);
}

//----------------------------------------------------

void TouchShapeObject::renderGraphics(int x, int y, int w, int h)
{
    mOutputShapeImage.draw(0,0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
}

//----------------------------------------------------

void TouchShapeObject::drawGuiScreen(int x, int y, int w, int h)
{
    int pixelSize = 5;
    
    ofPushMatrix();
    ofTranslate(630, 0);
    ofFill();
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        if(i==16 || i == 32){
            ofTranslate(5, 0);
        }
        
        int alX = i;
        if (i<16) {
            alX =  PINBLOCK_0_X_OFFSET +i ;
        } else if (i<32){
            alX = PINBLOCK_1_X_OFFSET +i -16 ;
        } else {
            alX = PINBLOCK_2_X_OFFSET +i  -32;
        }
        
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            int val = differenceHeight[i][j];
            if(val< 0){
                ofSetColor(ofMap(val, 0, -160, 0, 255),0,0);
            } else {
                ofSetColor(0,ofMap(val, 0, 160, 0, 255),0);
            }
            
            ofRect(i*pixelSize,j*pixelSize,pixelSize,pixelSize);
            
            
            int output = int(allPixels[RELIEF_PHYSICAL_SIZE_X*j+alX]);
            
            ofSetColor(0,0,ofMap(output, 0, 160, 0, 255));
            ofRect(i*pixelSize +270,j*pixelSize,pixelSize,pixelSize);
        }
    }
    
    ofTranslate(0, 130);
    for (int i = 0; i < RELIEF_PHYSICAL_SIZE_X; i++) {
        for(int j = 0; j < RELIEF_PHYSICAL_SIZE_Y; j++){
            
            int output = int(allPixels[RELIEF_PHYSICAL_SIZE_X*j+i]);
            ofSetColor(0,0,ofMap(output, 0, 160, 0, 255));
            ofRect(i*pixelSize,j*pixelSize,pixelSize,pixelSize);
        }
    }
    ofNoFill();
    ofSetColor(255, 0, 0);
    ofRect(PINBLOCK_0_X_OFFSET*pixelSize, 0, PINBLOCK_0_WIDTH*pixelSize, RELIEF_SIZE_Y*pixelSize);
    ofRect(PINBLOCK_1_X_OFFSET*pixelSize, 0, PINBLOCK_1_WIDTH*pixelSize, RELIEF_SIZE_Y*pixelSize);
    ofRect(PINBLOCK_2_X_OFFSET*pixelSize, 0, PINBLOCK_2_WIDTH*pixelSize, RELIEF_SIZE_Y*pixelSize);
    
    ofPopMatrix();
    
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