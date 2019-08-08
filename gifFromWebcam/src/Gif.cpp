#include "Gif.h"

#include "cinder/Utilities.h"
#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mlx;

Gif::Gif(int maxFrames)
: mMaxFrames( maxFrames )
, mFrameCounter(0)
, mSaved(false)
{
	mGifEncoder.setup(640,480,0.02f);
}

void Gif::AddFrame(gl::TextureRef frame)
{
    if (mFrameCounter< mMaxFrames)
    {
        //CI_LOG_I(mFrameCounter);
        //mGifEncoder.addFrame(mTexture);
        mGifEncoder.addFrame( frame );
        mFrameCounter++;
    }
    if (mFrameCounter >= mMaxFrames && !mSaved)
    {
        CI_LOG_I("saving");
        CI_LOG_I(getHomeDirectory());
        CI_LOG_I(getAssetPath(""));
        mGifEncoder.save("test.gif");
        mSaved = true;
    }
}
