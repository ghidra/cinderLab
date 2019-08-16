#include "CameraManager.h"
#include "CommandManager.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"


using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mlx;
#ifdef CINDER_MSW
const string awsCommand =  "\"C:\\Program Files\\Amazon\\AWSCLI\\bin\\aws.exe\" s3 cp ";
const string curlCommand =  "\"C:\\Windows\\System32\\curl.exe\" -d \"gameid=";
#else
const string awsCommand = "/usr/local/bin/aws s3 cp ";
const string curlCommand =  "/usr/bin/curl -d \"gameid=";
#endif //CINDER_MSW

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
	mGif->mMaxFrames = 30;
	mGif->mFrameCounter = 0;
	mFrameCounter = 0;
	mCurrentCamera = -1;

#ifdef CINDER_MSW
	// S3 Upload
    string command = awsCommand;
    command += filePath;
    command += " s3://joyridegame/ --acl public-read";
    CI_LOG_V(command.c_str());
    string result = exec(command.c_str());
    CI_LOG_V(result);
    
    // End Game Endpoint API
    command = curlCommand;
    command += gameID;
    command += "&image_url=";
    command += url_encode("https://joyridegame.s3.amazonaws.com/" + gameID + ".gif");
    command +="\" -X POST \"http://joyridegame.reconstrukt.net/api/v1/game/finish\"";

    CI_LOG_V(command.c_str());
    result = exec(command.c_str());
    CI_LOG_V(result);
#endif //CINDER_MSW
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

