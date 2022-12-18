#include <HMap.h>

HMap::HMap(string const& path){
	_map_model = new HModel(path, false);
	gen_height_map();
}

void HMap::Draw() {

	_map_model->Draw();
}

HModel* HMap::get_map_model() {
	return _map_model;
}


void HMap::gen_height_map() {
	cout << _map_model->min_x << endl;
	cout << _map_model->max_x << endl;
	cout << _map_model->min_y << endl;
	cout << _map_model->max_y << endl;
	cout << _map_model->min_z << endl;
	cout << _map_model->max_z << endl;
	
	for (int i = 0; i < x_range; i++)
		for (int j = 0; j < y_range; j++)
			_height_map[i][j] = INVALID_HEIGHT;

	/* 将地图坐标映射到高度图中 */
	float k_x = x_range / (_map_model->max_x - _map_model->min_x);
	float b_x = -k_x * _map_model->min_x;
	float k_y = y_range / (_map_model->max_y - _map_model->min_y);
	float b_y = -k_y * _map_model->min_y;

	int cnt = 0;
	for(int i = 0; i<_map_model->meshes.size(); i++)
		for (int j = 0; j < _map_model->meshes[i].vertices.size(); j++) {
			glm::vec3 position = _map_model->meshes[i].vertices[j].Position;
			int px = (int)(position.x * k_x + b_x);
			int py = (int)(position.y * k_y + b_y);

			_height_map[px][py] = position.z * Scale_Z;
			//cout << "px: " << px << " py: " << py << " position_z: " << _height_map[px][py] << endl;
			cnt++;
		}
	//cout << "Point num: " << cnt << endl;
}

float HMap::get_height(int x, int y) {
	if (x<0 || x>x_range || y<0 || y>y_range)
		return INVALID_HEIGHT;
	else
		return _height_map[x][y];
}


void HMap::insert_model(HModel* model) {
	// Caculate the 4 points which defines the board of the model
	float model_center_x = model->position.x, model_center_y = model->position.y;
	int map_x = 0, map_y = 0;
	int sign_x[4] = { 1, 1, -1, -1 }, sign_y[4] = { 1, -1, 1, -1 };
	glm::vec2* board = model->collider->get_board();
	float model_length = fabs(board[0][0] - board[0][1]);
	float model_width = fabs(board[2][0] - board[2][1]);
	for (int i = 0; i < 4; i++) {
		get_point_position(model_center_x + sign_x[i] * model_length / 2, model_center_y + sign_y[i] * model_width / 2, map_x, map_y);
		_map[map_x][map_y].insert(model);
	}
}

void HMap::remove_model(HModel* model) {
	float model_center_x = model->position.x, model_center_y = model->position.y;
	int map_x = 0, map_y = 0;
	int sign_x[4] = { 1, 1, -1, -1 }, sign_y[4] = { 1, -1, 1, -1 };
	glm::vec2* board = model->collider->get_board();
	float model_length = fabs(board[0][0] - board[0][1]);
	float model_width = fabs(board[2][0] - board[2][1]);
	for (int i = 0; i < 4; i++) {
		get_point_position(model_center_x + sign_x[i] * model_length / 2, model_center_y + sign_y[i] * model_width / 2, map_x, map_y);
		_map[map_x][map_y].erase(model);
	}
}

set<HModel*> HMap::get_model_nearby(HModel* model) {
	float model_center_x = model->position.x, model_center_y = model->position.y;
	int map_x = 0, map_y = 0;
	int sign_x[4] = { 1, 1, -1, -1 }, sign_y[4] = { 1, -1, 1, -1 };
	set<HModel*> ret_set;
	glm::vec2* board = model->collider->get_board();
	float model_length = fabs(board[0][0] - board[0][1]);
	float model_width = fabs(board[2][0] - board[2][1]);
	for (int i = 0; i < 4; i++) {
		get_point_position(model_center_x + sign_x[i] * model_length / 2, model_center_y + sign_y[i] * model_width / 2, map_x, map_y);
		ret_set.insert(_map[map_x][map_y].begin(), _map[map_x][map_y].end());
	}
	return ret_set;
}


void HMap::get_point_position(float x, float y, int& map_x, int& map_y) {
	//float factor_x = x / (_map_length/2), factor_y = y / (_map_width/2);
	//float k_b = pow(2, _level) / 2;
	//map_x = (int)(k_b * factor_x + k_b + 0.5);
	//map_y = (int)(k_b * factor_y + k_b + 0.5);
	;
}

