#include "scenebasic_uniform.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include "helper/glutils.h"
#include "helper/texture.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"

using std::cerr;
using std::endl;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform() :
    tPrev(0),
    plane(50.0f, 50.0f, 1, 1),
    sky(100.0f),
    cameraPosition(0.0f, 0.0f, 10.0f),
    cameraFront(0.0f, 0.0f, -1.0f),
    cameraUp(0.0f, 1.0f, 0.0f),
    cameraYaw(-90.0f),
    cameraPitch(0.0f),
    mouseFirstEntry(true),
    particleLifetime(5.0f),
    nParticles(3000),
    emitterPos(2, 4, 2), emitterDir(0, 1, 0),
    rotSpeed(0.1f)
{
    //Load Models:
    Canmesh = ObjMesh::load("media/soda can.obj", true);
    Wallmesh = ObjMesh::load("media/wall.obj", true);
    Tablemesh = ObjMesh::load("media/table.obj", true);
}


void SceneBasic_Uniform::initScene()
{
    compile();
    glEnable(GL_DEPTH_TEST);

    //Lock mouse to screen
    GLFWwindow* Falloutscene = glfwGetCurrentContext();
    glfwSetInputMode(Falloutscene, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // MVP 
    model = mat4(1.0f);
    view = glm::lookAt(vec3(0.0f, 6.0f, 12.0f), vec3(0.0f, 0.2f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = mat4(1.0f);
    angle = 0.0f;



    //Material Properties
    prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
    prog.setUniform("Material.Kd", vec3(0.8f, 0.8f, 0.8f));
    prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Shininess", 50.0f);



    //Texture Scaling
    prog.setUniform("texScale", 1.0f);
    prog.setUniform("mixFactor", 0.5f);

    //Load Textures
    glActiveTexture(GL_TEXTURE1);
    sodaCanTex = Texture::loadTexture("media/texture/nukacan.jpg");
    glBindTexture(GL_TEXTURE_2D, sodaCanTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);


    //Load Plane Texture
    glActiveTexture(GL_TEXTURE2);
    planeTex = Texture::loadTexture("media/texture/FALLOUTFLOOR.jpg");
    glBindTexture(GL_TEXTURE_2D, planeTex);

    //Load Wall Texture
    glActiveTexture(GL_TEXTURE3);
    wallTex = Texture::loadTexture("media/texture/wall.jpg");
    glBindTexture(GL_TEXTURE_2D, wallTex);

    //Load Table Texture
    glActiveTexture(GL_TEXTURE4);
    GLuint tableTex = Texture::loadTexture("media/texture/Table.png");
    glBindTexture(GL_TEXTURE_2D, tableTex);

    //Mix Texture 
    glActiveTexture(GL_TEXTURE2);
    mixTex = Texture::loadTexture("media/texture/moss.jpg");
    glBindTexture(GL_TEXTURE_2D, mixTex);


    //Particle Fnt
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    model = mat4(1.0f);

    glActiveTexture(GL_TEXTURE0);
    particleTex = Texture::loadTexture("media/texture/green.png");
    if (particleTex == 0) std::cerr << "Failed to load particle texture!" << std::endl;
    glBindTexture(GL_TEXTURE_2D, particleTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);


    glActiveTexture(GL_TEXTURE1);
    ParticleUtils::createRandomTex1D(nParticles * 3);
    initBuffers();
    particlefnt.use();
    particlefnt.setUniform("RandomTex", 1);
    particlefnt.setUniform("ParticleTex", 0);
    particlefnt.setUniform("ParticleLifetime", particleLifetime);
    particlefnt.setUniform("ParticleSize", 0.05f);
    particlefnt.setUniform("Accel", vec3(0.0f, -0.5f, 0.0f));
    particlefnt.setUniform("EmitterPos", emitterPos);
    particlefnt.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));


    //Gassian Blur


}



void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        skyProg.compileShader("shader/skybox.vert");
        skyProg.compileShader("shader/skybox.frag");
        particlefnt.compileShader("shader/particles.vert");
        particlefnt.compileShader("shader/particles.frag");
        gassblr.compileShader("shader/blur.vert");
        gassblr.compileShader("shader/blur.frag");

        GLuint  particlefntHandle = particlefnt.getHandle();
        const char* outputNames[] = { "Position", "Velocity", "Age" };
        glTransformFeedbackVaryings(particlefntHandle, 3, outputNames, GL_SEPARATE_ATTRIBS);

        gassblr.link();
        particlefnt.link();
        skyProg.link();
        prog.link();
        prog.use();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}



void SceneBasic_Uniform::update(float t)
{
    deltaT = t - tPrev;  // Make sure this is set before rendering


    time = t;
    angle = std::fmod(angle + 0.01f, glm::two_pi<float>());
    float deltaT = t - tPrev;
    if (tPrev == 0.0f) deltaT = 0.0f;
    tPrev = t;

    angle += rotSpeed * deltaT;
    if (angle > glm::two_pi<float>()) {
        angle -= glm::two_pi<float>();
    }
    // Update the time variable
    time += deltaT;


    handleKeyboardInput(deltaT);
    handleMouseInput();

    // Recalculate view matrix after inputs
    view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

}


