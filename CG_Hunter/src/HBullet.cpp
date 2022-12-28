#include <HBullet.h>
#include <HMap.h>

HBullet::HBullet(string const& path, bool gamma): HModel(path, false) {
	model_type = Model_Type::Bullet;
}

void HBullet::Draw() {
  shader->use();
  string model_str = "model[";
  vector<glm::mat4> models(_bullets.size());
  for (int i = 0; i < models.size(); i++) {
    models[i] = glm::translate(glm::mat4(1.0f), _bullets[i]._current_position) * _bullets[i]._rotation * GetRotationMat() * GetScalingMat();
    shader->setMat4(model_str + to_string(i) + "]", models[i]);
  }

  unsigned int Matrix_index = glGetUniformBlockIndex(shader->ID, "Matrices");
  glUniformBlockBinding(shader->ID, Matrix_index, binding_point);

  glBindBuffer(GL_UNIFORM_BUFFER, matrix_buffer_id);
  unsigned int buffer_offset = 0;
  glm::mat4 identity(1.0f);
  // Bind projection and view
  glm::mat4   perspective = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
  glm::mat4 view = camera->GetViewMatrix();
  BindUniformData(buffer_offset, &perspective);
  BindUniformData(buffer_offset, &view);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);


  for (unsigned int k = 0; k < meshes.size(); k++) {
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    //cout << "Texture size: " << textures.size() << endl;
    // 
    //cout << "size: " << textures.size() << endl;
    for (unsigned int i = 0; i < meshes[k].textures.size(); i++)
    {
      // The defalut skybox id GL_TEXTRUE15 is left for skybox texture
      if (i == SKYBOX_ID)
        continue;
      glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
      // retrieve texture number (the N in diffuse_textureN)
      string number;
      string name = meshes[k].textures[i].type;
      if (name == "texture_diffuse")
        number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
        number = std::to_string(specularNr++); // transfer unsigned int to string
      else if (name == "texture_normal")
        number = std::to_string(normalNr++); // transfer unsigned int to string
      else if (name == "texture_height")
        number = std::to_string(heightNr++); // transfer unsigned int to string

      // now set the sampler to the correct texture unit
      glUniform1i(glGetUniformLocation(shader->ID, (name + number).c_str()), i);

      //cout << "texture_name: " << name + number << endl;
      // and finally bind the texture
      glBindTexture(GL_TEXTURE_2D, meshes[k].textures[i].id);
    }

    // draw mesh
    glBindVertexArray(meshes[k].VAO);
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(meshes[k].indices.size()), GL_UNSIGNED_INT, 0, _bullets.size());
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
  }
}

vector<glm::vec3> HBullet::get_collider() {
  vector<glm::vec3> ret_points;
  glm::vec3 _current_bullet_position = _bullets[_current_bullet]._current_position;
  glm::vec3 _last_bullet_position = _bullets[_current_bullet]._last_position;

  glm::vec3 world_up = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 right = glm::cross(_bullets[_current_bullet]._direction, world_up);
  right = glm::normalize(right);

  float rate = 0.001f;
  float offset_world_up[4] = {-1, -1, 1, 1};
  float offset_right[4] = { -1, 1, 1, -1 };
  for (int i = 0; i < 4; i++) 
    ret_points.emplace_back(_current_bullet_position + world_up * offset_world_up[i] * rate + right * offset_right[i] * rate);
  for (int i = 0; i < 4; i++)
    ret_points.emplace_back(_last_bullet_position + world_up * offset_world_up[i] * rate + right * offset_right[i] * rate);

  return ret_points;
}

void HBullet::collision_detection(HMap* _map) {
  vector<Model_Data> nearby;
  if (engine_detect_collision) {
    for (int i = 0; i < _bullets.size(); i++) {
      _current_bullet = i;
      SetPosition(_bullets[i]._current_position);
      nearby = _map->get_model_nearby(this, 8.0f);

      for (int j = 0; j < nearby.size(); j++) {
        if (nearby[j]._adjust_pos)
          nearby[j]._model->SetPosition(*nearby[j]._adjust_pos);

        Collision* collision_type = get_collide_type(this, nearby[j]._model);
        this->Event(collision_type);
        nearby[j]._model->Event(collision_type);
        delete collision_type;
        if (!_bullets[i]._exist)
          break;
      }
      nearby.clear();
      nearby = _map->get_model_nearby(this, 3.0f);

      for (int j = 0; j < nearby.size(); j++) {
        if (nearby[j]._adjust_pos)
          nearby[j]._model->SetPosition(*nearby[j]._adjust_pos);
        
        Observation* observation_type = new Observation(this, nearby[j]._model);
        this->Event(observation_type);
        nearby[j]._model->Event(observation_type);
        delete observation_type;
      }
      }
    }
 }

void HBullet::Action(HMap* map, float duration_time) {
  //cout << "_bullets size: " << _bullets.size() << endl;
  for (int i = 0; i < _bullets.size(); i++) {
    _bullets[i]._last_position = _bullets[i]._current_position;
    _bullets[i]._current_position += _bullets[i]._direction * _bullets[i]._speed;
    if (fabs(_bullets[i]._current_position.x) > map->get_map_width() / 2 || fabs(_bullets[i]._current_position.z) > map->get_map_height() / 2 || fabs(_bullets[i]._current_position.y) > 2000.0f || _bullets[i]._exist == false) {
      _bullets.erase(_bullets.begin() + i);
    }
  }
  //UpdateColliderTransform();
}

void HBullet::Event(Events* event) {
  switch (event->_event_type) {
  case Event_Type::Collision:
    Collision* collision_event = reinterpret_cast<Collision*>(event);
    HModel* another_model = collision_event->_model_1 == this ? collision_event->_model_2 : collision_event->_model_1;
    if (collision_event->_is_collide && another_model->get_model_type() != Model_Type::Hunter) {
      _bullets[_current_bullet]._exist = false;
    }
  }
}

void HBullet::insert_bullet(bullet insert_bullet) {
  _bullets.emplace_back(insert_bullet);
}


