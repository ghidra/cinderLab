#include "CameraManager.h"

#include "cinder/Utilities.h"
#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mlx;

CameraManager::CameraManager(Capture::DeviceRef device)
{
    //connect to the camera
	try {
        mCapture = Capture::create(640, 480, device );
        mCapture->start();
    }
    catch (ci::Exception &exc) {
        CI_LOG_EXCEPTION("Failed to init capture ", exc);
    }
    
}
void CameraManager::NewGif()
{
    mGifs.push_back( GifRef(new Gif(60)) );
}

bool CameraManager::Update()
{
    if (mCapture && mCapture->checkNewFrame()) {
        if (!mTexture) {
            // Capture images come back as top-down, and it's more efficient to keep them that way
            mTexture = gl::Texture::create(*mCapture->getSurface(), gl::Texture::Format().loadTopDown());
        }
        else {
            mTexture->update(*mCapture->getSurface());
        }
        
        //update all the gifs
        for(std::size_t i=0; i<mGifs.size(); ++i)
        {
            mGifs[i]->AddFrame(mTexture);
        }
        
        return true;
    }
    else
    {
        return false;
    }
}

