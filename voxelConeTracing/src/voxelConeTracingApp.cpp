#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

//#include "cinder/GeomIO.h"//for the vbomeshref

#include "Assets.h"
#include "Compute.h"
#include "CameraBasic.h"

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
        void keyDown(KeyEvent event) override;
        void keyUp(KeyEvent event) override;
		void update() override;
		void draw() override;

		float mDeltaTime;

	private:
        float mLastTime;//for calulating deltatime

		int mVoxelTexSize;
		int mNumVoxels;//voxelsize to power of 3
		int mNumTris;//total number of triangles I want to store

		ComputeShaderRef			mVoxelizationShader;
        ComputeShaderRef            mVoxelizationResizeShader;
		ComputeShaderRef			mClearVoxelizationShader;//i need a shader that clears out anything from that last frame
		ComputeShaderRef			mClearTrianglesShader;

		struct Voxel
		{
			//alignas(16) vec3 N;
			alignas(16) vec3 Cd;
            alignas(16) float Alpha;
		};

		ComputeBufferRef                mVoxelBuffer;
        ComputeBufferRef                mVoxelResizeBuffer;
        ComputeBufferRef                mTriangleBuffer;//when we are voxelizing our geo... we can save the geo ssbo into a new ssbo, to render later.
        gl::VboRef                      mTriangleInd;//this is a vbo that we use to draw the ssbo as triangles
        GLuint                          mTriangleAtomicBuffer;

        //vosualize the voxels
		gl::GlslProgRef                 mVisualizeVoxelSimpleProg;
		signals::ScopedConnection		mVisualizeVoxelSimpleConnection;

		//This is the tmp shader to see rendered triangles
		gl::GlslProgRef                 mRenderTrisSimpleProg;
		signals::ScopedConnection		mRenderTrisSimpleConnection;
        
        gl::GlslProgRef                 mRenderTrisVoxelConeTracing;
        signals::ScopedConnection       mRenderTrisVoxelConeTracingConnection;
		//This is the master shader, that will render the objects
        //gl::GlslProgRef		           mVoxelConeTrace;
        //signals::ScopedConnection		mVoxelConeTraceConnection;


        CameraBasicRef					 mCamera;

        //------------------
        ComputeBufferRef                 mGeoBuffer;//this is my geo buffer
        ComputeBufferRef                 mCornellBuffer;//this is my geo buffer
		//std::shared_ptr<log::LoggerBreakpoint> mLog;

        //--------------------
        float                            mSceneRenderScale;///this is passed to the shader to scale to the voxel volume
};

void voxelConeTracingApp::prepareSettings( Settings *settings )
{
	settings->setWindowSize( 1024, 768 );
	settings->disableFrameRate();
	settings->setFullScreen( false );
}

