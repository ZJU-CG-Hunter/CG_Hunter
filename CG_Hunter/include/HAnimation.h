#pragma once

#include <def.h>

using namespace std;

struct Animation {
  int _seq_id                = 0; 
  int _animation_index       = INVALID_ANIMATION_INDEX;              // the id of the animation to define which animation it is
  int _animation_times       = DEAD_LOOP;                       // animation iterative times
  float _animation_duration  = INVALID_ANIMATION_DURATION;           // ticks per round  
  float _time_in_ticks       = 0;                                    // total ticks      
  float _ticks_per_second    = INVALID_ANIMATION_TICKS_PER_SECOND;   // ticks per second 
  Animation* _next_animation = nullptr;                              // next animation

  Animation(int seq_id, int animation_index, int animation_times, float animation_duration, \
    float time_in_ticks, float ticks_per_second, Animation* next_animation) {
    _seq_id = seq_id;
    _animation_index = animation_index;
    _animation_times = animation_times;
    _animation_duration = animation_duration;
    _time_in_ticks = time_in_ticks;
    _ticks_per_second = ticks_per_second;
    _next_animation = next_animation;
  }
};

class HAnimation {
private:
  Animation*  _start_animation;
  Animation*  _current_animation;
  int         _loop_num;
   

  void (*_animation_iteration_fun)(int) = nullptr;              
  void (*_animation_insert_fun)(int, bool, bool) = nullptr;

public:
  HAnimation();

  HAnimation(Animation* animation, int loop_num);

  void insert_animation_seq(HAnimation* hanimation, int insert_seq_id, int return_seq_id, int loop_num, bool is_protected, bool is_replace);

  void set_animation_iteration_callback(void (*animation_iteration_fun)(int));

  void set_animation_insert_callback(void (*animation_insert_fun)(int, bool, bool));

  void get_current_state(int& animation_index, float& animation_ticks);

  void run();
};