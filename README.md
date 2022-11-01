## Note

### 第一次会议记录



#### 主题

随机的森林地图里面打猎(猎人 / 猎物/ 随机的地图)



#### 时间节点

- 开题报告
  - ppt制作
    - 主题
      - 
    - 预期效果
    - 闪光点/难点
      - 倍镜效果
      - 碰撞
      - 动作
      - 物理真实性（重力）
    - 工具
      - Blender
      - OpenGL
      - 版本开发 (Github)
      - C++
    - 整体流程
      - Blender的OBJ文件应用到OpenGL **（11.6）**
        - 猎人
        - 猪
        - 漫游
      - 项目框架
        - 类
        - 库



### 第二次会议记录

#### 分工

- 黄顺晖 创建抽象类 HObject

  - 所有游戏中存在行为的物体（猎人/ 动物）将以子类的方式继承自HObject类
  - HObject类存储渲染该物体需要的数据 (protected)
  - HObject类定义物体当前状态
    - 静止
    - 步进
    - 减速（步进到静止的过渡过程）
  - HObject类定义物体实现上述动作需要的接口
    - stay()
    - step()
    - slow()
  - HObject类定义并实现渲染物体的方法，需要调用上述动作接口
    - draw()
  - HObject类实现辅助方法
    - 以任一点为圆心

- 王欣辰/ 刘志坚 实现 HPig : public HObject类 / HHunter: public HObject类 

  - 利用blender构建相应的模型

  - 依据模型具现以下方法

    - stay()
    - step()
    - slow()

  - 测试模型

    

  

  
