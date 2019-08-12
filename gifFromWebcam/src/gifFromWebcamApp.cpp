#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "OSCManager.h"
#include "CameraManager.h"
#include "json.hpp"

// for convenience
using json = nlohmann::json;
using namespace ci;
using namespace ci::app;
using namespace std;

const int WIDTH = 640;
const int HEIGHT = 480;

class gifFromWebcamApp : public App {
public:
    //gifFromWebcamApp();
    static void prepare( Settings *settings );
	void setup() override;
	void update() override;
	void draw() override;

private:
    mlx::OSCManagerRef  mOSCManager;
    mlx::CameraManagerRef mCameraManager;
};

void gifFromWebcamApp::prepare( Settings *settings )
{
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setTitle( "Multi GIF Cam" );

    settings->setFullScreen( false );
}


void gifFromWebcamApp::setup()
{

    mCameraManager = mlx::CameraManagerRef(new mlx::CameraManager());
    //cameras
    int cameraIndex = 0;
    for (const auto &device : Capture::getDevices()) {
        console() << "Device: " << device->getName() << " Camera Index: " <<  cameraIndex << endl;
        mCameraManager->AddCamera(device);
        mCameraManager->mPreviews[cameraIndex] = gl::Texture::create(WIDTH, HEIGHT);
        mCameraManager->mPreviews[cameraIndex]->bind();
        cameraIndex++;
    }
    
    ///set up the OSCManager
    mOSCManager = mlx::OSCManagerRef(new mlx::OSCManager());
    //make the listener
    mOSCManager->mReceiver.setListener( "/game/start",
                          [&]( const osc::Message &msg ){
                              mOSCManager->callback("/game/start RECEIVED");
                              CI_LOG_D("Message received");
                              try {
                                  auto j = json::parse(msg.getArgString(0));
                                  CI_LOG_D(j.dump(4));
                                  int camera_index = j["camera"];
                                  auto game_id = std::string(j["game_id"]);
                                  mCameraManager->StartCapture(camera_index, game_id);
                              } catch (nlohmann::detail::parse_error& e) {
                                  CI_LOG_E("Message is no bueno");
                                  CI_LOG_E(e.what());
                                  return;
                              }
                          });
    
    mOSCManager->mReceiver.setListener( "/game/end",
                           [&]( const osc::Message &msg ){
                               mOSCManager->callback("/game/end RECEIVED");
                               CI_LOG_D("Message received");
                               try {
                                   auto j = json::parse(msg.getArgString(0));
                                   CI_LOG_D(j.dump(4));
                                   auto game_id = std::string(j["game_id"]);
                                   mCameraManager->EndGame(game_id);
                               } catch (nlohmann::detail::parse_error& e) {
                                   CI_LOG_E("Message is no bueno");
                                   CI_LOG_E(e.what());
                                   return;
                               }
                           });
}

void gifFromWebcamApp::update()
{
    mCameraManager->Update();
}

void gifFromWebcamApp::draw()
{
	gl::clear();

    for (auto const& element : mCameraManager->mPreviews)
    {
        gl::TextureRef texture = element.second;
        if(texture){
            int cameraIndex = element.first;
            Rectf drawRect( 0, 0, texture->getWidth(), texture->getHeight() );
            gl::draw( texture, drawRect );
        }
    }
}

CINDER_APP(gifFromWebcamApp, RendererGl, &gifFromWebcamApp::prepare)
