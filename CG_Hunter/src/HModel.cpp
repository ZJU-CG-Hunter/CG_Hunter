#include <HModel.h>
#include <HMap.h>

// constructor, expects a filepath to a 3D model.
HModel::HModel(string const& path, bool gamma) : gammaCorrection(gamma)
{
  genModelBuffer();
  loadModel(path);
  genModelCollider();
}

HModel::~HModel() {
  textures_loaded.clear();
  meshes.clear();
  channels_map.clear();
  bones.clear();
  delete scene;
}
// draws the model, and thus all its meshes
void HModel::Draw()
{
  shader->use();

  unsigned int Matrix_index = glGetUniformBlockIndex(shader->ID, "Matrices");
  glUniformBlockBinding(shader->ID, Matrix_index, binding_point);

  glBindBuffer(GL_UNIFORM_BUFFER, matrix_buffer_id);
  unsigned int buffer_offset = 0;

  // Bind WVP
  glm::mat4 identity(1.0f);
  glm::mat4 model = glm::translate(identity, position) * glm::mat4_cast(rotation) * glm::scale(identity, scaling);
  BindUniformData(buffer_offset, &model);

  // Bind projection and view
  glm::mat4 perspective = glm::perspective(glm::radians(camera->Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
  glm::mat4 view = camera->GetViewMatrix();
  BindUniformData(buffer_offset, &perspective);
  BindUniformData(buffer_offset, &view);

  // Bind bones
  for (unsigned int i = 0; i < bone_map.size(); i++)
    BindUniformData(buffer_offset, &bones[i].bone_transform);

  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  for (unsigned int i = 0; i < meshes.size(); i++) {
    meshes[i].Draw(shader);
  }


}

void HModel::DrawBox(HShader* shader) {
  
}

HShader* HModel::get_current_shader() {
  return shader;
}


void HModel::UpdateColliderTransform() {
  glm::mat4 identity(1.0f);
  glm::mat4 model = glm::translate(identity, position) * glm::mat4_cast(rotation) * glm::scale(identity, scaling);

  for (int i = 0; i < meshes.size(); i++) {
    meshes[i].mesh_transform_mat = glm::mat4(0.0f);
    for (int j = 0; j < meshes[i].mesh_bone.size(); j++) {
      meshes[i].mesh_transform_mat += bones[meshes[i].mesh_bone[j].bone_index].bone_transform * meshes[i].mesh_bone[j].weights;
    }
    meshes[i].mesh_transform_mat *= model;
  }
}

void HModel::Action(HMap* map, float duration_time) {
  if(scene->mNumAnimations >0){
    animation_index = 0;
    CalCurrentTicks(duration_time);
  }

  UpdateBoneTransform();
  UpdateColliderTransform();
  AdjustStepOnGround(map);
  
}

void HModel::Event(Events event) {
  //cout << "Not implemented, always ignore" << endl;
}

void HModel::UpdateBoneTransform() {
  /* Update the bone matrix */
  setBoneTransform_ini(shader);
}

void HModel::AdjustStepOnGround(HMap* map) {
  /* Always step on the ground */
      
  position.y = map->get_height(position.x, position.z) + Y_OFFSET;

  //cout << "Height: " << position.y << endl;
}

void HModel::CalCurrentTicks(float duration_time) {
  float animation_duration = scene->mAnimations[animation_index]->mDuration;
  float animation_ticks_per_second = scene->mAnimations[animation_index]->mTicksPerSecond;
  animation_ticks = fmod((animation_ticks + duration_time * animation_ticks_per_second), animation_duration);
}



void HModel::BindShaderUniformBuffer(int binding_point) {
  this->binding_point = binding_point;
  glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, matrix_buffer_id);
}

void HModel::SetPosition(const glm::vec3& position_vec) {
  position = position_vec;
}

void HModel::SetRotation(const glm::quat& rotation_quat) {
  rotation = rotation_quat;
}

void HModel::SetScaling(const glm::vec3& scaling_vec) {
  scaling = scaling_vec;
}


glm::mat4 HModel::GetPositionMat() {
  glm::mat4 identity(1.0f);
  return glm::translate(identity, position);
}

glm::mat4 HModel::GetRotationMat() {
  return glm::mat4_cast(rotation);
}

glm::mat4 HModel::GetScalingMat() {
  glm::mat4 identity(1.0f);
  return glm::scale(identity, scaling);
}

bool HModel::is_need_detect_collision() {
  return engine_detect_collision;
}

void HModel::set_need_detect_collision(bool flag) {
  engine_detect_collision = flag;
}

  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
void HModel::loadModel(string const& path)
{
  importer = new Assimp::Importer();
  // read file via ASSIMP
  scene = importer->ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

  // used for debug
  cout << "Number of animations: " << scene->mNumAnimations << endl;
  cout << "Scene Pointer: " << scene << endl;

  // check for errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
  {
    cout << "ERROR::ASSIMP:: " << importer->GetErrorString() << endl;
    return;
  }

  //cout << "Test:" << scene->mAnimations[0]->mTicksPerSecond;

  // retrieve the directory path of the filepath
  directory = path.substr(0, path.find_last_of('/'));

  inverse_root_matrix = aimat_to_glmmat(scene->mRootNode->mTransformation.Inverse());

  bones.resize(MAX_BONES);
  channels_map.resize(MAX_ANIMATIONS);

  // process ASSIMP's root node recursively
  processNode(scene->mRootNode);
  cout << "In loadModel() Animations num: " << scene->mNumAnimations << endl;
}

void HModel::BindShader(HShader* model_shader) {
  shader = model_shader;
}

void HModel::BindCamera(HCamera* model_camera) {
  camera = model_camera;
}

void HModel::genModelCollider() {
  vector<vector<float>> v;

  for (int i = 0; i < meshes.size(); i++) {
    for (int j = 0; j < meshes[i].vertices.size(); j++) {
      vector<float> temp;
      temp.push_back(meshes[i].vertices[j].Position.x);
      temp.push_back(meshes[i].vertices[j].Position.y);
      temp.push_back(meshes[i].vertices[j].Position.z);

      v.push_back(temp);
    }
  }

  collider = new HCollider(v);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void HModel::processNode(aiNode* node)
{
  cout << "Number of Mesh: " << node->mNumMeshes << endl;
  // process each mesh located at the current node
  for (unsigned int i = 0; i < node->mNumMeshes; i++)
  {
    // the node object only contains indices to index the actual objects in the scene. 
    // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
    cout << "current_mesh: " << i << endl;


    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh));
    meshes[meshes.size() - 1].ini_trans = aimat_to_glmmat(node->mTransformation);
    //meshes.push_back(processMesh(mesh, scene));
  }
  // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
  
  cout << "Number of children: " << node->mNumChildren << endl;
  for (unsigned int i = 0; i < node->mNumChildren; i++)
  {
    cout << "Current child: " << i << endl;
    processNode(node->mChildren[i]);
  }
}

