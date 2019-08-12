#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"

#include "Gif.h"

using namespace std;
using namespace ci;
using namespace ci::app;


namespace mlx
{
	typedef std::shared_ptr<class CameraManager> CameraManagerRef;
	class CameraManager {
		public:
        CameraManager(){
            mGif = GifRef(new Gif(-1));
        };
			~CameraManager(){}
        
        bool Update();
        
        bool Update(int cameraIndex);
        void StartCapture(int cameraIndex, std::string& gameID);
        void AddCamera(Capture::DeviceRef device);
        void EndGame(std::string& gameID);

            vector<CaptureRef>			mCapture;
            map<int, gl::TextureRef>    mPreviews;

        private:
            gl::TextureRef        mTexture;
            GifRef      mGif;
            int mCurrentCamera;
    };
}
