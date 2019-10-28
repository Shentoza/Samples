#include "InputManager.h"
#include <iostream>

InputManager * InputManager::pInstance = 0;

InputManager* InputManager::Instance() {
	if (!InputManager::pInstance){
		InputManager::pInstance = new InputManager();
	}
	return InputManager::pInstance;
}


InputManager::InputManager()
{
	if (InputManager::pInstance){
		this->~InputManager();
		return;
	}
	gpuTracing = true;
	pathTracing = true;
	changedCamera = true;
	m_Transform = new Transform();
	mouseStarted = vec2(0.0f);
	mMouseDragging = false;
	m_Transform->setRotation(quat(vec3(0, 0, 0)));
	m_Transform->applyTranslation(vec3(0, 0.1, -28));
}

InputManager::~InputManager()
{
}

void InputManager::UpdateMouse(int button, int state, int x, int y) {
	//Right Mouse was pressed
	if (button == GLUT_RIGHT_BUTTON){
		if (state == GLUT_DOWN){
			mMouseDragging = true;
			mouseStarted.x = (float)x;
			mouseStarted.y = (float)y;
		}
		if (state == GLUT_UP){
			mMouseDragging = false;
		}
	}
}

void InputManager::UpdateMotion(int x, int y){
	if (mMouseDragging){
		changedCamera = true;
		vec2 deltaMove = vec2((float)x, (float)y) - mouseStarted;
		deltaMove /= 100.0f;
		
		vec3 oldUp = m_Transform->getUpVector();

		quat Added = generateMouseRotation(m_Transform->getRightVector(), deltaMove.y);
		m_Transform->applyRotation(Added);

		Added = generateMouseRotation(m_Transform->getUpVector(), -deltaMove.x);
		m_Transform->applyRotation(Added);

		m_Transform->forceUpVector(oldUp);
		/*quat toAdd = quat(0, deltaMove.y, -deltaMove.x, 0);
		toAdd = 0.5f * toAdd * m_currentRotation;
		m_currentRotation += toAdd;
		m_currentRotation = normalize(m_currentRotation);*/
		//printMatrix(getRotationMatrix());
		mouseStarted.x = x;
		mouseStarted.y = y;
	}
}

void InputManager::UpdateKeyboard(unsigned char button){
	vec3 translateVector(0.0f, 0.0f, 0.0f);
	switch (button)
	{
	case('w') : {
		translateVector = normalize(m_Transform->getForwardVector()) * stepWidth;
		break;
	}
	case('a') : {
		translateVector = normalize(m_Transform->getRightVector()) *stepWidth;
		break;
	}
	case('s') : {
		translateVector = -normalize(m_Transform->getForwardVector()) *stepWidth;
		break;
	}
	case('d') : {
		translateVector = -normalize(m_Transform->getRightVector()) *stepWidth;
		break;
	}
	case(' ') : {
		translateVector = normalize(m_Transform->getUpVector()) *stepWidth;
		break;
	}
	case('x') : {
		translateVector = -normalize(m_Transform->getUpVector()) *stepWidth;
		break;
	}
	//Enables PathTracing
	case('p') : {
		if (!pathTracing) {
			mMouseDragging = false;
			pathTracing = true;
			changedCamera = true;
		}
		break;
	}
	//default OpenGL geometry rendering
	case('o') : {
		if (pathTracing) {
			mMouseDragging = false;
			pathTracing = false;
		}
		break;
	}
	case('c') : {
		if (gpuTracing){
			gpuTracing = false;
			changedCamera = true;
			mMouseDragging = false;
		}
		break;
	}

	case('g') : {
		if (!gpuTracing) {
			gpuTracing = true;
			changedCamera = true;
			mMouseDragging = false;
		}
		break;
	}
	case('r') :
		changedCamera = true;
		mMouseDragging = false;
		break;
	default:
		break;
	}
	if (translateVector != vec3(0)){
		changedCamera = true;
		m_Transform->applyTranslation(translateVector);
		m_Transform->forceUpVector(vec3(0, 1, 0));
	}
}

Transform* InputManager::getTransform() {
	return m_Transform;
}

quat InputManager::generateMouseRotation(vec3 axis, float radians){
	float half_theta = radians / 2.0;
	float s = sinf(half_theta);
	return glm::fquat(cos(half_theta), axis * s);
}