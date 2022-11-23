#include <HCollider.h>

HCollider::HCollider(vector<vector<float>> v)
{
	vector<GLfloat> Point = getcenter(v);
	Eigen::Matrix3d cov = getcov(v, Point);
	Eigen::Matrix3d feture = calcfeture(cov);
	Eigen::MatrixXd m = calcmaxormin(v, feture);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			_feature[i][j] = feture(i, j);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 2; j++)
			_board[i][j] = m(i, j);
}

glm::mat3x3 HCollider::get_feature()
{
	return _feature;
}

glm::mat3x2 HCollider::get_board()
{
	return _board;
}

vector<float> HCollider::getcenter(vector<vector<float>> v)
{
	vector<GLfloat> Point;
	GLfloat sumx = 0, sumy = 0, sumz = 0;
	for (size_t i = 0; i < v.size(); i++)
	{
		sumx += v[i][0];
		sumy += v[i][1];
		sumz += v[i][2];
	}
	Point.push_back(sumx / v.size());
	Point.push_back(sumy / v.size());
	Point.push_back(sumz / v.size());

	return Point;
}
//

Eigen::Matrix3d HCollider::getcov(vector<vector<float>> v, vector<float> Point)
{
	GLfloat c11 = 0, c12 = 0, c13 = 0;
	GLfloat c21 = 0, c22 = 0, c23 = 0;
	GLfloat c31 = 0, c32 = 0, c33 = 0;

	for (size_t i = 0; i < v.size(); i++)
	{
		c11 += (v[i][0] - Point[0]) * (v[i][0] - Point[0]);
		c12 += (v[i][0] - Point[0]) * (v[i][1] - Point[1]);
		c13 += (v[i][0] - Point[0]) * (v[i][2] - Point[2]);
		c21 += (v[i][0] - Point[0]) * (v[i][1] - Point[1]);
		c22 += (v[i][1] - Point[1]) * (v[i][1] - Point[1]);
		c23 += (v[i][1] - Point[1]) * (v[i][2] - Point[2]);
		c31 += (v[i][0] - Point[0]) * (v[i][2] - Point[2]);
		c32 += (v[i][1] - Point[1]) * (v[i][2] - Point[2]);
		c33 += (v[i][2] - Point[2]) * (v[i][2] - Point[2]);
	}
	Eigen::Matrix3d cov;
	int len = v.size();
	cov << c11 / len, c12 / len, c13 / len,
		c21 / len, c22 / len, c23 / len,
		c31 / len, c32 / len, c33 / len;

	return cov;
}

Eigen::Matrix3d HCollider::calcfeture(Eigen::Matrix3d cov)
{
	EigenSolver<MatrixXd> es(cov);
	Eigen::Matrix3d V = es.pseudoEigenvectors();

	return V;
}

Eigen::MatrixXd HCollider::calcmaxormin(vector<vector<float>> v, Eigen::Matrix3d feture)
{
	GLfloat xmin = 0, xmax = 0;
	GLfloat ymin = 0, ymax = 0;
	GLfloat zmin = 0, zmax = 0;

	GLfloat _xmin = 0, _xmax = 0;
	GLfloat _ymin = 0, _ymax = 0;
	GLfloat _zmin = 0, _zmax = 0;

	xmin = v[0][0] * feture(0, 0) + v[0][1] * feture(1, 0) + v[0][2] * feture(2, 0);
	xmax = v[0][0] * feture(0, 0) + v[0][1] * feture(1, 0) + v[0][2] * feture(2, 0);
	ymin = v[0][0] * feture(0, 1) + v[0][1] * feture(1, 1) + v[0][2] * feture(2, 1);
	ymax = v[0][0] * feture(0, 1) + v[0][1] * feture(1, 1) + v[0][2] * feture(2, 1);
	zmin = v[0][0] * feture(0, 2) + v[0][1] * feture(1, 2) + v[0][2] * feture(2, 2);
	zmax = v[0][0] * feture(0, 2) + v[0][1] * feture(1, 2) + v[0][2] * feture(2, 2);

	for (size_t i = 1; i < v.size(); i++)
	{
		_xmin = v[i][0] * feture(0, 0) + v[i][1] * feture(1, 0) + v[i][2] * feture(2, 0);
		_xmax = v[i][0] * feture(0, 0) + v[i][1] * feture(1, 0) + v[i][2] * feture(2, 0);
		_ymin = v[i][0] * feture(0, 1) + v[i][1] * feture(1, 1) + v[i][2] * feture(2, 1);
		_ymax = v[i][0] * feture(0, 1) + v[i][1] * feture(1, 1) + v[i][2] * feture(2, 1);
		_zmin = v[i][0] * feture(0, 2) + v[i][1] * feture(1, 2) + v[i][2] * feture(2, 2);
		_zmax = v[i][0] * feture(0, 2) + v[i][1] * feture(1, 2) + v[i][2] * feture(2, 2);

		xmin = xmin < _xmin ? xmin : _xmin;
		ymin = ymin < _ymin ? ymin : _ymin;
		zmin = zmin < _zmin ? zmin : _zmin;

		xmax = xmax > _xmax ? xmax : _xmax;
		ymax = ymax > _ymax ? ymax : _ymax;
		zmax = zmax > _zmax ? zmax : _zmax;
	}

	Eigen::MatrixXd res(3, 2);
	res << xmin, xmax,
		ymin, ymax,
		zmin, zmax;

	return res;
}