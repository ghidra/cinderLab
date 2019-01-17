#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/GlslProg.h"
#include "cinder/TriMesh.h"

//#include "bullet/DrawableRBD.h"
//#include "bullet/DrawablePlaneRBD.h"
//#include "Cinder-Bullet3D/BulletContext.h"

using namespace ci;
using namespace ci::app;
using namespace std;
//using namespace bullet;

class testBulletApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	//bullet::ContextRef	mContext;
	//std::vector<DrawableRBD> mVisualPhysicsObjs;
	gl::GlslProgRef		mPhongShader;
	CameraPersp			mCam;

	//gl::BatchRef			mVisPlane;
	//bullet::RigidBodyRef	mPhyPlane;
};

void testBulletApp::setup()
{
	mPhongShader = gl::GlslProg::create(gl::GlslProg::Format()
		.vertex(loadAsset("Phong.vert"))
		.fragment(loadAsset("Phong.frag"))
	);
	//mContext = bullet::Context::create(bullet::Context::Format().drawDebug(true).createDebugRenderer(true));

	gl::enableDepthRead();
	gl::enableDepthWrite();

	mCam.setPerspective(60.0, getWindowAspectRatio(), .01f, 1000.0f);
	mCam.lookAt(vec3(0, 5, 20), vec3(0.0f));

}

void testBulletApp::mouseDown( MouseEvent event )
{
}

void testBulletApp::update()
{
}

void testBulletApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( testBulletApp, RendererGl )
