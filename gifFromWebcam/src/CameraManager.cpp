#include "CameraManager.h"
#include "CommandManager.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"


using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mlx;


void CameraManager::AddCamera(Capture::DeviceRef device)
{
        //connect to the camera
        try {
            CaptureRef capture = Capture::create(640, 480, device );
            mCapture.push_back(capture);
            capture->start();
        }
        catch (ci::Exception &exc) {
            CI_LOG_EXCEPTION("Failed to init capture ", exc);
        }

}
void CameraManager::StartCapture(int cameraIndex, std::string& gameID)
{
    if(mGif->mCurrentGameID != gameID){
        // first game, reset gameID and fileName
        mGif->mCurrentGameID = gameID;

        mGif->mFileName = gameID + ".gif";
        mGif->mGifEncoder.clearFrames();
    }

    // reset frame counter effectively starting the countdown on the next tick
    mGif->mMaxFrames = 30;
    mGif->mFrameCounter = 0;
    mCurrentCamera = cameraIndex;
}

void CameraManager::EndGame(std::string& gameID){

    if(mGif->mCurrentGameID != gameID){
        CI_LOG_E("The current game ID and the requested one are different");
        return;
    }
    const string filePath = mGif->Save();

    string command = "/usr/local/bin/aws s3 cp ";
    command += filePath;
    command += " s3://joyridegame/ --acl public-read";
    CI_LOG_V(command.c_str());
    string result = exec(command.c_str());
    CI_LOG_V(result);

    command = "/usr/bin/curl -d 'gameid=";
    command += gameID;
    command += "&image_url=";
    command += url_encode("https://joyridegame.s3.amazonaws.com/" + gameID + ".gif");
    command +="' -X POST https://joyridegame.reconstrukt.net/api/v1/game/finish";

    CI_LOG_V(command.c_str());
    result = exec(command.c_str());
    CI_LOG_V(result);

}

bool CameraManager::Update(){
    bool result = false;
    for(std::size_t i=0; i<mCapture.size(); ++i)
    {
        result = Update(i);
    }
    
    return result;
}

bool CameraManager::Update(int cameraIndex)
{
    if (mCapture.size() && mCapture[cameraIndex]->checkNewFrame()) {
        auto surface = *mCapture[cameraIndex]->getSurface();
        mPreviews.at(cameraIndex)->update(surface);

        if(mCurrentCamera<0 || mCurrentCamera > mCapture.size() || mCurrentCamera != cameraIndex)return true;
        CI_LOG_V("attempting updating camera index");
        CI_LOG_V(mCurrentCamera);
        if (!mTexture) {
            // Capture images come back as top-down, and it's more efficient to keep them that way
            mTexture = gl::Texture::create(surface, gl::Texture::Format().loadTopDown());
        }
        else {
            mTexture->update(surface);
        }

        mGif->AddFrame(mTexture);
        //no more frames need to be added for this camera, reset;
        if(mGif->mFrameCounter == -1){
            mCurrentCamera = -1;
        }
        return true;
    }
    else
    {
        return false;
    }
}

