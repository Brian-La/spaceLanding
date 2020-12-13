#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "Octree.h"
#include <glm/gtx/intersect.hpp>

typedef enum { staticCam, trackCam, rotateCam, groundCam } TypeOfCam;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		bool mouseIntersectPlane(ofVec3f planePoint, ofVec3f planeNorm, ofVec3f &point);
		bool raySelectWithOctree(ofVec3f &pointRet);
		glm::vec3 getMousePointOnPlane(glm::vec3 p , glm::vec3 n);
        void updateForce(ofVec3f &p, ofVec3f &v, float &r, float &rv, float &f);           //integrate method for force computations

        ofEasyCam cam;
		ofxAssimpModelLoader moon, lander;
		Box boundingBox, landerBounds;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;


		int numLevels = 7;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
        //bool bDisplayLeafNodes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
    
        //Brian La--------------------------------------------------------------------------------
        //gameStart
        bool gameStart = true;         //game start
        //float lastTime;
        //float currentTime;
        int fuel = 1200;
    
    
        //background & sound
        ofImage bg;                     //image
        bool bgLoaded = false;
        ofSoundPlayer exhaustSound;      //sound player
        bool sndLoaded = false;
    
    
        //light
        ofLight landerLight, hoverLight;
        bool lightOn = false;       //lander light
    
    
        //camera togglers - enumerated static, tracking, lander-rotate, lander-ground
        TypeOfCam camType = staticCam;      //static came default
        ofVec3f camPos;
        ofVec3f mousePos = cam.getPosition();

    
        //scale factor
        float landerScale = 0.1;
    
    
        //forces
        ofVec3f gravityForce = ofVec3f(0, -1.64 * landerScale, 0);      //gravity
        ofVec3f turbulentForce = ofVec3f(ofRandom(-0.0164, 0.0164), ofRandom(-0.0164, 0.0164), ofRandom(-0.0164, 0.0164));      //turbulence
    
    
        //angular forces
        float rotation = 0.0;       //rotation value
        float degVeloc = 0.0;    //velocity of rotation
        float degAccel = 0.0;    //acceleration of rotation
        float degForce = 0.0;   //rotation force
        float degDamp = 0.99;    //deg damp
        
    
        //physics components for lander
        ofVec3f position = ofVec3f(0, 20, 0);     //set back terrain
        ofVec3f acceleration = ofVec3f(0, 0, 0);     //moon
        ofVec3f velocity = ofVec3f(0, 0, 0);           //lander velocity
        ofVec3f forces = gravityForce + turbulentForce;         //apply gravity force
        float mass = 1.0;           //default mass of lander
        float damping = 0.99;       //damp value
    
    
        //telemetry sensor (altitude/AGL)
        bool aglON = false;
        bool aglSelected = false;       //if selection occurs
        ofVec3f landerPoint;        //landerPoint
        TreeNode aglNode;           //node selected by agl
        void aglSensor(ofVec3f &pointRet);        //calculate telemetric sensor
        //---------------------------------------------------------------------------------------
    
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;
};
