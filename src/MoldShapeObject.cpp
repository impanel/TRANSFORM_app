//
//  MoldShapeObject.cpp
//  basicExample
//
//  Created by Daniel Windham on 6/22/15.
//
//

#include "MoldShapeObject.h"


MoldShapeObject::MoldShapeObject()
{
    //mKinectHeightImage.allocate(KINECT_X, KINECT_Y);
    mOutputShapeImage.allocate(RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
    allPixels = new unsigned char[RELIEF_SIZE];
    
    
    for(int i = 0; i< RELIEF_SIZE; i++){
        for(int j = 0; j< filterFrame; j++){
            allPixels_store[i][j] = 210;
        }
    }
    
    
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            differenceHeight[i][j] = 0;
        }
    }
    
}

MoldShapeObject::~MoldShapeObject() {
    moldedShapes.erase(moldedShapes.begin());
}

void MoldShapeObject::setup()
{
    
}

//----------------------------------------------------

void MoldShapeObject::update(float dt)
{
    unsigned char * pixels;
    int lineSize = sizeof(char) * RELIEF_SIZE_Y;
    
    
    //store data
    for(int i = 0; i < RELIEF_SIZE; i++){
        for(int j = filterFrame-1; j > 0; j--){
            allPixels_store[i][j] = allPixels_store[i][j-1];
        }
        allPixels_store[i][0] = allPixels[i];
    }
    
    //check each pin's difference over time - if it's flat or not
    Boolean flat[RELIEF_SIZE_X][RELIEF_SIZE_Y];
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        int XShift =  xCoordinateShift(i);

        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            
            int maxVal=0, minVal=255;
            for(int k =0; k < filterFrame; k++){
                maxVal = MAX(maxVal,(int)allPixels_store[RELIEF_PHYSICAL_SIZE_X*j+XShift][k]);
                minVal = MIN(minVal,(int)allPixels_store[RELIEF_PHYSICAL_SIZE_X*j+XShift][k]);
            }
            if (abs(maxVal - minVal) > 2) {
                flat[i][j] = false;
            } else {
                flat[i][j] = true;
            }
            
        }
    }

    
    
    // caliculate difference between send and receive value
    
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            int XShift =  xCoordinateShift(i);;

            int output = int(allPixels[RELIEF_PHYSICAL_SIZE_X*j+XShift]);
            int input = int(mPinHeightReceive[i * lineSize + j]);
            
            differenceHeight[i][j] =  output - input;

            
        }
    }

    Boolean someoneIsTouched = false;
    int minXTouched = RELIEF_SIZE_X;
    int minYTouched = RELIEF_SIZE_Y;

    // determine if each pin were touched or not
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            if(flat[i][j]){
                if (abs(differenceHeight[i][j]) > 21) {
                    isTouched[i][j] = true;
                    someoneIsTouched = true;
                    minXTouched = min(minXTouched, i);
                    minYTouched = min(minYTouched, j);
                } else {
                    isTouched[i][j] = false;
                }
                
                
            } else {
                isTouched[i][j] = false;
            }
        }
    }
    
    // determine what object resides in each pin
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            // initialize all values to -1
            holdsObject[i][j] = -1;
        }
    }

    if (someoneIsTouched) {
        // record new shape
        if (isRecording) {
            int depression[MOLDED_SHAPE_DIM][MOLDED_SHAPE_DIM];
            for (int i = 0; i < MOLDED_SHAPE_DIM; i++) {
                for (int j = 0; j < MOLDED_SHAPE_DIM; j++) {
                    int x = i + minXTouched;
                    int y = j + minYTouched;
                    if (x < RELIEF_SIZE_X && y < RELIEF_SIZE_Y && isTouched[x][y]) {
                        depression[i][j] = differenceHeight[x][y];
                    } else {
                        depression[i][j] = 0;
                    }
                }
            }
            
            MoldedShape *newShape = new MoldedShape(getUID(), depression);
            newShape->position.set(minXTouched, minYTouched);
            moldedShapes.push_back(newShape);

        } else if (moldedShapes.size() > 0) {
            // is an existing mold touched?
            MoldedShape *generator = NULL;
            ofVec2f touchedShapeLocation;
            for (vector<MoldedShape *>::iterator iter = moldedShapes.begin(); iter != moldedShapes.end(); iter++) {
                for (int i = (*iter)->position.x; i < (*iter)->position.x + MOLDED_SHAPE_DIM; i++) {
                    if (i >= RELIEF_SIZE_X || generator != NULL) { break; }
                    for (int j = (*iter)->position.y; j < (*iter)->position.y + MOLDED_SHAPE_DIM; j++) {
                        if (j >= RELIEF_SIZE_Y) { break; }
                        if (isTouched[i][j] && (*iter)->containsLocation(ofVec2f(i, j))) {
                            touchedShapeLocation.set(i, j);
                            generator = *iter;
                            break;
                        }
                    }
                }
                if (generator != NULL) {
                    break;
                }
            }
            // if so, generate copies that move towards any other touched locations
            if (generator != NULL) {
                for (int i = 0; i < RELIEF_SIZE_X; i++) {
                    for (int j = 0; j < RELIEF_SIZE_Y; j++) {
                        if (isTouched[i][j] && !generator->containsLocation(ofVec2f(i, j))) {
                            ofVec2f duplicationSpawnPoint(i, j);
                            if (isNearRecentDuplicationPoint(duplicationSpawnPoint)) {
                                break;
                            }
                            MoldedShape *duplicate = duplicateMoldedShape(generator);
                            ofVec2f locationDifference = duplicationSpawnPoint - touchedShapeLocation;
                            duplicate->direction = locationDifference.normalized();
                            duplicate->speed = locationDifference.length() / 3 + 10;
                            registerRecentDuplicationPoint(duplicationSpawnPoint);
                        }
                    }
                }
            }
        }
    }
    
    //*** MODE: Every Pin Input ***//

    int midHeight = (LOW_THRESHOLD + HIGH_THRESHOLD) / 2;
    
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            allPixels[RELIEF_PHYSICAL_SIZE_X* j+ xCoordinateShift(i)] = midHeight;
        }
    }

    updateMoldedShapes();
    updateRecentDuplicationPointsRegistry();

    // draw molded shapes into allPixels array
    for (vector<MoldedShape *>::iterator itr = moldedShapes.begin(); itr != moldedShapes.end(); itr++) {
        for (int i = 0; i < MOLDED_SHAPE_DIM; i++) {
            int x = i + (*itr)->position.x;
            if (x >= RELIEF_SIZE_X) { break; }
            for (int j = 0; j < MOLDED_SHAPE_DIM; j++) {
                int y = j + (*itr)->position.y;
                if (y >= RELIEF_SIZE_Y) { break; }
                int shapeHeight = (*itr)->heightMap[i][j];
                int proposedHeight = min(midHeight + shapeHeight, HIGH_THRESHOLD);
                int index = RELIEF_PHYSICAL_SIZE_X * y + xCoordinateShift(x);
                if (proposedHeight > (int) allPixels[index]) {
                    allPixels[index] = proposedHeight;
                }
            }
        }
    }

    // clear isRecording flag
    isRecording = false;

    // exit from here... no need for the rest
    return;

    int rangeDef = 10; //range of deformation
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            if (isTouched[i][j]) {
                unsigned char h = MAX(LOW_THRESHOLD,mPinHeightReceive[i * lineSize + j]);
                for (int ii = MAX(0,i - rangeDef); ii< MIN(RELIEF_SIZE_X,i+rangeDef); ii++) {
                    for (int jj = MAX(0,j - rangeDef); jj< MIN(RELIEF_SIZE_Y,j+rangeDef); jj++) {
                    int d = ofDist(i, j, ii, jj);
                    if(d>rangeDef){ d = rangeDef; };
                    int dHeight = ofMap(d, 0, rangeDef, (int)h, HIGH_THRESHOLD);
                    dHeight = MAX(LOW_THRESHOLD, dHeight);
                    allPixels[RELIEF_PHYSICAL_SIZE_X* jj+ xCoordinateShift(ii)] =  MIN(allPixels[RELIEF_PHYSICAL_SIZE_X* jj+ xCoordinateShift(ii)],dHeight);
                    }
                }
            }
            
        }
    }
    
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            if (isTouched[i][j]) {
                unsigned char h = MAX(LOW_THRESHOLD,mPinHeightReceive[i * lineSize + j]);
                h = MIN((int)h+35,HIGH_THRESHOLD);
                allPixels[RELIEF_PHYSICAL_SIZE_X* j+ xCoordinateShift(i)] = HIGH_THRESHOLD; //(int)h;
                for(int k = 0; k < filterFrame; k++){
                    allPixels_store[RELIEF_PHYSICAL_SIZE_X* j+ xCoordinateShift(i)][k] = HIGH_THRESHOLD; //(int)h;
                }
            }
        }
    }
    
    
    
    //*** MODE: One Center Pin Input ***//
