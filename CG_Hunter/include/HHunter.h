#pragma once

#include <HModel.h>
#include <HBullet.h>

const int Hunter_Run = 7; // 0
const int Hunter_Idle = 6; 
const int Hunter_Shoot_Up = 3;
const int Hunter_Shoot_Down = 9;

class HHunter : public HModel {
private:
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  float Yaw;
  float Pitch;
  float MovementSpeed = SPEED;
  float MouseSensitivity = SENSITIVITY;

  glm::vec3 last_position;

  HBullet* gun;

  bool is_aim = false;

  unsigned int magnifier_VAO = 0;
  unsigned int magnifier_VBO = 0;
  vector<float> magnifier_vertices;
  HShader* magnifier_shader;

public:
  HHunter(string const& path, const glm::vec3 front, const glm::vec3 up, const glm::vec3 right, const glm::vec3 worldup, float yaw, float pitch);

  void Action(HMap* map, float duration_time);

  void Event(Events* event);

  void move(Camera_Movement dirention, float deltaTime);

  void idle();

  void turn(float xoffset, float yoffset);

  void update_camera();

  void collision_detection(HMap* _map);

  void shoot();

  void aim(bool is_aim);

  void BindGun(HModel* gun);

  void BindMagnifierShader(HShader* magnifier_shader);

  void DrawMagnifier();

  void shoot_ready(bool up_down);
};
