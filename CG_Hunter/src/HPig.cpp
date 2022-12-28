#include <HPig.h>
#include <HMap.h>

HPig::HPig(string const& path, bool gamma): HModel(path, gamma, (rand()%2 == 0? Pig_Wander: Pig_Idle)), _is_alive(true), _is_dying(false) {

	model_type = Model_Type::Pig;
  Front = glm::vec3(0.0f, 0.0f, -1.0f);
  Up = glm::vec3(0.0f, 1.0f, 0.0f);
  WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
  Right = glm::vec3(1.0f, 0.0f, 0.0f);
  Yaw = 0.0f;
  Pitch = 0.0f;
  warning = 0.0f;
  warning_wander = 20.0f;
  warning_walk = 50.0f;
  crazy = 80.0f;
  alert = 1.0F;

  glm::vec3 last_position;

  //cout << "Pig animation_num: " << scene->mNumAnimations;

  /* Set a random initial direction */
}

void HPig::Action(HMap* map, float duration_time) {
  if (!_is_alive)
    return;
  if (!_is_dying) {
    if (warning > warning_wander && animation_index == Pig_Idle)
      animation_index = Pig_Wander;
    else if (warning > warning_walk && (animation_index == Pig_Idle || animation_index == Pig_Wander))
      animation_index = Pig_Walk;
    else if (warning == crazy && animation_index != crazy)
      animation_index = Pig_Jump;
  }

  switch (animation_index) {

  case INVALID_ANIMATION_INDEX:
    cout << "Invalid_animation_index" << endl;
    assert(0);
  case Pig_Wander:
    Forward(duration_time);
    CalCurrentTicks(duration_time);
    // Iterative do this
    animation_ticks = fmod(animation_ticks, scene->mAnimations[animation_index]->mDuration);
    break;
  case Pig_Jump:
    Forward(duration_time);
    CalCurrentTicks(duration_time);
    // Iterative do this
    animation_ticks = fmod(animation_ticks, scene->mAnimations[animation_index]->mDuration);
    break;
  case Pig_Idle:
    break;
  case Pig_Die:
    CalCurrentTicks(duration_time);
    if (animation_ticks > scene->mAnimations[animation_index]->mDuration) {
      animation_ticks = scene->mAnimations[animation_index]->mDuration - 0.01f;
      _is_alive = false;
    }
    break;
  }


  if (fabs(position.x) > map->get_map_width() / 2 - 0.5f || fabs(position.z) > map->get_map_height() / 2 - 0.5f) {
    position = last_position;
    turn(float(rand() % 360), 0.0f);
  }
  UpdateBoneTransform();
  //UpdateColliderTransform();
  AdjustStepOnGround(map);
  map->update_model(this);
}

