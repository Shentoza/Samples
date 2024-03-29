#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sceerror.h>
#include <iostream>

#include <gxm.h>
#include <gxt.h>
#include <kernel.h>
#include <ctrl.h>
#include <display.h>
#include <libdbg.h>
#include <math.h>

#include <libdbgfont.h>

#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;

#include <rtc.h>
//Profiling
#include <libsysmodule.h>
#include <razor_capture.h>
#include <libperf.h>

#include <map>
#include "Logic.h"
#include "InputSystem.h"
#include "AudioManager.h"


/*	Define the debug font pixel color format to render to. */
#define DBGFONT_PIXEL_FORMAT		SCE_DBGFONT_PIXELFORMAT_A8B8G8R8


/*	Define the width and height to render at the native resolution */
#define DISPLAY_WIDTH				960
#define DISPLAY_HEIGHT				544
#define DISPLAY_STRIDE_IN_PIXELS	1024

/*	Define the libgxm color format to render to.
	This should be kept in sync with the display format to use with the SceDisplay library.
*/
#define DISPLAY_COLOR_FORMAT		SCE_GXM_COLOR_FORMAT_A8B8G8R8
#define DISPLAY_PIXEL_FORMAT		SCE_DISPLAY_PIXELFORMAT_A8B8G8R8

/*	Define the number of back buffers to use with this sample.  Most applications
	should use a value of 2 (double buffering) or 3 (triple buffering).
*/
#define DISPLAY_BUFFER_COUNT		3

/*	Define the maximum number of queued swaps that the display queue will allow.
	This limits the number of frames that the CPU can get ahead of the GPU,
	and is independent of the actual number of back buffers.  The display
	queue will block during sceGxmDisplayQueueAddEntry if this number of swaps
	have already been queued.
*/
#define DISPLAY_MAX_PENDING_SWAPS	2


/*	Helper macro to align a value */
#define ALIGN(x, a)					(((x) + ((a) - 1)) & ~((a) - 1))


/*	The build process for the sample embeds the shader programs directly into the
	executable using the symbols below.  This is purely for convenience, it is
	equivalent to simply load the binary file into memory and cast the contents
	to type SceGxmProgram.
*/
extern const SceGxmProgram binaryClearVGxpStart;
extern const SceGxmProgram binaryClearFGxpStart;

/*	Data structure for clear geometry */

// !! Data related to rendering vertex.
extern const SceGxmProgram binaryBasicVGxpStart;
extern const SceGxmProgram binaryBasicFGxpStart;

/* Data Structure for Backgrounds*/
extern const SceGxmProgram binaryBackgroundVGxpStart;
extern const SceGxmProgram binaryBackgroundFGxpStart;

/*	Data structure to pass through the display queue.  This structure is
	serialized during sceGxmDisplayQueueAddEntry, and is used to pass
	arbitrary data to the display callback function, called from an internal
	thread once the back buffer is ready to be displayed.

	In this example, we only need to pass the base address of the buffer.
*/
typedef struct DisplayData
{
	void *address;
} DisplayData;

static SceGxmContextParams		s_contextParams;			/* libgxm context parameter */
static SceGxmRenderTargetParams s_renderTargetParams;		/* libgxm render target parameter */
static SceGxmContext			*s_context			= NULL;	/* libgxm context */
static SceGxmRenderTarget		*s_renderTarget		= NULL;	/* libgxm render target */
static SceGxmShaderPatcher		*s_shaderPatcher	= NULL;	/* libgxm shader patcher */

/*	display data */
static void							*s_displayBufferData[ DISPLAY_BUFFER_COUNT ];
static SceGxmSyncObject				*s_displayBufferSync[ DISPLAY_BUFFER_COUNT ];
static int32_t						s_displayBufferUId[ DISPLAY_BUFFER_COUNT ];
static SceGxmColorSurface			s_displaySurface[ DISPLAY_BUFFER_COUNT ];
static uint32_t						s_displayFrontBufferIndex = 0;
static uint32_t						s_displayBackBufferIndex = 0;
static SceGxmDepthStencilSurface	s_depthSurface;

/*	shader data */
static int32_t					s_clearVerticesUId;
static int32_t					s_clearIndicesUId;
static SceGxmShaderPatcherId	s_clearVertexProgramId;
static SceGxmShaderPatcherId	s_clearFragmentProgramId;
// !! Shader pactcher addded.
static SceGxmShaderPatcherId	s_basicVertexProgramId;
static SceGxmShaderPatcherId	s_basicFragmentProgramId;
static SceUID					s_patcherFragmentUsseUId;
static SceUID					s_patcherVertexUsseUId;
static SceUID					s_patcherBufferUId;
static SceUID					s_depthBufferUId;
static SceUID					s_vdmRingBufferUId;
static SceUID					s_vertexRingBufferUId;
static SceUID					s_fragmentRingBufferUId;
static SceUID					s_fragmentUsseRingBufferUId;
static ClearVertex				*s_clearVertices			= NULL;
static uint16_t					*s_clearIndices				= NULL;
static SceGxmVertexProgram		*s_clearVertexProgram		= NULL;
static SceGxmFragmentProgram	*s_clearFragmentProgram		= NULL;
// !! Data added.
static SceGxmVertexProgram		*s_basicVertexProgram		= NULL;
static SceGxmFragmentProgram	*s_basicFragmentProgram		= NULL;
static BasicVertex				*s_basicVertices			= NULL;
static uint16_t					*s_basicIndices				= NULL;
static int32_t					s_basicVerticesUId;
static int32_t					s_basicIndiceUId;
static SceGxmTexture			s_basicNormalMap;

static SceGxmVertexProgram		*s_backgroundVertexProgram	= NULL;
static SceGxmFragmentProgram	*s_backgroundFragmentProgram = NULL;
static SceGxmShaderPatcherId	s_backgroundVertexProgramId;
static SceGxmShaderPatcherId	s_backgroundFragmentProgramId;
static BackgroundVertex			*s_backgroundVertices		= NULL;
static uint16_t					*s_backgroundIndices		= NULL;
static int32_t					s_backgroundVerticesUId;
static int32_t					s_backgroundIndiceUId;
static SceGxmTexture			s_backgroundTexture;


//!! The program parameter for the transformation of the triangle
static float s_wvpData[16];
static const SceGxmProgramParameter *s_wvpParam = NULL;

// Rotation
static float s_rotation[16];
static const SceGxmProgramParameter *s_rotationParam = NULL;

static const SceGxmProgramParameter *s_animationAngleParam = NULL;




/* Callback function to allocate memory for the shader patcher */
static void *patcherHostAlloc( void *userData, uint32_t size );

/* Callback function to allocate memory for the shader patcher */
static void patcherHostFree( void *userData, void *mem );

/*	Callback function for displaying a buffer */
static void displayCallback( const void *callbackData );

/*	Helper function to allocate memory and map it for the GPU */
static void *graphicsAlloc( SceKernelMemBlockType type, uint32_t size, uint32_t alignment, uint32_t attribs, SceUID *uid );

/*	Helper function to free memory mapped to the GPU */
static void graphicsFree( SceUID uid );

/* Helper function to allocate memory and map it as vertex USSE code for the GPU */
static void *vertexUsseAlloc( uint32_t size, SceUID *uid, uint32_t *usseOffset );

/* Helper function to free memory mapped as vertex USSE code for the GPU */
static void vertexUsseFree( SceUID uid );

