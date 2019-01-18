#pragma once

#include "Cinder-Bullet3D/RigidBody.h"
#include "cinder/gl/Batch.h"

typedef std::shared_ptr<class DrawableRBD> DrawableRBDRef;
class DrawableRBD {
public:
	DrawableRBD(const ci::gl::BatchRef &visual, const bullet::RigidBodyRef &physics);
	~DrawableRBD() {}

	void update();
	void draw();

	ci::gl::BatchRef&		getBatch() { return mVisObj; }
	bullet::RigidBodyRef&	getPhyObj() { return mPhyObj; }
private:

	ci::gl::BatchRef						mVisObj;
	bullet::RigidBodyRef					mPhyObj;
	bullet::SimpleGlDynamicMotionStateRef	mMotionState;
	ci::mat4								mModelMatrix;
};