HMesh HModel::processMesh(aiMesh* mesh)
{
  string mesh_name(mesh->mName.C_Str());

  // data to fill
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vector<Texture> textures;
  HCollider* mesh_ini_collider;
  vector<MeshBone> mesh_bone;
  float weight_sum = 0.0f;

  // walk through each of the mesh's vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++)
  {
    Vertex vertex;
    glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
    // positions
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;

    if (first_record) {
      min_x = max_x = vector.x;
      min_y = max_y = vector.y;
      min_z = max_z = vector.z;
      first_record = false;
    }
    else {
      min_x = min(min_x, vector.x);
      max_x = max(max_x, vector.x);
      min_y = min(min_y, vector.y);
      max_y = max(max_y, vector.y);
      min_z = min(min_z, vector.z);
      max_z = max(max_z, vector.z);
    }

    vertex.Position = vector;
    // normals
    if (mesh->HasNormals())
    {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.Normal = vector;
    }
    // texture coordinates
    if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
    {
      glm::vec2 vec;
      // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
      // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;

      //cout << "texcoord: " << vec.x << ", " << vec.y << endl;

      vertex.TexCoords = vec;
      // tangent
      vector.x = mesh->mTangents[i].x;
      vector.y = mesh->mTangents[i].y;
      vector.z = mesh->mTangents[i].z;
      vertex.Tangent = vector;
      // bitangent
      vector.x = mesh->mBitangents[i].x;
      vector.y = mesh->mBitangents[i].y;
      vector.z = mesh->mBitangents[i].z;
      vertex.Bitangent = vector;
    }
    else
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);

    vertices.push_back(vertex);
  }

  // generate the collider
  vector<vector<float>> v;
  for (int i = 0; i < vertices.size(); i++) {
    vector<float> temp;
    temp.push_back(vertices[i].Position.x);
    temp.push_back(vertices[i].Position.y);
    temp.push_back(vertices[i].Position.z);

    v.push_back(temp);
  }
  mesh_ini_collider = new HCollider(v);


  for (unsigned int i = 0; i < mesh->mNumBones; i++) {

    unsigned int bone_index = 0;
    string bname(mesh->mBones[i]->mName.data);

    if (bone_map.find(bname) == bone_map.end()) {
      bone_index = bone_map.size();
      bone_map.emplace(bname, bone_index);
      for (unsigned int j = 0; j < scene->mNumAnimations; j++) {
        for (unsigned int k = 0; k < scene->mAnimations[j]->mNumChannels; k++) {
          if (bname == scene->mAnimations[j]->mChannels[k]->mNodeName.data)
            channels_map[j].emplace(bname, k);
        }      }
    }
    else
      bone_index = bone_map[bname];
    mesh_bone.emplace_back(MeshBone(bone_index, 0.0f));

    //cout << "bone_index: " << bone_index << endl;

    // Check if size is big enough
    if (bone_index >= bones.size())
      bones.resize(bones.size() * 2);

    bones.at(bone_index) = BoneData(aimat_to_glmmat(mesh->mBones[i]->mOffsetMatrix));


    //cout << "vectices_size: " << vertices.size() << endl;
    //cout << "number of weights: " << mesh->mBones[i]->mNumWeights << endl;
    for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
      int vertex_index = mesh->mBones[i]->mWeights[j].mVertexId;
      //cout << "vertex_index: " << vertex_index << endl;
      if (vertex_index < 0 || vertex_index >= vertices.size())
        cout << "vertex_index: " << vertex_index << endl;

      float bone_weight = mesh->mBones[i]->mWeights[j].mWeight;
      vertices[vertex_index].addBone(bone_index, bone_weight);
      mesh_bone[i].weights += bone_weight;
      weight_sum += bone_weight;
    }
  }
  for (int i = 0; i < mesh_bone.size(); i++)
    mesh_bone[i].weights /= weight_sum;
  
  // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
  for (unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace face = mesh->mFaces[i];
    // retrieve all indices of the face and store them in the indices vector
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  // process materials
  aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
  // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
  // Same applies to other texture as the following list summarizes:
  // diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // 1. diffuse maps
  vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
  textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

  // return a mesh object created from the extracted mesh data
  return HMesh(vertices, indices, textures, mesh_ini_collider, mesh_bone, mesh_name);
}

