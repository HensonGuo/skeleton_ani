## skeleton_ani
骨骼动画演示demo，通过assimp库解析3d模型获取模型的顶点、材质等数据，再将数据重新组织并传给显存供opengl绘制。骨骼动画是通过cpu实时插值计算每个骨骼变换的矩阵后再传给cpu的

![](http://cc.fp.ps.netease.com/file/60701614143cfa6b4daecf88ywbwqXRB03)

#### 依赖库：
* assimp：用于3d模型解析
* glfw：创建opengl窗口
* glm：opengl数学几何库
* imgui：gui组件库

#### todo:

*  骨骼对齐
*  静止时模型不重绘，骨骼变换matrix不重计算


#### 学习研究:
*  [快速了解3d](https://git-cc.nie.netease.com/pc/skeleton_ani/blob/master/doc/%E5%BF%AB%E9%80%9F%E4%BA%86%E8%A7%A33d.md)
*  [opengl学习](https://learnopengl-cn.github.io/)
*  [骨骼动画研究](https://git-cc.nie.netease.com/pc/skeleton_ani/blob/master/doc/%E9%AA%A8%E9%AA%BC%E5%8A%A8%E7%94%BB%E7%A0%94%E7%A9%B6.md)