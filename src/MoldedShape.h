
//
//  MoldedShape.h
//  basicExample
//
//  Created by Daniel Windham on 6/23/15.
//
//

#ifndef __basicExample__MoldedShape__
#define __basicExample__MoldedShape__

#include <iostream>
#include "ShapeObject.h"
#include "Constants.h"
#include "ShapeIOManager.h"
#include "ofxOpenCv.h"


#define MOLDED_SHAPE_DIM 16

class MoldedShape {
public:
    MoldedShape(int id, int _heightMap[MOLDED_SHAPE_DIM][MOLDED_SHAPE_DIM]);
    MoldedShape(int id, MoldedShape *moldedShape);
    int getId() {return id;};
    Boolean containsLocation(int _x, int _y);
    Boolean overlapsShape(MoldedShape *otherShape);

    int x = 0;
    int y = 0;
    int heightMap[MOLDED_SHAPE_DIM][MOLDED_SHAPE_DIM];
    int totalVolume = 0;
    ofVec2f centerOfVolume;
    int speed = 0;
    int direction = 0; // direction of movement in degrees

private:
    void calculateCenterOfVolume();
    int id;
};


#endif /* defined(__basicExample__MoldedShape__) */