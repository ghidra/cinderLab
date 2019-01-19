#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"

#include "bullet/DrawableRBD.h"
#include "bullet/DrawablePlaneRBD.h"
#include "Cinder-Bullet3D/BulletContext.h"

#include "CameraFollow.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace bullet;

class testBulletApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void keyDown(KeyEvent event)override;
	void keyUp(KeyEvent event)override;
	void update() override;
	void draw() override;

	bullet::ContextRef	mContext;
	std::vector<DrawableRBDRef> mVisualPhysicsObjs;
	DrawablePlaneRBDRef mVisualGround;
	gl::GlslProgRef		mPhongShader;
	CameraPersp			mCam;
	CameraFollowRef		mCamera;

	gl::BatchRef			mVisPlane;
	bullet::RigidBodyRef	mPhyPlane;

	bool	mTMPApplyForce;
private:
	float 				mLastTime = 0.0f;//for delta time calculation
	float				mDeltaTime = 0.0f;

};

void testBulletApp::setup()
{
	mPhongShader = gl::GlslProg::create(gl::GlslProg::Format()
		.vertex(loadAsset("Phong.vert"))
		.fragment(loadAsset("Phong.frag"))
	);
	mContext = bullet::Context::create(bullet::Context::Format().drawDebug(true).createDebugRenderer(true));

	//make some objects
	//before making the ground... i need a shader to pass in for now...
	mVisualGround = DrawablePlaneRBDRef(new DrawablePlaneRBD(mPhongShader));

	//make a cube
	// All I need to do is create a Box Shape at a half extents of .5, .5, .5.
	auto rigidBody = RigidBody::create(RigidBody::Format()
		.collisionShape(createBoxShape(vec3(0.5f, 0.5f, 0.5f)))
		.mass(1)
		.initialPosition(vec3(0, 2, 0))
		.addToWorld(true));
	
	mVisualPhysicsObjs.emplace_back(DrawableRBDRef(new DrawableRBD(gl::Batch::create(geom::Cube(), mPhongShader), rigidBody)));

	gl::enableDepthRead();
	gl::enableDepthWrite();

	mCam.setPerspective(60.0, getWindowAspectRatio(), .01f, 1000.0f);
	mCam.lookAt(vec3(0, 5, 20), vec3(0.0f));

	mCamera = CameraFollowRef(new CameraFollow());

	mTMPApplyForce = false;
}

void testBulletApp::mouseDown( MouseEvent event )
{
}

void testBulletApp::keyDown(KeyEvent event)
{
	if (event.getChar() == KeyEvent::KEY_w)
		mTMPApplyForce = true;
}

void testBulletApp::keyUp(KeyEvent event)
{
	if (event.getChar() == KeyEvent::KEY_w)
		mTMPApplyForce = false;
}
void testBulletApp::update()
{
	////////////////////////////////////////////////
	// DELTA TIME
	////////////////////////////////////////////////
	float current = static_cast<float>(app::getElapsedSeconds());
	mDeltaTime = current - mLastTime;
	mLastTime = current;


	mContext->update();
	// We'll update our Physics objects which will cash our Model Matrices.
	if (mTMPApplyForce)
	{
		mVisualPhysicsObjs[0]->getPhyObj()->applyForce(vec3(0.0f, 0.0f, 11.04f), vec3());
		//CI_LOG_I("PUSH IT");
	}
	for (unsigned i=0;i< mVisualPhysicsObjs.size();++i)
		mVisualPhysicsObjs[i]->update();

	//update the camera
	mCamera->Update(mVisualPhysicsObjs[0]->getCenter(),mDeltaTime);
}

void testBulletApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	//gl::setMatrices(mCam);
	gl::setMatrices(mCamera->GetPerspective());
	mVisualGround->draw();

	gl::pushModelMatrix();
	for (unsigned i = 0; i< mVisualPhysicsObjs.size(); ++i)
		mVisualPhysicsObjs[i]->draw();
	gl::popModelMatrix();
}

CINDER_APP( testBulletApp, RendererGl )
