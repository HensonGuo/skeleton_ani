## skeleton_ani
骨骼动画演示demo，通过assimp库解析3d模型获取模型的顶点、材质等数据，再将数据重新组织并传给显存供opengl绘制。骨骼动画是通过cpu实时插值计算每个骨骼变换的矩阵后再传给cpu的

![](http://cc.fp.ps.netease.com/file/606ec9738b74272da391ca6aYAmTJLO503)

#### 依赖库：
* assimp：用于3d模型解析
* glfw：创建opengl窗口
* glm：opengl数学几何库
* imgui：gui组件库

#### todo:

*  骨骼对齐
*  静止时模型不重绘，骨骼变换matrix不重计算