HCollider* HModel::get_collider() {
  return collider;
}

vector<HMesh>& HModel::get_meshes() {
  return meshes;
}


  // checks all material textures of a given type and loads the textures if they're not loaded yet.
  // the required info is returned as a Texture struct.
vector<Texture> HModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
  vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
  {
    aiString str;
    mat->GetTexture(type, i, &str);
    // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture

    cout << "texturestr:" << str.C_Str() << endl;

    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++)
    {
      if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
      {
        textures.push_back(textures_loaded[j]);
        skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
        break;
      }
    }
    if (!skip)
    {   // if texture hasn't been loaded already, load it
      Texture texture;
      texture.id = TextureFromFile(str.C_Str(), this->directory);
      texture.type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
    }
  }
  return textures;
}

void HModel::setBoneTransform_ini(HShader* shader) {
  if (animation_index == INVALID_ANIMATION_INDEX || animation_index >= scene->mNumAnimations)
    return;

  // used for debug 
  //cout << "In draw() Animations num: " << scene->mNumAnimations << endl;
  //cout << "In draw() Scene Pointer: " << scene << endl

  glm::mat4 identity(1.0f);
  // used for debug
  // cout << "animation_time: " << animation_time << endl;
  setBoneTransform_recursive(scene->mRootNode, identity);

  return;
}

