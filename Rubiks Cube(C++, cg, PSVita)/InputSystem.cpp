#include "InputSystem.h"
#include "AnimationEnum.h"


InputSystem::InputSystem(void)
{
	init();
}

InputSystem::InputSystem(Logic* logic)
{
	init();
	this->logic = logic;
}

InputSystem::InputSystem(Logic* logic,AnimationSystem * anim)
{
	init();
	this->logic = logic;
	this->anim = anim;
}


InputSystem::~InputSystem(void)
{
}

void InputSystem::init()
{
	//Button Init
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITALANALOG_WIDE);
	xPressed = circlePressed = squarePressed = trianglePressed = false;
	for(int i = 0;i<2;++i)
		leftStickAuslenkung[i] = rightStickAuslenkung[i] = 0.0f;

	buttonMap[SCE_CTRL_CROSS] = &xPressed;
	buttonMap[SCE_CTRL_CIRCLE] = &circlePressed;
	buttonMap[SCE_CTRL_SQUARE] = &squarePressed;
	buttonMap[SCE_CTRL_TRIANGLE] = &trianglePressed;

	//Touch Init Front
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,SCE_TOUCH_SAMPLING_STATE_START);
	m_FrontTouchMaxX = 1919.0f;
	m_FrontTouchMaxY = 1087.0f;
	m_FrontAspectRatio = m_FrontTouchMaxX / m_FrontTouchMaxY;

	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK,SCE_TOUCH_SAMPLING_STATE_START);
	m_BackTouchMaxX = 1919.0f;
	m_BackTouchMaxY = 889.0f;
	m_BackAspectRatio = m_BackTouchMaxX / m_BackTouchMaxY;
	m_backDragging = false;

	m_lastBackTouch = Vector2::zero();
	m_currentBackTouch = Vector2::zero();
	m_currentBackTouchAuslenkung = Vector2::zero();

	m_NearPoint = Vector4::zero();
	m_FarPoint = Vector4::zero();

	m_lastFrontTouchOrigin = Vector2::zero();
	m_FrontTouchCurrent = Vector2::zero();
	m_draggingAxis = -1;
}

void InputSystem::setLogic(Logic* logic)
{
	this->logic = logic;
}

void InputSystem::setAnim(AnimationSystem * anim)
{
	this->anim =anim;
}

float makeValue(SceUInt8 input)
{
	float result = (float)input;
	result = (result*2.0)/255.0f- 1.0f;
	return result;
}

void InputSystem::processInput()
{
	sceCtrlReadBufferPositive(0, &data, 1);
	
	leftStickAuslenkung[0] = makeValue(data.lx);
	leftStickAuslenkung[1] = makeValue(data.ly);
	rightStickAuslenkung[0] = makeValue(data.rx);
	rightStickAuslenkung[1] = makeValue(data.ry);
	

	//Deadzones, dann auf addieren
	if(fabs(leftStickAuslenkung[0]) < 0.1f)
		leftStickAuslenkung[0] = 0.0f;
	else
		leftStickAuslenkung[0]*= 0.04;


	if(fabs(leftStickAuslenkung[1]) < 0.1f)
		leftStickAuslenkung[1] = 0.0f;
	else
		leftStickAuslenkung[1]*= 0.04;

	if(anim->m_currentState != ANIMATOR_STATE_INACTIVE)
		return;

	//ButtonHandling X gedrückt
	if(buttonPressedInFrame(SCE_CTRL_CROSS))
	{
		if(!logic->m_currentSide || logic->m_currentSide == logic->m_bottomSide)
		{
			std::cout<<"Aktuelle Seite: Front"<<std::endl;
			logic->m_currentSide = logic->m_frontSide;
		}
		else if(logic->m_currentSide == logic->m_frontSide)
		{
			std::cout<<"Aktuelle Seite: Back"<<std::endl;
			logic->m_currentSide = logic->m_backSide;
		}


		else if(logic->m_currentSide == logic->m_backSide)
		{
			std::cout<<"Aktuelle Seite: Right"<<std::endl;
			logic->m_currentSide = logic->m_rightSide;
		}
		else if(logic->m_currentSide == logic->m_rightSide)
		{
			std::cout<<"Aktuelle Seite: Left"<<std::endl;
			logic->m_currentSide = logic->m_leftSide;
		}


		else if(logic->m_currentSide == logic->m_leftSide)
		{
			std::cout<<"Aktuelle Seite: Top"<<std::endl;
			logic->m_currentSide = logic->m_topSide;
		}
		else if(logic->m_currentSide == logic->m_topSide)
		{
			std::cout<<"Aktuelle Seite: Bottom"<<std::endl;
			logic->m_currentSide = logic->m_bottomSide;
		}
	}

	//Button Handling Quadrat gedrückt
	//CounterClock Wise
	
	if(buttonPressedInFrame(SCE_CTRL_SQUARE))
	{	
		if(logic->m_currentSide)
		{
			anim->animateSide(logic->m_currentSide, ANIM_STATE_COUNTERCLOCKWISE);
		}
		else
			std::cout<<"Keine Seite gewaehlt!";
	}

	//Button Handling Kreis gedrückt
	//Clock Wise
	
	if(buttonPressedInFrame(SCE_CTRL_CIRCLE))
	{
		if(logic->m_currentSide)
		{
			anim->animateSide(logic->m_currentSide,ANIM_STATE_CLOCKWISE);
		}
		else
			std::cout<<"Keine Seite gewaehlt!";
	}

	//Button Handling Dreieck gedrückt
	if(buttonPressedInFrame(SCE_CTRL_TRIANGLE))
	{
		logic->printCube();
		if(logic->m_currentSide)
			std::cout<<logic->m_currentSide->getFaceNumber()<<std::endl;
		else
			std::cout<<"Keine Seite gewaehlt!";
	}
}

