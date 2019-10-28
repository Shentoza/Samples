#pragma once
#ifndef InputManager_H_
#define InputManager_H_
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "GL\freeglut.h"
#include "Helper.h"
#include "Transform.h"

using namespace glm;
class InputManager
{
public:

	static InputManager* Instance();

	void UpdateMouse(int button, int state, int x, int y);
	void UpdateKeyboard(unsigned char button);
	void UpdateMotion(int x, int y);
	
	bool pathTracing;
	bool gpuTracing;
	bool changedCamera;
	Transform* getTransform();

	float limitPosition = 10.0f;
	float stepWidth = 0.1f;
	
private:
	vec2 mouseStarted;

	float mMouseSensitivity;
	float mScrollSensitivity;


	bool mMouseDragging;

	static InputManager* pInstance;
	InputManager();
	~InputManager();

	quat generateMouseRotation(vec3 axis, float radians);

	
	Transform* m_Transform;
};
#endif
