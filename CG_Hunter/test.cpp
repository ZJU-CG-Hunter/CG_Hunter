#include "test.h"

void display() {
	std::cout << "�����ͼ��ѧС����ҵ " << std::endl;
	std::cout << "С���Ա��" << std::endl;
	std::cout << member_1 << std::endl;
	std::cout << member_2 << std::endl;
	std::cout << member_3 << std::endl;
}

int main() {
	glutCreateWindow("���Դ���");
	glutDisplayFunc(display);
	glutMainLoop();
}
