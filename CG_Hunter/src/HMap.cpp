#include <HMap.h>

/* ---------------------------------------------Public Method -----------------------------------------------*/

HMap::HMap(string const& path){
	_map_model = new HModel(path, false);
	gen_height_map();
	adjust_map_size();
}

void HMap::Draw() {
	_map_model->Draw();
}

HModel* HMap::get_map_model() {
	return _map_model;
}


float HMap::get_height(float x, float y) {
	int map_x = (x + _map_width / 2) / _map_width * x_range;
	int map_y = (-y + _map_height / 2) / _map_height * y_range;

	if (map_x<0 || map_x>x_range || map_y<0 || map_y>y_range)
		return INVALID_HEIGHT;
	else
		return _map_data[map_y][map_x]._height;
}

void HMap::insert_model(HModel* model) {
	float x = model->position.x;
	float y = model->position.z;

	int map_x = (x + _map_width / 2) / _map_width * x_range;
	int map_y = (-y + _map_height / 2) / _map_height * y_range;

	if (map_x<0 || map_x>x_range || map_y<0 || map_y>y_range)
		return;
	else {
		_map_data[map_y][map_x]._models.emplace_back(model);
		model->map_x = map_x;
		model->map_y = map_y;
	}
	return;
}

void HMap::remove_model(HModel* model) {
	int map_x = model->map_x;
	int map_y = model->map_y;

	if (map_x<0 || map_x>x_range || map_y<0 || map_y>y_range)
		return;
	else {
		for (auto it = _map_data[map_y][map_x]._models.begin(); it != _map_data[map_y][map_x]._models.end(); ) {
			if (*it == model)
				it = _map_data[map_y][map_x]._models.erase(it);
			else
				it++;
		}
		model->map_x = -1;
		model->map_y = -1;
	}
		
			
	return;
}

void HMap::update_model(HModel* model) {
	remove_model(model);
	insert_model(model);
	
}


vector<HModel*> HMap::get_model_nearby(HModel* model, float range) {
	float x = model->position.x;
	float y = model->position.z;

	int map_x = (x + _map_width / 2) / _map_width * x_range;
	int map_y = (-y + _map_height / 2) / _map_height * y_range;

	int detect_range_x = range / _map_width * x_range;
	int detect_range_y = range / _map_height * y_range;

	vector<HModel*> ret;

	for (int i = -detect_range_x; i <= detect_range_x; i++) {
		for (int j = -detect_range_y; j <= detect_range_y; j++) {
			for (int k = 0; k < _map_data[map_y + j][map_x + i]._models.size(); k++) {
				if(map_y+j<=y_range && map_y+j>=0 && map_x+i<=x_range && map_x+i>=0 && _map_data[map_y + j][map_x + i]._models[k] != model)
					ret.emplace_back(_map_data[map_y + j][map_x + i]._models[k]);
			}
		}
	}

	return ret;
}


/* ---------------------------------------------Private Method -----------------------------------------------*/
void HMap::adjust_map_size() {
	_map_model->SetScaling(glm::vec3(Scale_X, Scale_Y, Scale_Z));
	_map_model->SetRotation(glm::quat(glm::highp_vec3(glm::radians(-90.0f), 0.0f, 0.0f)));
	_map_model->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
	_map_width = Scale_X * 2;
	_map_height = Scale_Y * 2;
}


void HMap::gen_height_map() {
	/* 1. Generate a sutibale diravation */
	int vertex_num = 0;
	for (int i = 0; i < _map_model->meshes.size(); i++)
		vertex_num += _map_model->meshes[i].vertices.size();

	float ratio = (_map_model->max_x - _map_model->min_x) / (_map_model->max_y - _map_model->min_y);
	y_range = sqrt(vertex_num / ratio);
	x_range = y_range * ratio;

	/* 2. Initialize the Map */
	_map_data.resize(y_range+1);
	for (int i = 0; i < _map_data.size(); i++)
		_map_data[i].resize(x_range+1, Map_Data());

	/* 将地图坐标映射到高度图中 */
	float k_x = x_range / (_map_model->max_x - _map_model->min_x);
	float b_x = -k_x * _map_model->min_x;
	float k_y = y_range / (_map_model->max_y - _map_model->min_y);
	float b_y = -k_y * _map_model->min_y;

	for (int i = 0; i < _map_model->meshes.size(); i++)
		for (int j = 0; j < _map_model->meshes[i].vertices.size(); j++) {
			glm::vec3 position = _map_model->meshes[i].vertices[j].Position;
			int px = (int)(position.x * k_x + b_x);
			int py = (int)(position.y * k_y + b_y);

			_map_data[py][px]._height = position.z * Scale_Z;
		}
}


