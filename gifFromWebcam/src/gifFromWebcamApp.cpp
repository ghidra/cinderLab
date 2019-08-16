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

const int WIDTH = 1680;
const int HEIGHT = 900;
const int TEXTURE_WIDTH = 1280;
const int TEXTURE_HEIGHT = 720;
const int GIF_WIDTH = 480;
const int GIF_HEIGHT = 480;
const float FEED_FPS = 30.0;
const int RECORD_FPS = 5;

class gifFromWebcamApp : public App {
public:
//    gifFromWebcamApp():mFPS(30.0){};
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
    settings->setFrameRate(FEED_FPS);
}


void gifFromWebcamApp::setup()
{

//    float frameDuration = 1.0 / RECORD_FPS;
    /// 1000.0f;
    mCameraManager = mlx::CameraManagerRef(new mlx::CameraManager(TEXTURE_WIDTH, TEXTURE_HEIGHT, GIF_WIDTH, GIF_HEIGHT, FEED_FPS, RECORD_FPS));
    //cameras
    int cameraIndex = 0;
    for (const auto &device : Capture::getDevices()) {
        console() << "Device: " << device->getName() << " Camera Index: " <<  cameraIndex << endl;
        mCameraManager->AddCamera(device, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        mCameraManager->mPreviews[cameraIndex] = gl::Texture::create(TEXTURE_WIDTH, TEXTURE_HEIGHT);
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
    gl::setMatricesWindow( getWindowSize() );
    int nCameras = mCameraManager->mPreviews.size();
    int maxPerRow = int(ceil(sqrt(double(nCameras))));
    const int currentCamera = mCameraManager->GetCurrentCamera();
    
    
    
    for (auto const& element : mCameraManager->mPreviews)
    {
        gl::TextureRef texture = element.second;
        if(texture){
            int cameraIndex = element.first;
            int xIndex = cameraIndex % maxPerRow;
            int yIndex = cameraIndex / maxPerRow;
            int width = texture->getWidth() / nCameras;
            int height = texture->getHeight() / nCameras;
            
            Rectf window = getWindowBounds();
            int halfHeight = height/2;
            int wierdOffset = window.getHeight()/nCameras;
            
            int startX1 = xIndex * width;
            int startY1 = wierdOffset + halfHeight + yIndex * height;
            int endX2 = startX1 + width;
            int endY2 = startY1 + height;

            Rectf drawRect( startX1, startY1, endX2, endY2 );

            gl::color( ColorA( 1, 1, 1, 1 ) );
            gl::draw( texture, drawRect);
            if (currentCamera!=cameraIndex)continue;

            gl::color( ColorA( .1, 1, .1, 1 ) );
            gl::drawStrokedRect( drawRect, 5.0 );
            
//            Area gifArea = mCameraManager->mGifAreaOffset[cameraIndex];
//            Area gifArea = Area(200, 200, 600, 600);
//            gl::setMatricesWindow( getWindowSize() );
//            Rectf gifAreaRect(gifArea);
//            Rectf myGifArea(0, 0, 480*(1.0f/maxPerRow), 480*(1.0f/maxPerRow));

//            myGifArea.offset(vec2(startX1, startY1));
//            myGifArea.scaleCentered(1.0f/maxPerRow);
            
//            gifAreaRect.offset(vec2(startX1, startY1));
            
            
//            gl::color( ColorA( .7, .3, .1, 1 ) );
//            gl::drawStrokedRect( gifArea, 2.0 );

        }
    }
}

CINDER_APP(gifFromWebcamApp, RendererGl, &gifFromWebcamApp::prepare)
