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

enum class Model_Type {
  Pig,
  Sheep,
  Hunter,
  Bullet,
  Tree,
  Unknown
};

class HModel;

class Collision : public Events {
public:
  Collision(bool is_collide, HModel* model_1, HModel* model_2, vector<int> model_1_meshes_index, vector<int> model_2_meshes_index): Events(Event_Type::Collision), _is_collide(is_collide), _model_1(model_1), _model_2(model_2), _model_1_meshes_index(model_1_meshes_index), _model_2_meshes_index(model_2_meshes_index) {}

  bool _is_collide;

  /* model_1 and model_2 are the 2 models detected collision */
  HModel* _model_1; 
  HModel* _model_2;

  /* mesh in model_1[i] collides with mesh in model_2[i] */
  vector<int> _model_1_meshes_index; // The collision meshes in model_1 
  vector<int> _model_2_meshes_index; // The collision meshes in model_2

};

class Observation : public Events {
public:
  HModel* _model_1;
  HModel* _model_2;


  Observation(HModel* model1, HModel* model2) : Events(Event_Type::Observation) {
    _model_1 = model1;
    _model_2 = model2;
  }
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

protected:
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
  unsigned int binding_point;
 
  const aiScene* scene;
  Assimp::Importer* importer;

  HShader* shader;

  HCamera* camera;

  int animation_index;
  float animation_ticks;

  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scaling;

  HCollider* collider;

  Model_Type model_type;

  bool engine_detect_collision;

  bool first_record = true;

  float speed;

  int map_x = -1;
  int map_y = -1;

public:
  float min_x, max_x;
  float min_y, max_y;
  float min_z, max_z;

  // constructor, expects a filepath to a 3D model.
  HModel(string const& path, bool gamma, int default_animation_index = INVALID_ANIMATION_INDEX);

  ~HModel();

  // update collider transformation matrix
  void UpdateColliderTransform();

  void UpdateBoneTransform();

  void AdjustStepOnGround(HMap* map);

  // draws the model, and thus all its meshes
  virtual void Draw();

  Model_Type get_model_type();

  void DrawBox(HShader* shader);

  void BindShader(HShader* model_shader);

  void BindCamera(HCamera* model_camera);

  void BindShaderUniformBuffer(int binding_point);

  virtual void Action(HMap* map, float duration_time);

  virtual void Event(Events* event);

  HShader* get_current_shader();

  virtual vector<glm::vec3> get_collider();

  vector<HMesh>& get_meshes();

  void SetPosition(const glm::vec3& position_vec);

  void SetRotation(const glm::vec3&);

  void SetScaling(const glm::vec3& scaling_vec);

  glm::vec3 get_position();

  glm::mat4 GetPositionMat();

  glm::mat4 GetRotationMat();

  glm::mat4 GetScalingMat();

  void set_need_detect_collision(bool flag);

  virtual void collision_detection(HMap* map);

protected:
  void genModelCollider();

  void genModelBuffer();

  void CalCurrentTicks(float duration_time);

  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void loadModel(string const& path);

  // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
  void processNode(aiNode* node);

  HMesh processMesh(aiMesh* mesh);

  vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);

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

  Collision* get_collide_type(HModel* model1, HModel* model2);

  bool if_collide(vector<glm::vec3> Points1, vector<glm::vec3> Points2);

  bool inspection_2D(glm::vec2* p1, glm::vec2* p2);
};

#endif
