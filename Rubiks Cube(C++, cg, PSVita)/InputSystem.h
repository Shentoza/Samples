#pragma once
#include "Logic.h"
#include "AnimationSystem.h"
#include "MyOwnTouchResult.h"
#include <iostream>
#include <ctrl.h>
#include <touch.h>
#include <map>
#include <math.h>
#include <vectormath.h>
using namespace sce::Vectormath::Simd::Aos;


class InputSystem
{
public:
	InputSystem(void);
	InputSystem(Logic* logic,AnimationSystem * anim);
	InputSystem(Logic* logic);
	~InputSystem(void);

	void processInput();
	void processTouch();

	void setLogic(Logic* logic);

	void setAnim(AnimationSystem * anim);

	std::map<int,bool*> buttonMap;

	float getLeftStickX();
	float getLeftStickY();

	float getBackTouchX();
	float getBackTouchY();

	float getRightStickX();
	float getRightStickY();

	void setTransformations(const Matrix4 transformationMatrix);

	Vector4 m_NearPoint;
	Vector4 m_FarPoint;

private:
	float m_FrontTouchMaxX;
	float m_FrontTouchMaxY;
	float m_FrontAspectRatio;

	float m_BackTouchMaxX;
	float m_BackTouchMaxY;
	float m_BackAspectRatio;

	Logic *logic;
	AnimationSystem * anim;


	SceCtrlData data;
	bool xPressed;
	bool circlePressed;
	bool squarePressed;
	bool trianglePressed;
	float leftStickAuslenkung[2];
	float rightStickAuslenkung[2];

	SceTouchData backTouchData;
	SceTouchReport* m_startBackTouch;
	SceTouchReport* m_continuedBackTouch;
	Vector2 m_lastBackTouch;
	Vector2 m_currentBackTouch;
	Vector2 m_currentBackTouchAuslenkung;

	bool m_backDragging;
	void processBackTouch();
	void backTouchRemainedActive();
	void startNewBackTouchpoint();


	SceTouchData touchData;
	SceTouchReport* m_startFrontTouch;
	SceTouchReport* m_continuedFrontTouch;
	Vector2 m_lastFrontTouchOrigin;
	Vector2 m_FrontTouchCurrent;

	MyTouchResult m_startFrontTouchResult;
	int m_draggingAxis;
	MyTouchResult m_continuedFrontTouchResult;

	Matrix4 inverted;
	Matrix4 transformation;

	void touchRemainedActive();
	void startNewTouchpoint();

	void executeRaycast(const Vector3 & rayOrigin, const Vector3 & rayDirection, MyTouchResult *touchData);


	void init();
	bool buttonPressedInFrame(int buttonCode);

};