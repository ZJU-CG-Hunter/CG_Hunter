#pragma once
//#include <GLAD/glad.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <GLUT/glut.h>

using namespace std;

class ObjLoader
{
public:
	struct vertex
	{
		float x;
		float y;
		float z;
	};
	ObjLoader(string filename);//读取函数
	void Draw();//绘制函数
private:
	vector<vector<GLfloat>> v;//存放顶点(x,y,z)坐标
	vector<vector<GLint>> f;//存放面的三个顶点索引
};

ObjLoader::ObjLoader(string filename)
{
	ifstream file(filename);
	if (!file)
	{
		cout << "hahaha";
	}
	string line;
	while (getline(file, line))
	{
		if (line.substr(0, 1) == "v")
		{
			vector<GLfloat> Point;
			GLfloat x, y, z;
			istringstream s(line.substr(2));
			s >> x; s >> y; s >> z;
			Point.push_back(x);
			Point.push_back(y);
			Point.push_back(z);
			v.push_back(Point);

		}
		else if (line.substr(0, 1) == "f")
		{
			vector<GLint> vIndexSets;
			GLint u, v, w;
			istringstream vtns(line.substr(2));
			vtns >> u; vtns >> v; vtns >> w;
			vIndexSets.push_back(u - 1);
			vIndexSets.push_back(v - 1);
			vIndexSets.push_back(w - 1);
			f.push_back(vIndexSets);
		}
	}
	file.close();
}
void ObjLoader::Draw()
{
	glBegin(GL_TRIANGLES);//开始绘制
	for (int i = 0; i < f.size(); i++) {
		GLfloat VN[3];//法线
		//三个顶点
		vertex a, b, c, normal;

		if ((f[i]).size() != 3) {
			cout << "ERRER::THE SIZE OF f IS NOT 3!" << endl;
		}
		else {
			GLint firstVertexIndex = (f[i])[0];//取出顶点索引
			GLint secondVertexIndex = (f[i])[1];
			GLint thirdVertexIndex = (f[i])[2];

			a.x = (v[firstVertexIndex])[0];//第一个顶点
			a.y = (v[firstVertexIndex])[1];
			a.z = (v[firstVertexIndex])[2];

			b.x = (v[secondVertexIndex])[0]; //第二个顶点
			b.y = (v[secondVertexIndex])[1];
			b.z = (v[secondVertexIndex])[2];

			c.x = (v[thirdVertexIndex])[0]; //第三个顶点
			c.y = (v[thirdVertexIndex])[1];
			c.z = (v[thirdVertexIndex])[2];


			GLfloat vec1[3], vec2[3], vec3[3];//计算法向量
			//(x2-x1,y2-y1,z2-z1)
			vec1[0] = a.x - b.x;
			vec1[1] = a.y - b.y;
			vec1[2] = a.z - b.z;

			//(x3-x2,y3-y2,z3-z2)
			vec2[0] = a.x - c.x;
			vec2[1] = a.y - c.y;
			vec2[2] = a.z - c.z;

			//(x3-x1,y3-y1,z3-z1)
			vec3[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
			vec3[1] = vec2[0] * vec1[2] - vec2[2] * vec1[0];
			vec3[2] = vec2[1] * vec1[0] - vec2[0] * vec1[1];

			GLfloat D = sqrt(pow(vec3[0], 2) + pow(vec3[1], 2) + pow(vec3[2], 2));

			VN[0] = vec3[0] / D;
			VN[1] = vec3[1] / D;
			VN[2] = vec3[2] / D;

			glNormal3f(VN[0], VN[1], VN[2]);//绘制法向量

			glVertex3f(a.x, a.y, a.z);//绘制三角面
			glVertex3f(b.x, b.y, b.z);
			glVertex3f(c.x, c.y, c.z);
		}
	}
	glEnd();
}
ObjLoader monkey = ObjLoader("./source/lefthand.obj");
/*void display(void)
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//像素传输
	//setLight();//渲染光照
	monkey.Draw();
}*/
double pi = 3.1415926;
using namespace std;
//float x_angle = 0.0f;
//float y_angle = 0.0f;
float xmove = 0.0f;
float ymove = 0.0f;
int mouse_move = 0;


//double aix_x = 0.0, aix_y = 1, aix_z = 0;


float ex = 0.0f, ey = 20.0f, ez = 150.0f;
float cx = 0.0f, cy = 0.0f, cz = 0.0f;
float ux = 0.0f, uy = 1.0f, uz = 0.0f;

bool firstmove = true;
float lastX, lastY;
int mouse_state;


void display();
void myidle();
void mouse_Position(int button, int state, int x, int y);
void mouse_Move(int x, int y);
void keyboard(unsigned char key, int x, int y);
void reshape(int w, int h);


static float earthtime = 0, moontime = 0, saturntime = 0;

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(700, 700);
	glutInitWindowPosition(100, 0);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("星球模拟");


	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_DEPTH_TEST);  //深度检测  

	glClearDepth(1.0f);
	glutDisplayFunc(display);
	glutIdleFunc(myidle);
	glutMouseFunc(mouse_Position);
	glutMotionFunc(mouse_Move);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	//float ex = 0.0f, ey = 5.0f, ez = 10.0f;
	//float cx = 0.0f, cy = 0.0f, cz = 0.0f;
	//float ux = 0.0f, uy = 1.0f, uz = 0.0f;
	gluLookAt(
		ex, ey, ez,
		cx, cy, cz,
		ux, uy, uz);

	// 实现鼠标旋转的核心
	//glRotatef(x_angle, 1.0f, 0.0f, 0.0f);
	//glRotatef(y_angle, 0.0f, 1.0f, 0.0f);

	monkey.Draw();


	glutSwapBuffers();
	glFlush();
}
void myidle() {
	glutPostRedisplay();
}
void mouse_Position(int button, int state, int x, int y)
{

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && mouse_state == 1)
	{
		firstmove = true;                  //左键点击并且上一个阶段是松开的，那么这次点击就是一个新的开始
		mouse_state = 0;                   //点击滑动不会重置
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		mouse_state = 1;                   //松开代表下一次点击视为开始，状态改变
	}
}

