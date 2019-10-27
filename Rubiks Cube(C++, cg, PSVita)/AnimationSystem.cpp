#include "AnimationSystem.h"
#include <math.h>
#define HALF_PI   1.57079632679f


AnimationSystem::AnimationSystem(Logic* logic, AudioManager *audio)
{
	this->logic = logic;
	this->audioManager = audio;
	init();
}


AnimationSystem::~AnimationSystem(void)
{
}

void AnimationSystem::init()
{
	m_rotationDirection = ANIM_STATE_NO_ROTATION;
	m_currentState = ANIMATOR_STATE_INACTIVE;
	m_rotationTime = 0.0f;
	NEEDED_TIME_FOR_ROTATION = .5f;
	NEEDED_LENGTH_FOR_ROTATION = 1.2f;
	touchRotationProgress = 0.0f;
}


void AnimationSystem::animateSide(Side* side,anim_enum animation_type)
{
	m_rotationDirection = animation_type;
	m_currentState = ANIMATOR_STATE_ACTIVE;
	m_rotatingSide = side;

	m_rotatingSide->m_vertices->setRotating(true);
}

void AnimationSystem::animateSideByTouch(Side* side,anim_enum animation_type,animator_states settingState)
{
	m_rotationDirection = animation_type;
	m_currentState = settingState;
	m_rotatingSide = side;

	m_rotatingSide->m_vertices->setRotating(true);
}

void AnimationSystem::UpdateAnimation(float deltaTime)
{
	if(m_currentState == ANIMATOR_STATE_ACTIVE)
	{
		if(fabs(m_rotationTime) == NEEDED_TIME_FOR_ROTATION)
		{
			stopAnimation();
		}

		else
		{
				m_rotationTime += deltaTime;
				if(m_rotationTime > NEEDED_TIME_FOR_ROTATION)
					m_rotationTime = NEEDED_TIME_FOR_ROTATION;


			m_rotationAngle = m_rotationTime / NEEDED_TIME_FOR_ROTATION * HALF_PI;

			if(m_rotationDirection == ANIM_STATE_CLOCKWISE)
				m_rotationAngle *= -1;
			if(m_rotatingSide->xRotation > 0)
				m_rotationAngle *= -1;
			if(m_rotatingSide->yRotation > 0)
				m_rotationAngle *= -1;

		}
	}
}

//Method used to "snap" back to the last full rotation
void AnimationSystem::UpdateTouchAnimation(float deltaTime)
{
	if (m_currentState != ANIMATOR_STATE_INACTIVE){
		audioManager->playSwipeSound();
	}

	if(m_currentState == ANIMATOR_STATE_REVERTING)
	{
		float progressFloat = deltaTime / NEEDED_TIME_FOR_ROTATION;
		float touchRotationsFloat = (float)((int)touchRotationProgress);

		float difference = touchRotationProgress - touchRotationsFloat;

		if(touchRotationProgress < 0.0f)
		{
			if(difference < -0.5f)
			{
				touchRotationProgress -= progressFloat;
			}
			else
			{
				touchRotationProgress += progressFloat;
				if(touchRotationProgress >= touchRotationsFloat)
					touchRotationProgress = touchRotationsFloat;
			}
		}
		if(touchRotationProgress > 0.0f)
		{
			if(difference > 0.5f)
			{
				touchRotationProgress += progressFloat;
			}
			else
			{
				touchRotationProgress -= progressFloat;
				if(touchRotationProgress <= touchRotationsFloat)
					touchRotationProgress = touchRotationsFloat;
			}
		}


			if(touchRotationProgress == touchRotationsFloat)
			{
				stopTouchAnimation();
			}
			m_rotationAngle = touchRotationProgress * HALF_PI;

			/*
		if(m_rotationDirection == ANIM_STATE_CLOCKWISE)
			m_rotationAngle *= -1;
		if(m_rotatingSide->xRotation > 0)
			m_rotationAngle *= -1;
		if(m_rotatingSide->yRotation > 0)
			m_rotationAngle *= -1;
			*/
	}
}

void AnimationSystem::stopAnimation()
{
	if(m_rotationDirection == ANIM_STATE_CLOCKWISE)
		logic->rotateClockwise(m_rotatingSide);

	else if(m_rotationDirection == ANIM_STATE_COUNTERCLOCKWISE)
		logic->rotateCounterClockwise(m_rotatingSide);
		
	m_rotationTime = 0.0f;
	m_rotationAngle = 0.0f;

	m_rotatingSide->m_vertices->setRotating(false);
	m_rotationDirection = ANIM_STATE_NO_ROTATION;
	m_currentState = ANIMATOR_STATE_INACTIVE;
}

