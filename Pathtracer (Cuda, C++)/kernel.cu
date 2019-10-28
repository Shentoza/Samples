/*
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

// This sample is an implementation of a simple line-of-sight algorithm:
// Given a height map and a ray originating at some observation point,
// it computes all the points along the ray that are visible from the
// observation point.
// It is based on the description made in "Guy E. Blelloch.  Vector models
// for data-parallel computing. MIT Press, 1990" and uses open source CUDA
// Thrust Library

#ifdef _WIN32
#  define NOMINMAX
#endif

//First some glew
#include "GL\glew.h"
#include "GL\freeglut.h"


// includes, system
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>

// includes, project"
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <helper_functions.h>
#include <helper_cuda.h>
#include <helper_math.h>
#include <curand.h>
#include <curand_kernel.h>

// includes, library
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <Thrust\execution_policy.h>
#include <thrust/scan.h>
#include <thrust/copy.h>
#include <cuda_gl_interop.h>


//includes migrated project
//#define TINYOBJLOADER_IMPLEMENTATION

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <iostream>
#include "Helper.h"
#include "InputManager.h"
#include "Transform.h"
#include "Core\Shader_Manager.h"
#include "Core\GameModels.h"
#include "Structs.h"

using namespace glm;

////////////////////////////////////////////////////////////////////////////////
// defines, declaration, variables

#define Samples 1
#define NUM_SPHERES 10
int maxFrames = -1;

// Height field texture reference
Managers::Shader_Manager* shaderManager;
GLuint program;
int uniformLocs[4];

//Geometry
Sphere *hSpheres;
Sphere *dSpheres;

vec2 resolution(1024, 1024);
vec3* cudaAccumulatedBuffer;


//openGL Interop
static cudaGraphicsResource* cuda_PixelBufferObject;
GLuint cuda_pixelBuffer;

GLuint host_pixelBuffer;
vec3* hostAccumulatedBuffer;

int frames = 0;
////////////////////////////////////////////////////////////////////////////////
// declaration, forward

//migrated
void renderScene(void);
void closeCallback();
void InitGL(int* argc, char** argv);
void keyboard(unsigned char key, int /*x*/, int /*y*/);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void Update();
void createOutputVBO();
__global__ void render_kernel(uchar4* deviceMem, vec3* buffer, int frameNumber, unsigned int frameHash, vec2 resolution, cudaMat4 inverse, Sphere* spheres);
void render_cpu(uchar4* deviceMem, vec3* buffer, int frameNumber, unsigned int frameHash, vec2 resolution, cudaMat4 inverse, Sphere* spheres);
void initCudaAndScene(int* argc, char** argv);

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
	
	InitGL(&argc, argv);
	initCudaAndScene(&argc, argv);
	glutMainLoop();

	checkCudaErrors(cudaFree(cudaAccumulatedBuffer));
	checkCudaErrors(cudaFree(dSpheres));
	checkCudaErrors(cudaFreeHost(hSpheres));
	free(hostAccumulatedBuffer);

	delete shaderManager;
	return 0;

}

cudaMat4 inverseWvp;

