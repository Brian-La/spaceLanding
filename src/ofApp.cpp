
//--------------------------------------------------------------
//
//  Kevin M. Smith
//
//  Octree Test - startup scene
// 
//
//  Student Name:   Brian La
//  Date: 12/07/2020


#include "ofApp.h"
#include "Util.h"

//--------------------------------------------------------------
// setup scene, lighting, state and load geometry
//
void ofApp::setup(){
    
    ofSetFrameRate(60);     //set frame rate to 60
    
    if(bg.load("geo/starfield.jpg"))        //check for image
        bgLoaded = true;
    else
        cout << "Unable to load background image" << endl;
       
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
    
    //camera setups
	cam.setDistance(25);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    staticPos = cam.getPosition();      //save current position
    camPos = staticPos;         //initialize
    
    
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();
    ofEnableLighting();

	// setup rudimentary lighting 
	//
	initLightingAndMaterials();

	moon.loadModel("geo/moon-low-v1.obj");
	moon.setScaleNormalization(false);
    //moon.setRotation(0, -15, 1, 0, 0);      //flatten moon obj. file


	//  Create Octree for testing.
	//
	octree.create(moon.getMesh(0), 20);
	
	cout << "Number of Verts: " << moon.getMesh(0).getNumVertices() << endl;
    
    //set lander position default
    //
    
    //mouseIntersectPlane(ofVec3f(0, 0, 0), cam.getZAxis(), point);
    if (lander.loadModel("geo/lander.obj")) {  //dragInfo.files[0])
        lander.setScaleNormalization(false);
        lander.setScale(landerScale, landerScale, landerScale);        //scale downwards

        
        //lander.setPosition(1, 1, 0);
        lander.setPosition(position.x, position.y, position.z);     //set back terrain
        
        bLanderLoaded = true;
        bboxList.clear();
        for (int i = 0; i < lander.getMeshCount(); i++) {
            bboxList.push_back(Octree::meshBounds(lander.getMesh(i)));
        }

        cout << "Mesh Count: " << lander.getMeshCount() << endl;
        
        // set up bounding box for lander while we are at it
        //
        glm::vec3 min = lander.getSceneMin(landerScale);        //scale
        glm::vec3 max = lander.getSceneMax(landerScale);
        landerBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
        
        //load landerLight
        landerLight.setup();
        landerLight.setSpotlight();
        landerLight.setScale(.01);
        landerLight.setSpotlightCutOff(15);
        landerLight.rotate(-90, ofVec3f(1, 0, 0));
        landerLight.setAttenuation(2, .001, .001);
        landerLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
        landerLight.setDiffuseColor(ofFloatColor(1, 1, 1));
        landerLight.setSpecularColor(ofFloatColor(1, 1, 1));
        
        //load hoverLight
        hoverLight.setup();
        hoverLight.enable();
        hoverLight.setPosition(ofVec3f(0, 30, 0));      //default
        hoverLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
        hoverLight.setDiffuseColor(ofFloatColor(1, 1, 1));
        hoverLight.setSpecularColor(ofFloatColor(1, 1, 1));
        
        
        //colorLight
        colorLight.setup();
        colorLight.enable();
        colorLight.setSpotlight();
        colorLight.setScale(0.01);
        colorLight.setSpotlightCutOff(15);
        colorLight.setPosition(ofVec3f(0, 5, -5));      //default
        colorLight.setAmbientColor(ofFloatColor(0, 1, 0));
        colorLight.setDiffuseColor(ofFloatColor(0, 1, 0));
        //colorLight.setSpecularColor(ofFloatColor(1, 1, 1));
        
    }
    else
        cout << "Error: Can't load model" << endl;
    
}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    //emitter update and re-position
    emitter.update();
    emitter.setCurrPos(lander.getPosition());
    
    //upon game start...can pause and select lander position
    if(gameStart) {
        
        checkCollisions(forces);
        if(fuel <= 0) {
            gameStart = false;
            gameOver = true;
            cout << "OUT OF FUEL";
        }

        
        
        //position, velocity, degVelocity, degAcceleration, rotation deg, force deg
        updateForce(position, velocity, rotation, degVeloc, degForce);
        lander.setPosition(position.x, position.y, position.z);     //set new position
        lander.setRotation(0, rotation, 0, 1, 0);       //set new rotation
        
        //switch based on camType - Brian L
        //
        switch(camType) {
            case trackCam:
                cam.setPosition(lander.getPosition() + ofVec3f(0, 2, -2));     //behind lander
                //cam.rotateDeg(-rotation, ofVec3f(0, 1, 0));     //reverse cam rotation if present
                cam.setTarget(lander.getPosition());
                break;
            case rotateCam:
                cam.setPosition(lander.getPosition() + ofVec3f(0, -0.1, 0));      //rotate based on UP vector
                cam.rotateDeg(degVeloc / ofGetFrameRate(), ofVec3f(0, 1, 0));
                break;
            case groundCam:
                cam.setPosition(lander.getPosition());      //set at position
                break;
            case traverseCam:
                cam.setPosition(camPos);        //set position to cam position
                break;
            default:
                cam.setPosition(staticPos);        //set position to cam position
                break;
        }
        
        //light switch
        if(lightOn) {
            landerLight.enable();
            landerLight.setPosition(lander.getPosition());      //set light position
        }
        else
            landerLight.disable();
        
        
        if(aglON)
            aglSensor(landerPoint);     //telemetric sensor
        
        //check game status
        if(gameOver) {
            status = "FAILED. PRESS = to RESTART.";      //display message
            statColor = ofColor::red;
        }
        else if(gameWin) {
            status = "SUCCESS. PRESS = to PLAY AGAIN.";      //display message
            statColor = ofColor::green;
        }
    }           //end gameStart
    else if(gameOver) {     //lander K'BOOM
        ofVec3f random = lander.getPosition() + ofVec3f(ofRandom(0, 1), ofRandom(0, 1), ofRandom(0, 1));
        lander.setPosition(random.x, random.y, random.z);     //set new position
        lander.setRotation(1, ofRandom(-10, 10), ofRandom(0, 1), ofRandom(0, 1), ofRandom(0, 1));       //set new rotation
    }
	
}
//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(ofColor::black);
    // draw screen data
    //
   
    //mission display
    string str3;
    str3 += "MISSION: " + status;
    ofSetColor(statColor);
    ofDrawBitmapString(str3, 0, 45);
    
    string str;
    str += "Frame Rate: " + std::to_string(ofGetFrameRate());
    ofSetColor(ofColor::white);
    ofDrawBitmapString(str, ofGetWindowWidth() -170, 15);
    
    //draw screen messages
    //
    //altitude display
    string str1;
    str1 += "Altitude: " + std::to_string(lander.getPosition().y - landerPoint.y);
    ofDrawBitmapString(str1, 0, 15);

    //fuel display
    string str2;
    str2 += "Fuel: " + std::to_string(fuel);
    ofDrawBitmapString(str2, 0, 30);

	cam.begin();
    
    //matrix to perform background starfield - Brian L
    ofPushMatrix();
    if(bgLoaded) {
        bg.draw(-250, -250, -250, 500, 500);
        ofRotateDeg(90, 0, 1, 0);
        bg.draw(-250, -250, -250, 500, 500);
        ofRotateDeg(90, 0, 1, 0);
        bg.draw(-250, -250, -250, 500, 500);
        ofRotateDeg(90, 0, 1, 0);
        bg.draw(-250, -250, -250, 500, 500);
    }
    ofPopMatrix();
    
    
	ofPushMatrix();
    emitter.draw();
    //ofEnableLighting();              // shaded mode
    moon.drawFaces();
    ofMesh mesh;
    if (bLanderLoaded) {
        lander.drawFaces();
        if (bDisplayBBoxes) {
            ofNoFill();
            ofSetColor(ofColor::white);
            for (int i = 0; i < lander.getNumMeshes(); i++) {
                ofPushMatrix();
                ofMultMatrix(lander.getModelMatrix());
                //ofRotate(-90, 1, 0, 0);
                Octree::drawBox(bboxList[i]);
                ofPopMatrix();
            }
        }

        if (bLanderSelected) {

            ofVec3f min = lander.getSceneMin(landerScale) + lander.getPosition();
            ofVec3f max = lander.getSceneMax(landerScale) + lander.getPosition();

            Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
            ofSetColor(ofColor::white);
            Octree::drawBox(bounds);

            // draw colliding boxes
            //
            ofSetColor(ofColor::lightBlue);
            for (int i = 0; i < colBoxList.size(); i++) {
                Octree::drawBox(colBoxList[i]);
            }
        }
    }



	if (bDisplayPoints) {                // display points as an option    
		glPointSize(3);
		ofSetColor(ofColor::green);
		moon.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}


	// recursively draw octree
	//
	//ofDisableLighting();
	int level = 0;
	ofNoFill();

    if (bDisplayOctree) {
        ofNoFill();
        ofSetColor(ofColor::white);
        octree.draw(numLevels, 0);
    }

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightBlue);
		ofDrawSphere(p, .02 * d.length());
	}
    
    // if point selected and mode on, draw sensor
    //
    if(aglON) {
        if (aglSelected) {
            ofVec3f a = octree.mesh.getVertex(aglNode.points[0]);
            ofVec3f b = a - lander.getPosition();
            ofSetColor(ofColor::orangeRed);
            ofDrawLine(lander.getPosition(), landerPoint);
            ofDrawSphere(a, .02 * b.length());
        }
    }

	ofPopMatrix();
    cam.end();
}


