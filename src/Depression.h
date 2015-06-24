//
//  Depression.h
//  basicExample
//
//  Created by Daniel Windham on 6/24/15.
//
//

#ifndef __basicExample__Depression__
#define __basicExample__Depression__

#include <iostream>
#include "ofMain.h"


class Depression {
public:
    Depression(ofVec2f position, int volume, bool interpretAsTorque=false);
    Depression(Depression const &depression);
    
    ofVec2f torque;
    int volume;
    ofVec2f getPosition();
    Depression & operator=(const Depression &depression);
    Depression operator+(const Depression &depression);
    Depression & operator+=(const Depression &depression);

    static Depression sumOfDepressions(vector<Depression> depressions);
};


#endif /* defined(__basicExample__Depression__) */