void voxelConeTracingApp::setup()
{
    mSceneRenderScale = 1.0f/10.5f;// this makes it so that -5 to 5 fit in the voxel volume //0.5f;
	//////////////////////////////////////////////////////////////////////////////////
	/// VOXEL DATA
	/////////////////////
	mVoxelTexSize=64;
	mNumVoxels = mVoxelTexSize*mVoxelTexSize*mVoxelTexSize;
	vector<Voxel> emptyVoxels;
    vector<Voxel> emptyResizeVoxels;

    emptyVoxels.assign(mNumVoxels, Voxel());
	for (unsigned int i = 0; i < emptyVoxels.size(); ++i)
	{
		auto &v = emptyVoxels.at(i);
		//v.P = vec3(0.0f, 0.0f, 0.0f);
		//v.N = vec3(0.0f, 1.0f, 0.0f);
		v.Cd = vec3(0.0f, 0.0f, 0.0f);
        v.Alpha = 0.0f;
	}
    
    //data for the resize buffer
    uint vts1 = (uint)mVoxelTexSize/2;//32,16,8
    uint vts2 = vts1/2;
    uint vts3 = vts2/2;
    uint numResizeVoxels = (vts1*vts1*vts1)+(vts2*vts2*vts2)+(vts3*vts3*vts3);//32768+4096+512=37376
    
    emptyResizeVoxels.assign(numResizeVoxels, Voxel());
    for (unsigned int i = 0; i < emptyResizeVoxels.size(); ++i)
    {
        auto &v = emptyResizeVoxels.at(i);
        //v.N = vec3(0.0f, 1.0f, 0.0f);
        v.Cd = vec3(0.0f, 0.0f, 0.0f);
        v.Alpha = 0.0f;
    }
	//////////////////////
	/// MAKE THE VOXEL BUFFER
	/////////////////////
	mVoxelBuffer = ComputeBuffer::create(emptyVoxels.data(), (int)emptyVoxels.size(), sizeof(Voxel));
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////
    /// MAKE THE VOXEL RESIZE BUFFER
    /////////////////////
    mVoxelResizeBuffer = ComputeBuffer::create(emptyResizeVoxels.data(), (int)emptyResizeVoxels.size(), sizeof(Voxel));
    /////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    /// MAKE ATOMIC COUNTER BUFFER... THIS WILL MAKE IT SO I CAN ADD SEQUENTIALLY TO MY TRIANGLE BUFFER, MAYBE
    /////////////////// http://www.lighthouse3d.com/tutorials/opengl-atomic-counters/
    // declare and generate a buffer object name
    glGenBuffers(1, &mTriangleAtomicBuffer);
    // bind the buffer and define its initial storage capacity
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mTriangleAtomicBuffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
    // unbind the buffer 
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// MAKE VBO FOR DRAWIN TRIANGLES FROM SSBO
	///////////////////
	mNumTris = 65536;//1048576;//2097152
	vector<uint32_t> tri_ids(mNumTris*3);
	uint32_t currid = 0;
	generate(tri_ids.begin(),tri_ids.end(),[&currid]()->uint32_t{return currid++;});
    ///////////////////
	/// MAKE VBO IDS
	///////////////////
	mTriangleInd = gl::Vbo::create<uint32_t>(GL_ELEMENT_ARRAY_BUFFER, tri_ids,GL_STATIC_DRAW);
	///////////////////
	/// MAKE TRIANGLE BUFFER DATA
	///////////////////
    vector<CustomGeo> tri_vert;
	tri_vert.assign(mNumTris*3, CustomGeo());//the first triangle is a dummy... to hold data
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
	/// MAKE THE COMPUTE SHADERS
	/////////////////////
	mVoxelizationShader = ComputeShader::create(loadAsset("voxelization.comp"));
    mVoxelizationResizeShader = ComputeShader::create(loadAsset("voxelizationresize.comp"));
	mClearVoxelizationShader = ComputeShader::create(loadAsset("clear_voxelization.comp"));
	mClearTrianglesShader = ComputeShader::create(loadAsset("clear_triangles.comp"));
	ivec3 count = gl::getMaxComputeWorkGroupCount();
	CI_ASSERT(count.x >= (mNumVoxels / mVoxelizationShader->getWorkGroupSize().x));
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////
	//make the simple visualization shader
	mVisualizeVoxelSimpleConnection = assets()->getShader( "quadpassthrough.vert", "voxelvisualizationsimple.frag",
                                      [this]( gl::GlslProgRef glsl ) {
                                          mVisualizeVoxelSimpleProg = glsl;
                                          //mVisualizeVoxelSimpleProg->uniform( "uVoxels",0);
										  mVisualizeVoxelSimpleProg->uniform( "uResolution", vec2((float)app::getWindowSize().x, (float)app::getWindowSize().y));
										  //mVisualizeVoxelSimpleProg->uniform( "uVoxelResolution", (float)mVoxelTexSize);
                                      } );
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////
	//make the temp triangle render shader
	mRenderTrisSimpleConnection = assets()->getShader( "trianglesimple.vert", "trianglesimple.frag",
                                      [this]( gl::GlslProgRef glsl ) {
                                          mRenderTrisSimpleProg = glsl;
                                          //mRenderTrisSimpleProg->uniform( "uVoxels",0);
                                      } );
    mRenderTrisVoxelConeTracingConnection = assets()->getShader( "trianglesimple.vert", "voxelconetracing.frag",
                                      [this]( gl::GlslProgRef glsl ) {
                                          mRenderTrisVoxelConeTracing = glsl;
                                      } );
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    //CAMERA
    mCamera = CameraBasicRef(new CameraBasic(10.0f));

    gl::enableDepthWrite();
    gl::enableDepthRead();

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MAKE THE GEO BUFFER
    //////////////////////
    mGeoBuffer = ComputeBuffer::create( tetrahedron.data(), (int)tetrahedron.size(), sizeof( CustomGeo ) );
    mCornellBuffer = ComputeBuffer::create( cornell.data(), (int)cornell.size(), sizeof( CustomGeo ) );
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

void voxelConeTracingApp::mouseDown( MouseEvent event )
{
}
void voxelConeTracingApp::keyDown( KeyEvent event )
{
    mCamera->keyDown(event);
}

void voxelConeTracingApp::keyUp( KeyEvent event )
{
    mCamera->keyUp(event);
}


void voxelConeTracingApp::update()
{
    {
        //CLEAR THE BUFFERS
        //THE DUMB WAY, BUT IT WORKS, SO FUCK IT
        gl::ScopedBuffer scopedVoxelSsbo(mVoxelBuffer->getSsbo());
        mVoxelBuffer->getSsbo()->bindBase(0);
        //bind the triangle buffer
        //gl::ScopedBuffer scopedTriangleSsbo(mTriangleBuffer->getSsbo());
        //mTriangleBuffer->getSsbo()->bindBase(1);
        //clear my buffers
        //vec3 val = vec3(0.0f);
        //glClearBufferData(mVoxelBuffer->getSsbo()->getTarget(), GL_RGB32F, GL_RGB, GL_FLOAT, &val);
        //glClearBufferData(mTriangleBuffer->getSsbo()->getTarget(), GL_RGBA32F, GL_RGBA, GL_FLOAT, &val);
        //mVoxelBuffer->clear(mEmptyVoxels.data(), (int)mEmptyVoxels.size(), sizeof(Voxel));
        mClearVoxelizationShader->dispatch( (int)glm::ceil( float( mNumVoxels ) / mVoxelizationShader->getWorkGroupSize().x ), 1, 1);
        mClearTrianglesShader->dispatch( (int)glm::ceil( float( mNumTris ) / mVoxelizationShader->getWorkGroupSize().x ), 1, 1);

        //////////////////CLEAR THE ATOMIC COUNTER
        ///////////////METHOD A
        // declare a pointer to hold the values in the buffer
        /*GLuint *userCounters;
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mTriangleAtomicBuffer);
        // map the buffer, userCounters will point to the buffers data
        userCounters = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0 , sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
        // set the memory to zeros, resetting the values in the buffer
        memset(userCounters, 0, sizeof(GLuint) );
        // unmap the buffer
        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);*/

        //////////////METHOD B
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mTriangleAtomicBuffer);
        GLuint a[1] = {0};
        glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0 , sizeof(GLuint) , a);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }

    float current = static_cast<float>(app::getElapsedSeconds());
    mDeltaTime = current - mLastTime;
    mLastTime = current;
	//gl::ScopedTextureBind scoped3dTex(mVoxelTex);