void InputSystem::processTouch()
{
	sceTouchRead(SCE_TOUCH_PORT_FRONT,&touchData,1);


	//Wenn Touchpoints da sind
	if(touchData.reportNum)
	{

		//Suche, ob Touch noch existiert / weitergeführt wird
		if(m_startFrontTouch)
		{
			for(int i = 0; i < touchData.reportNum; ++i)
			{
				//ID existiert noch, merken
				if(touchData.report[i].id == m_startFrontTouch->id)
				{
					m_continuedFrontTouch = &touchData.report[i];
					break;
				}
			}
		}


		//Touchpoint immernoch da
		if(m_continuedFrontTouch)
			touchRemainedActive();

		//Neuer Touchpoint
		else
			if(anim->m_currentState == ANIMATOR_STATE_INACTIVE)
				startNewTouchpoint();
	}
	//Alten TouchStart verwerfen -> neue Eingabe möglich
	else
	{
		m_startFrontTouch = NULL;
		anim->stopTouchDrag();
		m_draggingAxis = -1;
	}

	m_continuedFrontTouch = NULL;

	processBackTouch();
}

void InputSystem::touchRemainedActive()
{
	if(!m_startFrontTouchResult.side)
		return;


	m_FrontTouchCurrent.setX(2.0f * m_continuedFrontTouch->x / m_FrontTouchMaxX - 1.0f);
	m_FrontTouchCurrent.setY(-2.0f * m_continuedFrontTouch->y / m_FrontTouchMaxY + 1.0f);

	Vector2 difference = m_FrontTouchCurrent - m_lastFrontTouchOrigin;

	//Abbrechen wenn noch nich gedragged wurde, und auch die länge nicht überschritten wurde
	if(abs(length(difference)) < 0.125f && m_draggingAxis == -1)
		return;
	
	//Wenn dragging noch nicht auf 1 oder 2 ist, um die Achsen anzugeben
	if(m_draggingAxis < 1)
		m_draggingAxis = 0;


	Vector2 screenspace_uAxis = (transformation * m_startFrontTouchResult.localXAxis).getXY();
	Vector2 screenspace_vAxis = (transformation * m_startFrontTouchResult.localYAxis).getXY();

	normalize(screenspace_uAxis);
	normalize(screenspace_vAxis);

	float screenProjection_u = dot(screenspace_uAxis,difference);
	float screenProjection_v = dot(screenspace_vAxis,difference);

	//m_face[x][y] ->  U = "localXAxis"    V = "localYAxis"
	//Projezierte Strecke länger in U
	//in X Richtung kann nur Facedrehung der linken Seite, rechten Seite, oder mittlerer Streifen (special)
	if((abs(screenProjection_u) > abs(screenProjection_v) && m_draggingAxis == 0 ) || m_draggingAxis == 1)
	{
		if(!m_draggingAxis)
			m_draggingAxis = 1;
		anim->applyTouchAnimation(&m_startFrontTouchResult, true, screenProjection_u / m_FrontAspectRatio);
	}
	//in Y Richtung kann nur Facedrehung der oberen Seite, unteren Seite, oder mittleren Scheibe (special)
	//Projezierte Strecke länger in V
	else if((abs(screenProjection_u) <= abs(screenProjection_v) && m_draggingAxis == 0 ) || m_draggingAxis == 2)
	{
		if(!m_draggingAxis)
			m_draggingAxis = 2;
		anim->applyTouchAnimation(&m_startFrontTouchResult, false, screenProjection_v);
	}
}

