#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"

#include "GifEncoder.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class gifFromWebcamApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;

private:
	void printDevices();

	CaptureRef			mCapture;
	gl::TextureRef		mTexture;
	GifEncoder			mGifEncoder;

	int					mFrameCounter;
	int					mMaxFrames;
	bool				mSaved;
};

void gifFromWebcamApp::setup()
{
	printDevices();

	try {
		mCapture = Capture::create(640, 480);
		mCapture->start();
	}
	catch (ci::Exception &exc) {
		CI_LOG_EXCEPTION("Failed to init capture ", exc);
	}

	////make the gif encoder
	mGifEncoder.setup(640,480,0.02f);
	mFrameCounter = 0;
	mMaxFrames = 60;
	mSaved = false;
}

void gifFromWebcamApp::mouseDown(MouseEvent event)
{
}

void gifFromWebcamApp::update()
{
	if (mCapture && mCapture->checkNewFrame()) {
		if (!mTexture) {
			// Capture images come back as top-down, and it's more efficient to keep them that way
			mTexture = gl::Texture::create(*mCapture->getSurface(), gl::Texture::Format().loadTopDown());
		}
		else {
			mTexture->update(*mCapture->getSurface());
		}

		////now save a frame
		if (mFrameCounter< mMaxFrames)
		{
			//CI_LOG_I(mFrameCounter);
			mGifEncoder.addFrame(mTexture);
		}
		if (mFrameCounter >= mMaxFrames && !mSaved)
		{
			CI_LOG_I("saving");
			CI_LOG_I(getHomeDirectory());
			CI_LOG_I(getAssetPath(""));
			mGifEncoder.save("test.gif");
			mSaved = true;
		}
		mFrameCounter++;
	}
}

void gifFromWebcamApp::draw()
{
	gl::clear();

	if (mTexture) {
		gl::ScopedModelMatrix modelScope;
		gl::draw(mTexture);
	}
}

void gifFromWebcamApp::printDevices()
{
	for (const auto &device : Capture::getDevices()) {
		console() << "Device: " << device->getName() << " " << endl;
	}
}

CINDER_APP(gifFromWebcamApp, RendererGl)