void HModel::setBoneTransform_recursive(const aiNode* current_node, const glm::mat4 parent_transform) {
  string node_name(current_node->mName.data);
  glm::mat4 node_transform(aimat_to_glmmat(current_node->mTransformation));

  const aiNodeAnim* current_node_anim = findNodeAnim(node_name, animation_index);

  if (current_node_anim) {
    // Interpolate scaling and generate scaling transformation matrix
    aiVector3D Scaling;
    CalcInterpolatedScaling(Scaling, animation_ticks, current_node_anim);
    glm::mat4 ScalingM(1.0f);
    ScalingM = glm::scale(ScalingM, glm::vec3(Scaling.x, Scaling.y, Scaling.z));
    // debug
      //show_mat4(ScalingM, "ScalingM: ");

    // Interpolate rotation and generate rotation transformation matrix
    aiQuaternion Rotation;
    CalcInterpolatedRotation(Rotation, animation_ticks, current_node_anim);
    glm::mat4 RotationM(aimat_to_glmmat(Rotation.GetMatrix()));
    // debug
    //show_mat4(RotationM, "RotationM: ");

    // Interpolate translation and generate translation transformation matrix
    aiVector3D Translation;
    CalcInterpolatedPosition(Translation, animation_ticks, current_node_anim);
    glm::mat4 TranslationM(1.0f);
    //cout << Translation.x << ":::" << Translation.y << ":::" << Translation.z << endl;
    TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));
    // debug
    //show_mat4(TranslationM, "TranslationM: ");

    node_transform = TranslationM * RotationM * ScalingM;
    // debug
      //show_mat4(node_transform, "Node_Transform: ");
  }

  glm::mat4 GlobalTransformation = parent_transform * node_transform;
  // debug
    //show_mat4(GlobalTransformation, "GlobalTransformation: ");

  if (bone_map.find(node_name) != bone_map.end()) {
    unsigned int bone_index = bone_map[node_name];
    bones[bone_index].bone_transform = inverse_root_matrix * GlobalTransformation * bones[bone_index].bone_offset;
    //show_mat4(bones[bone_index].bone_offset, "bones[bone_index].bone_offset: ");
    //show_mat4(bones[bone_index].bone_transform, "bones[bone_index].bone_transform: ");

  }

  for (unsigned int i = 0; i < current_node->mNumChildren; i++)
    setBoneTransform_recursive(current_node->mChildren[i], GlobalTransformation);
}

const aiNodeAnim* HModel::findNodeAnim(string node_name, int animation_index) {
  if (channels_map[animation_index].find(node_name) == channels_map[animation_index].end())
    return nullptr;

  return scene->mAnimations[animation_index]->mChannels[channels_map[animation_index][node_name]];
}

