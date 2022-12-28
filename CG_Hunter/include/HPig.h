#pragma once

#include <HModel.h>

const int Pig_Walk = 0;
const int Pig_Jump = 1;
const int Pig_Wander = 2;
const int Pig_Idle = 3;
const int Pig_Die = 4;

const float Pig_Idle_Speed = 0.0f;
const float Pig_Wander_Speed = 5.0f;
const float Pig_Walk_Speed = 10.0f;
const float Pig_Jump_Speed = 18.0f;

class HPig : public HModel {
private:
	bool _is_alive;
	bool _is_dying;

	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	glm::vec3 last_position;

	float Yaw;
	float Pitch;
	

	float warning;
	float warning_wander;
	float warning_walk;
	float warning_borad;
	float crazy;
	float alert;

public:
	HPig(string const& path, bool gamma);

	void Action(HMap* map, float duration_time);

	void Forward(float deltaTime);

	void turn(float xoffset, float yoffset);

	void Event(Events* event);

	void collision_detection(HMap* _map);

	void warn(float influence);
};

