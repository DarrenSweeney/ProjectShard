#ifndef TRACK_BLOCK_H
#define TRACK_BLOCK_H

#include "..\Core\ResourceManager.h"
#include "..\Math\Matrix4x4.h"
#include "Camera.h"
#include "..\Math\CollisionBox.h"
#include <math.h>

class TrackBlock
{
private:
	Model *modelBlock;
	Shader *shaderBlock;
	unsigned int sectorID;

public:
	TrackBlock();
	~TrackBlock();

	CollisionBox boundingBox;
	Vector3 position;
	Matrix4 rotate;
	enum BlockType
	{
		oscillation,
		stationary,
		rotating
	};
	BlockType blockType;

	void Update();
	void OscillationUpdate();
	void StationaryUpdate();
	void RotatingUpdate();
	void Render(Camera &camera, GLsizei screenWidth, GLsizei screenHeight);
	void RenderReflection(Camera &camera, GLsizei screenWidth, GLsizei screenHeight);
};

#endif