void InputSystem::startNewTouchpoint()
{
	m_startFrontTouch = &touchData.report[0];

	m_lastFrontTouchOrigin.setX(2.0f * m_startFrontTouch->x / m_FrontTouchMaxX - 1.0f);
	m_lastFrontTouchOrigin.setY(-2.0f * m_startFrontTouch->y / m_FrontTouchMaxY + 1.0f);


	m_NearPoint.setXY(Vector2(m_lastFrontTouchOrigin[0],m_lastFrontTouchOrigin[1]));
	m_FarPoint.setXY(Vector2(m_lastFrontTouchOrigin[0],m_lastFrontTouchOrigin[1]));
	m_NearPoint.setZ(0.9f);
	m_FarPoint.setZ(0.1f);
	m_NearPoint.setW(1);
	m_FarPoint.setW(1);

	m_NearPoint = inverted * m_NearPoint;
	m_FarPoint = inverted * m_FarPoint;

	m_NearPoint /= m_NearPoint.getW();
	m_FarPoint /= m_FarPoint.getW();

	Vector3 ray = (m_FarPoint - m_NearPoint).getXYZ();

	//den eventuell gesetzten wert wieder resetten
	executeRaycast(m_NearPoint.getXYZ(),ray, &m_startFrontTouchResult);
}


void InputSystem::executeRaycast(const Vector3 & rayOrigin, const Vector3 & rayDirection, MyTouchResult *touchData)
{
	Side* currentSide;
	Vector3 localXAxis, localYAxis;
	float xScale,yScale;
	int xIndex = 0,yIndex=0, sideIndex = -1;
	int normalType = -1, direction = 2;


	for(int i = 0; i<6;++i)
	{
		localXAxis = Vector3(0,0,0);
		localYAxis = Vector3(0,0,0);
		currentSide = logic->m_SideArray[i];
		normalType = currentSide->m_vertices->m_normalType;
		direction = currentSide->m_vertices->m_direction;

		BasicVertex * bottomLeftVert = currentSide->m_vertices->m_faceVertices[1][1][0];
		BasicVertex * topRightVert = currentSide->m_vertices->m_faceVertices[1][1][2];
		//Mittelpunkt des Mittleren MiniCubes bestimmen und als Mittelpunkt der Ebene setzen
		Vector3 distanceVerts = Vector3
			(topRightVert->position[0] - bottomLeftVert->position[0],
			topRightVert->position[1] - bottomLeftVert->position[1],
			topRightVert->position[2] - bottomLeftVert->position[2]);
		Vector3 planeOrigin = Vector3(
			bottomLeftVert->position[0],
			bottomLeftVert->position[1],
			bottomLeftVert->position[2]
		) +(0.5f*distanceVerts);


		Vector3 planeNormal = Vector3(
			bottomLeftVert->normal[0],
			bottomLeftVert->normal[1],
			bottomLeftVert->normal[2]
		);
		float d = (dot(rayOrigin,planeNormal)-dot(planeOrigin,planeNormal)) / length(planeNormal);

		//Ebene auf der anderen Seite(relativ zur anderen drehung)? Interessiert uns nicht!
		if(d < 0)
			continue;
		//Schnittpunkt Formel HNF Gerade
		float alpha = (dot(planeOrigin,planeNormal) - dot(rayOrigin,planeNormal) ) / dot(rayDirection,planeNormal);
		Vector3 intersect = rayOrigin + (alpha*rayDirection);

		//Lokalen X und Y Achsen herausfinden, und projezieren
		int localXDim,localYDim;
		if(normalType == 0)
		{
			localXDim = (normalType+1) % 3;
			localYDim = (normalType+2) % 3;
			localXAxis[localXDim] = 1.0f;
			localYAxis[localYDim] = 1.0f * direction;
		}
		else if(normalType == 1)
		{
			localXDim = (normalType+1) % 3;
			localYDim = (normalType+2) % 3;
			localXAxis[localXDim] = 1.0f * direction;
			localYAxis[localYDim] = 1.0f;
		}
		else if(normalType == 2)
		{
			localXDim = (normalType+2) % 3;
			localYDim = (normalType+1) % 3;
			localXAxis[localXDim] = 1.0f;
			localYAxis[localYDim] = -1.0f * direction;
		}
		
		xScale = dot(localXAxis,intersect - planeOrigin);
		yScale = dot(localYAxis,intersect - planeOrigin);
		if(abs(xScale) > 1.6f || abs(yScale) > 1.6f)
			continue;

		sideIndex = i;

		if(xScale >= -1.70f && xScale < -.525f)
			xIndex = 0;
		else if(xScale >= -.525f && xScale < .525f)
			xIndex = 1;
		else if(xScale >= .525f && xScale <= 1.70f)
			xIndex = 2;

		if(yScale >= -1.70f && yScale < -.525f)
			yIndex = 0;
		else if(yScale >= -.525f && yScale < .525f)
			yIndex = 1;
		else if(yScale >= .525f && yScale <= 1.70f)
			yIndex = 2;
		break;
	}

	touchData->side = logic->m_SideArray[sideIndex];
	touchData->xIndex = xIndex;
	touchData->yIndex = yIndex;

	touchData->normalType = normalType;
	touchData->direction = direction;

	touchData->xScale = xScale;
	touchData->yScale = yScale;

	touchData->localXAxis = localXAxis;
	touchData->localYAxis = localYAxis;
}

