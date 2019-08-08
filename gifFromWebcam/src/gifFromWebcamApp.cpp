#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "OSCManager.h"
#include "CameraManager.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class gifFromWebcamApp : public App {
public:
    //gifFromWebcamApp();
	void setup() override;
	void update() override;
	void draw() override;

private:
    mlx::OSCManagerRef  mOSCManager;
    vector<mlx::CameraManagerRef> mCameraManagers;
};

/*gifFromWebcamApp::gifFromWebcamApp()
{
}*/

void gifFromWebcamApp::setup()
{
    ///set up the OSCManager
    mOSCManager = mlx::OSCManagerRef(new mlx::OSCManager());
    //make the listener
    mOSCManager->mReceiver.setListener( "/game/player_count",
                          [&]( const osc::Message &msg ){
                              //mCurrentCirclePos.x = msg[0].int32();
                              //mCurrentCirclePos.y = msg[1].int32();
                              mOSCManager->callback("YOOOOOOOOOO");
							  //CI_LOG_W("-------------------WHAT");
                          });
    //cameras
    for (const auto &device : Capture::getDevices()) {
        console() << "Device: " << device->getName() << " " << endl;
        mCameraManagers.push_back( mlx::CameraManagerRef(new mlx::CameraManager(device)) );
    }

    ///////GIF STUFF
    //make a hard coded 1 gif for now
    for(std::size_t i=0; i<mCameraManagers.size(); ++i)
    {
        mCameraManagers[i]->NewGif();
    }
}

void gifFromWebcamApp::update()
{
    mCameraManagers[0]->Update();
}

void gifFromWebcamApp::draw()
{
	gl::clear();

	if (mCameraManagers[0]->mTexture) {
		gl::ScopedModelMatrix modelScope;
		gl::draw(mCameraManagers[0]->mTexture);
	}
}

CINDER_APP(gifFromWebcamApp, RendererGl)
