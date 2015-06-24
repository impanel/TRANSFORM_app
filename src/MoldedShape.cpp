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
    position.set(0, 0);
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

    position = moldedShape->position;
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
            position.x++;
            motionAccumulator.x-= MOTION_ACCUMULATION_THRESHOLD;
        }
        while (motionAccumulator.x < -MOTION_ACCUMULATION_THRESHOLD) {
            position.x--;
            motionAccumulator.x+= MOTION_ACCUMULATION_THRESHOLD;
        }
        while (motionAccumulator.y > MOTION_ACCUMULATION_THRESHOLD) {
            position.y++;
            motionAccumulator.y-= MOTION_ACCUMULATION_THRESHOLD;
        }
        while (motionAccumulator.y < -MOTION_ACCUMULATION_THRESHOLD) {
            position.y--;
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

Boolean MoldedShape::containsLocation(ofVec2f location) {
    location -= position;
    int x = location.x;
    int y = location.y;

    if (x < 0 || x >= MOLDED_SHAPE_DIM || y < 0 || y >= MOLDED_SHAPE_DIM) {
        return false;
    } else {
        return heightMap[x][y] > 0;
    }
}

ofVec2f MoldedShape::vectorFromNearestEdgeToLocation(ofVec2f location) {
    float nearestDistance = -1;
    ofVec2f vec;
    for (int i = 0; i < MOLDED_SHAPE_DIM; i++) {
        for (int j = 0; j < MOLDED_SHAPE_DIM; j++) {
            if (heightMap[i][j] > 0) {
                ofVec2f candidateVec = position + ofVec2f(i, j);
                float candidateDistance = candidateVec.distance(location);
                if (candidateDistance < nearestDistance || nearestDistance < 0) {
                    nearestDistance = candidateDistance;
                    vec = candidateVec;
                }
            }
        }
    }
    return vec;
}

float MoldedShape::distanceFromLocation(ofVec2f location) {
    return vectorFromNearestEdgeToLocation(location).length();
}

Boolean MoldedShape::overlapsShape(MoldedShape *otherShape) {
    int minX = max(position.x, otherShape->position.x);
    int minY = max(position.y, otherShape->position.y);
    int maxX = min(position.x + MOLDED_SHAPE_DIM, otherShape->position.x + MOLDED_SHAPE_DIM);
    int maxY = min(position.y + MOLDED_SHAPE_DIM, otherShape->position.y + MOLDED_SHAPE_DIM);
    for (int i = minX; i < maxX; i++) {
        for (int j = minY; j < maxY; j++) {
            if (containsLocation(ofVec2f(i, j)) && otherShape->containsLocation(ofVec2f(i, j))) {
                return true;
            }
        }
    }

    return false;
}