#include "Gif.h"

#include "cinder/Utilities.h"
#include "cinder/Log.h"

using namespace std;
using namespace ci;
using namespace ci::app;
using namespace mlx;

Gif::Gif(int width, int height, int maxFrames, float frameDuration)
: mMaxFrames( maxFrames )
, mFrameCounter(0)
{
	mGifEncoder.setup(width, height, frameDuration);
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

const string Gif::Save(){
    CI_LOG_I("saving gif");
    
    fs::path fr = getAssetPath("") / mFileName;
    CI_LOG_I(fr.string());
    mGifEncoder.save(fr.string());
    mGifEncoder.clearFrames();
    return fr.string();
}
