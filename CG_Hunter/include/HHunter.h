#pragma once

#include <HModel.h>

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


public:
  HHunter(string const& path, const glm::vec3 front, const glm::vec3 up, const glm::vec3 right, const glm::vec3 worldup, float yaw, float pitch);

  void Event(Collision event, float duration_time);

  void Event(Events event, float duration_time);

  void move(Camera_Movement dirention, float deltaTime);

  void turn(float xoffset, float yoffset);

  void update_camera();

  glm::vec3 get_position();
};
