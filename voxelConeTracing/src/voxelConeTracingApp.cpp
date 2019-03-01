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

		int mVoxelTexSize;
		int mNumVoxels;//voxelsize to power of 3
		int mNumTris;//total number of triangles I want to store

		ComputeShaderRef			mVoxelizationShader;
		//ComputeShaderRef			mClearVoxelizationShader;//i need a shader that clears out anything from that last frame

		struct Voxel
		{
			alignas(16) vec3 N;
			alignas(16) vec3 Cd;
		};

		ComputeBufferRef        mVoxelBuffer;
        ComputeBufferRef        mTriangleBuffer;//when we are voxelizing our geo... we can save the geo ssbo into a new ssbo, to render later.
        gl::VboRef                    mTriangleInd;//this is a vbo that we use to draw the ssbo as triangles

		//------------------
        ComputeBufferRef        mGeoBuffer;//this is my geo buffer

        //This is the master shader, that will render the objects
        gl::GlslProgRef		        mVoxelConeTrace;

        //vosualize the voxels
		gl::GlslProgRef                         mVisualizeVoxelSimpleProg;
		signals::ScopedConnection		mVisualizeVoxelSimpleConnection;

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
	//////////////////////////////////////////////////////////////////////////////////
	/// VOXEL DATA
	/////////////////////
	mVoxelTexSize=64;
	mNumVoxels = mVoxelTexSize*mVoxelTexSize*mVoxelTexSize;
	vector<Voxel> voxels;
	voxels.assign(mNumVoxels, Voxel());
	for (unsigned int i = 0; i < voxels.size(); ++i)
	{
		auto &v = voxels.at(i);
		//v.P = vec3(0.0f, 0.0f, 0.0f);
		v.N = vec3(0.0f, 1.0f, 0.0f);
		v.Cd = vec3(0.0f, 0.0f, 0.0f);
	}
	//////////////////////
	/// MAKE THE VOXEL BUFFER
	/////////////////////
	mVoxelBuffer = ComputeBuffer::create(voxels.data(), (int)voxels.size(), sizeof(Voxel));
    /////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MAKE VBO FOR DRAWIN TRIANGLES FROM SSBO
	///////////////////
	mNumTris = 65536;//1048576;//2097152
	vector<GLuint> tri_ids;(mNumTris*3);
	GLuint currid = 0;
	generate(tri_ids.begin(),tri_ids.end(),[&currid]()->GLuint{return currid++;});
    ///////////////////
	/// MAKE VBO IDS
	///////////////////
	mTriangleInd = gl::Vbo::create<GLuint>(GL_ARRAY_BUFFER, tri_ids);
	///////////////////
	/// MAKE TRIANGLE BUFFER DATA
	///////////////////
    vector<CustomGeo> tri_vert;
	tri_vert.assign(mNumTris*3+1, CustomGeo());//add 1. element 0, is the number of triangles stored this round
	for (unsigned int i = 0; i < tri_vert.size()-1; ++i)
	{
		auto &vt = tri_vert.at(i);
		vt = CustomGeo(vec3(0.0f,0.0f,0.0f),vec3(0.0f,1.0f,0.0f),vec2(0.0f,0.0f));
	}
    ///////////////////
	/// MAKE TRIANGLE BUFFER
	///////////////////
	mTriangleBuffer = ComputeBuffer::create(tri_vert.data(), (int)tri_vert.size(), sizeof(CustomGeo));
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MAKE THE COMPUTE SHADER
	/////////////////////
	mVoxelizationShader = ComputeShader::create(loadAsset("voxelization.comp"));
	ivec3 count = gl::getMaxComputeWorkGroupCount();
	CI_ASSERT(count.x >= (mNumVoxels / mVoxelizationShader->getWorkGroupSize().x));
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MAKE THE GEO BUFFER
	//////////////////////
	mGeoBuffer = ComputeBuffer::create( tetrahedron.data(), (int)tetrahedron.size(), sizeof( CustomGeo ) );
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////
	//make the simple visualization shader
	mVisualizeVoxelSimpleConnection = assets()->getShader( "quadpassthrough.vert", "voxelvisualizationsimple.frag",
                                      [this]( gl::GlslProgRef glsl ) {
                                          mVisualizeVoxelSimpleProg = glsl;
                                          //mVisualizeVoxelSimpleProg->uniform( "uVoxels",0);
										  mVisualizeVoxelSimpleProg->uniform( "uResolution", vec2((float)app::getWindowSize().x, (float)app::getWindowSize().y));
										  mVisualizeVoxelSimpleProg->uniform( "uVoxelResolution", (float)mVoxelTexSize);
                                      } );
    ////////////////////////////////////////////////////////////////////////////////////////////////////



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