void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color & depth buffers

    prog.setUniform("ViewMatrix", view);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    prog.use();
    model = mat4(1.0f);
    setMatrices(skyProg);
    prog.setUniform("IsSkybox", true);
    sky.render();

    prog.setUniform("IsSkybox", false);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, planeTex);
    model = mat4(1.0f);
    model = translate(model, vec3(0.0f, -1.0f, 0.0f));
    setMatrices(prog);
    prog.setUniform("texScale", 5.0f);
    prog.setUniform("UseSecondTexture", false);
    plane.render();

    //Render can
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sodaCanTex);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 1.75f, 2.0f));
    model = glm::scale(model, glm::vec3(0.3f)); 
    setMatrices(prog);
    prog.setUniform("texScale", 1.0f);
    prog.setUniform("UseSecondTexture", true);
    Canmesh->render();

    //Render wall
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, wallTex);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.0f, -3.0f));         
    //model = glm::scale(model, vec3(5.0f, 2.5f, 4.0f));            
    model = glm::scale(model, glm::vec3(5.0f));
    setMatrices(prog);
    prog.setUniform("texScale", 1.0f);
    prog.setUniform("UseSecondTexture", false);
    Wallmesh->render();

    // Render Table
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, tableTex);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.0f));
    model = glm::scale(model, glm::vec3(3.5f));
    setMatrices(prog);
    prog.setUniform("texScale", 1.0f);
    prog.setUniform("UseSecondTexture", false);
    Tablemesh->render();

    // Bind moss mix texture for later usage
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mixTex);


    //Particle Fnt
    particlefnt.use();
    particlefnt.setUniform("Time", time);
    particlefnt.setUniform("DeltaT", deltaT);
    particlefnt.setUniform("Pass", 1);

    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuf]);
    glBeginTransformFeedback(GL_POINTS);
    glBindVertexArray(particleArray[1 - drawBuf]);
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glDrawArrays(GL_POINTS, 0, nParticles);
    glBindVertexArray(0);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    particlefnt.setUniform("Pass", 2);

    setMatrices(particlefnt);

    glDepthMask(GL_FALSE);

    glBindVertexArray(particleArray[drawBuf]);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    drawBuf = 1 - drawBuf;


}


void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);

}


void SceneBasic_Uniform::setMatrices(GLSLProgram& p)
{
    mat4 mv = view * model;
    p.setUniform("ModelViewMatrix", mv);
    p.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    p.setUniform("MVP", projection * mv);
    p.setUniform("ProjectionMatrix", projection);
}


void SceneBasic_Uniform::initBuffers()
{

    //Particle Fnt
    glGenBuffers(2, posBuf);
    glGenBuffers(2, velBuf);
    glGenBuffers(2, age);


    int size = nParticles * 3 * sizeof(GLfloat);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), 0, GL_DYNAMIC_COPY);


    std::vector<GLfloat> tempData(nParticles);
    float rate = particleLifetime / nParticles;
    for (int i = 0; i < nParticles; i++)
    {
        tempData[i] = rate * (i - nParticles);
    }


    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), tempData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glGenVertexArrays(2, particleArray);
    //ParticleArray 0
    glBindVertexArray(particleArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    //ParticleArray 1
    glBindVertexArray(particleArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    glGenTransformFeedbacks(2, feedback);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[0]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[1]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
}



void SceneBasic_Uniform::handleKeyboardInput(float deltaTime)
{
    GLFWwindow* Falloutscene = glfwGetCurrentContext();
    const float movementSpeed = 5.0f * deltaTime;

    if (glfwGetKey(Falloutscene, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += movementSpeed * cameraFront;

    if (glfwGetKey(Falloutscene, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= movementSpeed * cameraFront;

    if (glfwGetKey(Falloutscene, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * movementSpeed;

    if (glfwGetKey(Falloutscene, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * movementSpeed;

    if (glfwGetKey(Falloutscene, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPosition += movementSpeed * cameraUp;

    if (glfwGetKey(Falloutscene, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPosition -= movementSpeed * cameraUp;

    if (glfwGetKey(Falloutscene, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(Falloutscene, true);
}


void SceneBasic_Uniform::handleMouseInput()
{
    GLFWwindow* Falloutscene = glfwGetCurrentContext();
    double xpos, ypos;
    glfwGetCursorPos(Falloutscene, &xpos, &ypos);

    if (mouseFirstEntry)
    {
        cameraLastXPos = (float)xpos;
        cameraLastYPos = (float)ypos;
        mouseFirstEntry = false;
    }

    float xOffset = (float)xpos - cameraLastXPos;
    float yOffset = cameraLastYPos - (float)ypos;
    cameraLastXPos = (float)xpos;
    cameraLastYPos = (float)ypos;

    const float sensitivity = 0.025f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    cameraYaw += xOffset;
    cameraPitch += yOffset;

    if (cameraPitch > 89.0f)
    {
        cameraPitch = 89.0f;
    }
    else if (cameraPitch < -89.0f)
    {
        cameraPitch = -89.0f;
    }

    glm::vec3 direction;
    direction.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    direction.y = sin(glm::radians(cameraPitch));
    direction.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    cameraFront = glm::normalize(direction);
}

//--------Gauss Blur Attempt Bellow-----------------


void SceneBasic_Uniform::setupFBO()
{
    glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

    glGenTextures(1, &renderTex);
    glBindTexture(GL_TEXTURE_2D, renderTex);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer is complete" << endl;
    }
    else
    {
        std::cout << "Framebuffer error" << endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

    glGenTextures(1, &intermediateTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, intermediateTex);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediateTex, 0);

    glDrawBuffers(1, drawBuffers);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float SceneBasic_Uniform::gauss(float x, float sigma2)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
    double exponent = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(exponent));
}