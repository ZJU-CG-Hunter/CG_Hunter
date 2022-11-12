#pragma once

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

#include <HMesh.h>
#include <HShader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

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
private:
  // model data 
  vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
  vector<HMesh>   meshes; 
  string directory;
  bool gammaCorrection;
  map<string, unsigned int> bone_map;
  vector<BoneData> bones;
  glm::mat4 inverse_root_matrix;
  const aiScene* scene;
  int animation_index = -1;


public:
  // constructor, expects a filepath to a 3D model.
  HModel(string const& path, bool gamma = false) : gammaCorrection(gamma)
  {
    loadModel(path);
  }

  // draws the model, and thus all its meshes
  void Draw(HShader& shader)
  {
    setBoneTransform_ini(static_cast<float>(glfwGetTime()));
 
    string uniform_bone("bone[]");
    for (int i = 0; i < bones.size(); i++) {

      uniform_bone = uniform_bone.insert(5, to_string(i));
      shader.setMat4(uniform_bone, bones[i].bone_transform);
    }

    for (unsigned int i = 0; i < meshes.size(); i++)
      meshes[i].Draw(shader);
  }

  void SetAnimation(int aidx) {
    animation_index = aidx;
    return;
  }


private:

  // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
  void loadModel(string const& path)
  {
    // read file via ASSIMP
    Assimp::Importer importer;
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
      cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
      return;
    }
    
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    inverse_root_matrix = aimat_to_glmmat(scene->mRootNode->mTransformation.Inverse());
    bones.resize(scene->mNumMeshes);

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode);
  }

  // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
  void processNode(aiNode* node)
  {
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      // the node object only contains indices to index the actual objects in the scene. 
      // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      meshes.push_back(processMesh(mesh));
      //meshes.push_back(processMesh(mesh, scene));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
      processNode(node->mChildren[i]);
    }
  }

  HMesh processMesh(aiMesh* mesh)
  {
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    vector<BoneData> bones;


    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      Vertex vertex;
      glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
      // positions
      vector.x = mesh->mVertices[i].x;
      vector.y = mesh->mVertices[i].y;
      vector.z = mesh->mVertices[i].z;
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

    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
      unsigned int bone_index = 0;
      string bname(mesh->mBones[i]->mName.data);

      if (bone_map.find(bname) == bone_map.end()) {
        bone_index = bone_map.size();
        bone_map.emplace(bname, bone_index);
      }
      else
        bone_index = bone_map[bname];

      bones.at(bone_index) = BoneData(aimat_to_glmmat(mesh->mBones[i]->mOffsetMatrix));

      for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
        int vertex_index = mesh->mBones[i]->mWeights[j].mVertexId;
        float bone_weight = mesh->mBones[i]->mWeights[j].mWeight;
        vertices[vertex_index].addBone(bone_index, bone_weight);
      }
    }

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
    return HMesh(vertices, indices, textures);
  }

  // checks all material textures of a given type and loads the textures if they're not loaded yet.
  // the required info is returned as a Texture struct.
  vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
  {
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
      aiString str;
      mat->GetTexture(type, i, &str);
      // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
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

  void setBoneTransform_ini(float time_in_seconds) {
    if (animation_index < 0)
      return;

    if (animation_index >= scene->mNumAnimations) {
      cout << "ERROR::SETBONETRANSFORM:: The index: " << animation_index << " is splited.(Number of animations: " << scene->mNumAnimations << ")" << endl;
      return;
    }
    float ticks_per_second = (scene->mAnimations[animation_index]->mTicksPerSecond != 0) ? scene->mAnimations[animation_index]->mTicksPerSecond : 25.0f;
    float time_in_ticks = time_in_seconds * ticks_per_second;
    float animation_time = fmod(time_in_ticks, scene->mAnimations[animation_index]->mDuration);

    glm::mat4 identity(1.0f);
    setBoneTransform_recursive(animation_time, scene->mRootNode, identity);
    
    return;
  }

  void setBoneTransform_recursive(float animation_time, const aiNode* current_node, const glm::mat4 parent_transform) {
    string node_name(current_node->mName.data);
    const aiAnimation* current_animation = scene->mAnimations[animation_index];
    glm::mat4 node_transform(aimat_to_glmmat(current_node->mTransformation));

    const aiNodeAnim* current_node_anim = findNodeAnim(current_animation, node_name);

    if (current_node_anim) {
      // Interpolate scaling and generate scaling transformation matrix
      aiVector3D Scaling;
      CalcInterpolatedScaling(Scaling, animation_time, current_node_anim);
      glm::mat4 ScalingM(1.0f);
      ScalingM = glm::scale(ScalingM, glm::vec3(Scaling.x, Scaling.y, Scaling.z));
      
      // Interpolate rotation and generate rotation transformation matrix
      aiQuaternion Rotation;
      CalcInterpolatedRotation(Rotation, animation_time, current_node_anim);
      glm::mat4 RotationM(aimat_to_glmmat(Rotation.GetMatrix()));

      // Interpolate translation and generate translation transformation matrix
      aiVector3D Translation;
      CalcInterpolatedPosition(Translation, animation_time, current_node_anim);
      glm::mat4 TranslationM(1.0f);
      TranslationM = glm::translate(TranslationM, glm::vec3(Translation.x, Translation.y, Translation.z));

      node_transform = TranslationM * RotationM * ScalingM;
    }

    glm::mat4 GlobalTransformation = parent_transform * node_transform;

    if (bone_map.find(node_name) != bone_map.end()) {
      unsigned int bone_index = bone_map[node_name];
      bones[bone_index].bone_transform = inverse_root_matrix * GlobalTransformation * bones[bone_index].bone_offset;
    }

    for (unsigned int i = 0; i < current_node->mNumChildren; i++)
      setBoneTransform_recursive(animation_time, current_node, GlobalTransformation);
  }

  const aiNodeAnim* findNodeAnim(const aiAnimation* current_animation, string node_name) {
    for (int i = 0; i < current_animation->mNumChannels; i++) {
      if (node_name == current_animation->mChannels[i]->mNodeName.data)
        return current_animation->mChannels[i];
    }
    cout << "ERROR::FINDNODEANIM:: Failed to find node: " << node_name << endl;
    return nullptr;
  }

  void CalcInterpolatedScaling(aiVector3D& Scaling, float animation_time, const aiNodeAnim* current_node_anim) {
    if (current_node_anim->mNumScalingKeys == 1) {
      Scaling = current_node_anim->mScalingKeys[0].mValue;
      return;
    }

    unsigned int scaling_index = findScaling(animation_time, current_node_anim);
    unsigned int next_scaling_index = scaling_index + 1;

    float delta_time = current_node_anim->mScalingKeys[next_scaling_index].mTime - current_node_anim->mScalingKeys[scaling_index].mTime;
    float factor = (animation_time - current_node_anim->mScalingKeys[scaling_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    const aiVector3D start_scaling(current_node_anim->mScalingKeys[scaling_index].mValue);
    const aiVector3D end_scaling(current_node_anim->mScalingKeys[next_scaling_index].mValue);
    
    Interpolate(Scaling, start_scaling, end_scaling, factor);
    Scaling = Scaling.Normalize();
  }

  void CalcInterpolatedRotation(aiQuaternion& quaterntion, float animation_time, const aiNodeAnim* current_node_anim) {
    if (current_node_anim->mNumRotationKeys == 1) {
      quaterntion = current_node_anim->mRotationKeys[0].mValue;
      return;
    }

    unsigned int rotation_index = findRotation(animation_time, current_node_anim);
    unsigned int next_rotation_index = rotation_index + 1;

    float delta_time = current_node_anim->mRotationKeys[next_rotation_index].mTime - current_node_anim->mRotationKeys[rotation_index].mTime;
    float factor = (animation_time - current_node_anim->mRotationKeys[rotation_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    const aiQuaternion start_rotation(current_node_anim->mRotationKeys[rotation_index].mValue);
    const aiQuaternion end_rotation(current_node_anim->mRotationKeys[next_rotation_index].mValue);

    Interpolate(quaterntion, start_rotation, end_rotation, factor);
    quaterntion = quaterntion.Normalize();
  }

  void CalcInterpolatedPosition(aiVector3D& Translation, float animation_time, const aiNodeAnim* current_node_anim) {
    if (current_node_anim->mNumPositionKeys == 1) {
      Translation = current_node_anim->mPositionKeys[0].mValue;
      return;
    }

    unsigned int translation_index = findtranslation(animation_time, current_node_anim);
    unsigned int next_translation_index = translation_index + 1;

    float delta_time = current_node_anim->mPositionKeys[next_translation_index].mTime - current_node_anim->mPositionKeys[translation_index].mTime;
    float factor = (animation_time - current_node_anim->mPositionKeys[translation_index].mTime) / delta_time;
    assert(factor >= 0.0f && factor <= 1.0f);
    const aiVector3D start_translation(current_node_anim->mPositionKeys[translation_index].mValue);
    const aiVector3D end_translation(current_node_anim->mPositionKeys[next_translation_index].mValue);

    Interpolate(Translation, start_translation, end_translation, factor);
    Translation = Translation.Normalize();
  }

  unsigned int findScaling(float animation_time, const aiNodeAnim* current_node_anim) {
    if (current_node_anim->mNumScalingKeys <= 0) {
      cout << "ERROR::FINDSCALING:: Number of Scaling Keys <= 0" << endl;
      assert(0);
    }

    for (unsigned int i = 0; i < current_node_anim->mNumScalingKeys - 1; i++)
      if (animation_time < (float)current_node_anim->mScalingKeys[i + 1].mTime)
        return i;

    cout << "ERROR::FINDSCALING:: animation_time splited(animation_time: " << animation_time << ", last_time: "
      << (float)current_node_anim->mScalingKeys[current_node_anim->mNumScalingKeys - 1].mTime << endl;
    assert(0);
  }

  unsigned int findRotation(float animation_time, const aiNodeAnim* current_node_anim) {
    if (current_node_anim->mNumRotationKeys <= 0) {
      cout << "ERROR::FINDROTATION:: NUmber of Rotation Keys <= 0 " << endl;
      assert(0);
    }

    for (unsigned int i = 0; i < current_node_anim->mNumRotationKeys - 1; i++)
      if (animation_time < (float)current_node_anim->mRotationKeys[i + 1].mTime)
        return i;

    cout << "ERROR::FINDROTATION:: animation_time splited(animation_time: " << animation_time << ", last_time: "
      << (float)current_node_anim->mRotationKeys[current_node_anim->mNumRotationKeys - 1].mTime << endl;
    assert(0);
  }

  unsigned int findtranslation(float animation_time, const aiNodeAnim* current_node_anim) {
    if (current_node_anim->mNumPositionKeys <= 0) {
      cout << "ERROR::FINDROTATION:: NUmber of Rotation Keys <= 0 " << endl;
      assert(0);
    }

    for (unsigned int i = 0; i < current_node_anim->mNumPositionKeys - 1; i++)
      if (animation_time < (float)current_node_anim->mPositionKeys[i + 1].mTime)
        return i;

    cout << "ERROR::FINDROTATION:: animation_time splited(animation_time: " << animation_time << ", last_time: "
      << (float)current_node_anim->mPositionKeys[current_node_anim->mNumPositionKeys - 1].mTime << endl;
    assert(0);
  }


  void Interpolate(aiVector3D& inter, const aiVector3D& start, const aiVector3D& end, float factor) {
    inter.x = start.x + (end.x - start.x) * factor;
    inter.y = start.y + (end.y - start.y) * factor;
    inter.z = start.z + (end.z - start.z) * factor;
    return;
  }

  void Interpolate(aiQuaternion& inter, const aiQuaternion& start, const aiQuaternion& end, float factor) {
    aiQuaternion::Interpolate(inter, start, end, factor);
  }

  glm::mat4 aimat_to_glmmat(aiMatrix4x4 ai_matrix) {
    glm::vec4 row1(ai_matrix.a1, ai_matrix.a2, ai_matrix.a3, ai_matrix.a4);
    glm::vec4 row2(ai_matrix.b1, ai_matrix.b2, ai_matrix.b3, ai_matrix.b4);
    glm::vec4 row3(ai_matrix.c1, ai_matrix.c2, ai_matrix.c3, ai_matrix.c4);
    glm::vec4 row4(ai_matrix.d1, ai_matrix.d2, ai_matrix.d3, ai_matrix.d4);

    glm::mat4 ret(row1, row2, row3, row4);
    return ret;
  }

  glm::mat4 aimat_to_glmmat(aiMatrix3x3 ai_matrix) {
    glm::vec4 row1(ai_matrix.a1, ai_matrix.a2, ai_matrix.a3, 0.0f);
    glm::vec4 row2(ai_matrix.b1, ai_matrix.b2, ai_matrix.b3, 0.0f);
    glm::vec4 row3(ai_matrix.c1, ai_matrix.c2, ai_matrix.c3, 0.0f);
    glm::vec4 row4(0.0f,         0.0f        , 0.0f        , 1.0f);

    glm::mat4 ret(row1, row2, row3, row4);
    return ret;
  }

};


unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
  string filename = string(path);
  filename = directory + '/' + filename;

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
#endif