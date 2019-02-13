#pragma once

#include "Cinder-Bullet3D/RigidBody.h"
#include "cinder/gl/Batch.h"

typedef std::shared_ptr<class DrawableRBD> DrawableRBDRef;
class DrawableRBD {
public:
	DrawableRBD();
	~DrawableRBD() {}

	virtual void Setup(const ci::gl::BatchRef &visual, const bullet::RigidBodyRef &physics);

	virtual void update();
	virtual void draw();

	ci::gl::BatchRef&		getBatch() { return mVisObj; }
	bullet::RigidBodyRef&	getPhyObj() { return mPhyObj; }
	ci::vec3 getCenter() { return ci::vec3(mModelMatrix*ci::vec4(0,0,0,1));}
protected:

	ci::gl::BatchRef						mVisObj;
	bullet::RigidBodyRef					mPhyObj;
	bullet::SimpleGlDynamicMotionStateRef	mMotionState;
	ci::mat4								mModelMatrix;
};