void renderScene(void)
{
	mat4 wvp = mat4();

	Transform* cameraTransform = InputManager::Instance()->getTransform();
	vec3 camPos = cameraTransform->getPosition();
	vec3 camForward = cameraTransform->getForwardVector();
	vec3 camUp = cameraTransform->getUpVector();
	mat4 rotation = mat4_cast(cameraTransform->getRotation());
	mat4 lookAt = glm::lookAt(camPos, camPos + camForward, camUp);
	mat4 perspective = glm::perspective(3.141592f / 4.0f, resolution.x / resolution.y, 0.1f, 30.0f);
	wvp = perspective * lookAt;
	


	// if we are pathTracing
	if (InputManager::Instance()->pathTracing){
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		//Camera Changed, generate new Picture!
		if (InputManager::Instance()->changedCamera){
			InputManager::Instance()->changedCamera = false;
			inverseWvp = glmMat4ToCudaMat4(inverse(wvp));
			frames = 1;
			checkCudaErrors(cudaMemset(cudaAccumulatedBuffer, 0, resolution.x * resolution.y * sizeof(vec3)));
			memset(hostAccumulatedBuffer, 0, resolution.x * resolution.y * sizeof(vec3));
		}
		else {
			frames++;
		}
		if (InputManager::Instance()->gpuTracing){
			uchar4* deviceMem;
			size_t num_bytes;
			checkCudaErrors(cudaThreadSynchronize());

			checkCudaErrors(cudaGraphicsMapResources(1, &cuda_PixelBufferObject));
			checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void**)&deviceMem, &num_bytes, cuda_PixelBufferObject));
		if (maxFrames == -1 || frames <= maxFrames) {


			dim3 blockDim(16, 16);
			dim3 gridDim(resolution.x / 16, resolution.y / 16);
			//int rng =rand();

			render_kernel << < gridDim, blockDim >> >(deviceMem, cudaAccumulatedBuffer, frames, Hash(frames), resolution, inverseWvp, dSpheres);
			//render_kernel << < gridDim, blockDim >> >(deviceMem, cudaAccumulatedBuffer, 0, Hash(rng), resolution, inverseWvp, dSpheres);
			cudaError_t errSync = cudaGetLastError();
			if (errSync != cudaSuccess)
				printf("Sync Kernel error: %s \n", cudaGetErrorString(errSync));
			
		}

			checkCudaErrors(cudaThreadSynchronize());
			checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_PixelBufferObject));

			glRasterPos2i(-1, -1);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, cuda_pixelBuffer);
			glDrawPixels(resolution.x, resolution.y, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
			glutSwapBuffers();
		}
		else {
			if (maxFrames == -1 || frames <= maxFrames) {
				uchar4* hostMem = (uchar4*)malloc(sizeof(uchar4) * resolution.x * resolution.y);;
				render_cpu(hostMem, hostAccumulatedBuffer, frames, Hash(frames), resolution, inverseWvp, hSpheres);
				glRasterPos2i(-1, -1);
				glDrawPixels(resolution.x, resolution.y, GL_RGBA, GL_UNSIGNED_BYTE, hostAccumulatedBuffer);
				glutSwapBuffers();
			}
		}
	}


	//openGL Geometry Rendering
	else {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(program);
		glUniformMatrix4fv(uniformLocs[0], 1, FALSE, (float*)&wvp);
		glUniformMatrix4fv(uniformLocs[1], 1, FALSE, (float*)&rotation);

		std::map<std::string, Models::Model*>::iterator it;
		Models::GameModels* gameModels = Models::GameModels::Instance();
		for (it = gameModels->GameModelList.begin(); it != gameModels->GameModelList.end(); ++it){
			Models::Model model = *it->second;
			glUniformMatrix4fv(uniformLocs[2], 1, FALSE, (float*)&model.modelTransform->getModelMatrix());
			vec3 col = model.color;
			glUniform3f(uniformLocs[3], col.x, col.y, col.z);
			printOglError("kernel.cu", 223);
			glBindVertexArray(model.vao);
			glDrawArrays(GL_TRIANGLES, 0, model.indicesCount);
			glBindVertexArray(0);
		}
		glUseProgram(0);
		glutSwapBuffers();
	}
}

//Checks if a given Ray has an intersection with an object (currently only spheres)
__device__ __host__ bool hasIntersection(const Ray &r, float &minDistance, int &sphereIndex, OBJECT_TYPE &objectType, Sphere* spheres, int &avoid){


	float distanceToSphere = 1e21;
	float inf = minDistance = 1e21;

	for (int i = 0; i < NUM_SPHERES; ++i) {
		distanceToSphere = spheres[i].intersect(r);
		if (avoid != i && abs(distanceToSphere) > EPSILON && distanceToSphere < minDistance){
			minDistance = distanceToSphere;
			sphereIndex = i;
			objectType = OBJECT_TYPE::SPHERE;
		}
	}

	return minDistance < inf;
}

