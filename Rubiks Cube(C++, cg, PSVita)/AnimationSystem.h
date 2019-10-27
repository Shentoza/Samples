#pragma once
#include "Side.h"
#include "AnimationEnum.h"
#include "Logic.h"
#include "MyOwnTouchResult.h"
#include "AudioManager.h"

class AnimationSystem
{
private:
	Side* m_rotatingSide;
	float NEEDED_TIME_FOR_ROTATION;
	float m_rotationTime;
	
	Side* m_rotatingSideTouch;
	float NEEDED_LENGTH_FOR_ROTATION;
	float touchRotationProgress;
	int touchRotations;
	bool m_yAxis;

	void init();

public:
	Logic* logic;
	AudioManager* audioManager;
	
	float m_rotationAngle;
	anim_enum m_rotationDirection;
	animator_states m_currentState;

	void animateSide(Side* side,anim_enum animation_type);
	void animateSideByTouch(Side* side,anim_enum animation_type,animator_states settingState);

	void applyTouchAnimation(MyTouchResult* touchBegin, bool yAxis, float projectionValue);
	void ReleaseTouchAnimation();

	void UpdateAnimation(float deltaTime);
	void UpdateTouchAnimation(float deltaTime);

	void stopAnimation();
	void stopTouchDrag();
	void stopTouchAnimation();


	bool newTouchPossible();

	AnimationSystem(Logic* logic, AudioManager* audio);
	~AnimationSystem(void);
};

