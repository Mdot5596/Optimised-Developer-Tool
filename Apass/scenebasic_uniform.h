#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H
#include "helper/scene.h"
#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/torus.h"
#include "helper/teapot.h"
#include "glm/glm.hpp"
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/cube.h"
#include "helper/sphere.h"

class SceneBasic_Uniform : public Scene
{
private:
  //  Cube cube;
    Sphere sphere;
//   std::unique_ptr<ObjMesh> spot;
    GLuint hdrFbo, blurFbo, fsQuad, hdrTex, tex1, tex2;
    GLuint linearSampler, nearestSampler;
   // Torus torus;
    Plane plane;
    Teapot teapot;

    std::unique_ptr<ObjMesh> Canmesh;
    GLuint sodaCanTex;


    int bloomBufWidth, bloomBufHeight;
    float tPrev;
    float angle;
    float rotSpeed;
    GLSLProgram prog;
    void setMatrices();
    void compile();

    void setupFBO();
    void pass1();
    void pass2();
    void pass3();
    void pass4();
    void pass5();
    void computeLogAveLuminance();
    void drawScene();
    float gauss(float, float);


public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
