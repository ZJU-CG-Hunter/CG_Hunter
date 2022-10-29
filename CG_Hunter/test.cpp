#include "test.h"

void display() {
	std::cout << "计算机图形学小组作业 " << std::endl;
	std::cout << "小组成员：" << std::endl;
	std::cout << member_1 << std::endl;
	std::cout << member_2 << std::endl;
	std::cout << member_3 << std::endl;
}

int main() {
	glutCreateWindow("测试窗口");
	glutDisplayFunc(display);
	glutMainLoop();
}