void ofApp::keyPressed(int key) {
    
	switch (key) {
        case '1':
            camType = staticCam;        //staticCam
            cam.setTarget(ofVec3f(0, 0, -1000));        //target land position once
            //cam.rotateDeg(-rotation, ofVec3f(0, 1, 0));     //reverse cam rotation if present
            break;
        case '2':
            camType = trackCam;     //track cam on
            break;
        case '3':
            camType = rotateCam;   //ground cam on
            break;
        case '4':
            camType = groundCam;   //rotate cam on
            cam.setTarget(lander.getPosition() + ofVec3f(0, -1000, 0));     //downward
            break;
        case '5':
            camType = traverseCam;
            cam.setTarget(lander.getPosition());        //target land position once
            break;
        case ' ':
            gameStart = !gameStart;       //start game toggle w/ spacebar
            break;
        case 'A':
        case 'a':
            aglON = !aglON;             //telemetry sensor toggle
            break;
        case 'W':
        case 'w':
            fuel--;                     //fuel reduction
            emitter.start();           //start emitter and one shot
            emitter.setOneShot(true);
            forces += ofVec3f(0, 0, 5);
            break;
        case 'S':
        case 's':
            fuel--;
            emitter.start();           //start emitter and one shot
            emitter.setOneShot(true);
            forces += ofVec3f(0, 0, -5);
            break;
        case OF_KEY_UP:
            fuel -= 2;
            emitter.start();           //start emitter and one shot
            emitter.setOneShot(true);
            forces += ofVec3f(0, 5, 0);     //scaled UP thrust force
            break;
        case OF_KEY_DOWN:
            fuel--;
            emitter.start();           //start emitter and one shot
            emitter.setOneShot(true);
            forces += ofVec3f(0, -5, 0);     //scaled DOWN thrust force
            break;
        case OF_KEY_RIGHT:
            fuel--;
            emitter.start();           //start emitter and one shot
            emitter.setOneShot(true);
            forces += ofVec3f(-5, 0, 0);     //scaled RIGHT thrust force
            break;
        case OF_KEY_LEFT:
            fuel--;
            emitter.start();           //start emitter and one shot
            emitter.setOneShot(true);
            forces += ofVec3f(5, 0, 0);     //scaled LEFT thrust force
            break;
        case 'Q':
        case 'q':
            degForce = -75;     //forward rotation force
            break;
        case 'E':
        case 'e':
            degForce += 75;       //backward rotation force
            break;
        case 'B':
        case 'b':
            bDisplayBBoxes = !bDisplayBBoxes;
            break;
        case 'C':
        case 'c':
            if (cam.getMouseInputEnabled()) cam.disableMouseInput();
            else cam.enableMouseInput();
            break;
        case 'F':
        case 'f':
            ofToggleFullscreen();
            break;
        case 'L':
        case 'l':
            lightOn = !lightOn;
            break;
        case 'O':
        case 'o':
            bDisplayOctree = !bDisplayOctree;
            break;
        case 'R':
        case 'r':
            cam.reset();
            break;
        case 'T':
        case 't':
            setCameraTarget();
            break;
        case 'V':
        case 'v':
            togglePointsDisplay();
            break;
        case OF_KEY_ALT:
            cam.enableMouseInput();
            bAltKeyDown = true;
            break;
        case OF_KEY_CONTROL:
            bCtrlKeyDown = true;
            break;
        case OF_KEY_SHIFT:
            break;
        case OF_KEY_DEL:
            break;
        default:
            break;
	}
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {
    
    //UI messages
    if(gameStart && key == ' ') {
        statColor = ofColor::yellow;
        status = "LAND SAFELY";
    }
    else if (!gameStart && key == ' ') {
        statColor = ofColor::white;
        status = "PAUSED";
    }
    else if ((gameWin || gameOver) && key == '=') {
        gameWin = false;
        gameOver = false;
        gameStart = true;
        fuel = 250;
        position = startingPosition;
        rotation = 0;
        statColor = ofColor::yellow;
        status = "LAND SAFELY";
        emitter.setEmitterType(DirectionalEmitter);
        impulseForce.set(0, 0, 0);
    }
    
	switch (key) {
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	default:
		break;

	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

    ofVec3f mousePos = ofVec3f(x, y, 0);
	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;


	// if rover is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = cam.getPosition();
		glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		ofVec3f min = lander.getSceneMin() + lander.getPosition();
		ofVec3f max = lander.getSceneMax() + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		bool hit = bounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			mouseDownPos = getMousePointOnPlane(lander.getPosition(), cam.getZAxis());
			mouseLastPos = mouseDownPos;
			bInDrag = true;
		}
		else {
			bLanderSelected = false;
		}
	}
	else {
		raySelectWithOctree(camPos);        //assign camPos
	}
}

