#include "CameraManager.h"
#include "CommandManager.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"
#include "cinder/ConcurrentCircularBuffer.h"

ConcurrentCircularBuffer<gl::TextureRef>    *mImages;
using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mlx;


void CameraManager::AddCamera(Capture::DeviceRef device, int width, int height)
{
        //connect to the camera
        try {
            CaptureRef capture = Capture::create(width, height, device);
            mCapture.push_back(capture);
            capture->start();
        }
        catch (ci::Exception &exc) {
            CI_LOG_EXCEPTION("Failed to init capture ", exc);
            return;
        }
    
    
    vec2 center = vec2(mWidth/2, mHeight/2);
    float gifHalfWidth = mGifWidth/2;
    float gifHalfHeight = mGifHeight/2;
    vec2 ul = vec2(center.x - gifHalfWidth, center.y - gifHalfHeight);
    vec2 lr = ul + vec2(mGifWidth, mGifHeight);
    Area gifArea = Area(ul, lr);
    mGifAreaOffset.push_back(gifArea);

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
    mFrameCounter = 0;
    mCurrentCamera = cameraIndex;
}

void CameraManager::EndGame(std::string& gameID){

    if(mGif->mCurrentGameID != gameID){
        CI_LOG_E("The current game ID and the requested one are different");
        return;
    }
    const string filePath = mGif->Save();
//
//    string command = "/usr/local/bin/aws s3 cp ";
//    command += filePath;
//    command += " s3://joyridegame/ --acl public-read";
//    CI_LOG_V(command.c_str());
//    string result = exec(command.c_str());
//    CI_LOG_V(result);
//
//    command = "/usr/bin/curl -d 'gameid=";
//    command += gameID;
//    command += "&image_url=";
//    command += url_encode("https://joyridegame.s3.amazonaws.com/" + gameID + ".gif");
//    command +="' -X POST https://joyridegame.reconstrukt.net/api/v1/game/finish";
//
//    CI_LOG_V(command.c_str());
//    result = exec(command.c_str());
//    CI_LOG_V(result);
    // reset frame counter effectively starting the countdown on the next tick
    mGif->mMaxFrames = 30;
    mGif->mFrameCounter = 0;
    mFrameCounter = 0;
    mCurrentCamera = -1;
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

        // check that no camera need to be captured
        if(mCurrentCamera<0 || mCurrentCamera > mCapture.size() || mCurrentCamera != cameraIndex)return true;
        // if a camera needs capturing, check if we can capture this frame
        if(mFrameCounter % mRecordEveryFrame != 0){
            console() << "skipping frame: " << mFrameCounter << endl;
            mFrameCounter++;
            return true;
        }
        console() << "capturing frame from camera index: " << mCurrentCamera << endl;

        if(mGifWidth == mWidth && mGifHeight == mHeight){
            if (!mTexture) {
                mTexture = gl::Texture::create(surface, gl::Texture::Format().loadTopDown());
            }
            else {
                mTexture->update(surface);
            }
        }
        else{
            Area gifArea = mGifAreaOffset[cameraIndex];
            Surface newSurface( mGifWidth, mGifHeight, false );
            newSurface.copyFrom( surface, gifArea,  -gifArea.getUL() );
            
            if (!mTexture) {
                mTexture = gl::Texture::create(newSurface, gl::Texture::Format().loadTopDown());
            }
            else {
                mTexture->update(newSurface);
            }
        }
        

        mGif->AddFrame(mTexture);
        console() << "gif now contains: " << mGif->GetNumberOfFrames() << " frames" << endl;
        //no more frames need to be added for this camera, reset;
        if(mGif->mFrameCounter == -1){
            mCurrentCamera = -1;
            mFrameCounter = 0;
        }
        else{
            mFrameCounter++;
        }
        
        return true;
    }
    else
    {
        return false;
    }
}

