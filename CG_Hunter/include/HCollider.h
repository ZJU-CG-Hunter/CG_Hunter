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
	glm::mat3x3 _feature;
	glm::mat3x2 _board;
public:
	HCollider(vector<vector<float>> v);

	glm::mat3x3 get_feature();

	glm::mat3x2 get_board();
private:
	//void generate_board(vector<vector<float>> v);

	Eigen::Matrix3d getcov(vector<vector<float>> v, vector<GLfloat> Point);

	Eigen::Matrix3d calcfeture(Eigen::Matrix3d cov);

	Eigen::MatrixXd calcmaxormin(vector<vector<float>> v, Eigen::Matrix3d feture);

	vector<float> getcenter(vector<vector<float>> v);
};