__device__ __host__ vec3 sampleHemisphere(vec3 d, float phi, float sina, float cosa) {
	vec3 w = normalize(d), u = normalize(cross(vec3(w.y, w.z, w.x), w)), v = cross(w, u);
	return (u*cos(phi) + v*sin(phi)) * sina + w * cosa;
}

//Compute path bounces, accumulate colors
__device__ __host__ vec3 radiance(Ray &r, curandState *randstate, Sphere* spheres){


	vec3 mask(1.0f, 1.0f, 1.0f);

	vec3 color(0.0f, 0.0f, 0.0f);
	int avoid = -1;
	for (int bounce = 0; bounce < 8; ++bounce) {
		//Scene Intersections
		float t; //distance to intersection
		int sphereIndex = -1;
		OBJECT_TYPE objectType = OBJECT_TYPE::NONE;
		vec3 primitiveColor;
		vec3 emissionColor;
		vec3 intersection;
		vec3 normal;
		vec3 n1; //oriented normal
		vec3 nextDir;
		refl_t reflType;

		if (!hasIntersection(r, t, sphereIndex, objectType, spheres, avoid))
		{
			return vec3(0.0f);
		}

		//determine the properties of the object hit
		//Currently only spheres
		switch (objectType) {
			case OBJECT_TYPE::SPHERE:
				Sphere &sp = spheres[sphereIndex]; 
				intersection = getPointOnRay(r, t);
				normal = normalize(intersection - sp.pos);
				n1 = normal * sign(-dot(normal, r.dir));
				primitiveColor = sp.colour;
				reflType = sp.refl;
				avoid = sphereIndex;
				emissionColor = sp.emi;
				//Debug: See all the shapes how they are
				//return primitiveColor;
				color += (mask * emissionColor);
				break;
		}


		//different Shading types
		switch (reflType) {
		case DIFF:
		{
			//2 random numbers
			float rn1 = 0.f, rn2 = 0.f;
#ifdef __CUDA_ARCH__
			rn1 = curand_uniform(randstate);
			rn2 = curand_uniform(randstate);
#else
			rn1 = (float)rand() / RAND_MAX;
			rn2 = (float)rand() / RAND_MAX;
#endif
			//cosine weighted sampling
			nextDir = sampleHemisphere(n1, rn1*TWO_PI, sqrtf(rn2), sqrtf(1. - rn2));
			//intersection += n1 * 0.03f;
			mask *= primitiveColor;
			break;
		}
		case SPEC:
			nextDir = reflect(r.dir, normal);
			//intersection += n1 * 0.03f;
			mask *= primitiveColor;

			break;


			//ideal refraction (smallpt code by Kevin Beason)
		case REFR:
			float a = dot(normal, r.dir), ddn = abs(a);
			float nc = 1., nt = 1.5, nnt = mix(nc / nt, nt / nc, float(a>0.));
			float cos2t = 1. - nnt*nnt*(1. - ddn*ddn);
			nextDir = reflect(r.dir, normal);
			
			if (cos2t>0.) {
				vec3 tdir = normalize(r.dir*nnt + sign(a)*normal*(ddn*nnt + sqrt(cos2t)));
				float R0 = (nt - nc)*(nt - nc) / ((nt + nc)*(nt + nc)),
					c = 1. - mix(ddn, dot(tdir, normal), float(a>0.));
				float Re = R0 + (1. - R0)*c*c*c*c*c, P = .25 + .5*Re, RP = Re / P, TP = (1. - Re) / (1. - P);
				//intersection += 0.01f * n1;
#ifdef __CUDA_ARCH__
				if (curand_uniform(randstate) < P) { mask *= RP; }
#else
				if (((float)rand() / RAND_MAX) < P) { mask *= RP; }
#endif
				else { mask *= primitiveColor*TP; nextDir = tdir;}
			}
			break;
		}
		
		r.orig = intersection;
		r.dir = nextDir;
	}
	return color;
}

