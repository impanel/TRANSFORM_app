//
//  MoldedShape.cpp
//  basicExample
//
//  Created by Daniel Windham on 6/23/15.
//
//

#include "MoldedShape.h"


MoldedShape::MoldedShape(int id, int _heightMap[MOLDED_SHAPE_DIM][MOLDED_SHAPE_DIM]) : id(id) {
    for (int i = 0; i < MOLDED_SHAPE_DIM; i++) {
        for (int j = 0; j < MOLDED_SHAPE_DIM; j++) {
            heightMap[i][j] = _heightMap[i][j];
        }
    }

    calculateCenterOfVolume();
}

// calculate the volumetric center (analogue of center of mass) by weighting each location by
// its volume and dividing the sum by the total volume
void MoldedShape::calculateCenterOfVolume() {
    totalVolume = 0;
    ofVec2f volumeTorque(0, 0); // 2D spatial integral of x * vol(x)
    for (int i = 0; i < MOLDED_SHAPE_DIM; i++) {
        for (int j = 0; j < MOLDED_SHAPE_DIM; j++) {
            int volume = heightMap[i][j];
            totalVolume += volume;
            volumeTorque += ofVec2f(i, j) * volume;
        }
    }

    centerOfVolume = volumeTorque / totalVolume;
}