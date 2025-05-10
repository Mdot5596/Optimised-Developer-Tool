#include "scenebasic_uniform.h"
#include <cstdio>
#include <cstdlib>
#include <string>
using std::string;
#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include "helper/glutils.h"
#include "helper/texture.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform() :
    tPrev(0), angle(0.0f), rotSpeed(glm::pi<float>()/8.0f), plane(50.0f, 50.0f, 1, 1), teapot(14,mat4(1.0f))
    , sphere(2.0f,50,50)
{
    Canmesh = ObjMesh::load("media/soda can.obj", true);
}

void SceneBasic_Uniform::initScene()
{
    compile();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    vec3 intense = vec3(0.6f);
    prog.setUniform("Lights[0].L", intense);
    prog.setUniform("Lights[1].L", intense);
    prog.setUniform("Lights[2].L", intense);
    intense = vec3(0.2f);
    prog.setUniform("Lights[0].La", intense);
    prog.setUniform("Lights[1].La", intense);
    prog.setUniform("Lights[2].La", intense);
    projection = mat4(1.0f);
    angle = glm::pi<float>() / 2.0f;
    setupFBO();
    // Array for full-screen quad
    GLfloat verts[] = {
    -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] = {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };
    // Set up the buffers
    unsigned int handle[2];
    glGenBuffers(2, handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);
    // Set up the vertex array object
    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0); // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2); // Texture coordinates
    glBindVertexArray(0);
    prog.setUniform("LumThresh", 1.7f);
    float weights[10], sum, sigma2 = 25.0f;
    // Compute and sum the weights
    weights[0] = gauss(0, sigma2);
    sum = weights[0];
    for (int i = 1; i < 10; i++) {
        weights[i] = gauss(float(i), sigma2);
        sum += 2 * weights[i];
    }
    // Normalize the weights and set the uniform
    for (int i = 0; i < 10; i++) {
        std::stringstream uniName;
        uniName << "Weight[" << i << "]";
        float val = weights[i] / sum;
        prog.setUniform(uniName.str().c_str(), val);
    }
    // Set up two sampler objects for linear and nearest filtering
    GLuint samplers[2];
    glGenSamplers(2, samplers);
    linearSampler = samplers[0];
    nearestSampler = samplers[1];
    GLfloat border[] = { 0.0f,0.0f,0.0f,0.0f };
    // Set up the nearest sampler
    glSamplerParameteri(nearestSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glSamplerParameterfv(nearestSampler, GL_TEXTURE_BORDER_COLOR, border);
    // Set up the linear sampler
    glSamplerParameteri(linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glSamplerParameterfv(linearSampler, GL_TEXTURE_BORDER_COLOR, border);
    // We want nearest sampling except for the last pass.
    glBindSampler(0, nearestSampler);
    glBindSampler(1, nearestSampler);
    glBindSampler(2, nearestSampler);

    glActiveTexture(GL_TEXTURE3);
    sodaCanTex = Texture::loadTexture("media/texture/nukacan.jpg");
    glBindTexture(GL_TEXTURE_2D, sodaCanTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

}



void SceneBasic_Uniform::compile()
{
    try {
        prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.frag");
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
    float deltaT = t - tPrev;
    if (tPrev == 0.0f) deltaT = 0.0f;
    tPrev = t;
    angle += rotSpeed * deltaT;
    if (angle > glm::two_pi<float>())angle -= glm::two_pi<float>();
}

void SceneBasic_Uniform::render()
{
    pass1();
    computeLogAveLuminance();
    pass2();
    pass3();
    pass4();
    pass5();

}

void SceneBasic_Uniform::pass1()
{
    prog.setUniform("Pass", 1);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

   // view = glm::lookAt(vec3(7.0f * cos(angle), 4.0f, 7.0f * sin(angle)), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    view = glm::lookAt(vec3(2.0f, 0.0f, 14.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 100.0f);

    drawScene();
}

void SceneBasic_Uniform::pass2()
{
    prog.setUniform("Pass", 2);
    glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);
    glViewport(0, 0, bloomBufWidth, bloomBufHeight);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);

    setMatrices();

    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);

}