__global__ void render_kernel(uchar4* deviceMem, vec3* buffer, int frameNumber, unsigned int frameHash, vec2 resolution, cudaMat4 inverse, Sphere* spheres){
	unsigned int pixelX = (blockIdx.x * blockDim.x) + threadIdx.x;
	unsigned int pixelY = (blockIdx.y* blockDim.y) + threadIdx.y;
	
	int blockId = blockIdx.x + blockIdx.y * gridDim.y;
	int threadId = blockId * (blockDim.x * blockDim.y) + (threadIdx.y * blockDim.x) + threadIdx.x;

	int i = (pixelY * resolution.x) + pixelX;

	curandState randState;
	curand_init(frameHash + threadId, 0, 0, &randState);

	//Convert Pixel Coords to Screenspace
	vec4 nearPoint{ 2.0f * (float)pixelX / resolution.x - 1.0f,  2.0f * (float)pixelY / resolution.y - 1.0f, 0.1f, 1.0f };
	vec4 farPoint { 2.0f * (float)pixelX / resolution.x - 1.0f,  2.0f * (float)pixelY / resolution.y  - 1.0f, 0.9f, 1.0f};
	
	
	nearPoint = multiplyMV(inverse, nearPoint);
	farPoint = multiplyMV(inverse, farPoint);
	nearPoint /= nearPoint.w;
	farPoint /= farPoint.w;

	vec4 rayD = normalize(farPoint - nearPoint);
	Ray ray(vec3(nearPoint.x, nearPoint.y, nearPoint.z), vec3( rayD.x, rayD.y, rayD.z ));



	vec3 finalColor(0, 0, 0);
	for (int s = 0; s < Samples; ++s){
		// Radiance
		finalColor += radiance(ray, &randState, spheres) * (1.0f / (float)Samples);
	}
	buffer[i] += finalColor;
	vec3 tempCol = (buffer[i]/(float)frameNumber);

	
	vec3 colour = vec3(clamp(tempCol.x, 0.0f, 1.0f), clamp(tempCol.y, 0.0f, 1.0f), clamp(tempCol.z, 0.0f, 1.0f));
	//convert from 96-bit to 24-bit colour + perform gamma correction
	deviceMem[i] = make_uchar4((unsigned char)(powf(colour.x, 1 / 2.2f) * 255), (unsigned char)(powf(colour.y, 1/ 2.2f) * 255), (unsigned char)(powf(colour.z, 1.f / 2.2f) * 255.f), 1);
}

void render_cpu(uchar4* mem, vec3* buffer, int frameNumber, unsigned int frameHash, vec2 resolution, cudaMat4 inverse, Sphere* spheres){
	for (int pixelX = 0; pixelX < resolution.x; ++pixelX)
		for (int pixelY = 0; pixelY < resolution.y; ++pixelY) {
			int i = (pixelY * resolution.x) + pixelX;

			srand(frameHash + i);


			vec4 nearPoint{ 2.0f * (float)pixelX / resolution.x - 1.0f, 2.0f * (float)pixelY / resolution.y - 1.0f, 0.1f, 1.0f };
			vec4 farPoint{ 2.0f * (float)pixelX / resolution.x - 1.0f, 2.0f * (float)pixelY / resolution.y - 1.0f, 0.9f, 1.0f };


			nearPoint = multiplyMV(inverse, nearPoint);
			farPoint = multiplyMV(inverse, farPoint);
			nearPoint /= nearPoint.w;
			farPoint /= farPoint.w;

			vec4 rayD = normalize(farPoint - nearPoint);
			Ray ray(vec3(nearPoint.x, nearPoint.y, nearPoint.z), vec3(rayD.x, rayD.y, rayD.z));



			vec3 finalColor(0, 0, 0);
			for (int s = 0; s < Samples; ++s){
				// Radiance
				finalColor += radiance(ray, 0, spheres) * (1.0f / (float)Samples);
			}
			buffer[i] += finalColor;
			vec3 tempCol = (buffer[i] / (float)frameNumber);


			vec3 colour = vec3(clamp(tempCol.x, 0.0f, 1.0f), clamp(tempCol.y, 0.0f, 1.0f), clamp(tempCol.z, 0.0f, 1.0f));
			//convert from 96-bit to 24-bit colour + perform gamma correction
			mem[i] = make_uchar4((unsigned char)(powf(colour.x, 1 / 2.2f) * 255), (unsigned char)(powf(colour.y, 1 / 2.2f) * 255), (unsigned char)(powf(colour.z, 1.f / 2.2f) * 255.f), 1);
		}
}

