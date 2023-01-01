#pragma once
#include <HModel.h>

class HTree : public HModel {
protected:
	unsigned int num;
	vector<glm::mat4> models;

public:
	void set_models(vector<glm::mat4> models);

	HTree(string const& path, bool gamma);

	void Draw();

	void set_num(int count);

	vector<glm::vec3> get_collider();

};