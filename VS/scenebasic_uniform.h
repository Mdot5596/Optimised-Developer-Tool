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
#include "helper/skybox.h"
#include "helper/random.h"
#include "helper/particleutils.h"


class SceneBasic_Uniform : public Scene
{
private:
    Plane plane;
    std::unique_ptr<ObjMesh> Canmesh;
    std::unique_ptr<ObjMesh> Wallmesh;
    std::unique_ptr<ObjMesh> Tablemesh;


    GLuint sodaCanTex;
    GLuint cubeTex;
    GLuint wallTex;
    GLuint tableTex;


    SkyBox sky;
    GLuint planeTex;
    GLuint mixTex;
    float tPrev;
    float angle;
    GLSLProgram prog, skyProg, particlefnt, gassblr;
    void setMatrices(GLSLProgram& p);
    void compile();

    // Camera and mouse variables
    glm::vec3 cameraPosition;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float cameraYaw;
    float cameraPitch;
    bool mouseFirstEntry;
    float cameraLastXPos;
    float cameraLastYPos;


    //Particle Fnt
    float particleLifetime;
    int nParticles;
    void initBuffers();
    float time, deltaT;
    float rotSpeed;
    GLuint particleTex;
    glm::vec3 emitterPos, emitterDir;
    GLuint posBuf[2], velBuf[2], age[2];
    GLuint particleArray[2];
    GLuint feedback[2];
    GLuint drawBuf;

    //Gassian Blur


public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);

    //Movement 
    void handleKeyboardInput(float deltaTime);
    void handleMouseInput();
};

#endif // SCENEBASIC_UNIFORM_H