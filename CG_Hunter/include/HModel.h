#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <STB/stb_image.h>
#include <ASSIMP/Importer.hpp>
#include <ASSIMP/scene.h>
#include <ASSIMP/postprocess.h>
#include <glm/gtc/quaternion.hpp>

#include <HMesh.h>
#include <HShader.h>
#include <HCamera.h>
#include <utility.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

enum class Event_Type {
  Collision,
  Unknown
};

enum class Model_Type {
  Pig,
  Deer,
  Hunter,
  Arrow,
  Unknown
};

struct BoneData {
  glm::mat4 bone_offset;
  glm::mat4 bone_transform;

  BoneData(glm::mat4 offset) {
    bone_offset = offset;
    bone_transform = glm::mat4(1.0f);
  }
  BoneData() {
    bone_offset = glm::mat4(1.0f);
    bone_transform = glm::mat4(1.0f);
  }

};

class HModel
{
friend class HMap;

private:
  // model data 
  vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
  vector<HMesh>   meshes; 
  string          directory;
  bool gammaCorrection;

  map<string, unsigned int> bone_map;
  vector<map<string, unsigned int>> channels_map;
  vector<BoneData> bones;
  glm::mat4 inverse_root_matrix;
  unsigned int matrix_buffer_id;
 
  const aiScene* scene;
  Assimp::Importer* importer;

  int animation_index;
  float animation_ticks;

  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scaling;

  float model_width;
  float model_length;

  Model_Type model_type;

public:
  // constructor, expects a filepath to a 3D model.
  HModel(string const& path, bool gamma, float width, float length);

  ~HModel();
  // draws the model, and thus all its meshes
  void Draw(HShader* shader, HCamera* camera);

  virtual void Action(HMap* map, float duration_time);

  virtual void Event(Event_Type event_type, HModel* another_model);

  void BindShader(int binding_point);

  void SetPosition(const glm::vec3& position_vec);

  void SetRotation(const glm::quat& rotation_quat);

  void SetScaling(const glm::vec3& scaling_vec);

private:

  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void loadModel(string const& path);

  // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
  void processNode(aiNode* node);

  HMesh processMesh(aiMesh* mesh);

  vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

  void setBoneTransform_ini(HShader* shader);

  void setBoneTransform_recursive(const aiNode* current_node, const glm::mat4 parent_transform);

  const aiNodeAnim* findNodeAnim(string node_name, int animation_index);

  void CalcInterpolatedScaling(aiVector3D& Scaling, float animation_time, const aiNodeAnim* current_node_anim);

  void CalcInterpolatedRotation(aiQuaternion& quaterntion, float animation_time, const aiNodeAnim* current_node_anim);

  void CalcInterpolatedPosition(aiVector3D& Translation, float animation_time, const aiNodeAnim* current_node_anim);

  unsigned int findScaling(float animation_time, const aiNodeAnim* current_node_anim);

  unsigned int findRotation(float animation_time, const aiNodeAnim* current_node_anim);

  unsigned int findtranslation(float animation_time, const aiNodeAnim* current_node_anim);


  void Interpolate(aiVector3D& inter, const aiVector3D& start, const aiVector3D& end, float factor);

  void Interpolate(aiQuaternion& inter, const aiQuaternion& start, const aiQuaternion& end, float factor);

  glm::mat4 aimat_to_glmmat(aiMatrix4x4 ai_matrix);

  glm::mat4 aimat_to_glmmat(aiMatrix3x3 ai_matrix);

  unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);
};

#endif