void HModel::CalcInterpolatedScaling(aiVector3D& Scaling, float animation_time, const aiNodeAnim* current_node_anim) {
  if (current_node_anim->mNumScalingKeys == 1) {
    Scaling = current_node_anim->mScalingKeys[0].mValue;
    return;
  }

  unsigned int scaling_index = findScaling(animation_time, current_node_anim);

  unsigned int next_scaling_index = (scaling_index == current_node_anim->mNumScalingKeys - 1) ? scaling_index : scaling_index + 1;

  float delta_time = current_node_anim->mScalingKeys[next_scaling_index].mTime - current_node_anim->mScalingKeys[scaling_index].mTime;
  float factor = next_scaling_index == scaling_index ? 0 : (animation_time - current_node_anim->mScalingKeys[scaling_index].mTime) / delta_time;
  assert(factor >= 0.0f && factor <= 1.0f);
  const aiVector3D start_scaling(current_node_anim->mScalingKeys[scaling_index].mValue);
  const aiVector3D end_scaling(current_node_anim->mScalingKeys[next_scaling_index].mValue);

  Interpolate(Scaling, start_scaling, end_scaling, factor);
}

void HModel::CalcInterpolatedRotation(aiQuaternion& quaterntion, float animation_time, const aiNodeAnim* current_node_anim) {
  if (current_node_anim->mNumRotationKeys == 1) {
    quaterntion = current_node_anim->mRotationKeys[0].mValue;
    return;
  }

  unsigned int rotation_index = findRotation(animation_time, current_node_anim);
  unsigned int next_rotation_index = (rotation_index == current_node_anim->mNumRotationKeys - 1) ? rotation_index : rotation_index + 1;
  float delta_time = current_node_anim->mRotationKeys[next_rotation_index].mTime - current_node_anim->mRotationKeys[rotation_index].mTime;
  float factor = next_rotation_index == rotation_index ? 0 : (animation_time - current_node_anim->mRotationKeys[rotation_index].mTime) / delta_time;
  assert(factor >= 0.0f && factor <= 1.0f);
  const aiQuaternion start_rotation(current_node_anim->mRotationKeys[rotation_index].mValue);
  const aiQuaternion end_rotation(current_node_anim->mRotationKeys[next_rotation_index].mValue);

  Interpolate(quaterntion, start_rotation, end_rotation, factor);
  quaterntion = quaterntion.Normalize();
}

void HModel::CalcInterpolatedPosition(aiVector3D& Translation, float animation_time, const aiNodeAnim* current_node_anim) {
  if (current_node_anim->mNumPositionKeys == 1) {
    Translation = current_node_anim->mPositionKeys[0].mValue;
    return;
  }

  unsigned int translation_index = findtranslation(animation_time, current_node_anim);
  unsigned int next_translation_index = translation_index == current_node_anim->mNumPositionKeys - 1 ? translation_index : translation_index + 1;

  float delta_time = current_node_anim->mPositionKeys[next_translation_index].mTime - current_node_anim->mPositionKeys[translation_index].mTime;
  float factor = next_translation_index == translation_index ? 0 : (animation_time - current_node_anim->mPositionKeys[translation_index].mTime) / delta_time;
  assert(factor >= 0.0f && factor <= 1.0f);
  const aiVector3D start_translation(current_node_anim->mPositionKeys[translation_index].mValue);
  const aiVector3D end_translation(current_node_anim->mPositionKeys[next_translation_index].mValue);
  // debug
  //cout << "start_translation: " << current_node_anim->mPositionKeys[translation_index].mValue.x << ", " << current_node_anim->mPositionKeys[translation_index].mValue.y << ", " << current_node_anim->mPositionKeys[translation_index].mValue.z << endl;
  //cout << "end_translation: " << current_node_anim->mPositionKeys[next_translation_index].mValue.x << ", " << current_node_anim->mPositionKeys[next_translation_index].mValue.y << ", " << current_node_anim->mPositionKeys[next_translation_index].mValue.z << endl;

  Interpolate(Translation, start_translation, end_translation, factor);
  //Interpolate(Translation, aiVector3D(0.0f), end_translation - start_translation, factor);
}

