/* Name: Kelly Lewis
*  Date: 12/13/2022
*  Course: CS-330
*  School: Southern New Hampshire University
*
*  Title: 7-1 Final Project Submission
*  Description: We were tasked with recreating an image selected during our second week
*   to the best of our ability. This recreation was required to have 4 different primitives.
*   Additionally, we were required to add lighting and camera controls.
* 
*   All Images Sourced from: https://pixabay.com/ under a free use license.
*   License: https://pixabay.com/service/license/
*/

#include <iostream>         // Output log and error
#include <cstdlib>          // C Standard Library for EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "camera.h"

//My Headers
#include "Cylinder.h"


using namespace std; // standard namespace

// SHADER CORE SOURCES -------------------------------------------------------------------------------------

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

/* Object Vertex Shader Source Code*/
const GLchar* objectVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Object Fragment Shader Source Code*/
const GLchar* objectFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 kLightColor; // Key Light color
uniform vec3 fLightColor; // Fill Light color
uniform vec3 kLightPos;   // Key Light Pos
uniform vec3 fLightPos;   // Fill Light Pos
uniform vec3 viewPosition;
uniform sampler2D uTexture;

uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    // Begin Fill Light
    //Calculate Ambient fill lighting*/
    float fAmbientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 fAmbient = fAmbientStrength * fLightColor; // Generate ambient light color

    //Calculate Diffuse fill lighting*/
    vec3 fNorm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 fLightDirection = normalize(fLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float fImpact = max(dot(fNorm, fLightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 fDiffuse = fImpact * fLightColor; // Generate diffuse light color

    //Calculate Specular fill lighting*/
    float fSpecularIntensity = 0.1f; // Set specular light strength
    float fHighlightSize = 32.0f; // Set specular highlight size
    vec3 fViewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 fReflectDir = reflect(-fLightDirection, fNorm);// Calculate reflection vector
    //Calculate specular fill component
    float fSpecularComponent = pow(max(dot(fViewDir, fReflectDir), 0.0), fHighlightSize);
    vec3 fSpecular = fSpecularIntensity * fSpecularComponent * fLightColor;
    //END Fill Light

    //Begin Key Light
    //Calculate Ambient key lighting*/
    float kAmbientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 kAmbient = kAmbientStrength * kLightColor; // Generate ambient light color

    //Calculate Diffuse key lighting*/
    vec3 kNorm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 kLightDirection = normalize(kLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float kImpact = max(dot(kNorm, kLightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 kDiffuse = kImpact * kLightColor; // Generate diffuse light color

    //Calculate Specular key lighting*/
    float kSpecularIntensity = 0.5f; // Set specular light strength
    float kHighlightSize = 16.0f; // Set specular highlight size
    vec3 kViewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 kReflectDir = reflect(-kLightDirection, kNorm);// Calculate reflection vector
    //Calculate specular key component
    float kSpecularComponent = pow(max(dot(kViewDir, kReflectDir), 0.0), kHighlightSize);
    vec3 kSpecular = kSpecularIntensity * kSpecularComponent * kLightColor;
    //END Key Light

    // Texture holds the color to be used for all three components

    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);


    // Calculate phong result
    vec3 fResult = (fAmbient + fDiffuse + fSpecular);
    vec3 kResult = (kAmbient + kDiffuse + kSpecular);
    vec3 lightingResult = fResult + kResult;
    vec3 phong = lightingResult * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);

//END SHADER SOURCES --------------------------------------------------------------------------------------------

namespace {

    // Struct to store GL data relative to a given mesh
    struct GLMesh {
        GLuint vao;         // vertex array object
        GLuint vbo;         // vertex buffer object
        GLuint ibo;         // Index buffer object
        GLuint nVertices;   // Vertices of the mesh
        GLuint nIndices;    // Indices of the mesh
    };

    // Mesh data
    GLMesh pyramidMesh;
    GLMesh paperMesh;
    GLMesh paperBMesh;
    GLMesh tableMesh;
    GLMesh lampMainMesh;
    GLMesh lampMeshA;
    GLMesh lampMeshB;
    GLMesh pencilBodyMesh;
    GLMesh pencilTipMesh;
    GLMesh handleMesh;
    GLMesh threadMesh;
    GLMesh spoolMesh;
    GLMesh spoolBMesh;
    GLMesh glassRingMesh;
    GLMesh glassMesh;
    GLMesh magHandleMesh;

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    const char* const WINDOW_TITLE = "7-1 Final Project: Re-Creating a 3D Scene by Kelly Lewis"; // Window Title Variable

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Timing Variables
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Camera Init and Variables
    Camera gCamera(glm::vec3(0.0f, 4.0f, 10.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // Shader programs
    GLuint gProgramId; // Object Shader
    GLuint gLampProgramId; // Lamp Shader

    // Texture
    GLuint galTexture;
    GLuint PencilBody;
    GLuint PencilCut;
    GLuint PaperTexture;
    GLuint PhotoTexture;
    GLuint darkWood;
    GLuint threadTexture;
    GLuint spoolWood;
    GLuint lightWood;
    GLuint Glass;
    glm::vec2 gUVScale(1.0f, 1.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Object and light color
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColorA(1.0f, 1.0f, 1.0f); // Key Light - White to clearly display the scene
    glm::vec3 gLightColorB(0.0f, 0.0f, 0.0f); // Fill Light - Black to simulate Shadows

    // Light A (Key) position and scale
    glm::vec3 gLightPositionA(0.0f, 5.0f, 5.0f);
    glm::vec3 gLightScaleA(0.3f);

    // Light B (Fill) position and scale
    glm::vec3 gLightPositionB(0.0f, 5.0f, -5.0f);
    glm::vec3 gLightScaleB(0.3f);

    // Lamp animation
    bool gIsLampOrbiting = true;

    //view mode
    bool isOrtho = false;
    
}

//Function Declarations
// Window Operations and Init
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);

//Input Handling
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

//Shader Construction and Destruction
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
bool ULoadShaders();
void UDestroyShaderProgram(GLuint programId);

//Texture
void flipImageVertically(unsigned char* image, int width, int height, int channels);
bool UCreateTexture(const char* filename, GLuint& textureId);
bool ULoadTextures();
void UDestroyTexture(GLuint textureId);

//Rendering
void URender();
void UCreateMeshObjects();
void UDestroyScene();

//Shape
void UCreatePyramid(GLMesh& mesh);
void UCreateCube(GLMesh& mesh);
void UCreatePlane(GLMesh& mesh);
void UCreateCylinder(GLMesh& mesh, GLfloat tRad, GLfloat bRad, GLfloat h, GLint slices, GLint stacks);

void UDestroyMesh(GLMesh& mesh);

// Main Function and Entry point for OpenGL
int main(int argc, char* argv[]) {

    // If UInitialize fails
    if (!UInitialize(argc, argv, &gWindow)) { // Error Check
        return EXIT_FAILURE; // Error Handle
    }

    // Create mesh objects
    UCreateMeshObjects();

    ULoadShaders();

    ULoadTextures();

    // Set the background color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Start Render Loop (aka Game Loop)
    while (!glfwWindowShouldClose(gWindow)) { // Infinite Loop with Exit Condition

        // Frame Timing Variables
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // Lamp orbits around the origin
        const float angularVelocity = glm::radians(45.0f);
        if (gIsLampOrbiting)
        {
            //Lamp A
            glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPositionA, 1.0f);
            gLightPositionA.x = newPosition.x;
            gLightPositionA.y = newPosition.y;
            gLightPositionA.z = newPosition.z;
            //Lamp B
            newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPositionB, 1.0f);
            gLightPositionB.x = newPosition.x;
            gLightPositionB.y = newPosition.y;
            gLightPositionB.z = newPosition.z;
        }

        // Enable z-depth
        glEnable(GL_DEPTH_TEST);

        // Clear background set to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(gProgramId);

        // Call to handle input
        UProcessInput(gWindow);

        // Render frame function call
        URender();

        // Poll for events (aka inputs)
        glfwPollEvents();
    }

    // Clean-up call
    UDestroyScene();

    exit(EXIT_SUCCESS); // Terminates the program successfully
}

// Function to call mesh creation
void UCreateMeshObjects() {
    UCreatePyramid(pyramidMesh); // Call to create a standard pyramid
    UCreatePlane(paperMesh); // Call to create a standard cube
    UCreatePlane(paperBMesh); // Call to create a standard cube
    UCreateCube(lampMeshA); // Call to create a standard cube
    UCreateCube(lampMeshB); // Call to create a standard cube
    UCreateCube(magHandleMesh); // Call to create a standard cube
    UCreatePlane(tableMesh); // Call to create a plane
    UCreateCylinder(pencilBodyMesh, 1.0f, 1.0f, 3.0f, 6, 1); // Call to create a cylinder
    UCreateCylinder(pencilTipMesh, 1.0f, 0.0f, 3.0f, 6, 1); // Call to create a cylinder
    UCreateCylinder(threadMesh, 1.0f, 1.0f, 3.0f, 25, 1); // Call to create a cylinder
    UCreateCylinder(spoolMesh, 1.0f, 1.0f, 0.5f, 25, 1); // Call to create a cylinder
    UCreateCylinder(spoolBMesh, 1.0f, 1.0f, 0.5f, 25, 1); // Call to create a cylinder
    UCreateCylinder(glassRingMesh, 1.0f, 1.0f, 0.5f, 25, 1); // Call to create a cylinder
    UCreateCylinder(glassMesh, 1.0f, 1.0f, 0.5f, 25, 1); // Call to create a cylinder

}

//Function to call mesh destruction
void UDestroyScene() {

    // Mesh Clean-up
    UDestroyMesh(pyramidMesh);
    UDestroyMesh(paperMesh);
    UDestroyMesh(tableMesh);
    UDestroyMesh(lampMeshA);
    UDestroyMesh(lampMeshB);
    UDestroyMesh(pencilBodyMesh);
    UDestroyMesh(pencilTipMesh);
    UDestroyMesh(threadMesh);
    UDestroyMesh(spoolMesh);
    UDestroyMesh(glassRingMesh);
    UDestroyMesh(glassMesh);
    UDestroyMesh(magHandleMesh);
    

    // Shader Clean-up
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    // Texture Clean-up
    UDestroyTexture(darkWood);
    UDestroyTexture(PaperTexture);
    UDestroyTexture(PencilBody);
    UDestroyTexture(PencilCut);
    UDestroyTexture(PhotoTexture);
    UDestroyTexture(galTexture);
    UDestroyTexture(lightWood);
    UDestroyTexture(Glass);
    UDestroyTexture(galTexture);
}

// Render Function
void URender() {

    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 model;

    GLint modelLoc;
    GLint viewLoc;
    GLint projLoc;
    GLint UVScaleLoc;


    //View and Projection
    // Camera View Matrix
    view = gCamera.GetViewMatrix();

    // Perspective Projection
    if (!isOrtho){
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else{
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
    }


    // Create Matrix variables to pass to Uniform
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");
    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");

    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint keyLightColorLoc = glGetUniformLocation(gProgramId, "kLightColor");
    GLint keyLightPositionLoc = glGetUniformLocation(gProgramId, "kLightPos");
    GLint fillLightColorLoc = glGetUniformLocation(gProgramId, "fLightColor");
    GLint fillLightPositionLoc = glGetUniformLocation(gProgramId, "fLightPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Set Uniforms
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(keyLightColorLoc, gLightColorA.r, gLightColorA.g, gLightColorA.b);
    glUniform3f(fillLightColorLoc, gLightColorB.r, gLightColorB.g, gLightColorB.b);
    glUniform3f(keyLightPositionLoc, gLightPositionA.x, gLightPositionA.y, gLightPositionA.z);
    glUniform3f(fillLightPositionLoc, gLightPositionB.x, gLightPositionB.y, gLightPositionB.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // Draw Primitives with Texture and Transformations
    // Cubes ---------------------------------------------------------------------------------------------------
    //Cube  
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, lightWood); // Set Active Texture
    glBindVertexArray(magHandleMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(2.5f, 0.075f, -3.8f)) *      // Change object position (Translate)
        glm::rotate(0.5f, glm::vec3(0.0f, 1.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(2.0f, 0.1f, 0.25f));            // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, magHandleMesh.nVertices);
    glBindVertexArray(0);  // Deativate the VAO
    // END Cube

    // Pyramids -------------------------------------------------------------------------------------------------

    //Pyramid
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, galTexture); // Set Active Texture
    glBindVertexArray(pyramidMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(-3.0f, 0.51f, -3.0f)) *    // Change object position (Translate)
        glm::rotate(-10.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(1.0f, 1.0f, 1.0f));           // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, pyramidMesh.nVertices);
    glBindVertexArray(0);  // Deativate the VAO
    // END Pyramid

    // Cylinders -------------------------------------------------------------------------------------------------

    //Cylinder - Pencil Body
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, PencilBody); // Set Active Texture
    glBindVertexArray(pencilBodyMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(3.0f, 0.175f, 1.5f)) *    // Change object position (Translate)
        glm::rotate(-15.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(0.2f, 0.2f, 0.75f));           // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawElements(GL_TRIANGLES, pencilBodyMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);  // Deativate the VAO
    //End cylinder

    //Cylinder - Pencil Tip
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, PencilCut); // Set Active Texture
    glBindVertexArray(pencilTipMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(2.076f, 0.175f, 0.418f)) *    // Change object position (Translate)
        glm::rotate(-15.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));           // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawElements(GL_TRIANGLES, pencilTipMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);  // Deativate the VAO
    //End cylinder

    //Cylinder - Thread Side
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, threadTexture); // Set Active Texture
    glBindVertexArray(threadMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(-4.0f, 0.3f, 0.0f)) *    // Change object position (Translate)
        glm::rotate(-45.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(0.2f, 0.2f, 0.2f));           // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawElements(GL_TRIANGLES, threadMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);  // Deativate the VAO
    //End cylinder

    //Cylinder - Thread Spool Top
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, spoolWood); // Set Active Texture
    glBindVertexArray(spoolMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(-3.725f, 0.3f, -0.175f)) *    // Change object position (Translate)
        glm::rotate(-45.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(0.3f, 0.3f, 0.2f));           // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawElements(GL_TRIANGLES, spoolMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);  // Deativate the VAO
    //End cylinder

    //Cylinder - Thread Spool bottom
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, spoolWood); // Set Active Texture
    glBindVertexArray(spoolBMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(-4.275f, 0.3f, 0.175f)) *    // Change object position (Translate)
        glm::rotate(-45.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(0.3f, 0.3f, 0.2f));           // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawElements(GL_TRIANGLES, spoolBMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);  // Deativate the VAO
    //End cylinder

    //Cylinder - Magnifying Glass Ring
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, lightWood); // Set Active Texture
    glBindVertexArray(glassRingMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(1.0f, 0.075f, -3.0f)) *    // Change object position (Translate)
        glm::rotate(1.5713f, glm::vec3(1.0f, 0.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(0.75f, 0.75f, 0.25f));           // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawElements(GL_TRIANGLES, glassRingMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);  // Deativate the VAO
    //End cylinder

    //Cylinder - Glass
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, Glass); // Set Active Texture
    glBindVertexArray(glassMesh.vao); // Bind VAO
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(1.0f, 0.08f, -3.0f)) *    // Change object position (Translate)
        glm::rotate(1.5713f, glm::vec3(1.0f, 0.0f, 0.0f)) * // Change object rotation
        glm::scale(glm::vec3(0.70f, 0.70f, 0.25f));           // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawElements(GL_TRIANGLES, glassMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
    glBindVertexArray(0);  // Deativate the VAO
    //End cylinder

    // Planes ------------------------------------------------------------------------------------------------------

    //Plane - Table
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, darkWood); // Set Active Texture
    glBindVertexArray(tableMesh.vao);
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)) *      // Change object position (Translate) 
        glm::rotate(0.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *   // Change object rotation
        glm::scale(glm::vec3(10.0f, 1.0f, 10.0f));         // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, tableMesh.nVertices);
    glBindVertexArray(0);  // Deativate the VAO
    // END Plane

    //Plane - Paper
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, PaperTexture); // Set Active Texture
    glBindVertexArray(paperMesh.vao);
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(2.0f, 0.01f, 1.5f)) *      // Change object position (Translate) 
        glm::rotate(-10.0f, glm::vec3(0.0f, 1.0f, 0.0f)) *   // Change object rotation
        glm::scale(glm::vec3(3.0f, 1.0f, 5.0f));         // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, paperMesh.nVertices);
    glBindVertexArray(0);  // Deativate the VAO
    // END Plane

    //Plane - Paper 2
    glActiveTexture(GL_TEXTURE0); // Set Active Texture Location
    glBindTexture(GL_TEXTURE_2D, PaperTexture); // Set Active Texture
    glBindVertexArray(paperBMesh.vao);
    model = glm::mat4(1.0f); // Set Identity Matrix
    model = glm::translate(glm::vec3(2.0f, 0.001f, 1.5f)) *      // Change object position (Translate) 
        glm::rotate(-9.9f, glm::vec3(0.0f, 1.0f, 0.0f)) *   // Change object rotation
        glm::scale(glm::vec3(3.0f, 1.0f, 5.0f));         // Change object scale
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model)); // Set Transformation Uniform before Draw
    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, paperBMesh.nVertices);
    glBindVertexArray(0);  // Deativate the VAO
    // END Plane

    // Draw Light Sources---------------------------------------------------------------------------------------------

     // Begin Lamp A (Key Light)
    glUseProgram(gLampProgramId); // Switch Shaders
    glBindVertexArray(lampMeshA.vao); // Activate First Lamp Mesh

    model = glm::translate(gLightPositionA) * glm::scale(gLightScaleA); // Transform cube used as visual indicator

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, lampMeshA.nVertices); // Drawn Visual Cube

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);
    // END Lamp A (Key)

    //Begin Lamp B (Fill)
    glUseProgram(gLampProgramId); //Activate Lamp Shader
    glBindVertexArray(lampMeshB.vao); // Bind Mesh

    model = glm::translate(gLightPositionB) * glm::scale(gLightScaleB); // Transform cube used as visual indicator

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, lampMeshB.nVertices); // Draw Visual Cube

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);
    // END Lamp B (Fill)

    // END Light Sources and Primitives ----------------------------------------------------------------------------------

    // Call GLFW to swap buffers while checking for input
    glfwSwapBuffers(gWindow);    // Swap Back buffer with Front buffer
}

