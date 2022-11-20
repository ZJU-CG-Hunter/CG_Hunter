#include <HMap.h>

HMap::HMap(float map_width, float map_length, int level): _map_width(map_width), _map_length(map_length), _level(level) {
	_map.resize((int)pow(2, level));
	for (int i = 0; i < _map.size(); i++)
		_map[i].resize((int)pow(2, level));
}

void HMap::insert_model(HModel* model) {
	// Caculate the 4 points which defines the board of the model
	float model_center_x = model->position.x, model_center_y = model->position.y;
	int map_x = 0, map_y = 0;
	int sign_x[4] = { 1, 1, -1, -1 }, sign_y[4] = { 1, -1, 1, -1 };
	for (int i = 0; i < 4; i++) {
		get_point_position(model_center_x + sign_x[i] * model->model_length / 2, model_center_y + sign_y[i] * model->model_width / 2, map_x, map_y);
		_map[map_x][map_y].insert(model);
	}
}

void HMap::remove_model(HModel* model) {
	float model_center_x = model->position.x, model_center_y = model->position.y;
	int map_x = 0, map_y = 0;
	int sign_x[4] = { 1, 1, -1, -1 }, sign_y[4] = { 1, -1, 1, -1 };
	for (int i = 0; i < 4; i++) {
		get_point_position(model_center_x + sign_x[i] * model->model_length / 2, model_center_y + sign_y[i] * model->model_width / 2, map_x, map_y);
		_map[map_x][map_y].erase(model);
	}
}

set<HModel*> HMap::get_model_nearby(HModel* model) {
	float model_center_x = model->position.x, model_center_y = model->position.y;
	int map_x = 0, map_y = 0;
	int sign_x[4] = { 1, 1, -1, -1 }, sign_y[4] = { 1, -1, 1, -1 };
	set<HModel*> ret_set;
	for (int i = 0; i < 4; i++) {
		get_point_position(model_center_x + sign_x[i] * model->model_length / 2, model_center_y + sign_y[i] * model->model_width / 2, map_x, map_y);
		ret_set.insert(_map[map_x][map_y].begin(), _map[map_x][map_y].end());
	}
	return ret_set;
}


void HMap::get_point_position(float x, float y, int& map_x, int& map_y) {
	float factor_x = x / (_map_length/2), factor_y = y / (_map_width/2);
	float k_b = pow(2, _level) / 2;
	map_x = (int)(k_b * factor_x + k_b + 0.5);
	map_y = (int)(k_b * factor_y + k_b + 0.5);
}

