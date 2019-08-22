#include "HTTPRequest.hpp"
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
#else
const string awsCommand = "/usr/local/bin/aws s3 cp ";
#endif //CINDER_MSW

void CameraManager::AddCamera(Capture::DeviceRef device)
{
    //connect to the camera
    try {
        CaptureRef capture = Capture::create(mFeedWidth, mFeedWidth, device);
        mCaptures.push_back(capture);
        capture->start();
    }
    catch (ci::Exception &exc) {
        CI_LOG_EXCEPTION("Failed to init capture ", exc);
        return;
    }
    
 

}
void CameraManager::SetupPreviewsAndOffsets(){
    
    int nCameras = mCaptures.size();
    int maxPerRow = int(ceil(sqrt(double(nCameras))));
    int widthPreview = mFeedWidth / nCameras;
    int heightPreview = mFeedHeight / nCameras;
    
    
    vec2 center = vec2(mFeedWidth/2, mFeedHeight/2);
    float gifHalfWidth = mGifWidth/2;
    float gifHalfHeight = mGifHeight/2;
    vec2 ul = vec2(center.x - gifHalfWidth, center.y - gifHalfHeight);
    vec2 lr = ul + vec2(mGifWidth, mGifHeight);
    
    float scale = 1.0f/maxPerRow;

    for (auto cameraIndex=0; cameraIndex<nCameras; cameraIndex++){

        Area gifArea(ul, lr);
        mGifAreaOffset.push_back(gifArea);

        // create the preview texture at original resuolution
        mFeeds[cameraIndex] = gl::Texture::create(mFeedWidth, mFeedHeight);
        
        int xIndex = cameraIndex % maxPerRow;
        int yIndex = cameraIndex / maxPerRow;

        int startX1 = xIndex * widthPreview;
        int startY1 = yIndex * heightPreview;
        int endX2 = startX1 + widthPreview;
        int endY2 = startY1 + heightPreview;

        // create feed drawing rectangle
        Rectf drawRect( startX1, startY1, endX2, endY2 );
        mFeedRectPreview.push_back(drawRect);
        
        // create gif drawing rectangle
        Rectf myGifArea(gifArea);
        myGifArea.scale(scale);
        myGifArea.offset(drawRect.getUpperLeft());
        mGifRectPreview.push_back(myGifArea);
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
    mGif->mMaxFrames = mMaxFramesPerGame;
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

	mGif->mMaxFrames = mMaxFramesPerGame;
	mGif->mFrameCounter = 0;
	mFrameCounter = 0;
	mCurrentCamera = -1;

#ifdef CINDER_MSW
	// S3 Upload
    string command = awsCommand;
    command += filePath;
    command += " s3://joyridegame/ --acl public-read";
	console() << command.c_str() << endl;
    string result = exec(command.c_str());
	console() << result << endl;


//    End Game Endpoint API
	try
	{
		http::Request request("http://joyridegame.reconstrukt.net/api/v1/game/finish");

		std::map<std::string, std::string> parameters = { {"gameid", gameID}, {"image_url",  "https://joyridegame.s3.amazonaws.com/" + gameID + ".gif"} };
		http::Response response = request.send("POST", parameters, {
			"Content-Type: application/x-www-form-urlencoded"
			});
		console() << std::string(response.body.begin(), response.body.end()) << std::endl; // print the result
	}
	catch (const std::exception& e)
	{
		console() << "Request failed, error: " << e.what() << std::endl;
	}
#endif //CINDER_MSW
}

bool CameraManager::Update(){
    bool result = false;
	
    for(std::size_t i=0; i<mCaptures.size(); ++i)
    {
        result = Update(i);
    }
    
    return result;
}

bool CameraManager::Update(int cameraIndex)
{
    if (mCaptures.size() && mCaptures[cameraIndex]->checkNewFrame()) {
        auto surface = *mCaptures[cameraIndex]->getSurface();
        mFeeds.at(cameraIndex)->update(surface);

        // check that no camera need to be captured
        if(mCurrentCamera<0 || mCurrentCamera > mCaptures.size() || mCurrentCamera != cameraIndex)return true;
        // if a camera needs capturing, check if we can capture this frame
        if(mFrameCounter % mRecordEveryFrame != 0){
            console() << "skipping frame: " << mFrameCounter << endl;
            mFrameCounter++;
            return true;
        }
        console() << "capturing frame from camera index: " << mCurrentCamera << endl;

        if(mGifWidth == mFeedWidth && mGifHeight == mFeedHeight){
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

            Surface::ConstIter maskIter( overlay.getIter() );
            Surface::Iter targetIter( newSurface.getIter() );
            while( maskIter.line() && targetIter.line() ) {
                while( maskIter.pixel() && targetIter.pixel() ) {
                    float maskValue = maskIter.a() / 255.0;
                    //targetIter.r() = (maskIter.r()*maskValue) + ((1.0 - maskValue) * targetIter.r());
                    //targetIter.g() = (maskIter.g()*maskValue) + ((1.0 - maskValue) * targetIter.g());
                    //targetIter.b() = (maskIter.b()*maskValue) + ((1.0 - maskValue) * targetIter.b());
                    targetIter.r() = (255.0*maskValue) + ((1.0 - maskValue) * targetIter.r());
                    targetIter.g() = (255.0*maskValue) + ((1.0 - maskValue) * targetIter.g());
                    targetIter.b() = (255.0*maskValue) + ((1.0 - maskValue) * targetIter.b());
                }
            }
            
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

