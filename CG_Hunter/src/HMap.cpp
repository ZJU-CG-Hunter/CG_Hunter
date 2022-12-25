#include <HMap.h>

/* ---------------------------------------------Public Method -----------------------------------------------*/

HMap::HMap(string const& path){
	_map_model = new HModel(path, false);
	gen_height_map();

  adjust_map_size();

	gen_landscape();

	fill_height_map();
}

void HMap::Draw() {
	_map_model->Draw();

	//cout << "Size: " << _draw_model.size() << endl;
	for (int i = 0; i < _draw_model.size(); i++) {
		_draw_model[i]._model->position = *_draw_model[i]._adjust_pos;
		_draw_model[i]._model->Draw();
	}
	
}

HModel* HMap::get_map_model() {
	return _map_model;
}

float HMap::get_height(float x, float y) {
	float map_x = (x + _map_width / 2) / _map_width * (float)x_range;
	float map_y = (-y + _map_height / 2) / _map_height * (float)y_range;

	float right_rate = map_x - (int)(map_x);
	float top_rate = map_y - (int)(map_y);

	float left_bottom_height = _map_data[(int)(map_y)][(int)(map_x)]._height;
	float left_top_height = _map_data[(int)(map_y)+1][(int)(map_x)]._height;
	float right_bottom_height = _map_data[(int)(map_y)][(int)(map_x)+1]._height;
	float right_top_height = _map_data[(int)(map_y)+1][(int)(map_x)+1]._height;

	float height = (left_bottom_height + left_top_height) * (1 - right_rate) + (right_bottom_height + right_top_height) * right_rate + (left_bottom_height + right_bottom_height) * (1 - top_rate) + (left_top_height + right_top_height) * top_rate;
	height /= 4;

	return height;
}

