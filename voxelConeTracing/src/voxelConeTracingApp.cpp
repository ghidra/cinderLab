#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class voxelConeTracingApp : public App {
	public:
		void setup() override;
		void mouseDown( MouseEvent event ) override;
		void update() override;
		void draw() override;

	private:
		gl::Texture3dRef mVoxelTex;
		int mVoxelTexSize;
		gl::GlslProgRef mVoxelizationProg;
		gl::GlslProgRef mTexture3dDebugProg;//a program to look at the texture3D flat on screen
};

void voxelConeTracingApp::setup()
{
	auto loadGlslProg = [&](const gl::GlslProg::Format& format) -> gl::GlslProgRef
	{
		string names = format.getVertexPath().string() + " + " +
			format.getFragmentPath().string();
		gl::GlslProgRef glslProg;
		try {
			glslProg = gl::GlslProg::create(format);
		}
		catch (const Exception& ex) {
			CI_LOG_EXCEPTION(names, ex);
			quit();
		}
		return glslProg;
	};

	mVoxelizationProg = loadGlslProg( gl::GlslProg::Format().version(450)
		.vertex(loadAsset("voxelization.vert"))
		.geometry(loadAsset("voxelization.geom"))
		.fragment(loadAsset("voxelization.frag")));
	mTexture3dDebugProg = loadGlslProg(gl::GlslProg::Format().version(450)
		.vertex(loadAsset("pass_through.vert"))
		.fragment(loadAsset("texture3dDebug.frag")));

	mVoxelTexSize = 256;

	gl::Texture3d::Format tex3dFmt;
	tex3dFmt.setWrapR(GL_REPEAT);
	tex3dFmt.setWrapS(GL_REPEAT);
	tex3dFmt.setWrapT(GL_REPEAT);
	tex3dFmt.setMagFilter(GL_LINEAR);
	tex3dFmt.setMinFilter(GL_LINEAR);
	tex3dFmt.setDataType(GL_FLOAT);
	tex3dFmt.setInternalFormat(GL_RGBA8_SNORM);

	mVoxelTex = gl::Texture3d::create(mVoxelTexSize, mVoxelTexSize, mVoxelTexSize, tex3dFmt);
	//mVoxelTex->update(data.data(), GL_RGBA, tex3dFmt.getDataType(), 0, mVoxelTex->getWidth(), mVoxelTex->getHeight(), mVoxelTex->getDepth());

}

void voxelConeTracingApp::mouseDown( MouseEvent event )
{
}

void voxelConeTracingApp::update()
{

	gl::ScopedTextureBind scoped3dTex(mVoxelTex);

}

void voxelConeTracingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
}

CINDER_APP( voxelConeTracingApp, RendererGl )
