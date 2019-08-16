#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"

#include "GifEncoder.h"

using namespace std;
using namespace ci;
using namespace ci::app;


namespace mlx
{
	typedef std::shared_ptr<class Gif> GifRef;
	class Gif {
		public:
			Gif(int width, int height, int maxFrames, float frameDuration);
			~Gif(){}

			void AddFrame(gl::TextureRef frame);
            int GetNumberOfFrames() const {return mGifEncoder.getNumberOfFrames();};
            const string Save();

			GifEncoder	mGifEncoder;

			int			mFrameCounter;
			int			mMaxFrames;
            std::string mFileName;
            std::string mCurrentGameID;


    };
}
