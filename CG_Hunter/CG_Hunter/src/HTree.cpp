#include <HTree.h>

HTree::HTree(string const& path, bool gamma): HModel(path, gamma), num(0) {
  model_type = Model_Type::Tree;
}

void HTree::set_models(vector<glm::mat4> models) {
  this->models = models;
}

void HTree::Draw() {
  shader->use();

  string model_str = "model[";
  for (int j = 0; j < models.size(); j++) {
    shader->setMat4(model_str + to_string(j) + "]", models[j]);
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
    glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(meshes[k].indices.size()), GL_UNSIGNED_INT, 0, num);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
  }
}

void HTree::set_num(int count) {
  num = count;
}

vector<glm::vec3> HTree::get_collider() {
  vector<glm::vec3> ret_points;

  glm::mat4 shrink_collider = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 1.0f, 0.3f));

  glm::mat4 wvp = GetPositionMat() * shrink_collider * GetRotationMat() * GetScalingMat();

  for (int i = 0; i < 8; i++) {
    ret_points.emplace_back(glm_vec4_to_glm_vec3(wvp * glm::vec4(collider->get_Points(i), 1.0f)));
  }

  return ret_points;
}