// Define UCreateShaderProgram
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId) {
    // Compilation and error reporting
    int success = 0;
    char infoLog[512];

    // Create Object to hold Shader Program
    programId = glCreateProgram();

    // Create Two Shader Objects: Vertex Shader and Fragment Shader
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER); // Vertex
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER); // Fragment

    // Fetch Shader Source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile vertex shader
    glCompileShader(vertexShaderId);

    // Shader Compilation Error Check
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success) {  //Fail to create
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog); //Gather Log Data
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl; //Print Error Data

        return false; //Error Handle
    }

    // Compile fragment shader
    glCompileShader(fragmentShaderId);

    // Shader Compulation Error Check
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);

    if (!success) { // Fail to create
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog); // Gather Log Data
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl; //Print Error Data

        return false; // Error Handle
    }

    // Attach compiled shaders to shader program
    glAttachShader(programId, vertexShaderId); //Vertex
    glAttachShader(programId, fragmentShaderId); //Fragment

    glLinkProgram(programId);   // Link Shader Program

    // Link Error Check
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) { // Fail to link
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog); // Gather Data
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl; //Print Data

        return false; // Error Handle
    }

    glUseProgram(programId);    // Use Shader Program

    return true;
}

//Load Shaders
bool ULoadShaders() {
    // Create shader program
    if (!UCreateShaderProgram(objectVertexShaderSource, objectFragmentShaderSource, gProgramId)) // Error check
        return EXIT_FAILURE; // Error Handle

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;
}