void SceneBasic_Uniform::pass3()
{
    prog.setUniform("Pass", 3);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);

    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);

}

void SceneBasic_Uniform::pass4()
{
    prog.setUniform("Pass", 4);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);

    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);

}

void SceneBasic_Uniform::drawScene()
{
    vec3 intense = vec3(1.0f);
    prog.setUniform("Lights[0].L", intense);
    prog.setUniform("Lights[1].L", intense);
    prog.setUniform("Lights[2].L", intense);
    
    vec4 lightPos = vec4(0.0f, 4.0f, 2.5f, 1.0f);
    lightPos.x = -7.0f;
    prog.setUniform("Lights[0].Position", view * lightPos);
    lightPos.x = 0.0f;
    prog.setUniform("Lights[1].Position", view * lightPos);
    lightPos.x = 7.0f;
    prog.setUniform("Lights[2].Position", view * lightPos);

    prog.setUniform("Material.Kd", vec3(0.9f, 0.3f, 0.2f));
    prog.setUniform("Material.Ks", vec3(1.0f, 1.0f, 1.0f));
    prog.setUniform("Material.Ka", vec3(0.2f, 0.2f, 0.2f));
    prog.setUniform("Material.Shininess", 25.0f);              
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices();
    plane.render();

    model = glm::translate(model, vec3(0.0f, -5.0f, 0.0f));
    setMatrices();
    plane.render();
   
    prog.setUniform("Material.Kd", vec3(0.4f, 0.9f, 0.4f));
    model = mat4(1.0f);
    model = glm::translate(mat4(1.0f), vec3(-3.0f, -3.0f, 2.0f));
    setMatrices(); 
    sphere.render();

    
    prog.setUniform("Material.Kd", vec3(0.4f, 0.4f, 0.9f));
    model = mat4(1.0f);
    model = glm::translate(mat4(1.0f), vec3(3.0f, -4.0f, 2.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis
    setMatrices(); // Apply model-view-projection transformations
    prog.setUniform("UseCanTexture", true);
    prog.setUniform("CanTex", 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sodaCanTex);
    Canmesh->render();
    prog.setUniform("UseCanTexture", false);

}

void SceneBasic_Uniform::pass5()
{

    prog.setUniform("Pass", 5);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width, height);

    glBindSampler(1, linearSampler);
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindSampler(1, nearestSampler);

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

//sets up the fbo for rendering to a texture
void SceneBasic_Uniform::setupFBO() {
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &hdrFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFbo);
    // Create the texture object
    glGenTextures(1, &hdrTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, width, height);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTex, 0);
    // Create the depth buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, depthBuf);
    // Set the targets for the fragment output variables
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    // Create an FBO for the bright-pass filter and blur
    glGenFramebuffers(1, &blurFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
    // Create two texture objects to ping-pong for the bright-pass filter
    // and the two-pass blur
    bloomBufWidth = width / 8;
    bloomBufHeight = height / 8;
    glGenTextures(1, &tex1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, bloomBufWidth, bloomBufHeight);
    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &tex2);
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, bloomBufWidth, bloomBufHeight);
    // Bind tex1 to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);
    glDrawBuffers(1, drawBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void SceneBasic_Uniform::computeLogAveLuminance()
{
    int size = width * height;
    std::vector<GLfloat>texData(size * 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texData.data());
    float sum = 0.0f;
    for (int i = 0; i < size; i++)
    {
        float lum = glm::dot(vec3(texData[i * 3 + 0], texData[i * 3 + 1], texData[i * 3 + 2]), vec3(0.2126f, 0.7152f, 0.0722f));
        sum += logf(lum + 0.00001f);
    }
    prog.setUniform("AveLum", expf(sum / size));
}

float SceneBasic_Uniform::gauss(float x, float sigma2)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
    double exponent = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(exponent));
}