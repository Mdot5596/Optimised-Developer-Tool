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

class SceneBasic_Uniform : public Scene
{
private:
    //  Torus torus;
    Plane plane;
    // Teapot teapot;
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
    GLSLProgram prog, skyProg;
    void setMatrices();
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

    // Particle system fields
    float time, deltaT, particleLifetime;
    int nParticles;
    GLuint posBuf[2], velBuf[2], age[2];
    GLuint particleArray[2];
    GLuint feedback[2];
    GLuint drawBuf;
    glm::vec3 emitterPos, emitterDir;
    Random rand;
    GLSLProgram fireProg;

public:
    SceneBasic_Uniform();

    void initScene();
    void update(float t);
    void render();
    void resize(int, int);
    void initBuffers();                  // Fire particle buffers
    void setMatrices(GLSLProgram& prog); // Overload for fire shader


    //Movement 
    void handleKeyboardInput(float deltaTime);
    void handleMouseInput();
};

#endif // SCENEBASIC_UNIFORM_H