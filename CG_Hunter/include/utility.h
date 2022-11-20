#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <STB/stb_image.h>
#include <ASSIMP/Importer.hpp>
#include <ASSIMP/scene.h>
#include <ASSIMP/postprocess.h>
#include <glm/gtc/quaternion.hpp>

#include <def.h>

#include <string>
#include <iostream>
#include <vector>

using namespace std;

void Adjustoffset(unsigned int& buffer_offset, unsigned int base_offset);

void BindUniformData(unsigned int& buffer_offset, glm::mat4* mat);

void show_mat4(glm::mat4 mat, string mat_name);