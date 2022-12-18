#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <eigen3/Eigen/Dense>
#include <vector>

using namespace std;
using namespace Eigen;
class HCollider {
private:
	glm::vec3 _feature[3];
	glm::vec2 _board[3];
	glm::vec3 _Points[8];
public:
	HCollider(vector<vector<float>> v);

	glm::vec3* get_feature();

	glm::vec2* get_board();
private:
	//void generate_board(vector<vector<float>> v);

	void CalPoints();

	Eigen::Matrix3d getcov(vector<vector<float>> v, vector<GLfloat> Point);

	Eigen::Matrix3d calcfeture(Eigen::Matrix3d cov);

	Eigen::MatrixXd calcmaxormin(vector<vector<float>> v, Eigen::Matrix3d feture);

	vector<float> getcenter(vector<vector<float>> v);
};