bool ofApp::raySelectWithOctree(ofVec3f &pointRet) {
	ofVec3f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(mouse);
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
		Vector3(rayDir.x, rayDir.y, rayDir.z));

	pointSelected = octree.intersect(ray, octree.root, selectedNode);

	if (pointSelected) {
		pointRet = octree.mesh.getVertex(selectedNode.points[0]);       //point selected returned
	}
	return pointSelected;
}




//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	if (bInDrag) {

		glm::vec3 landerPos = lander.getPosition();

		glm::vec3 mousePos = getMousePointOnPlane(landerPos, cam.getZAxis());
		glm::vec3 delta = mousePos - mouseLastPos;
	
		landerPos += delta;
		lander.setPosition(landerPos.x, landerPos.y, landerPos.z);
		mouseLastPos = mousePos;

		ofVec3f min = lander.getSceneMin(landerScale) + lander.getPosition();
		ofVec3f max = lander.getSceneMax(landerScale) + lander.getPosition();

		Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

		colBoxList.clear();
		octree.intersect(bounds, octree.root, colBoxList);


	}
	else {
		raySelectWithOctree(camPos);
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bInDrag = false;
    //assign new position as lander position upon release after game pause
    position = lander.getPosition();
    bLanderSelected = false;        //erase drawing bounds
}



