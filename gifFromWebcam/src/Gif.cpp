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
{
	mGifEncoder.setup(640,480,0.02f);
    mCurrentGameID = "THIS-IS-NOT-A-GAME";
}

void Gif::AddFrame(gl::TextureRef frame)
{
    if (mFrameCounter >=0 && mFrameCounter< mMaxFrames)
    {
        mGifEncoder.addFrame( frame );
        mFrameCounter++;
    }
    else{
        mFrameCounter = -1;
    }
}

void Gif::Save(){
    CI_LOG_D("saving gif");
    CI_LOG_D(mFileName);
    mGifEncoder.save(mFileName);
    mGifEncoder.clearFrames();      
}
