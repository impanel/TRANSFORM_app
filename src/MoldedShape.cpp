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

void MoldedShape::setVelocity(ofVec2f velocity) {
    direction = velocity.normalize();
    speed = velocity.length();
}

ofVec2f MoldedShape::getVelocity() {
    return ofVec2f(direction * speed);
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

ofVec2f MoldedShape::nearestEdgeToLocation(ofVec2f location) {
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

ofVec2f MoldedShape::vectorFromNearestEdgeToLocation(ofVec2f location) {
    location - nearestEdgeToLocation(location);
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

Depression MoldedShape::netDepressionVector(int depressionMap[RELIEF_SIZE_X][RELIEF_SIZE_Y], int allowedDistance) {
    vector<Depression> depressions;
    for (int i = 0; i < RELIEF_SIZE_X; i++) {
        for (int j = 0; j < RELIEF_SIZE_Y; j++) {
            if (distanceFromLocation(ofVec2f(i, j)) <= allowedDistance) {
                if (depressionMap[i][j] > 0) {
                    depressions.push_back(Depression(ofVec2f(i, j), depressionMap[i][j]));
                };
            }
        }
    }
    return Depression::sumOfDepressions(depressions);
}

void MoldedShape::updateVelocityFromDepressions(int depressionMap[RELIEF_SIZE_X][RELIEF_SIZE_Y]) {
    float speedPerVolumePerFrame = 15.0 / 50.0 / ofGetFrameRate();
    Depression netDepression = netDepressionVector(depressionMap);
    if (netDepression.volume > 0) {
        ofVec2f depressionCenter = netDepression.getPosition();
        ofVec2f relativeCenter = depressionCenter - centerOfVolume;
        if (relativeCenter.length() > 1) {
            ofVec2f impulse = -relativeCenter.normalize() * netDepression.volume * speedPerVolumePerFrame;
            setVelocity(getVelocity() + impulse);
        }
    }
}