#pragma once

#include <HModel.h>

const int Walk = 0;
const int Jump = 1;
const int Wander = 2;
const int Idle = 3;
const int Die = 4;

class HPig : public HModel {
private:
	bool _is_alive;

public:
	HPig(string const& path, bool gamma);

	void Action(HMap* map, float duration_time);

	void Event(Events* event);

	void collision_detection(HMap* _map);
};

