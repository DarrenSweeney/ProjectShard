#ifndef PLAYER_H
#define PLAYER_H

#include "..\Renderer\Model.h"
#include "..\Renderer\Shader.h"
#include "..\Core\ResourceManager.h"
#include "..\Math\Matrix4x4.h"
#include "..\Math\Quaternion.h"
#include "..\Input\InputManager.h"
#include "Camera.h"
#include "..\Math\CollisionBox.h"

class Player
{
public:
	Player();
	~Player();

	void Update(float deltaTime);
	void Render(GLsizei screenWidth, GLsizei screenHeight);
	void Reflection(GLsizei screenWidth, GLsizei screenHeight);

	CollisionBox boundingBox;
	Vector3 position;

	// TODO(Darren): Take this out.
	Camera camera;

private:
	Model *model;
	Shader *shaderModel;
	Vector3 linearVelocity;	
	Vector3 acceleration;
	Quaternion orientation;
	float rotationSpeed;
	Matrix4 modelRotate;
};

#endif