void AnimationSystem::stopTouchAnimation()
{
	for(int i = 0; i < touchRotations; ++i)
	{
		if(m_rotatingSide == logic->xZeroSlice)
		{
			if(m_rotationDirection == ANIM_STATE_CLOCKWISE)
			{
				logic->rotateXSliceClockwise();
			}

			else if(m_rotationDirection == ANIM_STATE_COUNTERCLOCKWISE)
			{
				logic->rotateXSliceCounterClockwise();
			}
		}
		else if(m_rotatingSide == logic->yZeroSlice)
		{
			if(m_rotationDirection == ANIM_STATE_CLOCKWISE)
			{
				logic->rotateYSliceClockwise();
			}

			else if(m_rotationDirection == ANIM_STATE_COUNTERCLOCKWISE)
			{
				logic->rotateYSliceCounterClockwise();
			}
		}
		else if(m_rotatingSide == logic->zZeroSlice)
		{
			if(m_rotationDirection == ANIM_STATE_CLOCKWISE)
			{
				logic->rotateZSliceClockwise();
			}

			else if(m_rotationDirection == ANIM_STATE_COUNTERCLOCKWISE)
			{
				logic->rotateZSliceCounterClockwise();
			}
		}
		else
		{
			if(m_rotationDirection == ANIM_STATE_CLOCKWISE)
				logic->rotateClockwise(m_rotatingSideTouch);

			else if(m_rotationDirection == ANIM_STATE_COUNTERCLOCKWISE)
				logic->rotateCounterClockwise(m_rotatingSideTouch);
		}
	}

	audioManager->stopSwipeSound();
	m_rotationTime = 0.0f;
	m_rotationAngle = 0.0f;
	m_rotatingSide->m_vertices->setRotating(false);
	m_rotationDirection = ANIM_STATE_NO_ROTATION;
	m_currentState = ANIMATOR_STATE_INACTIVE;
	m_rotatingSideTouch = NULL;
}

void AnimationSystem::stopTouchDrag()
{
	if(m_currentState != ANIMATOR_STATE_DRAGGING)
		return;

	if(touchRotationProgress == (float)((int)touchRotationProgress))
	{
		touchRotations = (int)abs(touchRotationProgress);
		stopTouchAnimation();
		return;
	}
	else
		m_currentState = ANIMATOR_STATE_REVERTING;


	touchRotations = (int) abs(touchRotationProgress);
	if(touchRotationProgress >= 0.0f)
	{
		if(touchRotationProgress - (float)touchRotations > 0.5f)
			touchRotations++;
	}
	else
	{
		if(touchRotationProgress + (float)touchRotations < -0.5f)
			touchRotations++;
	}
	m_rotationTime = touchRotationProgress * NEEDED_TIME_FOR_ROTATION;
}

//in X Richtung kann nur Facedrehung der linken Seite, rechten Seite, oder mittlerer Streifen (special)
//in Y Richtung kann nur Facedrehung der oberen Seite, unteren Seite, oder mittleren Scheibe (special)
void AnimationSystem::applyTouchAnimation(MyTouchResult* touchBegin, bool yAxis, float projectionValue)
{
	if(m_currentState == ANIMATOR_STATE_REVERTING || m_currentState == ANIMATOR_STATE_ACTIVE)
		return;

	if(m_currentState == ANIMATOR_STATE_INACTIVE)
	{
		m_currentState = ANIMATOR_STATE_DRAGGING;
		m_yAxis = yAxis;
	}
	//STREIFEN
	if(m_yAxis)
	{
		switch(touchBegin->yIndex)
		{
		case 0:
			{
				m_rotatingSideTouch = touchBegin->side->getLeftSide();
			}
			break;
		case 1:
			{
				switch(touchBegin->normalType)
				{
				case 0: 
					m_rotatingSideTouch = logic->zZeroSlice;
					break;
				case 1:
					m_rotatingSideTouch = logic->xZeroSlice;
					break;
				case 2:
					m_rotatingSideTouch = logic->xZeroSlice;
					break;
				}
			}
			break;
		case 2:
			{
				m_rotatingSideTouch = touchBegin->side->getRightSide();
			}
			break;
		}
	}
	//SCHEIBEN
	else
	{
		switch(touchBegin->xIndex)
		{
		case 0:
			m_rotatingSideTouch = touchBegin->side->getTopSide();
			break;
		case 1:
			{
				switch(touchBegin->normalType)
				{
				case 0: 
					m_rotatingSideTouch = logic->yZeroSlice;
					break;
				case 1:
					m_rotatingSideTouch = logic->zZeroSlice;
					break;
				case 2:
					m_rotatingSideTouch = logic->yZeroSlice;
					break;
				}
			}
			break;
		case 2:
			m_rotatingSideTouch = touchBegin->side->getBottomSide();
			break;
		}
	}
	short xRotation = touchBegin->side->xRotation;
	short yRotation = touchBegin->side->yRotation;

	if(((xRotation == 2 || xRotation == -1) && m_yAxis) || (yRotation == -1 &&  !m_yAxis))
		projectionValue *=-1.0f;

	touchRotationProgress = -projectionValue / NEEDED_LENGTH_FOR_ROTATION;

	m_rotationAngle = touchRotationProgress * HALF_PI;
	
	projectionValue*=m_rotatingSideTouch->m_vertices->m_direction;


	//Logik Only
	if(m_rotatingSideTouch->yRotation != 0)
			projectionValue *= -1;
	if(projectionValue < 0)
		animateSideByTouch(m_rotatingSideTouch, ANIM_STATE_CLOCKWISE, ANIMATOR_STATE_DRAGGING);
	else
		animateSideByTouch(m_rotatingSideTouch, ANIM_STATE_COUNTERCLOCKWISE, ANIMATOR_STATE_DRAGGING);
}

bool AnimationSystem::newTouchPossible()
{
	return m_currentState == ANIMATOR_STATE_INACTIVE;
}