unsigned int HModel::findScaling(float animation_time, const aiNodeAnim* current_node_anim) {
  if (current_node_anim->mNumScalingKeys <= 0) {
    cout << "ERROR::FINDSCALING:: Number of Scaling Keys <= 0" << endl;
    assert(0);
  }

  for (unsigned int i = 0; i < current_node_anim->mNumScalingKeys - 1; i++)
    if (animation_time < (float)current_node_anim->mScalingKeys[i + 1].mTime)
      return i;

  return current_node_anim->mNumScalingKeys - 1;
}

unsigned int HModel::findRotation(float animation_time, const aiNodeAnim* current_node_anim) {
  if (current_node_anim->mNumRotationKeys <= 0) {
    cout << "ERROR::FINDROTATION:: NUmber of Rotation Keys <= 0 " << endl;
    assert(0);
  }

  for (unsigned int i = 0; i < current_node_anim->mNumRotationKeys - 1; i++)
    if (animation_time < (float)current_node_anim->mRotationKeys[i + 1].mTime)
      return i;

  return current_node_anim->mNumRotationKeys - 1;
}

unsigned int HModel::findtranslation(float animation_time, const aiNodeAnim* current_node_anim) {
  if (current_node_anim->mNumPositionKeys <= 0) {
    cout << "ERROR::FINDROTATION:: NUmber of Rotation Keys <= 0 " << endl;
    assert(0);
  }

  for (unsigned int i = 0; i < current_node_anim->mNumPositionKeys - 1; i++)
    if (animation_time < (float)current_node_anim->mPositionKeys[i + 1].mTime)
      return i;

  return current_node_anim->mNumPositionKeys - 1;
}


void HModel::Interpolate(aiVector3D& inter, const aiVector3D& start, const aiVector3D& end, float factor) {
  inter.x = start.x + (end.x - start.x) * factor;
  inter.y = start.y + (end.y - start.y) * factor;
  inter.z = start.z + (end.z - start.z) * factor;
  return;
}

void HModel::Interpolate(aiQuaternion& inter, const aiQuaternion& start, const aiQuaternion& end, float factor) {
  aiQuaternion::Interpolate(inter, start, end, factor);
}

glm::mat4 HModel::aimat_to_glmmat(aiMatrix4x4 ai_matrix) {
  glm::vec4 col1(ai_matrix.a1, ai_matrix.b1, ai_matrix.c1, ai_matrix.d1);
  glm::vec4 col2(ai_matrix.a2, ai_matrix.b2, ai_matrix.c2, ai_matrix.d2);
  glm::vec4 col3(ai_matrix.a3, ai_matrix.b3, ai_matrix.c3, ai_matrix.d3);
  glm::vec4 col4(ai_matrix.a4, ai_matrix.b4, ai_matrix.c4, ai_matrix.d4);

  glm::mat4 ret(col1, col2, col3, col4);
  //show_mat4(ret, "Ret_Matrix");
  return ret;
}

glm::mat4 HModel::aimat_to_glmmat(aiMatrix3x3 ai_matrix) {
  glm::vec4 row1(ai_matrix.a1, ai_matrix.b1, ai_matrix.c1, 0.0f);
  glm::vec4 row2(ai_matrix.a2, ai_matrix.b2, ai_matrix.c2, 0.0f);
  glm::vec4 row3(ai_matrix.a3, ai_matrix.b3, ai_matrix.c3, 0.0f);
  glm::vec4 row4(0.0f, 0.0f, 0.0f, 1.0f);

  glm::mat4 ret(row1, row2, row3, row4);
  return ret;
}

unsigned int HModel::TextureFromFile(const char* path, const string& directory, bool gamma)
{
  cout << "path:" << path << endl;

  string filename = string(path);
  filename = directory + '/' + filename;

  cout << "filename: " << filename << endl;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

void HModel::genModelBuffer() {
  glGenBuffers(1, &matrix_buffer_id);
  glBindBuffer(GL_UNIFORM_BUFFER, matrix_buffer_id);
  glBufferData(GL_UNIFORM_BUFFER, MATRIX_UNIFROM_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}