/* Helper function to allocate memory and map it as fragment USSE code for the GPU */
static void *fragmentUsseAlloc( uint32_t size, SceUID *uid, uint32_t *usseOffset );

/* Helper function to free memory mapped as fragment USSE code for the GPU */
static void fragmentUsseFree( SceUID uid );


/*	@brief Main entry point for the application
	@return Error code result of processing during execution: <c> SCE_OK </c> on success,
	or another code depending upon the error
*/
int main( void );


// !! Here we create the matrix.
void Update(float deltaTime);



/*	@brief Initializes the graphics services and the libgxm graphics library
	@return Error code result of processing during execution: <c> SCE_OK </c> on success,
	or another code depending upon the error
*/
static int initGxm( void );

/*	 @brief Creates scenes with libgxm */
static void createGxmData( void );


/*	@brief Main rendering function to draw graphics to the display */
static void render( void );

/*	@brief render libgxm scenes */
static void renderGxm( void );



/*	@brief cycle display buffer */
static void cycleDisplayBuffers( void );

/*	@brief Destroy scenes with libgxm */
static void destroyGxmData( void );




/*	@brief Function to shut down libgxm and the graphics display services
	@return Error code result of processing during execution: <c> SCE_OK </c> on success,
	or another code depending upon the error
*/
static int shutdownGxm( void );

void ApplyTexture(const char* path, SceGxmTexture *texture);
void CreateBackground();



/*	@brief User main thread parameters */
extern const char			sceUserMainThreadName[]		= "simple_main_thr";
extern const int			sceUserMainThreadPriority	= SCE_KERNEL_DEFAULT_PRIORITY_USER;
extern const unsigned int	sceUserMainThreadStackSize	= SCE_KERNEL_STACK_SIZE_DEFAULT_USER_MAIN;

/*	@brief libc parameters */
unsigned int	sceLibcHeapSize	= 2*1024*1024;

/* My Stuff*/
InputSystem* inputSystem;
AnimationSystem* anim;
Logic* logic;
AudioManager * audio;
float deltaTime;
static Quat s_accumulatedRotationQuat = Quat(0,0,0,1);


static void createSideVertices(BasicVertex* field, int type, int direction, int xOffset, int yOffset, int zOffset);




/* Main entry point of program */
int main( void )
{
	int returnCode = SCE_OK;
	
	//PROFILING
	returnCode = sceSysmoduleLoadModule(SCE_SYSMODULE_PERF);
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	returnCode = sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_CAPTURE);
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	//returnCode = sceSysmoduleLoadModule(SCE_SYSMODULE_RAZOR_HUD);
	//SCE_DBG_ALWAYS_ASSERT(returnCode == SCE_OK);
	


	/* initialize libdbgfont and libgxm */
	returnCode =initGxm();
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

    SceDbgFontConfig config;
	memset( &config, 0, sizeof(SceDbgFontConfig) );
	config.fontSize = SCE_DBGFONT_FONTSIZE_LARGE;

	returnCode = sceDbgFontInit( &config );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* Message for SDK sample auto test */
	printf( "## simple: INIT SUCCEEDED ##\n" );

	/* MY STUFF */
	SceRtcTick lastTick;
	SceRtcTick currentTick;
	SceUInt64 difference;
	//char debugString[256];
	float fps;
	

	logic = new Logic();
	audio = new AudioManager();
	audio->initialize("app0:swipeCut_loop.vag");
	anim = new AnimationSystem(logic, audio);
	inputSystem = new InputSystem(logic,anim);
		
	/* create gxm graphics data */
	createGxmData();
	logic->setVertexColors();
	logic->randomizeRotations(200);
	ApplyTexture("app0:background_vita.gxt", &s_backgroundTexture);
	ApplyTexture("app0:normalsText.gxt", &s_basicNormalMap);
	/* 6. main loop */
	while ( true)
	{
		//Calculate some DeltaTime
		sceRtcGetCurrentTick(&currentTick);
		difference = currentTick.tick - lastTick.tick;
		deltaTime = (float)difference * 0.000001f;
		fps = 1/deltaTime;
		fps = (int)(fps*100)/100;
		lastTick = currentTick;
		//sprintf(&debugString[0],"FPS: %f", fps);
		//sceDbgFontPrint( 20, 20, 0xffffffff, (const SceChar8*) &(debugString[0]));
		
		//anim->UpdateAnimation(deltaTime);
		anim->UpdateTouchAnimation(deltaTime);
		audio->update(deltaTime);
		//inputSystem->processInput();
		inputSystem->processTouch();

        Update(deltaTime);

		

		render();
		cycleDisplayBuffers();
	}


    /*
	// 10. wait until rendering is done 
	sceGxmFinish( s_context );

	// destroy gxm graphics data 
	destroyGxmData();

	// shutdown libdbgfont and libgxm 
	returnCode = shutdownGxm();
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	// Message for SDK sample auto test
	printf( "## api_libdbgfont/simple: FINISHED ##\n" );

	return returnCode;
    */
}

float makeFloat(unsigned char input)
{
	return (((float)(input)) / 255.0f*2.0f)-1.0f;
}

