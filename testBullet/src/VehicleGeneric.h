#pragma once

#include "cinder/app/App.h"

#include "bullet/DrawableRBD.h"
#include "cinder/Log.h"

typedef std::shared_ptr<class VehicleGeneric> VehicleGenericRef;
class VehicleGeneric : public DrawableRBD {
public:
	VehicleGeneric();
	~VehicleGeneric() {}
	
	virtual void Setup(const ci::vec3 position, const ci::gl::BatchRef &visual);
	virtual void keyDown(ci::app::KeyEvent event);
	virtual void keyUp(ci::app::KeyEvent event);
	//virtual void update(float deltaTime);
	virtual void update() override;
	virtual void draw() override;

private:
	float			mMovementSpeed, mLocalDeltaTime;
	bool			mMovingForward, mMovingBackward, mMovingLeft, mMovingRight, mMovingUpward, mMovingDownward;

	ci::vec3		mInputForce,mInputTorqueForce;
};