//CI_LOG_I(mDeltaTime);
        mCamera->Update(mDeltaTime);
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
       int call_count;// = (int)tetrahedron.size()/3;
       auto computeGlsl = mVoxelizationShader->getGlsl();
       
       //computeGlsl->uniform("uBufferSize",(uint32_t)mNumVoxels);
       
       computeGlsl->uniform("uVoxelResolution",(float)mVoxelTexSize);
       computeGlsl->uniform("uSceneScale",(float)mSceneRenderScale);

       //I should pass in the cameras direction.. at leas the camera point position
       //computeGlsl->uniform("uCameraPosition",(float)mSceneRenderScale);
       //right now I want to only call this for the number of verts in the mesh

        //compute version of voxilization
        //bind the voxel buffer
        gl::ScopedBuffer scopedVoxelSsbo(mVoxelBuffer->getSsbo());
        mVoxelBuffer->getSsbo()->bindBase(0);
        //bind the triangle buffer
        gl::ScopedBuffer scopedTriangleSsbo(mTriangleBuffer->getSsbo());
        mTriangleBuffer->getSsbo()->bindBase(1);
        //now bind the geo buffer that we want to voxelize
        ///bind the atomic counter
        //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mTriangleAtomicBuffer);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, mTriangleAtomicBuffer);
        GLuint userCounters[1];//tmp for reading 
        {
            call_count = (int)tetrahedron.size()/3;
            computeGlsl->uniform("uBufferSize",(uint32_t)call_count);
            //computeGlsl->uniform("uTriangleVertOffset",(uint32_t)0);

            gl::ScopedBuffer scopedGeoSsbo(mGeoBuffer->getSsbo());
            mGeoBuffer->getSsbo()->bindBase( 2 );

            //model matrix
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            glm::vec3 rotAxis(0.33f,0.33f,0.33f);
            glm::mat4 rotMat = glm::rotate(static_cast<float>(app::getElapsedSeconds()),rotAxis);

            computeGlsl->uniform("uModelMatrix",modelMatrix*rotMat);
            
            //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mTriangleAtomicBuffer);

            mVoxelizationShader->dispatch( (int)glm::ceil( float( call_count ) / mVoxelizationShader->getWorkGroupSize().x ), 1, 1);

             ////TEMP READ BACK ATAOMIC BUFFER
            //glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), userCounters);
            //CI_LOG_I(userCounters[0]);
        }
        {
            call_count = (int)tetrahedron.size()/3;
            computeGlsl->uniform("uBufferSize",(uint32_t)call_count);
            //computeGlsl->uniform("uTriangleVertOffset",(uint32_t)0);

            gl::ScopedBuffer scopedGeoSsbo(mGeoBuffer->getSsbo());
            mGeoBuffer->getSsbo()->bindBase( 2 );

            //model matrix
            glm::mat4 modelMatrix = glm::translate( glm::mat4(1.0f),glm::vec3(1.5,1.5,1.5) );
            computeGlsl->uniform("uModelMatrix",modelMatrix);
            
            //glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, mTriangleAtomicBuffer);

            mVoxelizationShader->dispatch( (int)glm::ceil( float( call_count ) / mVoxelizationShader->getWorkGroupSize().x ), 1, 1);

             ////TEMP READ BACK ATAOMIC BUFFER
            //glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), userCounters);
            //CI_LOG_I(userCounters[0]);
        }
       
        {
            call_count = (int)cornell.size()/3;
            computeGlsl->uniform("uBufferSize",(uint32_t)call_count);
            //computeGlsl->uniform("uTriangleVertOffset",(uint32_t)tetrahedron.size());

            gl::ScopedBuffer scopedGeoSsbo(mCornellBuffer->getSsbo());
            mCornellBuffer->getSsbo()->bindBase( 2 );

            //model matrix
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            computeGlsl->uniform("uModelMatrix",modelMatrix);

            mVoxelizationShader->dispatch( (int)glm::ceil( float( call_count ) / mVoxelizationShader->getWorkGroupSize().x ), 1, 1);
        }
        //unbind the atomic counter
         glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
    }

    //resize voxel buffer.. like mipmaps
    {
        auto computeResizeGlsl = mVoxelizationResizeShader->getGlsl();
        
        gl::ScopedBuffer scopedVoxelSsbo(mVoxelBuffer->getSsbo());
        mVoxelBuffer->getSsbo()->bindBase(0);
        gl::ScopedBuffer scopedVoxelResizeSsbo(mVoxelResizeBuffer->getSsbo());
        mVoxelResizeBuffer->getSsbo()->bindBase(1);
        computeResizeGlsl->uniform("uVoxelResolution",(float)mVoxelTexSize);
        //do the first resize
        int re_call_count = (float)pow(mVoxelTexSize/2,3);
        computeResizeGlsl->uniform("uBufferSize",(float)re_call_count);
        computeResizeGlsl->uniform("uStep",(uint32_t)0);//(uint32_t)
        mVoxelizationResizeShader->dispatch( (int)glm::ceil( float( re_call_count ) / mVoxelizationResizeShader->getWorkGroupSize().x ), 1, 1);
        //then do second resize
        re_call_count = (float)pow(mVoxelTexSize/4,3);
        computeResizeGlsl->uniform("uBufferSize",(float)re_call_count);
        computeResizeGlsl->uniform("uStep",(uint32_t)1);//(uint32_t)
        mVoxelizationResizeShader->dispatch( (int)glm::ceil( float( re_call_count ) / mVoxelizationResizeShader->getWorkGroupSize().x ), 1, 1);
        //then do second resize
        re_call_count = (float)pow(mVoxelTexSize/8,3);
        computeResizeGlsl->uniform("uBufferSize",(float)re_call_count);
        computeResizeGlsl->uniform("uStep",(uint32_t)2);//(uint32_t)
        mVoxelizationResizeShader->dispatch( (int)glm::ceil( float( re_call_count ) / mVoxelizationResizeShader->getWorkGroupSize().x ), 1, 1);

    }
	//gl::ScopedGlslProg scopedRenderProg(mVoxelizationProg);
	//mVoxelizationProg->uniform("spriteSize", mSpriteSize);//set the uniforms in here

	//now render the objects with these settings

	//-------------done with the voxelization
    //render my triangles
    {
        gl::clear();
        gl::setMatricesWindow( getWindowSize() );
        gl::viewport(getWindowSize());

        gl::setMatrices(mCamera->GetPerspective());

        gl::ScopedGlslProg glslTriScp( mRenderTrisVoxelConeTracing );
        gl::context()->setDefaultShaderVars();
        mRenderTrisVoxelConeTracing->uniform( "uVoxelResolution", (float)mVoxelTexSize);
        mRenderTrisVoxelConeTracing->uniform( "uSceneScale", (float)mSceneRenderScale);
        mRenderTrisVoxelConeTracing->uniform( "uCameraPosition",mCamera->GetPerspective().getEyePoint() );
        
        //
        gl::bindBufferBase(mTriangleBuffer->getSsbo()->getTarget(),1,mTriangleBuffer->getSsbo() );
        gl::ScopedBuffer scopedIndices( mTriangleInd );
        gl::drawElements(GL_TRIANGLES,mNumTris*3,GL_UNSIGNED_INT,0);
    }
	//now visualize the voxelization somehow
    {
        //gl::clear();
        gl::setMatricesWindow( getWindowSize() );

        gl::ScopedGlslProg glslScp( mVisualizeVoxelSimpleProg );

        ///this should be binding the voxel buffer shader
        /*gl::ScopedBuffer scopedVoxelRenderSsbo(mVoxelBuffer->getSsbo());
        mVoxelBuffer->getSsbo()->bindBase(0);
        mVisualizeVoxelSimpleProg->uniform( "uVoxelResolution", (float)mVoxelTexSize);
        */
        //if we want to look at my resized ones
        ///this should be binding the voxel buffer shader
        /**/gl::ScopedBuffer scopedVoxelRenderSsbo(mVoxelResizeBuffer->getSsbo());
        mVoxelResizeBuffer->getSsbo()->bindBase(0);
        mVisualizeVoxelSimpleProg->uniform( "uVoxelResolution", (float)mVoxelTexSize/2);
        mVisualizeVoxelSimpleProg->uniform( "uOffset", (uint32_t)0);

        /*mVisualizeVoxelSimpleProg->uniform( "uVoxelResolution", (float)mVoxelTexSize/4);
        mVisualizeVoxelSimpleProg->uniform( "uOffset", (uint32_t)pow(mVoxelTexSize/2,3));//higer resizes values are smaller resizes
        */
        //gl::ScopedTextureBind scoped3dTexb(mVoxelTex,0);//bind the voxel texture AGAIN JUST IN CASE
        gl::drawSolidRect( getWindowBounds() );

	}
	CI_CHECK_GL();
}

CINDER_APP( voxelConeTracingApp, RendererGl(RendererGl::Options().version(4,2)), &voxelConeTracingApp::prepareSettings )
