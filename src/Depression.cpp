//
//  Depression.cpp
//  basicExample
//
//  Created by Daniel Windham on 6/24/15.
//
//

#include "Depression.h"


Depression::Depression(ofVec2f position, int volume, bool interpretAsTorque) : volume(volume) {
    if (interpretAsTorque) {
        torque = position;
    } else {
        torque = position * volume;
    }
}

Depression::Depression(Depression const &depression) {
    volume = depression.volume;
    torque = depression.torque;
}

ofVec2f Depression::getPosition() {
    return torque / volume;
}

Depression & Depression::operator=(const Depression &depression) {
    torque = depression.torque;
    volume = depression.volume;
    return *this;
}

Depression Depression::operator+(const Depression &depression) {
    return Depression(torque + depression.torque, volume + depression.volume, true);
}

Depression & Depression::operator+=(const Depression &depression) {
    torque += depression.torque;
    volume += depression.volume;
    return *this;
}

Depression Depression::sumOfDepressions(vector<Depression> depressions) {
    Depression sum(ofVec2f(0, 0), 0);
    for (vector<Depression>::iterator iter = depressions.begin(); iter != depressions.end(); iter++) {
        sum += *iter;
    }
    return Depression(sum);
}