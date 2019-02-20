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

		//------------------
		// voxelization
		//------------------
		gl::Texture3dRef mVoxelTex;
		int mVoxelTexSize;
		gl::GlslProgRef mVoxelizationProg;
		//gl::GlslProgRef mTexture3dDebugProg;//a program to look at the texture3D flat on screen

		CameraOrtho mVoxelizationCamera;
		//void initVoxelization();
		//void voxelize(bool clearVoxelizationFirst=true);

		// visualization of voxels
		//gl::BatchRef		mVisualizeCubeFront;//this is the final rendered mesh
		//gl::BatchRef		mVisualizeCubeBack;
		//gl::GlslProgRef  mVisualizeVoxelWorldPosProg;
		gl::GlslProgRef  mVisualizeVoxelSimpleProg;
		//void initVoxelVisualization();
		//void renderVoxelVisualization();
		//------------------

		//our first object
		gl::BatchRef		mGeoCube;//this is the final rendered mesh
        gl::BatchRef		mGeoCubeVoxelize;//this is the batch that will render to the 3d texture after voxelization

        //This is the master shader, that will render the objects
        gl::GlslProgRef		mVoxelConeTrace;
};

void voxelConeTracingApp::setup()
{
	auto loadGlslProg = [&](const gl::GlslProg::Format& format) -> gl::GlslProgRef
	{
		string names = format.getVertexPath().string() + " + " + format.getFragmentPath().string();
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

	mVoxelizationProg = loadGlslProg( gl::GlslProg::Format().version(440)
		.vertex(loadAsset("voxelization.vert"))
		.geometry(loadAsset("voxelization.geom"))
		.fragment(loadAsset("voxelization.frag")));
	/*mTexture3dDebugProg = loadGlslProg(gl::GlslProg::Format().version(450)
		.vertex(loadAsset("pass_through.vert"))
		.fragment(loadAsset("texture3dDebug.frag")));*/

	mVoxelTexSize = 64;//256;

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

	//set the orthographic camera information

	mVoxelizationCamera.setOrtho(-1.0f,1.0f,-getWindowAspectRatio(), getWindowAspectRatio(),-1.0f,1.0f);
	//mVoxelizationCamera.setOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	//make the first batch
	mGeoCubeVoxelize = gl::Batch::create( geom::Cube() ,mVoxelizationProg);
}

void voxelConeTracingApp::mouseDown( MouseEvent event )
{
}

void voxelConeTracingApp::update()
{

	//gl::ScopedTextureBind scoped3dTex(mVoxelTex);

}

void voxelConeTracingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

	//---------------do the voxelization

	gl::disable(GL_DEPTH_TEST);
	gl::disable(GL_CULL_FACE);
	gl::disable(GL_BLEND);

	gl::setMatrices(mVoxelizationCamera);//set the camera up
	gl::ScopedTextureBind scoped3dTex(mVoxelTex,0);//bind the voxel texture

    mGeoCubeVoxelize->draw();//draw the cube
	//gl::ScopedGlslProg scopedRenderProg(mVoxelizationProg);
	//mVoxelizationProg->uniform("spriteSize", mSpriteSize);//set the uniforms in here

	//now render the objects with these settings

	//-------------done with the voxelization

	//now visualize the voxelization somehow

    gl::clear();
	gl::setMatricesWindow( getWindowSize() );

	gl::ScopedGlslProg glslScp( mVisualizeVoxelSimpleProg );
	mVisualizeVoxelSimpleProg->uniform( "texture3D", 0 );
	mVisualizeVoxelSimpleProg->uniform( "uVoxelTexSize,  mVoxelTexSize);
	gl::drawSolidRect( getWindowBounds() );
}

CINDER_APP( voxelConeTracingApp, RendererGl )
