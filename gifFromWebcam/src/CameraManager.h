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
			CameraManager(Capture::DeviceRef device);
			~CameraManager(){}
        
            bool Update();
            void NewGif();

			CaptureRef			mCapture;
			gl::TextureRef		mTexture;
        private:
            vector<GifRef>      mGifs;

    };
}
