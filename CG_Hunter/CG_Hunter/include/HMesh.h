#pragma once

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <HShader.h>
#include <HCollider.h>

#include <string>
#include <vector>
#include <def.h>

using namespace std;

#define MAX_BONE_INFLUENCE 20

struct Vertex {
  // position
  glm::vec3 Position;
  // normal
  glm::vec3 Normal;
  // texCoords
  glm::vec2 TexCoords;
  // tangent
  glm::vec3 Tangent;
  // bitangent
  glm::vec3 Bitangent;
  // bone indexes which will influence this vertex
  int m_BoneIDs[MAX_BONE_INFLUENCE];
  // weights from each bone
  float m_Weights[MAX_BONE_INFLUENCE];
  // number of bones
  int num_bones = 0;

  void addBone(int bone_index, float weight) {
    m_BoneIDs[num_bones] = bone_index;
    m_Weights[num_bones] = weight;
    num_bones++;
    if (num_bones >= MAX_BONE_INFLUENCE) {
      cout << "ERROR::SPLITED_BONE_INFLUENCE" << endl;
      assert(0);
    }
  }
};

struct Texture {
  unsigned int id;
  string type;
  string path;
};

struct MeshBone {
  int bone_index;
  float weights;

  MeshBone(int mbone_index, float mweights) {
    bone_index = mbone_index;
    weights = mweights;
  }
};

class HMesh {
public:
  // mesh Data
  vector<Vertex>       vertices;
  vector<unsigned int> indices;
  vector<Texture>      textures;
  unsigned int VAO;

  glm::mat4 ini_trans;

  // collider used for detecting collision
  HCollider* ini_collider;
  glm::mat4 mesh_transform_mat;
  vector<MeshBone> mesh_bone;
 
  // mesh name
  string mesh_name;

  // constructor
  HMesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures, HCollider* ini_collider, vector<MeshBone> mesh_bone, string mesh_name = "Unknown")
  {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->ini_collider = ini_collider;
    this->mesh_bone = mesh_bone;
    this->mesh_name = mesh_name;

    // now that we have all the required data, set the vertex buffers and its attribute pointers.
    setupMesh();
  }

  // render the mesh
  void Draw(HShader* shader)
  {
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    //cout << "Texture size: " << textures.size() << endl;

    //cout << "size: " << textures.size() << endl;
    for (unsigned int i = 0; i < textures.size(); i++)
    {
      // The defalut skybox id GL_TEXTRUE15 is left for skybox texture
      if (i == SKYBOX_ID)
        continue;
      glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
      // retrieve texture number (the N in diffuse_textureN)
      string number;
      string name = textures[i].type;
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
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);


    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
  }

private:
  // render data 
  unsigned int VBO, EBO;

  // initializes all the buffer objects/arrays
  void setupMesh()
  {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
    // ids
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

    // weights
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

    // number of bones
    glEnableVertexAttribArray(7);
    glVertexAttribIPointer(7, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, num_bones));
    glBindVertexArray(0);
  }
};
