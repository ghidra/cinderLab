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
			Gif(int maxFrames = 1);
			~Gif(){}

			void AddFrame(gl::TextureRef frame);

			GifEncoder	mGifEncoder;

			int			mFrameCounter;
			int			mMaxFrames;
			bool		mSaved;

    };
}
