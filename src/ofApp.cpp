
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
    
    if(exhaustSound.load("geo/exhaust.mp3")) {
        sndLoaded = true;
        exhaustSound.setVolume(0.1f);
    }
    else
        cout << "Unable to load sound file" << endl;
        
       
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bLanderLoaded = false;
	bTerrainSelected = true;
    
    //camera setups
	cam.setDistance(30);
	cam.setNearClip(.1);
	cam.setFov(65.5);   // approx equivalent to 28mm in 35mm format
    
    
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

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
    }
    else    {
        cout << "Error: Can't load model" << endl;
    }
    

}
 
//--------------------------------------------------------------
// incrementally update scene (animation)
//
void ofApp::update() {
    //upon game start...can pause and select lander position
    if(gameStart) {
        
        //position, velocity, degVelocity, degAcceleration, rotation deg, force deg
        updateForce(position, velocity, rotation, degVeloc, degForce);
        lander.setPosition(position.x, position.y, position.z);     //set new position
        lander.setRotation(0, rotation, 0, 1, 0);       //set new rotation
        
        
        //telemetric sensor
        if(aglON)
            aglSensor(landerPoint);
    }
	
}
//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(ofColor::black);
    // draw screen data
    //
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
    ofDrawBitmapString(str2, 0, 45);

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
    
    ofEnableLighting();              // shaded mode
    moon.drawFaces();
    ofMesh mesh;
    if (bLanderLoaded) {
        lander.drawFaces();
        if (!bTerrainSelected) drawAxis(lander.getPosition());
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
    
	if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));



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
	ofDisableLighting();
	int level = 0;
	ofNoFill();

    if (bDisplayLeafNodes) {
        octree.drawLeafNodes(octree.root);
    }
    else if (bDisplayOctree) {
        ofNoFill();
        ofSetColor(ofColor::white);
        octree.draw(numLevels, 0);
    }

	// if point selected, draw a sphere
	//
	if (pointSelected) {
		ofVec3f p = octree.mesh.getVertex(selectedNode.points[0]);
		ofVec3f d = p - cam.getPosition();
		ofSetColor(ofColor::lightGreen);
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

// 
// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
    
}


void ofApp::keyPressed(int key) {

	switch (key) {
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
        exhaustSound.play();        //play sound
        forces += ofVec3f(0, 0, 5);     //scaled FORWARD force
        break;
    case 'S':
    case 's':
        fuel--;
        exhaustSound.play();
        forces += ofVec3f(0, 0, -5);     //scaled BACK thrust force
        break;
    case OF_KEY_UP:
        fuel -= 2;
        exhaustSound.play();
        forces += ofVec3f(0, 5, 0);     //scaled UP thrust force
        break;
    case OF_KEY_DOWN:
        fuel--;
        exhaustSound.play();
        forces += ofVec3f(0, -5, 0);     //scaled DOWN thrust force
        break;
    case OF_KEY_RIGHT:
        fuel--;
        exhaustSound.play();
        forces += ofVec3f(5, 0, 0);     //scaled RIGHT thrust force
        break;
    case OF_KEY_LEFT:
        fuel--;
        exhaustSound.play();
        forces += ofVec3f(-5, 0, 0);     //scaled LEFT thrust force
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
	case 'H':
	case 'h':
		break;
    case 'L':
    case 'l':
        bDisplayLeafNodes = !bDisplayLeafNodes;
        break;
	case 'O':
	case 'o':
		bDisplayOctree = !bDisplayOctree;
		break;
    case 'R':
	case 'r':
		cam.reset();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
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

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {
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
		ofVec3f p;
		raySelectWithOctree(p);
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
		ofVec3f p;
		raySelectWithOctree(p);
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
    forces.set(gravityForce + turbulentForce);
    f = 0;
    
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
    return aglSelected;
    
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
//	glEnable(GL_LIGHT1);
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