void InputSystem::setTransformations(const Matrix4 transformationMatrix)
{
	inverted = inverse(transformationMatrix);
	transformation = transformationMatrix;
}

bool InputSystem::buttonPressedInFrame(int buttonCode)
{
	bool *currentlyChecking = buttonMap[buttonCode];
	if((data.buttons & buttonCode) == 0 )
	{
		*currentlyChecking = false;
		return false;
	}

	if(*currentlyChecking)
		return false;

	*currentlyChecking = true;
	return true;
}

float InputSystem::getLeftStickX()
{
	return leftStickAuslenkung[0];
}

float InputSystem::getLeftStickY()
{
	return leftStickAuslenkung[1];
}

void InputSystem::processBackTouch()
{
	sceTouchRead(SCE_TOUCH_PORT_BACK,&backTouchData,1);

	
	//Wenn Touchpoints da sind
	if(backTouchData.reportNum)
	{

		//Suche, ob Touch noch existiert / weitergeführt wird
		if(m_startBackTouch)
		{
			for(int i = 0; i < backTouchData.reportNum; ++i)
			{
				//ID existiert noch, merken
				if(backTouchData.report[i].id == m_startBackTouch->id)
				{
					m_continuedBackTouch = &backTouchData.report[i];
					break;
				}
			}
		}


		//Touchpoint immernoch da
		if(m_continuedBackTouch)
			backTouchRemainedActive();

		//Neuer Touchpoint
		else
			startNewBackTouchpoint();

	}
	//Alten TouchStart verwerfen -> neue Eingabe möglich
	else
	{
		m_startBackTouch = NULL;
		m_currentBackTouchAuslenkung.setX(0.0f);
		m_currentBackTouchAuslenkung.setY(0.0f);
	}

	if(m_continuedBackTouch && m_startBackTouch)
	{
		if(m_backDragging)
		{
			m_lastBackTouch = m_currentBackTouch;
			m_startBackTouch = m_continuedBackTouch;
		}
		m_continuedBackTouch = NULL;
	}
}
void InputSystem::backTouchRemainedActive()
{

	m_currentBackTouch.setX(2.0f * m_continuedBackTouch->x / m_BackTouchMaxX - 1.0f);
	m_currentBackTouch.setY(-2.0f * m_continuedBackTouch->y / m_BackTouchMaxY + 1.0f);

	m_currentBackTouchAuslenkung = m_currentBackTouch - m_lastBackTouch;
	if(length(m_currentBackTouchAuslenkung) <= 0.125f && !m_backDragging)
	{
		m_currentBackTouchAuslenkung.setX(0.0f);
		m_currentBackTouchAuslenkung.setY(0.0f);
		return;
	}
	m_backDragging = true;




}

void InputSystem::startNewBackTouchpoint()
{
	m_startBackTouch = &backTouchData.report[0];

	m_lastBackTouch.setX(2.0f * m_startBackTouch->x / m_BackTouchMaxX - 1.0f);
	m_lastBackTouch.setY(-2.0f * m_startBackTouch->y / m_BackTouchMaxY + 1.0f);

	m_backDragging = false;
}

float InputSystem::getBackTouchX()
{
	return m_currentBackTouchAuslenkung.getX()*m_BackAspectRatio;
}

float InputSystem::getBackTouchY()
{
	return m_currentBackTouchAuslenkung.getY();
}