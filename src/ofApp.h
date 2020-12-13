#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "Octree.h"
#include <glm/gtx/intersect.hpp>



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
        void updateForce(ofVec3f &p, ofVec3f &v);           //integrate method for force computations

        ofEasyCam cam;
		ofxAssimpModelLoader moon, lander;
		ofLight light;
		Box boundingBox, landerBounds;
		vector<Box> colBoxList;
		bool bLanderSelected = false;
		Octree octree;
		TreeNode selectedNode;
		glm::vec3 mouseDownPos, mouseLastPos;
		bool bInDrag = false;


		ofxIntSlider numLevels;
		ofxPanel gui;

		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bDisplayPoints;
		bool bPointSelected;
		bool bHide;
		bool pointSelected = false;
		bool bDisplayOctree = false;
		bool bDisplayBBoxes = false;
        bool bDisplayLeafNodes = false;
		
		bool bLanderLoaded;
		bool bTerrainSelected;
    
        //Brian La--------------------------------------------------------------------------------
    
        //background
        ofImage bg;             //image
        bool bgLoaded = false;      //check if image loaded
    
        //camera togglers - static, tracking, lander-rotate, lander-ground
        bool staticCam = false;
        bool trackCam = false;
        bool rotateCam = false;
        bool groundCam = false;
        bool gameStart = false;         //game start
    
        //scale factor
        float landerScale = 0.1;
    
        //forces
        ofVec3f gravityForce = ofVec3f(0, -1.64 * landerScale, 0);      //gravity
        ofVec3f turbulentForce = ofVec3f(ofRandom(-0.0164, 0.0164), ofRandom(-0.0164, 0.0164), ofRandom(-0.0164, 0.0164));      //turbulence
    
        //angular forces
        
    
        //physics components for lander
        ofVec3f position = ofVec3f(0, 20, 0);     //set back terrain
        ofVec3f acceleration = ofVec3f(0, 0, 0);     //moon
        ofVec3f velocity = ofVec3f(0, 0, 0);           //lander velocity
        ofVec3f forces = gravityForce + turbulentForce;         //apply gravity force
        float mass = 1.0;           //default mass of lander
        float damping = 0.999999;       //damp value
        
    
        //---------------------------------------------------------------------------------------
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;

		vector<Box> bboxList;

		const float selectionRange = 4.0;
};
