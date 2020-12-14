#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter() {
	sys = new ParticleSystem();
	createdSys = true;
	init();
}

ParticleEmitter::ParticleEmitter(ParticleSystem *s) {
	if (s == NULL)
	{
		cout << "fatal error: null particle system passed to ParticleEmitter()" << endl;
		ofExit();
	}
	sys = s;
	createdSys = false;
	init();
}

ParticleEmitter::~ParticleEmitter() {

	// deallocate particle system if emitter created one internally
	//
	if (createdSys) delete sys;
}

void ParticleEmitter::init() {
	rate = 10;
	velocity = ofVec3f(0, -20, 0);
	lifespan = 0.164;
	started = false;
	oneShot = false;
	fired = false;
	lastSpawned = 0;
	radius = .1;
	particleRadius = .1;
	visible = true;
	type = DirectionalEmitter;
	groupSize = 10;
    
    sndCheck = exhaust.load("geo/exhaust.mp3");     //sound check
    exhaust.setVolume(0.1f);    //volume adjust
}



void ParticleEmitter::draw() {
	if (visible) {
		switch (type) {
		case DirectionalEmitter:
			ofDrawSphere(position, radius/10);  // just draw a small sphere for point emitters 
			break;
		case RadialEmitter:
			ofDrawSphere(position, radius/10);  // just draw a small sphere as a placeholder
			break;
		default:
			break;
		}
	}
	sys->draw();  
}
void ParticleEmitter::start() {
	started = true;
	lastSpawned = ofGetElapsedTimeMillis();
}

void ParticleEmitter::stop() {
	started = false;
	fired = false;
}
void ParticleEmitter::update() {

	float time = ofGetElapsedTimeMillis();

    position = pos;     //particles in emitter to position based on lander

	if (oneShot && started) {       //one shot-fired + started
		if (!fired) {
            if(sndCheck && type == DirectionalEmitter)
                exhaust.play();     //play sound if available as DirectionalEmitter
            
			// spawn a new particle(s)
			//
			for (int i = 0; i < groupSize; i++)
				spawn(time);

			lastSpawned = time;
		}
		fired = true;
		stop();
	}

	else if (((time - lastSpawned) > (1000.0 / rate)) && started) {

		// spawn a new particle(s)
		//
		for (int i= 0; i < groupSize; i++)
			spawn(time);
	
		lastSpawned = time;
	}

	sys->update();
}

// spawn a single particle.  time is current time of birth
//
void ParticleEmitter::spawn(float time) {

	Particle particle;

	// set initial velocity and position
	// based on emitter type
	//
	switch (type) {
	case RadialEmitter:
	{
		ofVec3f dir = ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1));
		float speed = velocity.length();
		particle.velocity = dir.getNormalized() * speed;
		particle.position.set(position);
	}
    break;
	case DirectionalEmitter:
    {
        ofVec3f dir = ofVec3f(ofRandom(-0.1, 0.1), -1, ofRandom(-0.1, 0.1));
        float speed = velocity.length();
        particle.velocity = dir.getNormalized() * speed;
        particle.position.set(position + ofVec3f(0, 0.05, 0));
    }
    break;
	}

	// other particle attributes
	//
	particle.lifespan = lifespan;
	particle.birthtime = time;
	particle.radius = particleRadius;

	// add to system
	//
	sys->add(particle);
}
