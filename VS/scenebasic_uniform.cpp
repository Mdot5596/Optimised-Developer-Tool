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
    mouseFirstEntry(true)
{
    //Load Models:
    mesh = ObjMesh::load("media/soda can.obj", true);

}


void SceneBasic_Uniform::initScene()
{
    compile();
    glEnable(GL_DEPTH_TEST); // Enable depth testing

    //Lock mouse to screen
    GLFWwindow* Falloutscene = glfwGetCurrentContext();
    glfwSetInputMode(Falloutscene, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // MVP 
    model = mat4(1.0f);
    view = glm::lookAt(vec3(0.0f, 6.0f, 12.0f), vec3(0.0f, 0.2f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = mat4(1.0f);
    angle = 0.0f;

    //Spotlight Setup
    vec3 spotPos = vec3(2.0f, 5.0f, 3.0f);              // Position of spotlight in world space
    vec3 spotDir = normalize(vec3(-0.5f, -1.0f, 0.0f)); // Direction the spotlight points
    float spotCutoff = glm::cos(glm::radians(25.0f));   // Convert cutoff angle (25) to cosine
    float spotExponent = 0.0f;                        // Controls edge softness  SET TO 0 IF I WANT TO ACC SEE LOL

    cubeTex = Texture::loadHdrCubeMap("media/texture/cube/skybox-hdr/skybox");
    prog.setUniform("SkyBoxTex", 0);  
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);

    // Set spotlight uniforms
    prog.setUniform("Spot.Position", spotPos);  // world space
    prog.setUniform("Spot.Direction", spotDir); // world space
    prog.setUniform("Spot.L", vec3(1.0f, 1.0f, 1.0f));  // Light intensity
    prog.setUniform("Spot.La", vec3(0.2f, 0.2f, 0.2f)); // Ambient light
    prog.setUniform("Spot.Exponent", spotExponent);
    prog.setUniform("Spot.Cutoff", spotCutoff);

    //Material Properties
    prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
    prog.setUniform("Material.Kd", vec3(0.8f, 0.8f, 0.8f));
    prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Shininess", 50.0f);

    //Fog Properties
    prog.setUniform("Fog.MinDist", 5.0f); //5 and 25
    prog.setUniform("Fog.MaxDist", 25.0f);
    prog.setUniform("Fog.Color", vec3(0.5f, 0.5f, 0.5f));

    //Texture Scaling
    prog.setUniform("texScale", 1.0f);
    prog.setUniform("mixFactor", 0.5f);  // Adjust this value to control blending

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

    //Mix Texture 
    glActiveTexture(GL_TEXTURE2);
    mixTex = Texture::loadTexture("media/texture/moss.jpg");
    glBindTexture(GL_TEXTURE_2D, mixTex);
}



void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
        skyProg.compileShader("shader/skybox.vert");
        skyProg.compileShader("shader/skybox.frag");
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
    float deltaTime = t - tPrev;
    if (tPrev == 0.0f) deltaTime = 0.0f;
    tPrev = t;

    angle += 0.3f * deltaTime;
    if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();

    handleKeyboardInput(deltaTime);
    handleMouseInput();

    // Recalculate view matrix after inputs
    view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

}


void SceneBasic_Uniform::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color & depth buffers

    // Update spotlight uniforms
    vec3 spotPos = vec3(10.0f * cos(angle), 10.0f, 10.0f * sin(angle));
    prog.setUniform("Spot.Position", spotPos);
    prog.setUniform("ViewMatrix", view);

  
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    prog.use();
    model = mat4(1.0f);
    setMatrices();
    prog.setUniform("IsSkybox", true);
    sky.render();  // Draw the skybox

    // Reset to normal shading for other objects
    prog.setUniform("IsSkybox", false);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, planeTex);
    model = mat4(1.0f);
    model = translate(model, vec3(0.0f, -1.0f, 0.0f));
    setMatrices();
    prog.setUniform("texScale", 5.0f);
    prog.setUniform("UseSecondTexture", false);
    plane.render();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sodaCanTex);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 0.6f, 0.0f));
    setMatrices();
    prog.setUniform("texScale", 1.0f);
    prog.setUniform("UseSecondTexture", true);
    mesh->render();

    // Bind moss mix texture for later usage
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mixTex);
}


void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);

}


void SceneBasic_Uniform::setMatrices()
{
    mat4 mv = view * model; // Compute the Model-View matrix

    // Send Model-View matrix to the shader
    prog.setUniform("ModelViewMatrix", mv);

    // Compute and send Normal Matrix (used for transforming normals correctly)
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

    // Send Model-View-Projection matrix to the shader
    prog.setUniform("MVP", projection * mv);
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
