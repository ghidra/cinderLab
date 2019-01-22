#include "VehicleGeneric.h"

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
{
}

void VehicleGeneric::Setup(const ci::vec3 position, const ci::gl::BatchRef &visual)
{
	auto rigidBody = bullet::RigidBody::create(bullet::RigidBody::Format()
		.collisionShape(bullet::createBoxShape(ci::vec3(0.5f, 0.5f, 0.5f)))
		.mass(1)
		.initialPosition(position)
		.addToWorld(true));

	DrawableRBD::Setup(visual, rigidBody);
}

void VehicleGeneric::update()
{
	//mLocalDeltaTime = deltaTime;

	mInputForce = ci::vec3();

	ci::vec3 forwardDirection = glm::normalize(ci::vec3( mModelMatrix * ci::vec4(0, 0, 1, 0) ) );
	ci::vec3 upDirection = glm::normalize(ci::vec3(mModelMatrix * ci::vec4(0, 1, 0, 0)) );
	ci::vec3 sideDirection = glm::normalize(glm::cross(upDirection, forwardDirection));

	if (mMovingForward)
	{
		mInputForce += forwardDirection;
		//CI_LOG_I("MOVING");
	}
	if (mMovingLeft)
		mInputForce += sideDirection;
	if (mMovingBackward)
		mInputForce -= forwardDirection;
	if (mMovingRight)
		mInputForce -= sideDirection;
	/*if (mMovingUpward)
		mInputForce += viewUp;
	if (mMovingDownward)
		mInputForce -= viewUp;*/

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

