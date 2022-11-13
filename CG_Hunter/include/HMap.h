#pragma once

#include <HModel.h>
#include <vector>
#include <set>

using namespace std;

class HMap {
private:
	int _level;
	float _map_width;
	float _map_length;
	vector<vector<set<HModel*>>> _map;

public:
	HMap(float map_width = DEFAULT_MAP_WIDTH, float map_length = DEFAULT_MAP_LENGTH, int level = DEFAULT_LEVEL);

	void insert_model(HModel* model);

	void remove_model(HModel* model);

	set<HModel*> get_model_nearby(HModel* model);

	void get_point_position(float x, float y, int& map_x, int& map_y);
};
