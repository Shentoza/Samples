#include "Transform.h"


Transform::Transform()
{	
}


Transform::~Transform()
{
}

vec3 Transform::getRightVector(){
	return normalize(vec3(m_modelMat[0]));
}

vec3 Transform::getUpVector(){
	return normalize(vec3(m_modelMat[1]));
}

vec3 Transform::getForwardVector(){
	return normalize(vec3(m_modelMat[2]));
}

quat Transform::getRotation(){
	return m_rotation;
}

mat4 Transform::getTranslation(){
	return m_translationMat;
}

vec3 Transform::getPosition(){
	return vec3(m_modelMat[3]);
}

void Transform::setRotation(quat rotation){
	m_rotation = normalize(rotation);
	updateModelMatrix();
}

void Transform::forceUpVector(vec3 up){
	m_modelMat[1] = normalize(vec4(up, 0.0f));
}

void Transform::setScaling(mat4 scaling){
	m_scalingMat = scaling;
	updateModelMatrix();
}

void Transform::setScaling(vec3 scaling){
	m_scalingMat = glm::scale(mat4(), scaling);
	updateModelMatrix();
}

void Transform::setScaling(float scaling){
	setScaling(vec3(scaling, scaling, scaling));
}

void Transform::setTranslation(mat4 translation){
	m_translationMat = translation;
	updateModelMatrix();
}

float Transform::getSingleScale() {
	vec3 sc = getScalingVector();
	if (sc.x == sc.y && sc.y == sc.z)
		return sc.x;
	return 0.0f;
}

vec3 Transform::getScalingVector() {
	return vec3(m_scalingMat[0][0], m_scalingMat[1][1], m_scalingMat[2][2]);
}

void Transform::applyRotation(quat rotation){
	setRotation(normalize(rotation * m_rotation));
}

void Transform::applyTranslation(vec3 vector){
	setTranslation(translate(m_translationMat, vector));
}

void Transform::updateModelMatrix(){
	m_modelMat = m_translationMat * mat4(m_rotation) * m_scalingMat;
}

mat4 Transform::getModelMatrix() {
	return m_modelMat;
}