void Update (float deltaTime)
{
    float aspectRatio = (float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT;
	

	Quat toAdd = Quat(inputSystem->getBackTouchY(), -inputSystem->getBackTouchX(), 0, 0);
	toAdd = .5*toAdd*s_accumulatedRotationQuat;

	s_accumulatedRotationQuat += toAdd;
	s_accumulatedRotationQuat = normalize(s_accumulatedRotationQuat);

    Matrix4 rotation = Matrix4(s_accumulatedRotationQuat,Vector3(0,0,0));

	Matrix4 lookAt = Matrix4::lookAt(Point3(-0.0f,-0.0f,-7.0f), Point3(0.0f,0.0f,0.0f), Vector3(0.0f,-1.0f,0.0f));
	Matrix4 perspective = Matrix4::perspective(3.141592f / 4.0f, aspectRatio , 0.1f, 10.0f);
	
	Matrix4 final = perspective * lookAt * rotation;
	
	inputSystem->setTransformations(final);

	float* finalFloats = (float*)&final;
	float* rotationFloats = (float*)&rotation;

	memcpy(&s_wvpData[0],finalFloats, 16*4);
	memcpy(&s_rotation[0],rotationFloats, 16*4);
};


/* Initialize libgxm */
int initGxm( void )
{
/* ---------------------------------------------------------------------
	2. Initialize libgxm

	First we must initialize the libgxm library by calling sceGxmInitialize.
	The single argument to this function is the size of the parameter buffer to
	allocate for the GPU.  We will use the default 16MiB here.

	Once initialized, we need to create a rendering context to allow to us
	to render scenes on the GPU.  We use the default initialization
	parameters here to set the sizes of the various context ring buffers.

	Finally we create a render target to describe the geometry of the back
	buffers we will render to.  This object is used purely to schedule
	rendering jobs for the given dimensions, the color surface and
	depth/stencil surface must be allocated separately.
	--------------------------------------------------------------------- */

	int returnCode = SCE_OK;

	/* set up parameters */
	SceGxmInitializeParams initializeParams;
	memset( &initializeParams, 0, sizeof(SceGxmInitializeParams) );
	initializeParams.flags = 0;
	initializeParams.displayQueueMaxPendingCount = DISPLAY_MAX_PENDING_SWAPS;
	initializeParams.displayQueueCallback = displayCallback;
	initializeParams.displayQueueCallbackDataSize = sizeof(DisplayData);
	initializeParams.parameterBufferSize = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE;

	/* start libgxm */
	returnCode = sceGxmInitialize( &initializeParams );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* allocate ring buffer memory using default sizes */
	void *vdmRingBuffer = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE, 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_vdmRingBufferUId );

	void *vertexRingBuffer = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE, 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_vertexRingBufferUId );

	void *fragmentRingBuffer = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE, 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_fragmentRingBufferUId );

	uint32_t fragmentUsseRingBufferOffset;
	void *fragmentUsseRingBuffer = fragmentUsseAlloc( SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE, &s_fragmentUsseRingBufferUId, &fragmentUsseRingBufferOffset );

	/* create a rendering context */
	memset( &s_contextParams, 0, sizeof(SceGxmContextParams) );
	s_contextParams.hostMem = malloc( SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE );
	s_contextParams.hostMemSize = SCE_GXM_MINIMUM_CONTEXT_HOST_MEM_SIZE;
	s_contextParams.vdmRingBufferMem = vdmRingBuffer;
	s_contextParams.vdmRingBufferMemSize = SCE_GXM_DEFAULT_VDM_RING_BUFFER_SIZE;
	s_contextParams.vertexRingBufferMem = vertexRingBuffer;
	s_contextParams.vertexRingBufferMemSize = SCE_GXM_DEFAULT_VERTEX_RING_BUFFER_SIZE;
	s_contextParams.fragmentRingBufferMem = fragmentRingBuffer;
	s_contextParams.fragmentRingBufferMemSize = SCE_GXM_DEFAULT_FRAGMENT_RING_BUFFER_SIZE;
	s_contextParams.fragmentUsseRingBufferMem = fragmentUsseRingBuffer;
	s_contextParams.fragmentUsseRingBufferMemSize = SCE_GXM_DEFAULT_FRAGMENT_USSE_RING_BUFFER_SIZE;
	s_contextParams.fragmentUsseRingBufferOffset = fragmentUsseRingBufferOffset;
	returnCode = sceGxmCreateContext( &s_contextParams, &s_context );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* set buffer sizes for this sample */
	const uint32_t patcherBufferSize = 64*1024;
	const uint32_t patcherVertexUsseSize = 64*1024;
	const uint32_t patcherFragmentUsseSize = 64*1024;

	/* allocate memory for buffers and USSE code */
	void *patcherBuffer = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, patcherBufferSize, 4, SCE_GXM_MEMORY_ATTRIB_WRITE|SCE_GXM_MEMORY_ATTRIB_WRITE, &s_patcherBufferUId );

	uint32_t patcherVertexUsseOffset;
	void *patcherVertexUsse = vertexUsseAlloc( patcherVertexUsseSize, &s_patcherVertexUsseUId, &patcherVertexUsseOffset );

	uint32_t patcherFragmentUsseOffset;
	void *patcherFragmentUsse = fragmentUsseAlloc( patcherFragmentUsseSize, &s_patcherFragmentUsseUId, &patcherFragmentUsseOffset );

	/* create a shader patcher */
	SceGxmShaderPatcherParams patcherParams;
	memset( &patcherParams, 0, sizeof(SceGxmShaderPatcherParams) );
	patcherParams.userData = NULL;
	patcherParams.hostAllocCallback = &patcherHostAlloc;
	patcherParams.hostFreeCallback = &patcherHostFree;
	patcherParams.bufferAllocCallback = NULL;
	patcherParams.bufferFreeCallback = NULL;
	patcherParams.bufferMem = patcherBuffer;
	patcherParams.bufferMemSize = patcherBufferSize;
	patcherParams.vertexUsseAllocCallback = NULL;
	patcherParams.vertexUsseFreeCallback = NULL;
	patcherParams.vertexUsseMem = patcherVertexUsse;
	patcherParams.vertexUsseMemSize = patcherVertexUsseSize;
	patcherParams.vertexUsseOffset = patcherVertexUsseOffset;
	patcherParams.fragmentUsseAllocCallback = NULL;
	patcherParams.fragmentUsseFreeCallback = NULL;
	patcherParams.fragmentUsseMem = patcherFragmentUsse;
	patcherParams.fragmentUsseMemSize = patcherFragmentUsseSize;
	patcherParams.fragmentUsseOffset = patcherFragmentUsseOffset;
	returnCode = sceGxmShaderPatcherCreate( &patcherParams, &s_shaderPatcher );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* create a render target */
	memset( &s_renderTargetParams, 0, sizeof(SceGxmRenderTargetParams) );
	s_renderTargetParams.flags = 0;
	s_renderTargetParams.width = DISPLAY_WIDTH;
	s_renderTargetParams.height = DISPLAY_HEIGHT;
	s_renderTargetParams.scenesPerFrame = 1;
	s_renderTargetParams.multisampleMode = SCE_GXM_MULTISAMPLE_NONE;
	s_renderTargetParams.multisampleLocations	= 0;
	s_renderTargetParams.driverMemBlock = SCE_UID_INVALID_UID;

	returnCode = sceGxmCreateRenderTarget( &s_renderTargetParams, &s_renderTarget );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );


/* ---------------------------------------------------------------------
	3. Allocate display buffers, set up the display queue

	We will allocate our back buffers in CDRAM, and create a color
	surface for each of them.

	To allow display operations done by the CPU to be synchronized with
	rendering done by the GPU, we also create a SceGxmSyncObject for each
	display buffer.  This sync object will be used with each scene that
	renders to that buffer and when queueing display flips that involve
	that buffer (either flipping from or to).

	Finally we create a display queue object that points to our callback
	function.
	--------------------------------------------------------------------- */

	/* allocate memory and sync objects for display buffers */
	for ( unsigned int i = 0 ; i < DISPLAY_BUFFER_COUNT ; ++i )
	{
		/* allocate memory with large size to ensure physical contiguity */
		s_displayBufferData[i] = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA, ALIGN(4*DISPLAY_STRIDE_IN_PIXELS*DISPLAY_HEIGHT, 1*1024*1024), SCE_GXM_COLOR_SURFACE_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ|SCE_GXM_MEMORY_ATTRIB_WRITE, &s_displayBufferUId[i] );
		SCE_DBG_ALWAYS_ASSERT( s_displayBufferData[i] );

		/* memset the buffer to debug color */
		for ( unsigned int y = 0 ; y < DISPLAY_HEIGHT ; ++y )
		{
			unsigned int *row = (unsigned int *)s_displayBufferData[i] + y*DISPLAY_STRIDE_IN_PIXELS;

			for ( unsigned int x = 0 ; x < DISPLAY_WIDTH ; ++x )
			{
				row[x] = 0x0;
			}
		}

		/* initialize a color surface for this display buffer */
		returnCode = sceGxmColorSurfaceInit( &s_displaySurface[i], DISPLAY_COLOR_FORMAT, SCE_GXM_COLOR_SURFACE_LINEAR, SCE_GXM_COLOR_SURFACE_SCALE_NONE,
											 SCE_GXM_OUTPUT_REGISTER_SIZE_32BIT, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_STRIDE_IN_PIXELS, s_displayBufferData[i] );
		SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

		/* create a sync object that we will associate with this buffer */
		returnCode = sceGxmSyncObjectCreate( &s_displayBufferSync[i] );
		SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
	}

	/* compute the memory footprint of the depth buffer */
	const uint32_t alignedWidth = ALIGN( DISPLAY_WIDTH, SCE_GXM_TILE_SIZEX );
	const uint32_t alignedHeight = ALIGN( DISPLAY_HEIGHT, SCE_GXM_TILE_SIZEY );
	uint32_t sampleCount = alignedWidth*alignedHeight;
	uint32_t depthStrideInSamples = alignedWidth;

	/* allocate it */
	void *depthBufferData = graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 4*sampleCount, SCE_GXM_DEPTHSTENCIL_SURFACE_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ|SCE_GXM_MEMORY_ATTRIB_WRITE, &s_depthBufferUId );

	/* create the SceGxmDepthStencilSurface structure */
	returnCode = sceGxmDepthStencilSurfaceInit( &s_depthSurface, SCE_GXM_DEPTH_STENCIL_FORMAT_S8D24, SCE_GXM_DEPTH_STENCIL_SURFACE_TILED, depthStrideInSamples, depthBufferData, NULL );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	return returnCode;
}