// Set the camera to use the selected point as it's new target
//  
void ofApp::setCameraTarget() {
    ofVec2f mouse(mouseX, mouseY);
    ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
    cam.setTarget(rayPoint);
}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//update the forces acting upon the lander
void ofApp::updateForce(ofVec3f &p, ofVec3f &v, float &r, float &rv, float &f) {

    // update position based on velocity
    //
    p += (v / ofGetFrameRate());
    r += (rv / 60);    //rotation

    // update acceleration with accumulated particles forces
    // remember :  (f = ma) OR (a = 1/m * f)
    //
    ofVec3f accel = acceleration;    // start with any acceleration already on the particle
    accel += (forces * (1.0 / mass));
    
    //rotation
    float rcel = degAccel;
    rcel += f;
    
    v += (accel / ofGetFrameRate());
    rv += (rcel / 60);     //rotation
    
    // add a little damping for good measure
    //
    v *= damping;
    rv *= degDamp;
    
    
    //reset all forces on lander by setting it to gravity + turbulent forces
    //
    forces.set(gravityForce + turbulentForce);      //turbulence and gravity default

    
    f = 0;      //rotational force set to 0
    
}


//check for collisions (impulse force)
void ofApp::checkCollisions(ofVec3f &imp) {
    ofVec3f min = lander.getSceneMin(landerScale) + lander.getPosition();
    ofVec3f max = lander.getSceneMax(landerScale) + lander.getPosition();

    Box bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
    if(octree.intersect(bounds, octree.root, colBoxList)) {
        ofVec3f norm = ofVec3f(0, 1, 0);
        
        //force = (restitution + 1) * (-vdotn) * n
        imp = (restitution + 1.0) * (dot(-velocity, norm)) * norm;
        forces += (imp * ofGetFrameRate());       //add to forces
        
        if(imp.y > winCon) {
            gameOver = true;        //if impulse force is greater than designated value
            gameStart = false;
            emitter.setEmitterType(RadialEmitter);
            emitter.start();            //explosion once
            emitter.setOneShot(true);
        }
        else {
            gameWin = true;
        }
        
    }
}