//    int x = RELIEF_SIZE_X / 2;
//    int y = RELIEF_SIZE_Y / 2;
//    unsigned char h = MAX(LOW_THRESHOLD,mPinHeightReceive[x * lineSize + y]);
//    
//    int XShift = xCoordinateShift(x);
//    
//    for (int i = 0; i < RELIEF_PHYSICAL_SIZE_X; i++) {
//        for(int j = 0; j < RELIEF_PHYSICAL_SIZE_Y; j++){
//            
//            int d = ofDist(XShift, y, i, j);
//            if(d>15){ d = 15; };
//            int dHeight = ofMap(d, 0, 15, (int)h, HIGH_THRESHOLD);
//            dHeight = MAX(LOW_THRESHOLD, dHeight);
//            allPixels[RELIEF_PHYSICAL_SIZE_X* j+ i] =  dHeight;
//        }
//    }
//    
//    allPixels[RELIEF_PHYSICAL_SIZE_X*y+XShift]=HIGH_THRESHOLD;
    
    
    
}

//----------------------------------------------------

void MoldShapeObject::renderShape()
{
    mOutputShapeImage.draw(0,0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
}

//----------------------------------------------------

void MoldShapeObject::renderGraphics(int x, int y, int w, int h)
{
    mOutputShapeImage.draw(0,0, RELIEF_PROJECTOR_SIZE_X, RELIEF_PROJECTOR_SIZE_Y);
}

//----------------------------------------------------

void MoldShapeObject::drawGuiScreen(int x, int y, int w, int h)
{
    int pixelSize = 5;
    
    ofPushMatrix();
    ofTranslate(630, 0);
    ofFill();
    for(int i = 0; i< RELIEF_SIZE_X; i++){
        if(i==16 || i == 32){
            ofTranslate(5, 0);
        }
        
        int XShift = xCoordinateShift(i);
        
        for(int j = 0; j< RELIEF_SIZE_Y; j++){
            int val = differenceHeight[i][j];
            if(val< 0){
                ofSetColor(ofMap(val, 0, -160, 0, 255),0,0);
            } else {
                ofSetColor(0,ofMap(val, 0, 160, 0, 255),0);
            }
            
            ofRect(i*pixelSize,j*pixelSize,pixelSize,pixelSize);
            if (isTouched[i][j]) {
                ofNoFill();
                ofSetColor(255, 0, 0);
                ofRect(i*pixelSize,j*pixelSize,pixelSize,pixelSize);
                ofFill();
            }
            
            
            int output = int(allPixels[RELIEF_PHYSICAL_SIZE_X*j+XShift]);
            
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

void MoldShapeObject::setTableValuesForShape(ShapeIOManager *pIOManager)
{
    if (false && isRecording) {
        pIOManager->set_max_speed(0);
        pIOManager->set_gain_p(1.5f);
        pIOManager->set_gain_i(0.045f);
        pIOManager->set_max_i(25);
        pIOManager->set_deadzone(2);
    } else {
        pIOManager->set_max_speed(200);
        pIOManager->set_gain_p(1.5f);
        pIOManager->set_gain_i(0.045f);
        pIOManager->set_max_i(25);
        pIOManager->set_deadzone(2);
    }
};

//----------------------------------------------------



unsigned char* MoldShapeObject::getPixels()
{
    return allPixels;
}

int MoldShapeObject::xCoordinateShift (int num){
    int val = num;
    if (num<16) {
        val =  PINBLOCK_0_X_OFFSET +num ;
    } else if (num<32){
        val = PINBLOCK_1_X_OFFSET +num -16 ;
    } else {
        val = PINBLOCK_2_X_OFFSET +num  -32;
    }
    return val;
}

MoldedShape *MoldShapeObject::getMoldedShapeById(int id) {
    for (vector<MoldedShape *>::iterator iter = moldedShapes.begin(); iter != moldedShapes.end(); iter++) {
        if ((*iter)->getId() == id) {
            return (*iter);
        }
    }
    return NULL;
}

MoldedShape *MoldShapeObject::getMoldedShapeByIndex(int id) {
    if (id < moldedShapes.size()) {
        return moldedShapes.at(id);
    } else {
        return NULL;
    }
}

MoldedShape *MoldShapeObject::duplicateMoldedShape(MoldedShape *shape) {
    MoldedShape *newShape = new MoldedShape(getUID(), shape);
    newShape->speed = 10 + (rand() % 15);
    newShape->setDirection(rand() % 360);
    moldedShapes.push_back(newShape);
    return newShape;
}

void MoldShapeObject::updateMoldedShapes() {
    for (vector<MoldedShape *>::iterator iter = moldedShapes.begin(); iter != moldedShapes.end(); iter++) {
        (*iter)->update();
        (*iter)->updateVelocityFromDepressions(differenceHeight);
    }
}

bool MoldShapeObject::isNearRecentDuplicationPoint(ofVec2f point) {
    int allowedDistance = 5;
    for (vector<pair<ofVec2f, int> >::iterator iter = recentDuplicationPoints.begin(); iter != recentDuplicationPoints.end(); iter++) {
        if (point.distanceSquared((*iter).first) < allowedDistance * allowedDistance) {
            return true;
        }
    }
    return false;
}

void MoldShapeObject::registerRecentDuplicationPoint(ofVec2f duplicationPoint) {
    recentDuplicationPoints.push_back(pair<ofVec2f, int>(duplicationPoint, 0));
}

// increment all points in the registry, deleting those that have grown old
void MoldShapeObject::updateRecentDuplicationPointsRegistry() {
    int removalAge = 100;
    for (vector<pair<ofVec2f, int> >::iterator iter = recentDuplicationPoints.begin(); iter != recentDuplicationPoints.end(); /* custom incrementing */) {
        if ((*iter).second < removalAge) {
            (*iter).second++;
            iter++;
        } else {
            iter = recentDuplicationPoints.erase(iter);
        }
    }
}