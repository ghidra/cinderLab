#pragma once

#include "Cinder-Bullet3D/RigidBody.h"
#include "cinder/gl/Batch.h"

class DrawablePlaneRBD {
public:
	DrawablePlaneRBD(const ci::gl::GlslProgRef &shader);
	~DrawablePlaneRBD() {}

	//void update();
	void draw();

	ci::gl::BatchRef		getBatch() { return mVisObj; }
	bullet::RigidBodyRef	getPhyObj() { return mPhyObj; }
private:

	ci::gl::BatchRef						mVisObj;
	bullet::RigidBodyRef					mPhyObj;
	ci::mat4								mModelMatrix;
};

