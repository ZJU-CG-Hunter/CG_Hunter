#pragma once

#include <HModel.h>

#include <vector>
#include <set>

using namespace std;

class HMap {
private:
	float _map_width;
	float _map_length;
	vector<vector<set<HModel*>>> _map;
	HModel* _map_model;
	float _height_map[x_range+1][y_range+1];

public:
	HMap(string const& path);
	
	void Draw();

	HModel* get_map_model();

	void insert_model(HModel* model);

	void remove_model(HModel* model);

	set<HModel*> get_model_nearby(HModel* model);

	void get_point_position(float x, float y, int& map_x, int& map_y);

	float get_height(int x, int y);

private:
	void gen_height_map();

};
