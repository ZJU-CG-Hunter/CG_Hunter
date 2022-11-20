### 基于Assimp的骨骼动画



原版教程链接 [Tutorial 38 - Skeletal Animation With Assimp (ogldev.org)](https://ogldev.org/www/tutorial38/tutorial38.html)



#### 实现思路

- 创建包含层次结构的骨架
- 针对每一个顶点，指定至多4个带权骨架
- 插入关键帧
- 根据关键帧进行线性插值获得转换矩阵
- 在顶点着色器上应用该转换矩阵



#### 实现流程

##### 1. 数据结构

- 拓展顶点数据，对于每一个顶点，都应加上骨架ID及对应权重，更新后的Vertex数据结构如下

```C++
struct Vertex {
  // position
  glm::vec3 Position;
  // normal
  glm::vec3 Normal;
  // texCoords
  glm::vec2 TexCoords;
  
  /*切线向量与二阶切线向量，暂时不需要*/
  // tangent
  //glm::vec3 Tangent;
  // bitangent
  //glm::vec3 Bitangent;
    
  //bone indexes which will influence this vertex
  int m_BoneIDs[MAX_BONE_INFLUENCE];
  //weights from each bone
  float m_Weights[MAX_BONE_INFLUENCE];
};	
```

- bone IDs 是对于 骨骼变换矩阵数组的索引，即每个骨架都对应一个变换，将这些变换统一放到一个数组内方便管理，自然Vertex中也需要bone IDs

- weight 即顶点对应某个bone 相应的权重，权重总和为1，顶点着色器中实际上就是做了一个 

  ∑ transform[i] * weight[i] 得到最终的骨架变换 

- 对于骨架的 transformation，是一个缩放向量、一个旋转四元数（一个相对欧拉角更好的对旋转的实现方式[quaternion.pdf (krasjet.github.io)](https://krasjet.github.io/quaternion/quaternion.pdf)，摆脱了万向锁[bonus_gimbal_lock.pdf (krasjet.github.io)](https://krasjet.github.io/quaternion/bonus_gimbal_lock.pdf)的困局）以及一个位移向量组成

- 还需要考虑一个时间上的问题：不同机器渲染的速度（glfw主循环速度）不同，因此在做线性插值时”步进“的速度也不同，我们应尽量保证这个速度是相同的，例如对于glfw主循环更慢的机器应该让步进的次数缩短（步长增加）



#### 2. Assimp导入bones数据

- 下面这张图展示了与bones相关的Assimp数据结构![img](https://ogldev.org/www/tutorial38/assimp1.jpg)

- aiMesh应至少包含了以下信息位置、法向量、纹理坐标，实际上aiMesh也包含了一个aiBone的数组
- aiBone即骨架，其中包含了name，vertexweight(所有与该bone关联的vertex的对应权重)



需要检测的物体 碰撞类型 