void HMap::insert_model(HModel* model) {
	float x = model->position.x;
	float y = model->position.z;

	int map_x = (x + _map_width / 2) / _map_width * x_range;
	int map_y = (-y + _map_height / 2) / _map_height * y_range;

	if (map_x<0 || map_x>x_range || map_y<0 || map_y>y_range)
		return;
	else {
		_map_data[map_y][map_x]._models.emplace_back(Model_Data(model, nullptr));
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
			if (it->_model == model)
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


vector<Model_Data> HMap::get_model_nearby(HModel* model, float range) {
	float x = model->position.x;
	float y = model->position.z;

	int map_x = (x + _map_width / 2) / _map_width * x_range;
	int map_y = (-y + _map_height / 2) / _map_height * y_range;

	int detect_range_x = range / _map_width * x_range;
	int detect_range_y = range / _map_height * y_range;

	vector<Model_Data> ret;

	for (int i = -detect_range_x; i <= detect_range_x; i++) {
		for (int j = -detect_range_y; j <= detect_range_y; j++) {
			if(map_y + j <= y_range && map_y + j >= 0 && map_x + i <= x_range && map_x + i >= 0)
				for (int k = 0; k < _map_data[map_y + j][map_x + i]._models.size(); k++) {
					if(_map_data[map_y + j][map_x + i]._models[k]._model != model)
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

	cout << "Vertex_num: " << vertex_num << endl;

	float ratio = (_map_model->max_x - _map_model->min_x) / (_map_model->max_y - _map_model->min_y);
	y_range = pow((vertex_num / ratio), 0.45);
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

void HMap::gen_landscape() {
	srand((unsigned int)glfwGetTime());
	/* Gen Grass, Trees */
	HModel* grass = new HModel("./resources/model/map_model/map_tree1.fbx", false);
	HModel* tree1 = new HModel("./resources/model/map_model/map_tree2.fbx", false);
	HModel* tree2 = new HModel("./resources/model/map_model/map_tree3.fbx", false);
	HModel* tree3 = new HModel("./resources/model/map_model/map_tree4.fbx", false);
	HModel* tree4 = new HModel("./resources/model/map_model/map_tree5.fbx", false);
	HModel* tree5 = new HModel("./resources/model/map_model/map_tree6.fbx", false);

	grass->SetRotation(glm::quat(glm::highp_vec3(glm::radians(-90.0f), 0.0f, 0.0f)));
	grass->SetScaling(glm::vec3(0.03f, 0.03f, 0.03f));
	tree1->SetRotation(glm::quat(glm::highp_vec3(glm::radians(-90.0f), 0.0f, 0.0f)));
	tree1->SetScaling(glm::vec3(0.03f, 0.03f, 0.03f));

	tree2->SetRotation(glm::quat(glm::highp_vec3(glm::radians(-90.0f), 0.0f, 0.0f)));
	tree2->SetScaling(glm::vec3(0.03f, 0.03f, 0.03f));

	tree3->SetRotation(glm::quat(glm::highp_vec3(glm::radians(-90.0f), 0.0f, 0.0f)));
	tree3->SetScaling(glm::vec3(0.03f, 0.03f, 0.03f));

	tree4->SetRotation(glm::quat(glm::highp_vec3(glm::radians(-90.0f), 0.0f, 0.0f)));
	tree4->SetScaling(glm::vec3(0.03f, 0.03f, 0.03f));

	tree5->SetRotation(glm::quat(glm::highp_vec3(glm::radians(-90.0f), 0.0f, 0.0f)));
	tree5->SetScaling(glm::vec3(0.03f, 0.03f, 0.03f));

	_landscapes.emplace_back(grass);
	_landscapes.emplace_back(tree1);
	_landscapes.emplace_back(tree2);
	_landscapes.emplace_back(tree3);
	_landscapes.emplace_back(tree4);
	_landscapes.emplace_back(tree5);


	vector<int> num(6);
	num[0] = 150;
	num[1] = 150;
	num[2] = 150;
	num[3] = 150;
	num[4] = 150;
	num[5] = 150;


	for (int l = 0; l < 6; l++) {
		for (int k = 0; k < num[l]; k++) {
			int i, j;
			bool flag = true;
			while (flag) {
				flag = false;
				i = rand() % x_range;
				j = rand() % y_range;

				for (int t = 0; t < _map_data[j][i]._models.size(); t++)
					if (_map_data[j][i]._models[t]._model == _landscapes[l] && _map_data[j][i]._height != INVALID_HEIGHT)
						flag = true;
			}

			cout << "l: " << l << " k: " << k << endl;
		
			float x = -_map_width / 2 + (float)i / (float)x_range * _map_width;
			float z = _map_height / 2 - (float)j / (float)y_range * _map_height;
			float y = _map_data[j][i]._height;

			glm::vec3* adjust_pos= new glm::vec3(x, y, z);
			_map_data[j][i]._models.emplace_back(Model_Data(_landscapes[l], adjust_pos));
			_draw_model.emplace_back(Model_Data(_landscapes[l], adjust_pos));
		}
	}
}

vector<HModel*> HMap::get_landscape() {
	return _landscapes;
}

void HMap::fill_height_map() {
	int cnt = 0; 

	int offset_x[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	int offset_y[8] = { 1, 1, 1, 0, 0, -1, -1, -1 };

	vector<int> unsolved_x;
	vector<int> unsolved_y;

	for (int i = 0; i <= x_range; i++) {
		for (int j = 0; j <= y_range; j++) {
			if (_map_data[j][i]._height == INVALID_HEIGHT) {
				int valid_num = 0;
				float height_sum = 0;
				for (int k = 0; k < 8; k++) {
					if (j + offset_y[k] >= 0 && j + offset_y[k] <= y_range && i + offset_x[k] >= 0 && i + offset_x[k] <= x_range && _map_data[j + offset_y[k]][i + offset_x[k]]._height != INVALID_HEIGHT) {
						valid_num++;
						height_sum += _map_data[j + offset_y[k]][i + offset_x[k]]._height;
					}
				}
				if (valid_num > 0)
					_map_data[j][i]._height = height_sum / valid_num;
				else {
					unsolved_x.emplace_back(i);
					unsolved_y.emplace_back(j);
				}
			}
		}
	}

	int round = 1;
	while (unsolved_x.size() > 0) {
		cout << "Round: " << round << endl;
		auto it_x = unsolved_x.begin();
		auto it_y = unsolved_y.begin();
		for (; it_x != unsolved_x.end(); it_x++, it_y++) {
			int valid_num = 0;
			float height_sum = 0;
			for (int k = 0; k < 8; k++) {
				if (*it_y + offset_y[k] >= 0 && *it_y + offset_y[k] <= y_range && *it_x + offset_x[k] >= 0 && *it_x + offset_x[k] <= x_range && _map_data[*it_y + offset_y[k]][*it_x + offset_x[k]]._height != INVALID_HEIGHT) {
					valid_num++;
					height_sum += _map_data[*it_y + offset_y[k]][*it_x + offset_x[k]]._height;
				}
			}
			if (valid_num > 0) {
				_map_data[*it_y][*it_x]._height = height_sum / valid_num;
				unsolved_x.erase(it_x);
				unsolved_y.erase(it_y);
			}
		}

		round++;
	}

}




