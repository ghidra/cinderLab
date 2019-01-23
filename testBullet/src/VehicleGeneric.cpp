#include "VehicleGeneric.h"

using namespace ci;

VehicleGeneric::VehicleGeneric()
	: DrawableRBD()
	, mMovementSpeed(10.0f)
	, mLocalDeltaTime(0.0f)
	, mMovingForward(false)
	, mMovingBackward(false)
	, mMovingLeft(false)
	, mMovingRight(false)
	, mMovingUpward(false)
	, mMovingDownward(false)
	, mInputForce(ci::vec3())
	, mInputTorqueForce(ci::vec3())
{
}

void VehicleGeneric::Setup(const vec3 position, const gl::BatchRef &visual)
{
	auto rigidBody = bullet::RigidBody::create(bullet::RigidBody::Format()
		.collisionShape(bullet::createBoxShape(vec3(0.5f, 0.5f, 0.5f)))
		.mass(1)
		.initialPosition(position)
		.addToWorld(true));

	rigidBody->setDamping( rigidBody->getLinearDamping(),1.0f );

	DrawableRBD::Setup(visual, rigidBody);
}

void VehicleGeneric::update()
{
	//mLocalDeltaTime = deltaTime;

	mInputForce = vec3();
	mInputTorqueForce = vec3();

	vec3 forwardDirection = glm::normalize(vec3( mModelMatrix * vec4(0, 0, 1, 0) ) );
	vec3 upDirection = glm::normalize(vec3(mModelMatrix * vec4(0, 1, 0, 0)) );
	vec3 sideDirection = glm::normalize(glm::cross(upDirection, forwardDirection));

	if (mMovingForward)
		mInputForce += forwardDirection;
	if (mMovingBackward)
		mInputForce -= forwardDirection;

	if (mMovingLeft)
		mInputTorqueForce += upDirection;
	if (mMovingRight)
		mInputTorqueForce -= upDirection;
	/*if (mMovingUpward)
		mInputForce += viewUp;
	if (mMovingDownward)
		mInputForce -= viewUp;*/

	if (glm::length(mInputTorqueForce) > 0.0f)
	{
		mPhyObj->applyTorque(glm::normalize(mInputTorqueForce)*10.0f);
		//if we are turning at all... we need to update the linear force to face the direction that we are turning.
		vec3 lv = mPhyObj->getLinearVelocity();
		vec3 nlv = glm::normalize(lv);
		float llv = glm::length(lv);
		///just use part of the turning directoin
		vec3 updatedLinearVelocity = glm::lerp(nlv,forwardDirection,0.5f);
		mPhyObj->setLinearVelocity(updatedLinearVelocity*llv*0.99f);
	}

	if (glm::length(mInputForce) > 0.0f)
	{
		//mInputForce = glm::normalize(mInputForce)*(mLocalDeltaTime*mMovementSpeed);
		mInputForce = glm::normalize(mInputForce)*mMovementSpeed;// *(mLocalDeltaTime*mMovementSpeed);
		//CI_LOG_I("MOVING" );
		mPhyObj->applyForce(mInputForce, ci::vec3());
	}
	

	DrawableRBD::update();
}
void VehicleGeneric::draw()
{
	DrawableRBD::draw();
}
void VehicleGeneric::keyDown(ci::app::KeyEvent event)
{
	if (event.getChar() == 'w')
		mMovingForward = true;
	if (event.getChar() == 'a')
		mMovingLeft = true;
	if (event.getChar() == 's')
		mMovingBackward = true;
	if (event.getChar() == 'd')
		mMovingRight = true;
	if (event.getChar() == 'q')
		mMovingUpward = true;
	if (event.getChar() == 'e')
		mMovingDownward = true;
}
void VehicleGeneric::keyUp(ci::app::KeyEvent event)
{
	if (event.getChar() == 'w')
		mMovingForward = false;
	if (event.getChar() == 'a')
		mMovingLeft = false;
	if (event.getChar() == 's')
		mMovingBackward = false;
	if (event.getChar() == 'd')
		mMovingRight = false;
	if (event.getChar() == 'q')
		mMovingUpward = false;
	if (event.getChar() == 'e')
		mMovingDownward = false;
}

