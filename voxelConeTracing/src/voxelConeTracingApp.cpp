#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

//#include "cinder/GeomIO.h"//for the vbomeshref

#include "Assets.h"
#include "Compute.h"

#include "Material.h"
#include "CustomGeo.h"

#include "cinder/Log.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace vct;

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
		//gl::Texture3dRef mVoxelTex;
		int mVoxelTexSize;
		int mNumVoxels;//
		//gl::GlslProgRef mVoxelizationProg;
		//signals::ScopedConnection		mVoxelizationConnection;
		//gl::GlslProgRef mTexture3dDebugProg;//a program to look at the texture3D flat on screen

		//CameraOrtho mVoxelizationCamera;
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

		//LETS TRY GETTING EVERYTHING DONE IN THE COMPUTER SHADERS
		ComputeShaderRef			mVoxelizationShader;
		ComputeBufferRef			mVoxelBuffer;
		struct Voxel
		{
			alignas(16) vec3 P;
			alignas(16) vec3 N;
			alignas(16) vec3 Cd;
		};

		//------------------
        ComputeBufferRef mGeoBuffer;//this is my heo buffer
		//our first object
		//gl::BatchRef		mGeoCube;//this is the final rendered mesh
		//gl::VboMeshRef      mGeoCubeVboMesh;//this is to get the vbo for the compute shader
		//gl::VaoRef          mGeoCubeVao;
       // gl::BatchRef		mGeoCubeVoxelize;//this is the batch that will render to the 3d texture after voxelization
        //Material mGeoCubeMaterial;
        //gl::UboRef mGeoCubeUbo;


        //This is the master shader, that will render the objects
        gl::GlslProgRef		mVoxelConeTrace;

		//std::shared_ptr<log::LoggerBreakpoint> mLog;
};

void voxelConeTracingApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->disableFrameRate();
	settings->setFullScreen( false );


}

void voxelConeTracingApp::setup()
{
	//mLog = log::makeLogger<log::LoggerBreakpoint>();
/*
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
*/
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

    //////////////////////
	/// MAKE THE COMPUTE SHADER
	/////////////////////
	mVoxelizationShader = ComputeShader::create(loadAsset("voxelization.comp"));
	//////////////////////
	/// VOXEL DATA
	/////////////////////
	mVoxelTexSize=64;
	mNumVoxels = mVoxelTexSize*mVoxelTexSize*mVoxelTexSize;
	vector<Voxel> voxels;
	voxels.assign(mNumVoxels, Voxel());
	for (unsigned int i = 0; i < voxels.size(); ++i)
	{
		auto &v = voxels.at(i);
		v.P = vec3(0.0f, 0.0f, 0.0f);
		v.N = vec3(0.0f, 1.0f, 0.0f);
		v.Cd = vec3(0.0f, 0.0f, 0.0f);
	}
	//////////////////////
	/// MAKE THE VOXEL BUFFER
	/////////////////////
	ivec3 count = gl::getMaxComputeWorkGroupCount();
	CI_ASSERT(count.x >= (mNumVoxels / mVoxelizationShader->getWorkGroupSize().x));

	mVoxelBuffer = ComputeBuffer::create(voxels.data(), (int)voxels.size(), sizeof(Voxel));

	////////////////////
	///////////////////

    //////////////////////
	// MAKE THE GEO BUFFER
	//////////////////////
	mGeoBuffer = ComputeBuffer::create( tetrahedron.data(), (int)tetrahedron.size(), sizeof( CustomGeo ) );
	//gl::ScopedBuffer scopedQuadSsbo( mGeoBuffer->getSsbo() );
	//mGeoBuffer->getSsbo()->bindBase( 1 );

	//make the simple visualization shader
	mVisualizeVoxelSimpleConnection = assets()->getShader( "quadpassthrough.vert", "voxelvisualizationsimple.frag",
                                      [this]( gl::GlslProgRef glsl ) {
                                          mVisualizeVoxelSimpleProg = glsl;
                                          //mVisualizeVoxelSimpleProg->uniform( "uVoxels",0);
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
    /*{
        gl::clear( Color( 0, 0, 0 ) );

        //---------------do the voxelization

        gl::disable(GL_DEPTH_TEST);
        gl::disable(GL_CULL_FACE);
        gl::disable(GL_BLEND);

        gl::setMatrices(mVoxelizationCamera);//set the camera up
        gl::ScopedTextureBind scoped3dTex(mVoxelTex,0);//bind the voxel texture

        mGeoCubeVoxelize->draw();//draw the cube
    }*/
    {
        // gl::clear( Color( 0, 0, 0 ) );
       int call_count = (int)tetrahedron.size()/3;
       auto computeGlsl = mVoxelizationShader->getGlsl();
       //computeGlsl->uniform("uBufferSize",(uint32_t)mNumVoxels);
       computeGlsl->uniform("uBufferSize",(uint32_t)call_count);
       computeGlsl->uniform("uVoxelResolution",(float)mVoxelTexSize);
       //right now I want to only call this for the number of verts in the mesh

        //compute version of voxilization
        //bind the voxel buffer
        gl::ScopedBuffer scopedVoxelSsbo(mVoxelBuffer->getSsbo());
        mVoxelBuffer->getSsbo()->bindBase(0);
        //now bind the geo buffer that we want to voxelize

        gl::ScopedBuffer scopedGeoSsbo(mGeoBuffer->getSsbo());
        mGeoBuffer->getSsbo()->bindBase( 1 );
        //gl::ScopedVao vao( mGeoCubeVao );
        //auto tmp = mGeoCubeVboMesh->getVertexArrayVbos();
       //gl::ScopedBuffer scopedGeoVbo();
        //mGeoCubeVboMesh->getSsbo()->bindBase(1);
        //mVoxelizationShader->dispatch( (int)glm::ceil( float( mNumVoxels ) / mVoxelizationShader->getWorkGroupSize().x ), 1, 1);
        mVoxelizationShader->dispatch( (int)glm::ceil( float( call_count ) / mVoxelizationShader->getWorkGroupSize().x ), 1, 1);

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

            ///this should be binding the voxel buffer shader
        gl::ScopedBuffer scopedVoxelRenderSsbo(mVoxelBuffer->getSsbo());
        mVoxelBuffer->getSsbo()->bindBase(0);

        //gl::ScopedTextureBind scoped3dTexb(mVoxelTex,0);//bind the voxel texture AGAIN JUST IN CASE
        gl::drawSolidRect( getWindowBounds() );

	}
	CI_CHECK_GL();
}

CINDER_APP( voxelConeTracingApp, RendererGl(RendererGl::Options().version(4,2)), &voxelConeTracingApp::prepareSettings )
