#pragma once

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "cinder/ip/Premultiply.h"

#include "Gif.h"

using namespace std;
using namespace ci;
using namespace ci::app;


namespace mlx
{
	typedef std::shared_ptr<class CameraManager> CameraManagerRef;
	class CameraManager {
		public:
        CameraManager(int feedWidth, int feedHeight, int gifWidth, int gifHeight, int feedFPS, int recordFPS)
        : mFeedWidth(feedWidth),
        mFeedHeight(feedHeight),
        mGifWidth(gifWidth),
        mGifHeight(gifHeight),
        mFrameCounter(0),
        mCurrentCamera(-1),
        mRecordEveryFrame(feedFPS/recordFPS),
        mFrameDuration(1.0/recordFPS)
        {
            mGif = GifRef(new Gif(mGifWidth, mGifHeight, -1, mFrameDuration));
            overlay = Surface(loadImage(getAssetPath("") / "overlay.png"));
            ip::premultiply(&overlay);
        };
			~CameraManager(){}
        
        bool Update();
        
        bool Update(int cameraIndex);
        void StartCapture(int cameraIndex, std::string& gameID);
        void AddCamera(Capture::DeviceRef device);
        void EndGame(std::string& gameID);
        void SetupPreviewsAndOffsets();
        const int GetCurrentCamera() const {return mCurrentCamera;};

            vector<CaptureRef>			mCaptures;
            map<int, gl::TextureRef>    mFeeds;
            vector<Area> mGifAreaOffset;
            vector<Rectf> mGifRectPreview;
            vector<Rectf> mFeedRectPreview;

        private:
            gl::TextureRef        mTexture;
            GifRef      mGif;
            int mCurrentCamera;
            int mFeedWidth;
            int mFeedHeight;
            int mGifWidth;
            int mGifHeight;
            float mFrameDuration;
            int mRecordEveryFrame;
            int mFrameCounter;
            Surface overlay;
        
    };
}