void mouse_Move(int x, int y)
{

	float xpos = (float)(x);
	float ypos = (float)(y);

	if (firstmove)         //第一次移动时将当前位置设为初始
	{
		lastX = xpos;
		lastY = ypos;
		firstmove = false;
	}

	float xoffset = xpos - lastX;  //平面x轴方向和三维中一致，直接减

	float yoffset = lastY - ypos; //平面y轴方向和三维中相反，交换位置减

	lastX = xpos;                //当前x,y变成下一次检测的上一个

	lastY = ypos;

	cx += xoffset / 20;         //实际测试1/100比较适宜
	cy += yoffset / 20;

	glutPostRedisplay();
}
void keyboard(unsigned char key, int x, int y)
{
	//计算视线向量
	float etox = cx - ex;
	float etoy = cy - ey;
	float etoz = cz - ez;

	//计算视线方向向量，叉乘得到
	float sx = uy * etoz - etoy * uz;
	float sy = -(ux * etoz - etox * uz);
	float sz = ux * etoy - etox * uy;

	//单位化
	sx = sx / sqrt(sx * sx + sy * sy + sz * sz);
	sy = sy / sqrt(sx * sx + sy * sy + sz * sz);
	sz = sz / sqrt(sx * sx + sy * sy + sz * sz);

	etox = etox / sqrt(etox * etox + etoy * etoy + etoz * etoz);
	etoy = etoy / sqrt(etox * etox + etoy * etoy + etoz * etoz);
	etoz = etoz / sqrt(etox * etox + etoy * etoy + etoz * etoz);
	switch (key)
	{
	case 'W':
	case 'w':
		ex = ex + 0.2 * etox;
		ey = ey + 0.2 * etoy;
		ez = ez + 0.2 * etoz;
		cx = cx + 0.2 * etox;
		cy = cy + 0.2 * etoy;
		cz = cz + 0.2 * etoz;
		//	cout << ex << " , " << ey << " , " << ez;
		break;
	case 'S':
	case 's':
		ex = ex - 0.2 * etox;
		ey = ey - 0.2 * etoy;
		ez = ez - 0.2 * etoz;
		cx = cx - 0.2 * etox;
		cy = cy - 0.2 * etoy;
		cz = cz - 0.2 * etoz;
		//	cout << ex << " , " << ey << " , " << ez;
		break;
	case 'A':
	case 'a':
		ex = ex + 0.2 * sx;
		ey = ey + 0.2 * sy;
		ez = ez + 0.2 * sz;
		cx = cx + 0.2 * sx;
		cy = cy + 0.2 * sy;
		cz = cz + 0.2 * sz;
		//	cout << ex << " , " << ey << " , " << ez;
		break;
	case 'D':
	case 'd':
		ex = ex - 0.2 * sx;
		ey = ey - 0.2 * sy;
		ez = ez - 0.2 * sz;
		cx = cx - 0.2 * sx;
		cy = cy - 0.2 * sy;
		cz = cz - 0.2 * sz;
		//	cout << ex << " , " << ey << " , " << ez;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}
void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 5, 200.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}





