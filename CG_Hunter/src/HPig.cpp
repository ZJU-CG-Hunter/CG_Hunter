#include <HPig.h>
#include <HMap.h>

HPig::HPig(string const& path, bool gamma): HModel(path, gamma, Idle), _is_alive(true) {
	model_type = Model_Type::Pig;
}

void HPig::Action(HMap* map, float duration_time) {
  switch (animation_index) {
  case INVALID_ANIMATION_INDEX:
    cout << "Invalid_animation_index" << endl;
    assert(0);
  case Walk:
  case Jump:
  case Wander:
  case Idle:
    CalCurrentTicks(duration_time);
    // Iterative do this
    animation_ticks = fmod(animation_ticks, scene->mAnimations[animation_index]->mDuration);
    break;
  case Die:
    if (_is_alive) {
      CalCurrentTicks(duration_time);
      if (animation_ticks > scene->mAnimations[animation_index]->mDuration) {
        animation_ticks = scene->mAnimations[animation_index]->mDuration - 0.01f;
        _is_alive = false;
      }
    }
    break;
  }
  UpdateBoneTransform();
  //UpdateColliderTransform();
  AdjustStepOnGround(map);
  map->update_model(this);
}

void HPig::Event(Events* event) {
  switch (event->_event_type) {
  case Event_Type::Collision:
    Collision* collision_event = reinterpret_cast<Collision*>(event);
    HModel* another_model = collision_event->_model_1 == this ? collision_event->_model_2 : collision_event->_model_1;
    if(another_model->get_model_type() == Model_Type::Bullet)
      animation_index = Die;
    break;
  }
}

void HPig::collision_detection(HMap* _map) {
  ;
}

