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

const int WIDTH = 1280;
const int HEIGHT = 720;
const int FEED_WIDTH = 1280;
const int FEED_HEIGHT = 720;
const int GIF_WIDTH = 710;
const int GIF_HEIGHT = 710;
const float FEED_FPS = 30.0;
const int GIF_FPS = 5;
const int MAX_FRAMES_PER_GAME = 15;

class gifFromWebcamApp : public App {
public:
    static void prepare( Settings *settings );
	void setup() override;
	void update() override;
	void draw() override;
    void mouseDown( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;

private:
    mlx::OSCManagerRef  mOSCManager;
    mlx::CameraManagerRef mCameraManager;
    int selectedGifIndex;
    vec2 lastPosition;
    string interactionMode;
};

void gifFromWebcamApp::prepare( Settings *settings )
{
    settings->setWindowSize(WIDTH, HEIGHT);
    settings->setTitle( "Multi GIF Cam" );

    settings->setFullScreen( false );
    settings->setFrameRate(FEED_FPS);
    
#ifdef CINDER_COCOA
    //https://discourse.libcinder.org/t/window-issue-on-osx-mojave/1326/14
    settings->setHighDensityDisplayEnabled( true );
#endif // CINDER_COCOA

}


void gifFromWebcamApp::setup()
{
    mCameraManager = mlx::CameraManagerRef(new mlx::CameraManager(FEED_WIDTH, FEED_HEIGHT, GIF_WIDTH, GIF_HEIGHT, FEED_FPS, GIF_FPS, MAX_FRAMES_PER_GAME));
    
    //cameras

    for (const auto &device : Capture::getDevices()) {
        console() << "Device: " << device->getName() << endl;
        mCameraManager->AddCamera(device);
    }

    selectedGifIndex = -1;
    // setup feed and gif preview Rect and gif offset;
    mCameraManager->SetupPreviewsAndOffsets();

    ///set up the OSCManager
    mOSCManager = mlx::OSCManagerRef(new mlx::OSCManager());
    //make the listener
    mOSCManager->mReceiver.setListener( "/game/start",
                          [&]( const osc::Message &msg ){
                              mOSCManager->callback("/game/start RECEIVED");
                              CI_LOG_D("Message received");
                              try {
                                  json j = json::parse(msg.getArgString(0));
                                  CI_LOG_D(j.dump(4));
                                  int camera_index = j["camera"];
                                  std::string game_id = j["game_id"];
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
                                   json j = json::parse(msg.getArgString(0));
                                   CI_LOG_D(j.dump(4));
								   std::string game_id = j["game_id"];
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


void gifFromWebcamApp::mouseDown( MouseEvent event )
{
 
    int cameraIndex = 0;
    for (auto gifRect : mCameraManager->mGifRectPreview) {
        vec2 click = vec2(event.getX(), event.getY());
        
//        Rectf scaleHandle(gifRect.getLowerRight()-vec2(4.0), gifRect.getLowerRight());
//        if(scaleHandle.contains(click)){
//            console() << "Click intersected for translating gif area: " << cameraIndex << " at: " << click << " inside of: " << scaleHandle << endl;
//            selectedGifIndex = cameraIndex;
//            lastPosition = click;
//            interactionMode = "SCALE";
//            return;
//        }

        if(gifRect.contains(click)){
            console() << "Click intersected for scaling gif area: " << cameraIndex << " at: " << click << " inside of: " << gifRect << endl;
            selectedGifIndex = cameraIndex;
            lastPosition = click;
            interactionMode = "MOVE";
            return;
        }

        cameraIndex++;
        
    }
//    Rectf gifRect = mCameraManager->mGifRectPreview[cameraIndex];
}
void gifFromWebcamApp::mouseUp( MouseEvent event )
{
    selectedGifIndex = -1;
}
void gifFromWebcamApp::mouseDrag( MouseEvent event )
{
    if(selectedGifIndex<0)return;
    vec2 drag = event.getPos();
    
    vec2 offset = drag - lastPosition;

    int nCameras = mCameraManager->mFeeds.size();

    if(interactionMode == "MOVE"){
        mCameraManager->mGifRectPreview[selectedGifIndex] += offset;
        mCameraManager->mGifAreaOffset[selectedGifIndex] += offset*vec2(nCameras);
    }
//    else if (interactionMode == "SCALE"){
//        float scale = 1.01;
//        mCameraManager->mGifRectPreview[selectedGifIndex].scaleCentered(scale);
////        const Area ao = mCameraManager->mGifAreaOffset[selectedGifIndex];
////        Rectf aor(ao);
////        aor.scaleCentered(scale*nCameras);
////        mCameraManager->mGifAreaOffset[selectedGifIndex] = Area(aor);
//    }

    lastPosition = drag;

}

void gifFromWebcamApp::draw()
{
	gl::clear();
    gl::setMatricesWindow( getWindowSize() );

    const int currentCamera = mCameraManager->GetCurrentCamera();
    
    for (auto const& element : mCameraManager->mFeeds)
    {
        gl::TextureRef texture = element.second;
        if(texture){
            int cameraIndex = element.first;

            gl::color( ColorA( 1, 1, 1, 1 ) );
            Rectf feedRect = mCameraManager->mFeedRectPreview[cameraIndex];
            gl::draw( texture, feedRect);

            Rectf gifRect = mCameraManager->mGifRectPreview[cameraIndex];
            gl::color( ColorA( .7, .3, .1, 1 ) );
            gl::drawStrokedRect( gifRect, 2.0 );

          if (currentCamera!=cameraIndex)continue;
            gl::color( ColorA( .1, 1, .1, 1 ) );
            gl::drawStrokedRect( feedRect, 5.0 );

        }
    }
}

CINDER_APP(gifFromWebcamApp, RendererGl, &gifFromWebcamApp::prepare)