void makeFace(BasicVertex* field, int type, int direction,int xOffset, int yOffset, int zOffset)
{
	float distanceBetweenCubes = 1.05f;

	for(int i = 0; i<4;++i)
		field[i].position[type] = direction * 0.5f;
	//Achse 0 = X Achse 1= Y Achse 2 = Z, je nach Type der achse ist Local X und Local Y die 2 Achsen die die "Ebene"/Seite aufspannen, die orthogonal zum angegebenen Type steht
	int localXDim = (type + 1 ) % 3;
	int localYDim = (type + 2 ) % 3;

	field[0].position[localXDim] = -0.5f;
	field[0].position[localYDim] = 0.5f;
	field[0].texCoords[0] = 0.0f;
	field[0].texCoords[1] = 0.0f;

	field[1].position[localXDim] = -0.5f;
	field[1].position[localYDim] = -0.5f;
	field[1].texCoords[0] = 0.0f;
	field[1].texCoords[1] = 1.0f;

	field[2].position[localXDim] = 0.5f;
	field[2].position[localYDim] = -0.5f;
	field[2].texCoords[0] = 1.0f;
	field[2].texCoords[1] = 1.0f;

	field[3].position[localXDim] = 0.5f;
	field[3].position[localYDim] = 0.5f;
	field[3].texCoords[0] = 1.0f;
	field[3].texCoords[1] = 0.0f;


	uint32_t baseColor = 0x55555555;

	//Calculate normal

	float normal[3];
	normal[0] = normal[1] = normal[2] = 0.0f;
	normal[type] = direction;
	float tangent[3];
	tangent[0] = tangent[1] = tangent[2] = 0.0f;

	//Berechnung der Tangente
	for(int i = 0;i<4;++i)
	{
		//X Achse
		if (type == 0) {
			if (direction == -1)
				tangent[2] = 1.0f;
			else
				tangent[2] = -1.0f;
		}
		//Y Achse
		else if (type == 2){
			if (direction == -1)
				tangent[0] = -1.0f;
			else
				tangent[0] = 1.0f;
		}
		//Z Achse
		else {
			tangent[0] = 1.0f;
		}

		field[i].color = baseColor;
		for (int j = 0; j < 3; ++j){
			field[i].normal[j] = normal[j];
			field[i].tangent[j] = tangent[j];
		}

		field[i].position[0] += xOffset*distanceBetweenCubes;
		field[i].position[1] += yOffset*distanceBetweenCubes;
		field[i].position[2] += zOffset*distanceBetweenCubes;
		field[i].rotation = -1;
	}

	createSideVertices(field,type,direction,xOffset,yOffset,zOffset);

}



void createSideVertices(BasicVertex* field, int type, int direction, int xOffset, int yOffset, int zOffset)
{	
	int x,y;
	SideVertices * current;
	//Front Seite
	if(zOffset == -1)
	{
		current = logic->m_frontSide->m_vertices;
		current->addSideVertices(field);

		if(type == 2 && direction == -1)
		{
			x = yOffset+1;
			y = xOffset+1;
			current->addFaceVertices(field,x,y);
			current->setNormalTypeAndDirection(type,direction);
		}
	}

	//BackSide
	if(zOffset == 1)
	{
		current = logic->m_backSide->m_vertices;
		current->addSideVertices(field);

		if(direction == 1 && type == 2)
		{
			x = yOffset+1;
			y = (xOffset*-1)+1;
			current->addFaceVertices(field,x,y);
			current->setNormalTypeAndDirection(type,direction);
		}
	}

	//Top Side
	if(yOffset == -1)
	{
		current = logic->m_topSide->m_vertices;
		current->addSideVertices(field);

		if(direction == -1 && type == 1)
		{
			x = (zOffset*-1)+1;
			y = xOffset+1;
			current->addFaceVertices(field,x,y);
			current->setNormalTypeAndDirection(type,direction);
		}
	}

	//Bottom Side
	if(yOffset == 1)
	{
		current = logic->m_bottomSide->m_vertices;
		current->addSideVertices(field);

		if(direction == 1 && type == 1)
		{
			x = zOffset+1;
			y = xOffset+1;
			current->addFaceVertices(field,x,y);
			current->setNormalTypeAndDirection(type,direction);
		}
	}

	//Left Side
	if(xOffset == -1)
	{
		current = logic->m_leftSide->m_vertices;
		current->addSideVertices(field);

		if(direction == -1 && type == 0)
		{
			x = yOffset+1;
			y = (zOffset*-1)+1;
			current->addFaceVertices(field,x,y);
			current->setNormalTypeAndDirection(type,direction);
		}
	}

	//Right Side
	if(xOffset == 1)
	{
		current = logic->m_rightSide->m_vertices;
		current->addSideVertices(field);

		if(direction == 1 && type == 0)
		{
			x = yOffset+1;
			y = zOffset+1;
			current->addFaceVertices(field,x,y);
			current->setNormalTypeAndDirection(type,direction);
		}
	}

	//Mittleres Top Down Slice
	if(xOffset == 0)
	{
		current = logic->xZeroSlice->m_vertices;
		current->setCubeVerticesAmount(192);
		current->addSideVertices(field);
		current->setNormalTypeAndDirection(0,1);
	}
	//Mittleres Left Right Slice
	if(yOffset == 0)
	{
		current = logic->yZeroSlice->m_vertices;
		current->setCubeVerticesAmount(192);
		current->addSideVertices(field);
		current->setNormalTypeAndDirection(1,1);
	}
	if(zOffset == 0)
	{
		current = logic->zZeroSlice->m_vertices;
		current->setCubeVerticesAmount(192);
		current->addSideVertices(field);
		current->setNormalTypeAndDirection(2,1);
	}
}



/* Create libgxm scenes */
void createGxmData( void )
{
/* ---------------------------------------------------------------------
	4. Create a shader patcher and register programs

	A shader patcher object is required to produce vertex and fragment
	programs from the shader compiler output.  First we create a shader
	patcher instance, using callback functions to allow it to allocate
	and free host memory for internal state.

	In order to create vertex and fragment programs for a particular
	shader, the compiler output must first be registered to obtain an ID
	for that shader.  Within a single ID, vertex and fragment programs
	are reference counted and could be shared if created with identical
	parameters.  To maximise this sharing, programs should only be
	registered with the shader patcher once if possible, so we will do
	this now.
	--------------------------------------------------------------------- */

	/* register programs with the patcher */
	int returnCode = sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, &binaryClearVGxpStart, &s_clearVertexProgramId );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
	returnCode = sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, &binaryClearFGxpStart, &s_clearFragmentProgramId );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	returnCode = sceGxmShaderPatcherRegisterProgram(s_shaderPatcher, &binaryBackgroundVGxpStart, &s_backgroundVertexProgramId);
	SCE_DBG_ALWAYS_ASSERT(returnCode == SCE_OK);
	returnCode = sceGxmShaderPatcherRegisterProgram(s_shaderPatcher, &binaryBackgroundFGxpStart, &s_backgroundFragmentProgramId);
	SCE_DBG_ALWAYS_ASSERT(returnCode == SCE_OK);


    returnCode = sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, &binaryBasicVGxpStart, &s_basicVertexProgramId );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
	returnCode = sceGxmShaderPatcherRegisterProgram( s_shaderPatcher, &binaryBasicFGxpStart, &s_basicFragmentProgramId );
    SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );


/* ---------------------------------------------------------------------
	5. Create the programs and data for the clear

	On SGX hardware, vertex programs must perform the unpack operations
	on vertex data, so we must define our vertex formats in order to
	create the vertex program.  Similarly, fragment programs must be
	specialized based on how they output their pixels and MSAA mode
	(and texture format on ES1).

	We define the clear geometry vertex format here and create the vertex
	and fragment program.

	The clear vertex and index data is static, we allocate and write the
	data here.
	--------------------------------------------------------------------- */

	/* get attributes by name to create vertex format bindings */
	const SceGxmProgram *clearProgram = sceGxmShaderPatcherGetProgramFromId( s_clearVertexProgramId );
	SCE_DBG_ALWAYS_ASSERT( clearProgram );
	const SceGxmProgramParameter *paramClearPositionAttribute = sceGxmProgramFindParameterByName( clearProgram, "aPosition" );
	SCE_DBG_ALWAYS_ASSERT( paramClearPositionAttribute && ( sceGxmProgramParameterGetCategory(paramClearPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );

	/* create clear vertex format */
	SceGxmVertexAttribute clearVertexAttributes[1];
	SceGxmVertexStream clearVertexStreams[1];
	clearVertexAttributes[0].streamIndex = 0;
	clearVertexAttributes[0].offset = 0;
	clearVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	clearVertexAttributes[0].componentCount = 2;
	clearVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex( paramClearPositionAttribute );
	clearVertexStreams[0].stride = sizeof(ClearVertex);
	clearVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	/* create clear programs */
	returnCode = sceGxmShaderPatcherCreateVertexProgram( s_shaderPatcher, s_clearVertexProgramId, clearVertexAttributes, 1, clearVertexStreams, 1, &s_clearVertexProgram );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	returnCode = sceGxmShaderPatcherCreateFragmentProgram( s_shaderPatcher, s_clearFragmentProgramId,
														   SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4, SCE_GXM_MULTISAMPLE_NONE, NULL,
														   sceGxmShaderPatcherGetProgramFromId(s_clearVertexProgramId), &s_clearFragmentProgram );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* create the clear triangle vertex/index data */
	s_clearVertices = (ClearVertex *)graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 3*sizeof(ClearVertex), 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_clearVerticesUId );
	s_clearIndices = (uint16_t *)graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 3*sizeof(uint16_t), 2, SCE_GXM_MEMORY_ATTRIB_READ, &s_clearIndicesUId );

	s_clearVertices[0].x = -1.0f;
	s_clearVertices[0].y = -1.0f;
	s_clearVertices[1].x =  3.0f;
	s_clearVertices[1].y = -1.0f;
	s_clearVertices[2].x = -1.0f;
	s_clearVertices[2].y =  3.0f;

	s_clearIndices[0] = 0;
	s_clearIndices[1] = 1;
	s_clearIndices[2] = 2;


	/*Background Stuff*/
	const SceGxmProgram *backgroundProgram = sceGxmShaderPatcherGetProgramFromId(s_backgroundVertexProgramId);
	SCE_DBG_ALWAYS_ASSERT(backgroundProgram);

	const SceGxmProgramParameter *paramBackgroundPositionAttribute = sceGxmProgramFindParameterByName(backgroundProgram, "aPosition");
	SCE_DBG_ALWAYS_ASSERT(paramBackgroundPositionAttribute && (sceGxmProgramParameterGetCategory(paramBackgroundPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE));

	const SceGxmProgramParameter *paramBackgroundTexCoordsAttribute = sceGxmProgramFindParameterByName(backgroundProgram, "aTexCoords");
	SCE_DBG_ALWAYS_ASSERT(paramBackgroundTexCoordsAttribute && (sceGxmProgramParameterGetCategory(paramBackgroundTexCoordsAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE));

	SceGxmVertexAttribute backgroundVertexAttributes[2];
	SceGxmVertexStream backgroundVertexStreams[1];
	//Position
	backgroundVertexAttributes[0].streamIndex = 0;
	backgroundVertexAttributes[0].offset = 0;
	backgroundVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	backgroundVertexAttributes[0].componentCount = 2;
	backgroundVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex(paramBackgroundPositionAttribute);

	backgroundVertexAttributes[1].streamIndex = 0;
	backgroundVertexAttributes[1].offset = 8;
	backgroundVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	backgroundVertexAttributes[1].componentCount = 2;
	backgroundVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex(paramBackgroundTexCoordsAttribute);

	backgroundVertexStreams[0].stride = sizeof(BackgroundVertex);
	backgroundVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;

	returnCode = sceGxmShaderPatcherCreateVertexProgram(s_shaderPatcher, s_backgroundVertexProgramId, backgroundVertexAttributes, 2,
		backgroundVertexStreams, 1, &s_backgroundVertexProgram);
	SCE_DBG_ALWAYS_ASSERT(returnCode == SCE_OK);

	returnCode = sceGxmShaderPatcherCreateFragmentProgram(s_shaderPatcher, s_backgroundFragmentProgramId,
		SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4, SCE_GXM_MULTISAMPLE_NONE, NULL,
		sceGxmShaderPatcherGetProgramFromId(s_backgroundVertexProgramId), &s_backgroundFragmentProgram);
	SCE_DBG_ALWAYS_ASSERT(returnCode == SCE_OK);

	CreateBackground();

    // !! All related to triangle.

    /* get attributes by name to create vertex format bindings */
	/* first retrieve the underlying program to extract binding information */
	const SceGxmProgram *basicProgram = sceGxmShaderPatcherGetProgramFromId( s_basicVertexProgramId );
	SCE_DBG_ALWAYS_ASSERT( basicProgram );
	const SceGxmProgramParameter *paramBasicPositionAttribute = sceGxmProgramFindParameterByName( basicProgram, "aPosition" );
	SCE_DBG_ALWAYS_ASSERT( paramBasicPositionAttribute && ( sceGxmProgramParameterGetCategory(paramBasicPositionAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );

	const SceGxmProgramParameter *paramBasicNormalAttribute = sceGxmProgramFindParameterByName( basicProgram, "aNormal" );
	SCE_DBG_ALWAYS_ASSERT( paramBasicNormalAttribute && ( sceGxmProgramParameterGetCategory(paramBasicNormalAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );

	const SceGxmProgramParameter *paramBasicColorAttribute = sceGxmProgramFindParameterByName( basicProgram, "aColor" );
	SCE_DBG_ALWAYS_ASSERT( paramBasicColorAttribute && ( sceGxmProgramParameterGetCategory(paramBasicColorAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );

	const SceGxmProgramParameter *paramBasicRotationFlagAttribute = sceGxmProgramFindParameterByName( basicProgram, "aRotationFlag" );
	SCE_DBG_ALWAYS_ASSERT( paramBasicRotationFlagAttribute && ( sceGxmProgramParameterGetCategory(paramBasicRotationFlagAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE ) );

	const SceGxmProgramParameter *paramBasicTexCoordsAttribute= sceGxmProgramFindParameterByName(basicProgram, "aTexCoords");
	SCE_DBG_ALWAYS_ASSERT(paramBasicTexCoordsAttribute && (sceGxmProgramParameterGetCategory(paramBasicTexCoordsAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE));

	const SceGxmProgramParameter *paramBasicTangentAttribute = sceGxmProgramFindParameterByName(basicProgram, "aTangent");
	SCE_DBG_ALWAYS_ASSERT(paramBasicTangentAttribute && (sceGxmProgramParameterGetCategory(paramBasicTangentAttribute) == SCE_GXM_PARAMETER_CATEGORY_ATTRIBUTE));

	/* create shaded triangle vertex format */
	SceGxmVertexAttribute basicVertexAttributes[6];
	SceGxmVertexStream basicVertexStreams[1];

	//Position
	basicVertexAttributes[0].streamIndex = 0;
	basicVertexAttributes[0].offset = 0;
	basicVertexAttributes[0].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	basicVertexAttributes[0].componentCount = 3;
	basicVertexAttributes[0].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicPositionAttribute );

	//Normals
	basicVertexAttributes[1].streamIndex = 0;
	basicVertexAttributes[1].offset = 12;
	basicVertexAttributes[1].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	basicVertexAttributes[1].componentCount = 3;
	basicVertexAttributes[1].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicNormalAttribute );

	//Color
	basicVertexAttributes[2].streamIndex = 0;
	basicVertexAttributes[2].offset = 24;
	basicVertexAttributes[2].format = SCE_GXM_ATTRIBUTE_FORMAT_U8N; // Mapping relation clarified.
	basicVertexAttributes[2].componentCount = 4;
	basicVertexAttributes[2].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicColorAttribute );

	//RotationFlag
	basicVertexAttributes[3].streamIndex = 0;
	basicVertexAttributes[3].offset = 28;
	basicVertexAttributes[3].format = SCE_GXM_ATTRIBUTE_FORMAT_S8;
	basicVertexAttributes[3].componentCount = 1;
	basicVertexAttributes[3].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicRotationFlagAttribute );

	//TexCoords
	basicVertexAttributes[4].streamIndex = 0;
	basicVertexAttributes[4].offset = 32;
	basicVertexAttributes[4].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	basicVertexAttributes[4].componentCount = 2;
	basicVertexAttributes[4].regIndex = sceGxmProgramParameterGetResourceIndex( paramBasicTexCoordsAttribute );

	//Tangent
	basicVertexAttributes[5].streamIndex = 0;
	basicVertexAttributes[5].offset = 40;
	basicVertexAttributes[5].format = SCE_GXM_ATTRIBUTE_FORMAT_F32;
	basicVertexAttributes[5].componentCount = 3;
	basicVertexAttributes[5].regIndex = sceGxmProgramParameterGetResourceIndex(paramBasicTangentAttribute);
	
	basicVertexStreams[0].stride = sizeof(BasicVertex);
	basicVertexStreams[0].indexSource = SCE_GXM_INDEX_SOURCE_INDEX_16BIT;
	
	/* create shaded triangle shaders */
	returnCode = sceGxmShaderPatcherCreateVertexProgram( s_shaderPatcher, s_basicVertexProgramId, basicVertexAttributes, 6,
														 basicVertexStreams, 1, &s_basicVertexProgram );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	returnCode = sceGxmShaderPatcherCreateFragmentProgram( s_shaderPatcher, s_basicFragmentProgramId,
														   SCE_GXM_OUTPUT_REGISTER_FORMAT_UCHAR4, SCE_GXM_MULTISAMPLE_NONE, NULL,
														   sceGxmShaderPatcherGetProgramFromId(s_basicVertexProgramId), &s_basicFragmentProgram );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* find vertex uniforms by name and cache parameter information */

	//World Transformation
	s_wvpParam = sceGxmProgramFindParameterByName( basicProgram, "wvp" );
	SCE_DBG_ALWAYS_ASSERT( s_wvpParam && ( sceGxmProgramParameterGetCategory( s_wvpParam ) == SCE_GXM_PARAMETER_CATEGORY_UNIFORM ) );

	//Animation Time
	s_animationAngleParam = sceGxmProgramFindParameterByName( basicProgram, "animationAngle" );
	SCE_DBG_ALWAYS_ASSERT( s_animationAngleParam && ( sceGxmProgramParameterGetCategory( s_animationAngleParam ) == SCE_GXM_PARAMETER_CATEGORY_UNIFORM ) );

	//Rotation of Vertices
	s_rotationParam = sceGxmProgramFindParameterByName( basicProgram, "rot" );
	SCE_DBG_ALWAYS_ASSERT( s_rotationParam && ( sceGxmProgramParameterGetCategory( s_rotationParam ) == SCE_GXM_PARAMETER_CATEGORY_UNIFORM ) );

	/* create shaded triangle vertex/index data */
	s_basicVertices = (BasicVertex *)graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 26 *4 * 6 *sizeof(BasicVertex), 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_basicVerticesUId );
	s_basicIndices = (uint16_t *)graphicsAlloc( SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 26 * 6 * 6 *sizeof(uint16_t), 2, SCE_GXM_MEMORY_ATTRIB_READ, &s_basicIndiceUId );

	
	int count = 0;
	int verticesCount = 0;
	int baseIndex = 0;
	int xOffset,yOffset,zOffset;
	for(xOffset = -1;xOffset < 2; xOffset++)
	{
		for(yOffset = -1;yOffset < 2; yOffset++)
		{
			for(zOffset = -1;zOffset<2; zOffset++)
			{
				for(int type = 0; type < 3; ++type)
				{
					for(int dir = -1;dir < 2; dir+=2)
					{
						if(!(xOffset == 0 && yOffset == 0 && zOffset == 0))
						{
							makeFace(&(s_basicVertices[verticesCount]),type,dir,xOffset,yOffset,zOffset);
							verticesCount+=4;
						}
					}
				}
				if(!(xOffset == 0 && yOffset == 0 && zOffset == 0))
					for(int side = 0; side < 6; ++side)
					{
						s_basicIndices[count++] = baseIndex;
						s_basicIndices[count++] = baseIndex +1;
						s_basicIndices[count++] = baseIndex +2;

						s_basicIndices[count++] = baseIndex;
						s_basicIndices[count++] = baseIndex +3;
						s_basicIndices[count++] = baseIndex +2;

						baseIndex+= 4;
					}
			}
		}
	}
}

/* Main render function */
void render( void )
{
	/* render libgxm scenes */
	renderGxm();
}

/* render gxm scenes */
void renderGxm( void )
{
/* -----------------------------------------------------------------
	8. Rendering step

	This sample renders a single scene containing the clear triangle.
	Before any drawing can take place, a scene must be started.
	We render to the back buffer, so it is also important to use a
	sync object to ensure that these rendering operations are
	synchronized with display operations.

	The clear triangle shaders do not declare any uniform variables,
	so this may be rendered immediately after setting the vertex and
	fragment program.

	Once clear triangle have been drawn the scene can be ended, which
	submits it for rendering on the GPU.
	----------------------------------------------------------------- */

	/* start rendering to the render target */
	sceGxmBeginScene( s_context, 0, s_renderTarget, NULL, NULL, s_displayBufferSync[s_displayBackBufferIndex], &s_displaySurface[s_displayBackBufferIndex], &s_depthSurface );

	/* Set Textures */
	sceGxmSetFragmentTexture(s_context, 0, &s_backgroundTexture);
	sceGxmSetFragmentTexture(s_context, 1, &s_basicNormalMap);


	/* set clear shaders */
	sceGxmSetVertexProgram( s_context, s_clearVertexProgram );
	sceGxmSetFragmentProgram( s_context, s_clearFragmentProgram );

	/* draw ther clear triangle */
	sceGxmSetVertexStream( s_context, 0, s_clearVertices );
	sceGxmDraw( s_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, s_clearIndices, 3 );

	/* Set Background Shaders*/
	sceGxmSetVertexProgram(s_context, s_backgroundVertexProgram);
	sceGxmSetFragmentProgram(s_context, s_backgroundFragmentProgram);

	sceGxmSetVertexStream(s_context, 0, s_backgroundVertices);
	sceGxmDraw(s_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, s_backgroundIndices, 6);

    // !! Speciality for rendering a triangle.

    /* render the  triangle */
	sceGxmSetVertexProgram( s_context, s_basicVertexProgram );
	sceGxmSetFragmentProgram( s_context, s_basicFragmentProgram );
	


	/* set the vertex program constants */
	void *vertexDefaultBuffer;
	sceGxmReserveVertexDefaultUniformBuffer( s_context, &vertexDefaultBuffer );
	sceGxmSetUniformDataF( vertexDefaultBuffer, s_wvpParam, 0, 16, s_wvpData );
	sceGxmSetUniformDataF( vertexDefaultBuffer, s_rotationParam, 0, 16, s_rotation);
	sceGxmSetUniformDataF( vertexDefaultBuffer, s_animationAngleParam, 0, 1, &anim->m_rotationAngle);

	/* draw the spinning triangle */
	sceGxmSetVertexStream( s_context, 0, s_basicVertices );
	sceGxmDraw( s_context, SCE_GXM_PRIMITIVE_TRIANGLES, SCE_GXM_INDEX_FORMAT_U16, s_basicIndices, 12 * 3 * 27 );


	/* stop rendering to the render target */
	sceGxmEndScene( s_context, NULL, NULL );
}



/* queue a display swap and cycle our buffers */
void cycleDisplayBuffers( void )
{
/* -----------------------------------------------------------------
	9-a. Flip operation

	Now we have finished submitting rendering work for this frame it
	is time to submit a flip operation.  As part of specifying this
	flip operation we must provide the sync objects for both the old
	buffer and the new buffer.  This is to allow synchronization both
	ways: to not flip until rendering is complete, but also to ensure
	that future rendering to these buffers does not start until the
	flip operation is complete.

	Once we have queued our flip, we manually cycle through our back
	buffers before starting the next frame.
	----------------------------------------------------------------- */

	/* PA heartbeat to notify end of frame */
	sceGxmPadHeartbeat( &s_displaySurface[s_displayBackBufferIndex], s_displayBufferSync[s_displayBackBufferIndex] );

	/* queue the display swap for this frame */
	DisplayData displayData;
	displayData.address = s_displayBufferData[s_displayBackBufferIndex];

	/* front buffer is OLD buffer, back buffer is NEW buffer */
	sceGxmDisplayQueueAddEntry( s_displayBufferSync[s_displayFrontBufferIndex], s_displayBufferSync[s_displayBackBufferIndex], &displayData );

	/* update buffer indices */
	s_displayFrontBufferIndex = s_displayBackBufferIndex;
	s_displayBackBufferIndex = (s_displayBackBufferIndex + 1) % DISPLAY_BUFFER_COUNT;
}


/* Destroy Gxm Data */
void destroyGxmData( void )
{
/* ---------------------------------------------------------------------
	11. Destroy the programs and data for the clear and spinning triangle

	Once the GPU is finished, we release all our programs.
	--------------------------------------------------------------------- */

	/* clean up allocations */
	sceGxmShaderPatcherReleaseFragmentProgram( s_shaderPatcher, s_clearFragmentProgram );
	sceGxmShaderPatcherReleaseVertexProgram( s_shaderPatcher, s_clearVertexProgram );
	graphicsFree( s_clearIndicesUId );
	graphicsFree( s_clearVerticesUId );

	/* wait until display queue is finished before deallocating display buffers */
	sceGxmDisplayQueueFinish();

	/* unregister programs and destroy shader patcher */
	sceGxmShaderPatcherUnregisterProgram( s_shaderPatcher, s_clearFragmentProgramId );
	sceGxmShaderPatcherUnregisterProgram( s_shaderPatcher, s_clearVertexProgramId );
	sceGxmShaderPatcherDestroy( s_shaderPatcher );
	fragmentUsseFree( s_patcherFragmentUsseUId );
	vertexUsseFree( s_patcherVertexUsseUId );
	graphicsFree( s_patcherBufferUId );
}



/* ShutDown libgxm */
int shutdownGxm( void )
{
/* ---------------------------------------------------------------------
	12. Finalize libgxm

	Once the GPU is finished, we deallocate all our memory,
	destroy all object and finally terminate libgxm.
	--------------------------------------------------------------------- */

	int returnCode = SCE_OK;

	graphicsFree( s_depthBufferUId );

	for ( unsigned int i = 0 ; i < DISPLAY_BUFFER_COUNT; ++i )
	{
		memset( s_displayBufferData[i], 0, DISPLAY_HEIGHT*DISPLAY_STRIDE_IN_PIXELS*4 );
		graphicsFree( s_displayBufferUId[i] );
		sceGxmSyncObjectDestroy( s_displayBufferSync[i] );
	}

	/* destroy the render target */
	sceGxmDestroyRenderTarget( s_renderTarget );

	/* destroy the context */
	sceGxmDestroyContext( s_context );

	fragmentUsseFree( s_fragmentUsseRingBufferUId );
	graphicsFree( s_fragmentRingBufferUId );
	graphicsFree( s_vertexRingBufferUId );
	graphicsFree( s_vdmRingBufferUId );
	free( s_contextParams.hostMem );

	/* terminate libgxm */
	sceGxmTerminate();

	return returnCode;
}


/* Host alloc */
static void *patcherHostAlloc( void *userData, unsigned int size )
{
	(void)( userData );

	return malloc( size );
}

/* Host free */
static void patcherHostFree( void *userData, void *mem )
{
	(void)( userData );

	free( mem );
}

/* Display callback */
void displayCallback( const void *callbackData )
{
/* -----------------------------------------------------------------
	10-b. Flip operation

	The callback function will be called from an internal thread once
	queued GPU operations involving the sync objects is complete.
	Assuming we have not reached our maximum number of queued frames,
	this function returns immediately.
	----------------------------------------------------------------- */

	SceDisplayFrameBuf framebuf;

	/* cast the parameters back */
	const DisplayData *displayData = (const DisplayData *)callbackData;


    // Render debug text.
    /* set framebuffer info */
	SceDbgFontFrameBufInfo info;
	memset( &info, 0, sizeof(SceDbgFontFrameBufInfo) );
	info.frameBufAddr = (SceUChar8 *)displayData->address;
	info.frameBufPitch = DISPLAY_STRIDE_IN_PIXELS;
	info.frameBufWidth = DISPLAY_WIDTH;
	info.frameBufHeight = DISPLAY_HEIGHT;
	info.frameBufPixelformat = DBGFONT_PIXEL_FORMAT;

	/* flush font buffer */
	int returnCode = sceDbgFontFlush( &info );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
	

	/* wwap to the new buffer on the next VSYNC */
	memset(&framebuf, 0x00, sizeof(SceDisplayFrameBuf));
	framebuf.size        = sizeof(SceDisplayFrameBuf);
	framebuf.base        = displayData->address;
	framebuf.pitch       = DISPLAY_STRIDE_IN_PIXELS;
	framebuf.pixelformat = DISPLAY_PIXEL_FORMAT;
	framebuf.width       = DISPLAY_WIDTH;
	framebuf.height      = DISPLAY_HEIGHT;
	returnCode = sceDisplaySetFrameBuf( &framebuf, SCE_DISPLAY_UPDATETIMING_NEXTVSYNC );
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );

	/* block this callback until the swap has occurred and the old buffer is no longer displayed */
	returnCode = sceDisplayWaitVblankStart();
	SCE_DBG_ALWAYS_ASSERT( returnCode == SCE_OK );
}

/* Alloc used by libgxm */
static void *graphicsAlloc( SceKernelMemBlockType type, uint32_t size, uint32_t alignment, uint32_t attribs, SceUID *uid )
{
/*	Since we are using sceKernelAllocMemBlock directly, we cannot directly
	use the alignment parameter.  Instead, we must allocate the size to the
	minimum for this memblock type, and just SCE_DBG_ALWAYS_ASSERT that this will cover
	our desired alignment.

	Developers using their own heaps should be able to use the alignment
	parameter directly for more minimal padding.
*/

	if( type == SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RWDATA )
	{
		/* CDRAM memblocks must be 256KiB aligned */
		SCE_DBG_ALWAYS_ASSERT( alignment <= 256*1024 );
		size = ALIGN( size, 256*1024 );
	}
	else
	{
		/* LPDDR memblocks must be 4KiB aligned */
		SCE_DBG_ALWAYS_ASSERT( alignment <= 4*1024 );
		size = ALIGN( size, 4*1024 );
	}

	/* allocate some memory */
	*uid = sceKernelAllocMemBlock( "simple", type, size, NULL );
	SCE_DBG_ALWAYS_ASSERT( *uid >= SCE_OK );

	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( *uid, &mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* map for the GPU */
	err = sceGxmMapMemory( mem, size, attribs );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* done */
	return mem;
}

/* Free used by libgxm */
static void graphicsFree( SceUID uid )
{
	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase(uid, &mem);
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	// unmap memory
	err = sceGxmUnmapMemory(mem);
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	// free the memory block
	err = sceKernelFreeMemBlock(uid);
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);
}

/* vertex alloc used by libgxm */
static void *vertexUsseAlloc( uint32_t size, SceUID *uid, uint32_t *usseOffset )
{
	/* align to memblock alignment for LPDDR */
	size = ALIGN( size, 4096 );

	/* allocate some memory */
	*uid = sceKernelAllocMemBlock( "simple", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, size, NULL );
	SCE_DBG_ALWAYS_ASSERT( *uid >= SCE_OK );

	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( *uid, &mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* map as vertex USSE code for the GPU */
	err = sceGxmMapVertexUsseMemory( mem, size, usseOffset );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	return mem;
}

/* vertex free used by libgxm */
static void vertexUsseFree( SceUID uid )
{
	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( uid, &mem );
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	/* unmap memory */
	err = sceGxmUnmapVertexUsseMemory( mem );
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	/* free the memory block */
	err = sceKernelFreeMemBlock( uid );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );
}

/* fragment alloc used by libgxm */
static void *fragmentUsseAlloc( uint32_t size, SceUID *uid, uint32_t *usseOffset )
{
	/* align to memblock alignment for LPDDR */
	size = ALIGN( size, 4096 );

	/* allocate some memory */
	*uid = sceKernelAllocMemBlock( "simple", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, size, NULL );
	SCE_DBG_ALWAYS_ASSERT( *uid >= SCE_OK );

	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( *uid, &mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* map as fragment USSE code for the GPU */
	err = sceGxmMapFragmentUsseMemory( mem, size, usseOffset);
	SCE_DBG_ALWAYS_ASSERT(err == SCE_OK);

	// done
	return mem;
}

/* fragment free used by libgxm */
static void fragmentUsseFree( SceUID uid )
{
	/* grab the base address */
	void *mem = NULL;
	int err = sceKernelGetMemBlockBase( uid, &mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* unmap memory */
	err = sceGxmUnmapFragmentUsseMemory( mem );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );

	/* free the memory block */
	err = sceKernelFreeMemBlock( uid );
	SCE_DBG_ALWAYS_ASSERT( err == SCE_OK );
}

void CreateBackground()
{

	s_backgroundVertices = (BackgroundVertex*)graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 4 * sizeof(BackgroundVertex), 4, SCE_GXM_MEMORY_ATTRIB_READ, &s_backgroundVerticesUId);
	s_backgroundIndices = (uint16_t *)graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, 6 * sizeof(uint16_t), 2, SCE_GXM_MEMORY_ATTRIB_READ, &s_backgroundIndiceUId);


	s_backgroundVertices[0].position[0] = -1.0f;
	s_backgroundVertices[0].position[1] = 1.0f;

	s_backgroundVertices[1].position[0] = -1.0f;
	s_backgroundVertices[1].position[1] = -1.0f;

	s_backgroundVertices[2].position[0] = 1.0f;
	s_backgroundVertices[2].position[1] = -1.0f;

	s_backgroundVertices[3].position[0] = 1.0f;
	s_backgroundVertices[3].position[1] = 1.0f;

	s_backgroundIndices[0] = 0;
	s_backgroundIndices[1] = 1;
	s_backgroundIndices[2] = 3;
	s_backgroundIndices[3] = 1;
	s_backgroundIndices[4] = 2;
	s_backgroundIndices[5] = 3;


	s_backgroundVertices[0].texCoords[0] = 0.0f;
	s_backgroundVertices[0].texCoords[1] = 0.0f;

	s_backgroundVertices[1].texCoords[0] = 0.0f;
	s_backgroundVertices[1].texCoords[1] = 1.0f;

	s_backgroundVertices[2].texCoords[0] = 1.0f;
	s_backgroundVertices[2].texCoords[1] = 1.0f;

	s_backgroundVertices[3].texCoords[0] = 1.0f;
	s_backgroundVertices[3].texCoords[1] = 0.0f;
}

void ApplyTexture(const char* path, SceGxmTexture *texture) {
	SceUID fileID = sceIoOpen(path,SCE_O_RDONLY, 0);
	int size = sceIoLseek(fileID, 0, SCE_SEEK_END);


	sceIoLseek(fileID, 0, SCE_SEEK_SET);
	void* file = malloc(size);
	sceIoRead(fileID, file, size);
	const uint32_t dataSize = sceGxtGetDataSize(file);
	const void * dataSrc = sceGxtGetDataAddress(file);

	SceUID s_textureMemoryID;
	void * gpuMemory = graphicsAlloc(SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA_UNCACHE, dataSize, SCE_GXM_TEXTURE_ALIGNMENT, SCE_GXM_MEMORY_ATTRIB_READ, &s_textureMemoryID);
	memcpy(gpuMemory, dataSrc, dataSize);

	sceGxtInitTexture(texture, file, gpuMemory, 0);
	free(file);
}