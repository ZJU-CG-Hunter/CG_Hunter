#pragma once

#include <HModel.h>

struct bullet {
	glm::vec3 _current_position;
	glm::vec3 _last_position;
	glm::vec3 _direction;
	glm::mat4 _rotation;
	float _speed;
	bool _exist;

	bullet(glm::vec3 initial_position, glm::vec3 initial_direction, glm::mat4 rotation, float speed) {
		_current_position = initial_position;
		_last_position = _current_position;
		_direction = glm::normalize(initial_direction);
		_rotation = rotation;
		_speed = speed;
		_exist = true;
	}
};

class HBullet : public HModel {
private:
	vector<bullet> _bullets;
	int _current_bullet;

public:
	HBullet(string const& path, bool gamma);

	void Draw();

	vector<glm::vec3> get_collider();

	void collision_detection(HMap* _map);

	void Action(HMap* map, float duration_time);

	void Event(Events* event);

	void insert_bullet(bullet insert_bullet);
};