float ofApp::dot(ofVec3f obj1, ofVec3f obj2) {
    return ((obj1.x * obj2.x) + (obj1.y * obj2.y) + (obj1.z * obj2.z));
}


//alteration of raySelectWithOctree: replace w/ lander position
void ofApp::aglSensor(ofVec3f &pointRet) {
    ofVec3f origin = lander.getPosition();      //position of lander
    ofVec3f rayPoint = origin + ofVec3f(0, -100, 0);       //downward sensor
    ofVec3f rayDir = rayPoint - origin;
    rayDir.normalize();
    
    //create ray
    Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z), Vector3(rayDir.x, rayDir.y, rayDir.z));
    
    aglSelected = octree.intersect(ray, octree.root, aglNode);      //call intersect function
    
    if(aglSelected) {
        pointRet = octree.mesh.getVertex(aglNode.points[0]);
    }
    //else
        //pointRet = ofVec3f(0, -100, 0);
    
}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 




bool ofApp::mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point) {
	ofVec2f mouse(mouseX, mouseY);
	ofVec3f rayPoint = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	ofVec3f rayDir = rayPoint - cam.getPosition();
	rayDir.normalize();
	return (rayIntersectPlane(rayPoint, rayDir, planePoint, planeNorm, point));
}


void ofApp::dragEvent(ofDragInfo dragInfo) {


}

//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane(glm::vec3 planePt, glm::vec3 planeNorm) {
	// Setup our rays
	//
	glm::vec3 origin = cam.getPosition();
	glm::vec3 camAxis = cam.getZAxis();
	glm::vec3 mouseWorld = cam.screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, planePt, planeNorm, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;

		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}