void closeCallback()
{
	std::cout << "GLUT:\t Finished" << std::endl;
	glutLeaveMainLoop();
}

void resize(int width, int height) {
	glutReshapeWindow(resolution.x, resolution.y);
}

void InitGL(int* argc, char **argv)
{
	glutInit(argc, argv);
	//glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	
	glutInitWindowPosition(600, 0);
	glutInitWindowSize(resolution.x, resolution.y);

	glutCreateWindow("MC Tracer");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutDisplayFunc(Update);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(resize);
	glutCloseFunc(closeCallback);
	//glutIdleFunc(Update);

	
	glewInit();
	printOglError("Main.cpp", 433);
	if (glewIsSupported("GL_VERSION_4_5")) //lower your version if 4.5 is not supported by your video card
	{
		std::cout << " OpenGL Version is 4.5\n ";
	}
	else
	{
		std::cout << "OpenGL 4.5 not supported\n ";
	}
	printOglError("Main.cpp", 442);
	//Create output VBO
	createOutputVBO();
	setVSync(false);

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	printOglError("Main.cpp", 450);
	glFrontFace(GL_CCW);

	//load and compile shaders
	shaderManager = new Managers::Shader_Manager(); //thanks to Erik
	// for pointing this out
	shaderManager->CreateProgram("colorShader",
		"Shaders\\Vertex_Shader.glsl",
		"Shaders\\Fragment_Shader.glsl");
	program = Managers::Shader_Manager::GetShader("colorShader");
	uniformLocs[0] = glGetUniformLocation(program, "wvp");
	uniformLocs[1] = glGetUniformLocation(program, "rotation");
	uniformLocs[2] = glGetUniformLocation(program, "modelMat");
	uniformLocs[3] = glGetUniformLocation(program, "uColor");
}