void HPig::Event(Events* event) {
  if (!_is_alive || _is_dying)
    return;
  HModel* another_model;
  HPig* fellow;
  Collision* collision_event;
  Observation* observation_event;
  glm::vec3 hunter_position;
  glm::vec3 observe_hunter;

  switch (event->_event_type) {
  case Event_Type::Collision:
    collision_event = reinterpret_cast<Collision*>(event);
    another_model = collision_event->_model_1 == this ? collision_event->_model_2 : collision_event->_model_1;
    switch (another_model->get_model_type()) {
    case Model_Type::Bullet:
      animation_index = Pig_Die;
      _is_dying = true;
      cout << "Hit! " << endl;
      break;
    case Model_Type::Tree:
      position = last_position;
      turn(float(rand() % 20 - 10), 0.0f);
      break;
    case Model_Type::Hunter:
      animation_index = Pig_Jump;
      warning = crazy;
      break;
    }
    break;
  case Event_Type::Observation:
    observation_event = reinterpret_cast<Observation*>(event);
    another_model = observation_event->_model_1 == this ? observation_event->_model_2 : observation_event->_model_1;
    switch (another_model->get_model_type()) {
    case Model_Type::Pig:
      fellow = reinterpret_cast<HPig*>(another_model);
      if (fellow->animation_index == Pig_Walk) {
        this->animation_index = Pig_Walk;
        float mess = float(rand() % 10 - 20);
        this->turn(fellow->Yaw - this->Yaw + mess, fellow->Pitch - this->Pitch);
        warning = fellow->warning; 
      }
      else if (fellow->animation_index == Pig_Jump) {
        this->animation_index = Pig_Jump;
        float mess = float(rand() % 50 - 100);
        this->turn(fellow->Yaw - this->Yaw + mess, fellow->Pitch - this->Pitch);
        warning = fellow->warning;
      }
      else if (fellow->animation_index == Pig_Wander) {
        warn(1.0f);
      }
      else {
        warn(-1.0f);
      }
      break;
    case Model_Type::Hunter:
      hunter_position = another_model->get_position();
      observe_hunter = glm::normalize(hunter_position - this->position);
      warn(min(glm::dot(observe_hunter, this->Front), 0.0f));
      if (warning > warning_wander && animation_index == Pig_Idle) {
        float pig_angle = glm::atan(this->Front.z / this->Front.x);
        float flee_angle = -glm::atan(observe_hunter.z / observe_hunter.x);
        this->turn(flee_angle - pig_angle, 0.0f);
        animation_index = Pig_Wander;
      }
      else if (warning > warning_walk && (animation_index == Pig_Wander || animation_index == Pig_Idle)) {
        float pig_angle = glm::atan(this->Front.z / this->Front.x);
        float flee_angle = -glm::atan(observe_hunter.z / observe_hunter.x);
        this->turn(flee_angle - pig_angle, 0.0f);
        animation_index = Pig_Walk;
      }
      else if (warning == crazy) {
        float pig_angle = glm::atan(this->Front.z / this->Front.x);
        float flee_angle = -glm::atan(observe_hunter.z / observe_hunter.x);
        this->turn(flee_angle - pig_angle, 0.0f);
        animation_index = Pig_Jump;
      }
      break;
    case Model_Type::Bullet:
      warn(20.0f);
      break;
    }
    break;
  }
}

void HPig::collision_detection(HMap* _map) {
  vector<Model_Data> nearby;

  if (engine_detect_collision) {
    nearby = _map->get_model_nearby(this, 5.0f);

    for (int i = 0; i < nearby.size(); i++) {
      if (nearby[i]._adjust_pos)
        nearby[i]._model->SetPosition(*nearby[i]._adjust_pos);

      Collision* collision_type = get_collide_type(this, nearby[i]._model);
      this->Event(collision_type);
      nearby[i]._model->Event(collision_type);
      delete collision_type;
    }

    nearby.clear();
    nearby = _map->get_model_nearby(this, 5.0f);
    for (int i = 0; i < nearby.size(); i++) {
      if (nearby[i]._adjust_pos)
        nearby[i]._model->SetPosition(*nearby[i]._adjust_pos);

      Observation* observation_type = new Observation(this, nearby[i]._model);
      this->Event(observation_type);
      nearby[i]._model->Event(observation_type);
      delete observation_type;
    }

  }
}

void HPig::Forward(float deltaTime) {
  float MovementSpeed = Pig_Idle_Speed;
  switch (animation_index) {
  case Pig_Jump:
    MovementSpeed = Pig_Jump_Speed;
    break;
  case Pig_Wander:
    MovementSpeed = Pig_Wander_Speed;
    break;
  case Pig_Idle:
    MovementSpeed = Pig_Idle_Speed;
    break;
  case Pig_Die:
    MovementSpeed = Pig_Idle_Speed;
    break;
  }

  float velocity = MovementSpeed * deltaTime;
  last_position = position;
  position += Front * velocity;
}

void HPig::turn(float xoffset, float yoffset) {
  Yaw += xoffset;
  Pitch += yoffset;

  if (Pitch > 89.0f)
    Pitch = 89.0f;
  if (Pitch < -89.0f)
    Pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  front.y = sin(glm::radians(Pitch));
  front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

  Front = glm::normalize(front);
  Right = glm::normalize(glm::cross(Front, WorldUp));
  Up = glm::normalize(glm::cross(Right, Front));

  rotation.y -= xoffset;
}

void HPig::warn(float influence) {
  warning += alert * influence;
  if (warning > crazy)
    warning = crazy;
  if (warning < 0)
    warning = 0;
  cout << "current_warning: " << warning << endl;
}


