#include "DrawableRBD.h"
#include "Cinder-Bullet3D/MotionState.h"

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace bullet;

DrawableRBD::DrawableRBD()
{

}
void DrawableRBD::Setup(const ci::gl::BatchRef &visual, const bullet::RigidBodyRef &physics)
{
	mVisObj = visual;
	mPhyObj = physics;
	mMotionState = SimpleGlDynamicMotionStateRef(new SimpleGlDynamicMotionState(mPhyObj->getCenterOfMassTransform()));
	mPhyObj->setMotionState(mMotionState);
}

void DrawableRBD::update()
{
	mMotionState->getGLWorldTransform(&mModelMatrix);
}

void DrawableRBD::draw()
{
	gl::setModelMatrix(mModelMatrix);
	mVisObj->draw();
}
