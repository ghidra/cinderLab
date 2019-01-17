#include "DrawablePlaneRBD.h"
#include "Cinder-Bullet3D/MotionState.h"

using namespace std;
using namespace ci;
using namespace ci::gl;
using namespace bullet;

DrawablePlaneRBD::DrawablePlaneRBD(const gl::GlslProgRef &shader)
{

	auto plane = createStaticPlaneShape(vec3(0.0f, 1.0f, 0.0f), 0);
	// Dealing with floors like this is a bit hard. Basically, the Static Plane Shape doesn't really have
	// a motion state, it's infinite across a normal. Because of this, I'll use this drawableHelper to get
	// a vbomesh.
	mVisObj = gl::Batch::create(bt::drawableHelpers::getDrawablePlane(plane), shader);

	// Therefore, we cache these in our program to draw seperately. Now I know that I want the plane's normal
	// to be the yaxis and the offset means what distance on the normal I'd like to shift this. If I put 1 insted
	// of zero I'd push the RigidBody to 0, 1, 0. Remember, that's not the normal, even though they're the same
	// value. Same as if the offset were 50 and the normal were 0, 1, 0, the position would be 0, 50, 0, not 0, 51, 0
	mPhyObj = RigidBody::create(RigidBody::Format()
		.collisionShape(plane)
		.initialPosition(vec3(0, 0, 0))
		.addToWorld(true));

}

//void DrawablePlaneRBD::update()
//{
	//mMotionState->getGLWorldTransform(&mModelMatrix);
//}

void DrawablePlaneRBD::draw()
{
	//gl::setModelMatrix(mModelMatrix);
	mVisObj->draw();
}
