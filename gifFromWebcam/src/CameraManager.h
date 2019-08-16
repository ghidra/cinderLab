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
        CameraManager(int cameraWidth, int cameraHeight, int gifWidth, int gifHeight, int feedFPS, int recordFPS)
        : mWidth(cameraWidth),
        mHeight(cameraHeight),
        mGifWidth(gifWidth),
        mGifHeight(gifHeight),
        mFrameCounter(0),
        mCurrentCamera(-1),
        mRecordEveryFrame(feedFPS/recordFPS),
        mFrameDuration(1.0/recordFPS)
        {
            mGif = GifRef(new Gif(mGifWidth, mGifHeight, -1, mFrameDuration));
        };
			~CameraManager(){}
        
        bool Update();
        
        bool Update(int cameraIndex);
        void StartCapture(int cameraIndex, std::string& gameID);
        void AddCamera(Capture::DeviceRef device, int width, int height);
        void EndGame(std::string& gameID);
        const int GetCurrentCamera() const {return mCurrentCamera;};

            vector<CaptureRef>			mCapture;
            map<int, gl::TextureRef>    mPreviews;
            vector<Area> mGifAreaOffset;

        private:
            gl::TextureRef        mTexture;
            GifRef      mGif;
            int mCurrentCamera;
            int mWidth;
            int mHeight;
            int mGifWidth;
            int mGifHeight;
            float mFrameDuration;
            int mRecordEveryFrame;
            int mFrameCounter;
        
    };
}
