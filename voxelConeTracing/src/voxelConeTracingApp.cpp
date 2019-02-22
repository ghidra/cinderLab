#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Assets.h"

#include "Material.h"

#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//struct Material
//{
 //   vec3 diffuseColor;
  //  vec3 specularColor;
   // float diffuseReflectivity;
   // float specularReflectivity;
   // float emisivity;
    //float transparency;
//};

class voxelConeTracingApp : public App {
	public:
	    static void prepareSettings( Settings *settings );
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
		//signals::ScopedConnection		mVoxelizationConnection;
		//gl::GlslProgRef mTexture3dDebugProg;//a program to look at the texture3D flat on screen

		CameraOrtho mVoxelizationCamera;
		//void initVoxelization();
		//void voxelize(bool clearVoxelizationFirst=true);

		// visualization of voxels
		//gl::BatchRef		mVisualizeCubeFront;//this is the final rendered mesh
		//gl::BatchRef		mVisualizeCubeBack;
		//gl::GlslProgRef  mVisualizeVoxelWorldPosProg;
		gl::GlslProgRef  mVisualizeVoxelSimpleProg;
		signals::ScopedConnection		mVisualizeVoxelSimpleConnection;
		//void initVoxelVisualization();
		//void renderVoxelVisualization();

		//------------------

		//our first object
		gl::BatchRef		mGeoCube;//this is the final rendered mesh
        gl::BatchRef		mGeoCubeVoxelize;//this is the batch that will render to the 3d texture after voxelization
        //Material mGeoCubeMaterial;
        //gl::UboRef mGeoCubeUbo;

        //This is the master shader, that will render the objects
        gl::GlslProgRef		mVoxelConeTrace;
};

void voxelConeTracingApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->disableFrameRate();
	settings->setFullScreen( false );
}

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

    //mVoxelizationProg->uniform( "texture3D",0);
    //mGeoCubeUbo.diffuseColor = vec3(1,0,0);
    //mGeoCubeUbo.specularColor = vec3(1,1,1);
    //mGeoCubeUbo.diffuseReflectivity = 0.2f;
    //mGeoCubeUbo.specularReflectivity = 0.2f;
    //mGeoCubeUbo.emissivity = 0.0f;
    //mGeoCubeUbo.transparency = 0.0f;

    //mVoxelizationProg->uniform( "material.diffuseColor",vec3(1,0,0));
    //mVoxelizationProg->uniform( "material.specularColor",vec3(1,1,1));
    //mVoxelizationProg->uniform( "material.diffuseReflectivity",0.2f);
   // mVoxelizationProg->uniform( "material.specularReflectivity",0.2f);
    //mVoxelizationProg->uniform( "material.emissivity",0.0f);
    //mVoxelizationProg->uniform( "material.transparency",0.0f);


	mVoxelTexSize = 64;//128 //256;

	gl::Texture3d::Format tex3dFmt;
	tex3dFmt.setWrapR(GL_REPEAT);
	tex3dFmt.setWrapS(GL_REPEAT);
	tex3dFmt.setWrapT(GL_REPEAT);
	tex3dFmt.setMagFilter(GL_LINEAR);
	tex3dFmt.setMinFilter(GL_LINEAR);
	tex3dFmt.setDataType(GL_FLOAT);
	tex3dFmt.setInternalFormat(GL_RGBA8_SNORM);
	//tex3dFmt.setInternalFormat(GL_IMAGE_3D);

	mVoxelTex = gl::Texture3d::create(mVoxelTexSize, mVoxelTexSize, mVoxelTexSize, tex3dFmt);
	//mVoxelTex->update(data.data(), GL_RGBA, tex3dFmt.getDataType(), 0, mVoxelTex->getWidth(), mVoxelTex->getHeight(), mVoxelTex->getDepth());

	//set the orthographic camera information

	mVoxelizationCamera.setOrtho(-1.0f,1.0f,-getWindowAspectRatio(), getWindowAspectRatio(),-1.0f,1.0f);
	//mVoxelizationCamera.setOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	//make the first batch
	mGeoCubeVoxelize = gl::Batch::create( geom::Cube() ,mVoxelizationProg);
	//mGeoCubeUbo = gl::Ubo::create(sizeof(mGeoCubeUbo),&mGeoCubeUbo,GL_DYNAMIC_COPY)
    //mGeoCubeUbo->bindBufferBase(0);
	//make the simple visualization shader
	mVisualizeVoxelSimpleConnection = assets()->getShader( "quadpassthrough.vert", "voxelvisualizationsimple.frag",
                                      [this]( gl::GlslProgRef glsl ) {
                                          mVisualizeVoxelSimpleProg = glsl;
                                          mVisualizeVoxelSimpleProg->uniform( "uVoxels",0);
										  mVisualizeVoxelSimpleProg->uniform( "uResolution", vec2((float)app::getWindowSize().x, (float)app::getWindowSize().y));
										  mVisualizeVoxelSimpleProg->uniform( "uVoxelResolution", (float)mVoxelTexSize);
                                      } );
    /*mVisualizeVoxelSimpleProg = loadGlslProg( gl::GlslProg::Format()
		.vertex(loadAsset("quadpassthrough.vert"))
		.fragment(loadAsset("voxelvisualizationsimple.frag")));
    mVisualizeVoxelSimpleProg->uniform( "uVoxels",0);
      mVisualizeVoxelSimpleProg->uniform( "uResolution", vec2((float)app::getWindowSize().x, (float)app::getWindowSize().y));
      mVisualizeVoxelSimpleProg->uniform( "uVoxelResolution", (float)mVoxelTexSize);*/
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
    {
        gl::clear( Color( 0, 0, 0 ) );

        //---------------do the voxelization

        gl::disable(GL_DEPTH_TEST);
        gl::disable(GL_CULL_FACE);
        gl::disable(GL_BLEND);

        gl::setMatrices(mVoxelizationCamera);//set the camera up
        gl::ScopedTextureBind scoped3dTex(mVoxelTex,0);//bind the voxel texture

        mGeoCubeVoxelize->draw();//draw the cube
    }
	//gl::ScopedGlslProg scopedRenderProg(mVoxelizationProg);
	//mVoxelizationProg->uniform("spriteSize", mSpriteSize);//set the uniforms in here

	//now render the objects with these settings

	//-------------done with the voxelization

	//now visualize the voxelization somehow
    {
        gl::clear();
        gl::setMatricesWindow( getWindowSize() );

        gl::ScopedGlslProg glslScp( mVisualizeVoxelSimpleProg );
        gl::ScopedTextureBind scoped3dTexb(mVoxelTex,0);//bind the voxel texture AGAIN JUST IN CASE
        gl::drawSolidRect( getWindowBounds() );

	}
}

CINDER_APP( voxelConeTracingApp, RendererGl, &voxelConeTracingApp::prepareSettings )
