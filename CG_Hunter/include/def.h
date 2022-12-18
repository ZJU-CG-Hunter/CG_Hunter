#pragma once

#include <vector>
#include <string>
// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

// Default sky box texture offset
const int SKYBOX_ID = 15;
const int MAX_SKYBOX_VERTEXS = 300;

const unsigned int MAX_BONES = 500;
const unsigned int MAX_ANIMATIONS =  20;
const unsigned int MATRIX_BUFFER_SIZE = 64;
const unsigned int MATRIX_UNIFROM_BUFFER_SIZE = (MATRIX_BUFFER_SIZE) * (3 + (MAX_BONES));

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;

const unsigned int MAX_SHADER_BINDING = 4;

const int INVALID_ANIMATION_INDEX = -1;
const int DEAD_LOOP = -1;
const float INVALID_ANIMATION_TICKS_PER_SECOND = 25.0f;
const float INVALID_ANIMATION_DURATION = 100.0f;

const int DEFAULT_LEVEL = 4;
const float DEFAULT_MAP_WIDTH = 1000;
const float DEFAULT_MAP_LENGTH = 1000;

const float INVALID_HEIGHT = -100000.0f;

const float Scale_X = 100.0f;
const float Scale_Y = 100.0f;
const float Scale_Z = 100.0f;

const int x_range = 250;
const int y_range = 250;

const int land_x_range = 3;
const int land_y_range = 3;

const float Y_OFFSET = 3.5f;

enum class Events {
	//like run, stop...
	type1, type2, type3
};

struct collides
{
	bool if_collide;
	Events model1_event;
	Events model2_event;
};
