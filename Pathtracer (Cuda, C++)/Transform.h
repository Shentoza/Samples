#pragma once
#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm\gtc\matrix_transform.hpp"

using namespace glm;
class Transform
{
public:
	Transform();
	~Transform();

	vec3 getUpVector();
	vec3 getRightVector();
	vec3 getForwardVector();

	mat4 getModelMatrix();

	quat getRotation();

	mat4 getTranslation();
	vec3 getPosition();

	void applyRotation(quat rotation);
	void applyTranslation(vec3 vector);

	void setRotation(quat rotation);
	void setTranslation(mat4 translation);
	void setScaling(mat4 scaling);
	void setScaling(vec3 scaling);
	void setScaling(float scaling);

	float getSingleScale();
	vec3 getScalingVector();

	void forceUpVector(vec3 up);

private:
	void updateModelMatrix();

	mat4 m_scalingMat;
	mat4 m_translationMat;
	quat m_rotation;

	mat4 m_modelMat;
};