//Delete Shaders
void UDestroyShaderProgram(GLuint programId) {
    glDeleteProgram(programId); // Delete Shader Program
}

// Images are loaded with y axis down by default, this flips to y going up
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

/*Generate the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}

//Load Textures
bool ULoadTextures() {
    // Load texture (relative to project's directory)
    const char* texFilename = "resources/galaxyTexture.jpg";   // Texture 1
    if (!UCreateTexture(texFilename, galTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "resources/PencilBody.jpg";   // Texture 2
    if (!UCreateTexture(texFilename, PencilBody))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "resources/PencilCut.jpg";   // Texture 3
    if (!UCreateTexture(texFilename, PencilCut))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "resources/PaperTexture.jpg";   // Texture 5
    if (!UCreateTexture(texFilename, PaperTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "resources/darkWood.jpg";   // Texture 6
    if (!UCreateTexture(texFilename, darkWood))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "resources/Thread.jpg";   // Texture 6
    if (!UCreateTexture(texFilename, threadTexture))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "resources/spoolWood.jpg";   // Texture 6
    if (!UCreateTexture(texFilename, spoolWood))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "resources/lightWood.jpg";   // Texture 6
    if (!UCreateTexture(texFilename, lightWood))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    texFilename = "resources/Glass.jpg";   // Texture 6
    if (!UCreateTexture(texFilename, Glass))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // End Load Textures
}

//Destroy Texture Objects
void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

// Initialize GLFW, GLEW, and create window
bool UInitialize(int argc, char* argv[], GLFWwindow** window) {
    // Initialize GLFW with version info and specify core profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__ // Check if Mac OS
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW create window
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);

    if (*window == NULL) { //Error Check
        cout << "Failed to create GLFW window" << endl; // Error Handle
        glfwTerminate();
        return false;
    }
    // Set window to current (place on top)
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow); // Handle window size changes/ scaling
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // if using GLEW version 1.13 or earlier
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult) { // Error Check 
        cerr << glewGetErrorString(GlewInitResult) << endl; // Error Handle
        return false;
    }

    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl; // Displays GPU OpenGL version

    return true;
}

// Process input
//Keyboard Inputs
void UProcessInput(GLFWwindow* window) {

    static const float cameraSpeed = 2.5f; // Camera Speed Variable

    // Key Binds
    //-------------
    // Exit Program
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)          // ESC: Exit Key
        glfwSetWindowShouldClose(window, true);

    // Camera Movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {             // W: Move Camera Forward
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {             // S: Move Camera Back
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {             // A: Move Camera Left
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {             // D: Move Camera Right
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {             // Q: Move Camera Down
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {             // E: Move Camera Up
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    }
    //Change Perspective Mode
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        isOrtho = false;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        isOrtho = true;
    }

    // Light Cube A Movement
        // While NOT Holding Left Shift
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {      //Down Arrow Key: Move Cube in -Y Direction
            gLightPositionA.y -= 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {        //Up Arrow Key: Move Cube in +Y Direction
            gLightPositionA.y += 0.1f;
        }
    }
    // While Holding Left Shift
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {        //Up Arrow Key: Move Cube in +Z Direction
            gLightPositionA.z -= 0.1f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {      //Down Arrow Key: Move Cube in -Z Direction
            gLightPositionA.z += 0.1f;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {          //Left Arrow Key: Move Cube in -X Direction
        gLightPositionA.x -= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {         //Right Arrow Key: Move Cube in +X Direction
        gLightPositionA.x += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {             // R Key: Reset Cube to Starting Position
        gLightPositionA.x = 0.0f;
        gLightPositionA.y = 5.0f;
        gLightPositionA.z = 5.0f;

        gLightPositionB.x = 0.0f;
        gLightPositionB.y = 5.0f;
        gLightPositionB.z = -5.0f;
    }

    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting) { //L Key: Start Lighting Rotation
        gIsLampOrbiting = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting) { //K Key: Stop Lighting Rotation
        gIsLampOrbiting = false;
    }
}

//Mouse Inputs
// GLFW: Check Mouse Position and Translate movement
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (gFirstMouse) {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

// GLFW: Mouse Scroll Checking
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Mouse Scroll Wheel adjusts zoom, which affects speed.
    gCamera.ProcessMouseScroll(yoffset);
}

// GLFW: Mouse Button Binds
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

// Call to GLFW to process window resize (by user or system)
void UResizeWindow(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Define UCreateMesh function
void UCreatePyramid(GLMesh& mesh) {

    // Specifies Normalized Device Coordinates (x,y,z) and color (r,g,b,a) for triangle vertices
    GLfloat verts[] = {

        // Vertex Positions:
        // Vertex 0 ( -0.5f, -0.5f,  0.5f ) = Left Front Point
        // Vertex 1 ( -0.5f, -0.5f, -0.5f ) = Left Back Point
        // Vertex 2 (  0.5f, -0.5f, -0.5f ) = Right Back Point
        // Vertex 3 (  0.5f, -0.5f,  0.5f ) = Right Front Point
        // Vertex 4 (  0.0f,  0.5f,  0.0f ) = Top point

        // Vertex Positions     //Normals           //Texture Coordinates
        //Front Face            //Pos Z Normal
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // Vertex 0
         0.0f,  0.5f,  0.0f,    0.0f,  0.0f,  1.0f,   0.5f, 1.0f, // Vertex 4   Front Triangle
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // Vertex 3

         //Left Face            //Neg X Normal
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Vertex 0
         0.0f,  0.5f,  0.0f,   -1.0f,  0.0f,  0.0f,   0.5f, 1.0f, // Vertex 4   Left Triangle
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // Vertex 1

         //Back Face            //Neg Z Normal
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // Vertex 1
         0.0f,  0.5f,  0.0f,    0.0f,  0.0f, -1.0f,   0.5f, 1.0f, // Vertex 4   Back Triangle
         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f, // Vertex 2

         //Right Face           //Pos X Normal
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Vertex 2
         0.0f,  0.5f,  0.0f,    1.0f,  0.0f,  0.0f,   0.5f, 1.0f, // Vertex 4   Right Triangle
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // Vertex 3

         //Bottom Face          //Neg Y Normal
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f, // Vertex 1
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 0.0f, // Vertex 0   Bottom Triangle 1
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // Vertex 3

        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f, // Vertex 1
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // Vertex 2   Bottom Triangle 2
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f  // Vertex 3

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

// Cube
void UCreateCube(GLMesh& mesh) {

    // Point Coordinates and Texture Coordinates
    GLfloat verts[] = {

        // Vertex Positions:
        // Vertex 0 ( -0.5f, -0.5f,  0.5f ) = Bottom Left Front
        // Vertex 1 ( -0.5f, -0.5f, -0.5f ) = Bottom Left Back
        // Vertex 2 (  0.5f, -0.5f, -0.5f ) = Bottom Right Back
        // Vertex 3 (  0.5f, -0.5f,  0.5f ) = Bottom Right Front
        // Vertex 4 ( -0.5f,  0.5f,  0.5f ) = Top Left Front
        // Vertex 5 ( -0.5f,  0.5f, -0.5f ) = Top Left Back
        // Vertex 6 (  0.5f,  0.5f, -0.5f ) = Top Right Back
        // Vertex 7 (  0.5f,  0.5f,  0.5f ) = Top Right Front


        // Vertex Positions     //Normals           //Texture Coordinates

        //Bottom Face           //Neg Y Normal
        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 0.0f, // Vertex 0
        -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 1.0f, // Vertex 1   Bottom Back Triangle
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // Vertex 2

        -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   0.0f, 0.0f, // Vertex 0
         0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 0.0f, // Vertex 3   Bottom Front Triangle
         0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,   1.0f, 1.0f, // Vertex 2

         //Front Face           //Pos Z Normal
        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // Vertex 0
         0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 0.0f, // Vertex 3   Front Bottom Triangle
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // Vertex 7

        -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 0.0f, // Vertex 0
        -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   0.0f, 1.0f, // Vertex 4   Front Top Triangle
         0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,   1.0f, 1.0f, // Vertex 7

         //Left Face            //Neg X Normal
        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // Vertex 0
        -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Vertex 1   Left Bottom Triangle
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // Vertex 5

        -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // Vertex 0
        -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // Vertex 4   Left Top Triangle
        -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Vertex 5

         //Back Face            //Neg Z Normal
        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f, // Vertex 1
         0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 0.0f, // Vertex 2   Back Bottom Triangle
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 1.0f, // Vertex 6

        -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 0.0f, // Vertex 1
        -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   1.0f, 1.0f, // Vertex 5   Back Top Triangle
         0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,   0.0f, 1.0f, // Vertex 6

         //Right Face           //Pos X Normal
         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // Vertex 2
         0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 0.0f, // Vertex 3   Right Bottom Triangle
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // Vertex 7

         0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 0.0f, // Vertex 2
         0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,   1.0f, 1.0f, // Vertex 6   Right Top Triangle
         0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,   0.0f, 1.0f, // Vertex 7

         //Top Face             //Pos Y Normal
        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 0.0f, // Vertex 4
        -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 1.0f, // Vertex 5   Top Back Triangle
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 1.0f, // Vertex 6

        -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   0.0f, 0.0f, // Vertex 4
         0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 0.0f, // Vertex 7   Top Front Triangle
         0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,   1.0f, 1.0f  // Vertex 6

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

}

//Plane
void UCreatePlane(GLMesh& mesh) {

    // Point Coordinates and Texture Coordinates
    GLfloat verts[] = {

        // Vertex Positions:
        // Vertex 0 ( -0.5f, 0.0f,  0.5f ) = Left Front Point
        // Vertex 1 ( -0.5f, 0.0f, -0.5f ) = Left Back Point
        // Vertex 2 (  0.5f, 0.0f, -0.5f ) = Right Back Point
        // Vertex 3 (  0.5f, 0.0f,  0.5f ) = Right Front Point

        // Vertex Positions    //Neg Y Normal    //Texture Coordinates
        -0.5f,  0.0f,  0.5f,   0.0f, 1.0f,  0.0f,    0.0f, 0.0f, // Vertex 0
        -0.5f,  0.0f, -0.5f,   0.0f, 1.0f,  0.0f,    0.0f, 1.0f, // Vertex 1   Back Triangle
         0.5f,  0.0f, -0.5f,   0.0f, 1.0f,  0.0f,    1.0f, 1.0f, // Vertex 2

        -0.5f,  0.0f,  0.5f,   0.0f, 1.0f,  0.0f,   0.0f, 0.0f, // Vertex 0
         0.5f,  0.0f,  0.5f,   0.0f, 1.0f,  0.0f,   1.0f, 0.0f, // Vertex 3   Front Triangle
         0.5f,  0.0f, -0.5f,   0.0f, 1.0f,  0.0f,   1.0f, 1.0f, // Vertex 2

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

//Cylinder
void UCreateCylinder(GLMesh& mesh, GLfloat tRad, GLfloat bRad, GLfloat h, GLint slices, GLint stacks) {

    GLfloat tRadius = tRad;
    GLfloat bRadius = bRad;
    GLfloat height = h;

    GLint numSlices = slices;
    GLint numStacks = stacks;


    Cylinder cylinder(tRadius, bRadius, height, numSlices, numStacks);        // baseRadius, topRadius, height, slices, stacks

    //cylinder.printSelf(); //Use for Debug and triangle count

    mesh.nVertices = cylinder.getIVertSize() / sizeof(cylinder.getIVerts()) * (3 + 3 + 2);

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, cylinder.getIVertSize(), cylinder.getIVerts(), GL_STATIC_DRAW);

    // Buffer Object for Indices
    //mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    mesh.nIndices = cylinder.getIndexSize();

    //Index data buffer
    glGenBuffers(1, &mesh.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo); // Activate Buffer (1)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder.getIndexSize(), cylinder.getIndices(), GL_STATIC_DRAW); // Send index data to GPU

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cylinder.getIStride(), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cylinder.getIStride(), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, cylinder.getIStride(), (void*)(sizeof(float) * (3 + 3)));
    glEnableVertexAttribArray(2);

}

// Mesh destruction
void UDestroyMesh(GLMesh& mesh) {
    glDeleteVertexArrays(1, &mesh.vao); // Delete Vertex Array
    glDeleteBuffers(1, &mesh.vbo); // Delete Buffers
}

// END