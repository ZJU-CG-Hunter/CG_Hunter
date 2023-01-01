#pragma once

#include <HShader.h>
#include <HCamera.h>
#include <utility.h>

#include <iostream>

using namespace std;

extern vector<float> default_skyboxvertices;
extern vector<string> default_skyboxfaces;


class HSkybox {
private:
  vector<float> _skyboxvertices;

  vector<string> _skyboxfaces;

  unsigned int _skybox_buffer_id;

  unsigned int _skyboxVAO;
  unsigned int _skyboxVBO;
  unsigned int _skyboxtexture;

  unsigned int _binding_point;

  HShader* _shader;
  HCamera* _camera;

public:
  HSkybox(vector<float>& skyboxvertices, vector<string>& skyboxfaces);

  void Draw();

  void BindShader(HShader* shader);

  void BindCamera(HCamera* camera);

  void BindShaderUniformBuffer(unsigned int bind_point);

private:
  void bindbuffer();
  void loadskybox();
};