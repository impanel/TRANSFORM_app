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
    motionAccumulator.set(0, 0);
    direction.set(1, 0);
}

MoldedShape::MoldedShape(int id, MoldedShape *moldedShape) : id(id) {
    if (moldedShape == NULL) {
        throw "must pass live pointer to MoldedShape copy constructor";
    }

    for (int i = 0; i < MOLDED_SHAPE_DIM; i++) {
        for (int j = 0; j < MOLDED_SHAPE_DIM; j++) {
            heightMap[i][j] = moldedShape->heightMap[i][j];
        }
    }

    calculateCenterOfVolume();

    x = moldedShape->x;
    y = moldedShape->y;
    speed = moldedShape->speed;
    direction = moldedShape->direction;
    motionAccumulator.set(moldedShape->motionAccumulator);
    decelerationAccumulator = moldedShape->decelerationAccumulator;
}

void MoldedShape::update() {
    if (speed > 0) {
        decelerationAccumulator++;
        motionAccumulator += direction.getNormalized() * speed;

        // when accumulators overflow in positive or negative direction, bump the corresponding value
        while (motionAccumulator.x > MOTION_ACCUMULATION_THRESHOLD) {
            x++;
            motionAccumulator.x-= MOTION_ACCUMULATION_THRESHOLD;
        }
        while (motionAccumulator.x < -MOTION_ACCUMULATION_THRESHOLD) {
            x--;
            motionAccumulator.x+= MOTION_ACCUMULATION_THRESHOLD;
        }
        while (motionAccumulator.y > MOTION_ACCUMULATION_THRESHOLD) {
            y++;
            motionAccumulator.y-= MOTION_ACCUMULATION_THRESHOLD;
        }
        while (motionAccumulator.y < -MOTION_ACCUMULATION_THRESHOLD) {
            y--;
            motionAccumulator.y+= MOTION_ACCUMULATION_THRESHOLD;
        }
        while (decelerationAccumulator > DECELERATION_ACCUMULATION_THRESHOLD) {
            speed--;
            decelerationAccumulator -= DECELERATION_ACCUMULATION_THRESHOLD;
        }
    }
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

int MoldedShape::getId() {
    return id;
}

void MoldedShape::setDirection(float angleInDegrees) {
    direction = ofVec2f(1, 0).rotated(angleInDegrees);
}

Boolean MoldedShape::containsLocation(int _x, int _y) {
    _x -= x;
    _y -= y;

    return (_x < MOLDED_SHAPE_DIM && _y < MOLDED_SHAPE_DIM && heightMap[_x][_y]);
}

Boolean MoldedShape::overlapsShape(MoldedShape *otherShape) {
    int minX = max(x, otherShape->x);
    int minY = max(y, otherShape->y);
    int maxX = min(x + MOLDED_SHAPE_DIM, otherShape->x + MOLDED_SHAPE_DIM);
    int maxY = min(y + MOLDED_SHAPE_DIM, otherShape->y + MOLDED_SHAPE_DIM);
    for (int i = minX; i < maxX; i++) {
        for (int j = minY; j < maxY; j++) {
            if (containsLocation(i, j) && otherShape->containsLocation(i, j)) {
                return true;
            }
        }
    }

    return false;
}