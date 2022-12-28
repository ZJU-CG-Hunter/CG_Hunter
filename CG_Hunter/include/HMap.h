#pragma once

#include <HTree.h>
#include <vector>
#include <set>

using namespace std;

struct Model_Data {
	HModel* _model;

	glm::vec3* _adjust_pos;

	Model_Data(HModel* model, glm::vec3* adjust_pos) {
		_model = model;
		_adjust_pos = adjust_pos;
	}

};

struct Map_Data {
	float _height;

	vector<Model_Data> _models;

	Map_Type _map_type;

	Map_Data(): _height(INVALID_HEIGHT), _map_type(Map_Type::Unkown){}
};

class HMap {
private:
	HModel* _map_model; /* The model of the map */

	vector<HTree*> _landscapes;
	
	vector<vector<Map_Data>> _map_data;

	float _map_width;

	float _map_height;

	int x_range;

	int y_range;

	unsigned int _map_seed;
public:
	HMap(string const& path, unsigned int map_seed = 0);
	
	void Draw();

	HModel* get_map_model();

	void insert_model(HModel* model, glm::vec3* adjust_position = nullptr);

	void remove_model(HModel* model, glm::vec3* last_position = nullptr);

	void update_model(HModel* model, glm::vec3* adjust_position = nullptr, glm::vec3* last_position = nullptr);

	vector<HTree*> get_landscape();

	vector<Model_Data> get_model_nearby(HModel* model, float range = DEFALUT_NEARBY_RANGE);

	float get_height(float x, float y);

	float get_map_width();

	float get_map_height();

private:
	void gen_height_map();

	void adjust_map_size();

	void fill_height_map();

	void gen_landscape();

};