void initCudaAndScene(int* argc, char** argv){
	if (*argc >= 2) {
		if (sscanf(argv[1], "%i", &maxFrames) != 1) {
			maxFrames = -1;
			fprintf(stderr, "error - not an integer");
		}
		else
			printf("Max Frames: %i", maxFrames);
	}
	//Allocate Memory
	checkCudaErrors(cudaMalloc(&cudaAccumulatedBuffer, resolution.x * resolution.y * sizeof(vec3)));
	checkCudaErrors(cudaHostAlloc(&hSpheres, NUM_SPHERES * sizeof(Sphere), cudaHostAllocDefault));
	checkCudaErrors(cudaMalloc(&dSpheres, NUM_SPHERES * sizeof(Sphere)));
	Models::GameModels* gameModels = Models::GameModels::Instance();


	//Write spheres
	//Format::
	//				Radius	Position					Emission				Color							Material
	//LIGHT
	hSpheres[0] = { 4.f,	{ 0, 10, 0 },				{ 12.0f,12.0f,12.0f},		{ 0.0f, 0.0f, 0.0f },	DIFF };
	//Cornell box
	hSpheres[1] = { 1000,	{ 1010, 0, 0 },				{ 0, 0, 0 },			{ 0.75f, 0.25f, 0.25f },		DIFF}; //Left
	hSpheres[2] = { 1000,	{ -1010, 0, 0 },			{ 0, 0, 0 },			{ 0.25f, 0.25f, 0.75f },		DIFF }; //Right
	hSpheres[3] = { 1000,	{ 0, 0, 1010},				{ 0, 0, 0 },			{ 0.f, .75f, .75f },				SPEC}; //Front
	hSpheres[4] = { 1000,	{ 0, 0, -1030 },			{ 0, 0, 0 },			{ 0.75f, 0.2f, 0.2f },			DIFF }; //Back
	hSpheres[5] = { 1000,	{ 0, 1010, 0 },				{ 0, 0, 0 },			{ 0.0f, 0.75f, 0.0f },			DIFF}; //Top
	hSpheres[6] = { 1000,	{ 0, -1010, 0 },			{ 0, 0, 0 },			{ 0.8f, 0.8f, .8f },			DIFF}; //Bottom
	//2 Spheres
	hSpheres[7] = { 3.f,	{ -3.f, -7.f, 4.f },		{ 0, 0, 0 },			{ 1.f, 1.f, 1.f },				SPEC };
	hSpheres[8] = { 2.5f,	{ 3., -4.5f, -2.f },		{ 0, 0, 0 },			{ .7f, 1.f, .9f },				REFR };
	hSpheres[9] = { 1.75f,	{ -3.f, -8.25f, -4.f },		{ 0, 0, 0 },			{ 1.f, 1.f, 0.f },				DIFF};
	for (int i = 0; i < NUM_SPHERES; ++i){
		Sphere s = hSpheres[i];
		vec3 scale = vec3(s.rad);
		vec3 pos = vec3(s.pos.x, s.pos.y, s.pos.z);
		quat rot = quat();
		std::string name = "sphere" + std::to_string(i);
		std::string path = "Resources/Sphere.obj";
		gameModels->LoadObjFile(name, path);
		gameModels->SetTransform(name, scale, pos, rot);
		gameModels->SetColor(name, s.colour);
	}
	checkCudaErrors(cudaMemcpy(dSpheres, hSpheres, sizeof(Sphere) * NUM_SPHERES, cudaMemcpyHostToDevice));
	checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_PixelBufferObject, cuda_pixelBuffer, cudaGraphicsMapFlagsNone));

	hostAccumulatedBuffer = (vec3*)malloc(sizeof(vec3) * resolution.x * resolution.y);
}

int lastTime = 0;
void Update() {
	int currentTime = glutGet(GLUT_ELAPSED_TIME);

	char framerate[100];
	int difference = currentTime - lastTime;
	sprintf(framerate, "FPS: %4.2f", 1000.0f / (float)difference);
	glutSetWindowTitle(framerate);


	renderScene();
	lastTime = currentTime;
}

void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
	if (key == 27){
#if defined(__APPLE__) || defined(MACOSX)
		exit(EXIT_SUCCESS);
#else
		glutDestroyWindow(glutGetWindow());
		return;
#endif
	}
	else {
		InputManager::Instance()->UpdateKeyboard(key);
	}
}

void mouse(int button, int state, int x, int y)
{
	InputManager::Instance()->UpdateMouse(button, state, x, y);
}

void motion(int x, int y)
{
	InputManager::Instance()->UpdateMotion(x, y);
}

void createOutputVBO()
{
	//create vertex buffer object
	glGenBuffersARB(1, &cuda_pixelBuffer);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, cuda_pixelBuffer);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, resolution.x * resolution.y * sizeof(uchar4), 0, GL_STREAM_DRAW_ARB);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	/*glGenBuffersARB(1, &host_pixelBuffer);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, host_pixelBuffer);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, resolution.x * resolution.y * sizeof(uchar4), 0, GL_STREAM_DRAW_ARB);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);*/
}