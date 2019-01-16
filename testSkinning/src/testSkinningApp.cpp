
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Utilities.h"///for screencaps
#include "cinder/Rand.h"

#include "CameraBasic.h"
//#include "cinder/ImageIo.h"

//#include "Resources.h"
//#include "Assets.h"

#include "cinder/Log.h"

///TEST SKINNING
#include "model/AssimpLoader.h"
#include "model/SkeletalMesh.h"
#include "model/SkeletalTriMesh.h"
#include "model/Skeleton.h"
#include "model/Renderer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class testSkinningApp : public App {
public:
	static void prepareSettings( Settings *settings );

	void setup() override;
	void update() override;
	void draw() override;
	void keyDown( KeyEvent event ) override;
	void keyUp( KeyEvent event ) override;
	void mouseDown( MouseEvent event ) override;
	void mouseUp( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;

private:
	float 				mLastTime = 0.0f;//for delta time calculation
	float				mDeltaTime = 0.0f;

	//input
	//soul::AudioInRef 				mAudioIn;
	//soul::UserInputRef 			mUserInput;
	
	//merge shader
	//signals::ScopedConnection		mConnection;
	//gl::GlslProgRef					mShader;

	//////////////////////
	CameraBasicRef					mCamera;

	bool							mTakeScreenCap;
	bool							mShowStats;

	/////////////////////
	//SKINNING TEST
	model::SkeletalTriMeshRef		mSkeletalTriMesh;
	model::SkeletalMeshRef			mSkeletalMesh;
};

void testSkinningApp::prepareSettings( Settings *settings )
{
	//settings->setWindowSize(1366, 700);//768
	settings->setWindowSize(1200, 600);
	//settings->setWindowSize( 1920, 1080 );
	//settings->setFullScreen(true);
	settings->setFullScreen(false);

	//////
	
	//settings->setWindowPos();
	//settings->setBorderless();
	//settings->setDisplay(Display::getDisplays().at(0));
	settings->disableFrameRate();
}

void testSkinningApp::setup()
{
	Rand::randSeed( (uint32_t)time( NULL ) );

	mTakeScreenCap=false;
	mShowStats=false;

	mCamera = CameraBasicRef( new CameraBasic() );
    //hideCursor();

    //////SKINNING
    //ci::app::addAssetDirectory( fs::path{ std::getenv( "CINDER_PATH" ) } / "blocks" / "rebar" / "assets" );
    //const auto& loader = model::AssimpLoader( loadAsset( "astroboy_walk.dae" ) );
    const auto& loader = model::AssimpLoader( loadResource( "MAN.dae" ) );
	mSkeletalTriMesh = model::SkeletalTriMesh::create( loader );
	mSkeletalMesh = model::SkeletalMesh::create( loader, mSkeletalTriMesh->getSkeleton() );

	for( unsigned int an=0; an < mSkeletalTriMesh->getNumAnimations(); ++an ) {
		CI_LOG_I("ANIMATION NAME: "+mSkeletalTriMesh->getAnimName(an));
	}

	gl::getStockShader( gl::ShaderDef().color() )->bind();
}



void testSkinningApp::update()
{
	////////////////////////////////////////////////
	// DELTA TIME
	////////////////////////////////////////////////
	float current = static_cast<float>( app::getElapsedSeconds() );
	mDeltaTime = current - mLastTime;
	mLastTime = current;

	//CAMERA
	mCamera->Update(mDeltaTime);

	/////SKINNING
	if( mSkeletalMesh->hasAnimations() ) {
		//CI_LOG_I("HAS ANIMATION--------------"+to_string(mSkeletalMesh->getAnimDuration()));
		float time = fmod(current, mSkeletalMesh->getAnimDuration(2));// * mMouseHorizontalPos / math<float>::max( 200.0f, getWindowWidth() );
		mSkeletalMesh->setPose( time ,2);
		mSkeletalTriMesh->setPose( time ,2);
	}
}

void testSkinningApp::draw()
{
	gl::clear(Color( 0.2, 0.2, 0.2 ));

	{
		gl::setMatricesWindow( app::getWindowSize() );
		gl::ScopedDepth pushDepth{ false };//this keeps it from drawing over anything
		gl::viewport( app::getWindowSize() );

		gl::setMatrices( mCamera->GetPerspective() );

		gl::drawCube( vec3(), vec3( 2 ) );

		model::Renderer::draw( mSkeletalMesh );
		model::Renderer::draw( mSkeletalMesh->getSkeleton() );
		model::Renderer::drawLabels( mSkeletalMesh->getSkeleton(),  mCamera->GetPerspective() );

		// gl::ScopedTextureBind bind0( mCurrentVisualMode->GetBeauty(), 0 );
		// gl::ScopedTextureBind bind1( mLastVisualMode->GetBeauty(), 1 );

		// gl::ScopedGlslProg shaderScp( mShader );
		// mShader->uniform("uTransition",mTransitionNormalizedTime);

		//gl::drawSolidRect( app::getWindowBounds() );

	}

	// if (mTakeScreenCap) {
	// 	writeImage(getHomeDirectory() / "ScreenCaps" / ("frame_" + std::to_string(getElapsedFrames()) + ".png"), mCurrentVisualMode->GetBeauty()->createSource());
	// 	mTakeScreenCap = false;
	// }

	////////////////////////////
	//// DEBUG DRAW FPS
	////////////////////////////
	if(mShowStats)
	{
		gl::setMatricesWindow( getWindowSize() );
		gl::drawString( to_string( static_cast<int>( getAverageFps() ) ) + " fps", vec2( 32.0f, 52.0f ) );
	}
}

void testSkinningApp::keyDown( KeyEvent event )
{
	mCamera->keyDown(event);

	if (event.getCode() == KeyEvent::KEY_ESCAPE)
	{
		quit();
	}
	// if (event.getCode() == KeyEvent::KEY_SPACE)
	// {
	// }
	if( event.getChar() == 'p' )
	{
		mShowStats=!mShowStats;
	}
}
void testSkinningApp::keyUp( KeyEvent event )
{
	mCamera->keyUp(event);
}
void testSkinningApp::mouseDown( MouseEvent event )
{
	mCamera->mouseDown(event);
}
void testSkinningApp::mouseUp( MouseEvent event )
{
	mCamera->mouseUp(event);
}
void testSkinningApp::mouseDrag( MouseEvent event )
{
	mCamera->mouseDrag(event);
}

//////////////////
CINDER_APP( testSkinningApp, RendererGl, &testSkinningApp::prepareSettings )
