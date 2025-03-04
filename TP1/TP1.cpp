// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unistd.h>
// Include GLEW
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;
#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "common/stb_image.h"
#define STB_PERLIN_IMPLEMENTATION
#include "common/stb_perlin.h"
#include <cstdlib> 
#include <ctime> 
using namespace std;

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 camera_position   = glm::vec3(0.0f, 1.0f,  3.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f,  0.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//rotation
float angle = 0.;
float zoom = 1.;
/*******************************************************************************/

int nbSommetx = 16;
int nbSommety = 16;
vector<vec3> vertex;
vector<unsigned int> indices;
vector<vec2> texCoords;
GLuint vao = 0, vbo = 0, vboTex = 0, ebo = 0;
GLuint texture = 0;
GLuint programID = 0;
GLuint textureHeight = 0;
GLuint textureRock = 0;
GLuint textureNeige = 0;

bool keyEqualPressed = false;
bool key6Pressed = false;

//pour cam orbital

bool orbitalMode = false;       
float rotationSpeed = 0.5f;      
float rotationAngle = 0.0f;      
float cameraHeight = 1.0f;       
float cameraDistance = 3.0f;     
bool keyCPressed = false;      
glm::mat4 viewOrbi;

float seedX = (std::rand() % 10000) / 1000.0f;
float seedY = (std::rand() % 10000) / 1000.0f;


float noise(float x, float y, float scale, float amplitude) {
    return stb_perlin_noise3((x + seedX) * scale, (y + seedY) * scale, 0.0f, 0, 0, 0) * amplitude;
}


void computeGeometryAndTexture() {
    vertex.clear();
    indices.clear();
    texCoords.clear();

    float pasU = 1.0f / nbSommetx;
    float pasV = 1.0f / nbSommety;

    for (int y = 0; y < nbSommety; y++) {
        for (int x = 0; x < nbSommetx; x++) {
            float fx = (float)x / (nbSommetx - 1) - 0.5f; 
            float fy = (float)y / (nbSommety - 1) + 0.5f;


            float U = (float)x / (nbSommetx - 1); 
            float V = (float)y / (nbSommety - 1);
            //float height = noise(x, y, 0.35f, 0.15f); 

            vertex.push_back(vec3(fx,0.0f,fy));
            texCoords.push_back(vec2(U, V));
        }
    }

    for (int y = 0; y < nbSommety - 1; y++) {
        for (int x = 0; x < nbSommetx - 1; x++) {
            int A = y * nbSommetx + x;
            int B = A + 1;
            int D = A + nbSommetx;
            int C = D + 1;

            indices.push_back(A);
            indices.push_back(B);
            indices.push_back(D);

            indices.push_back(B);
            indices.push_back(C);
            indices.push_back(D);
        }
    }

    glGenVertexArrays(1, &vao);  // on fait le VAO (le buffer principal auquels les suivants sont associés)
    glBindVertexArray(vao);

    // on fait le VBO des sommets
    glGenBuffers(1, &vbo); // on fait 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo); // on associe
    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(vec3), vertex.data(), GL_STATIC_DRAW); // on remplie
    glEnableVertexAttribArray(0); // indice auquel le buf est lié
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

    // Création du VBO des coordonnées de texture de l'herbe
    glGenBuffers(1, &vboTex);
    glBindBuffer(GL_ARRAY_BUFFER, vboTex);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(vec2), texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);

    // Création de l'EBO (décrit comment les triangles sont construien)
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Chargement de la texture
    int width, height, nrChannels;
    int widthHeightMap, heightHeightMap, nrChannelsHeightMap;
    int widthRock, heightRock, nrChannelsRock;
    int widthNeige, heightNeige, nrChannelsNeige;


    unsigned char *data = stbi_load("../textures/grass.png", &width, &height, &nrChannels, 0);

    if (data) { // Vérification du chargement
        glGenTextures(1, &texture); // on génère la texture
        glBindTexture(GL_TEXTURE_2D, texture); // on associe la texture
        
        // les param de texture pour que ça soit joli 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); // on donne les data a la texture
        glGenerateMipmap(GL_TEXTURE_2D); // on fait des mim map pour que ça soit plus joli

        stbi_image_free(data);
    } else {
        std::cerr << "Erreur : Impossible de charger la texture !" << std::endl;
    }



    unsigned char *dataRock = stbi_load("../textures/rock.png", &widthRock, &heightRock, &nrChannelsRock, 0);
    if (dataRock) {
        glGenTextures(1, &textureRock);
        glBindTexture(GL_TEXTURE_2D, textureRock);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthRock, heightRock, 0, GL_RGB, GL_UNSIGNED_BYTE, dataRock);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(dataRock);
    } else {
        std::cerr << "Erreur : Impossible de charger la texture !" << std::endl;
    }


    unsigned char *dataNeige = stbi_load("../textures/snowrocks.png", &widthNeige, &heightNeige, &nrChannelsNeige, 0);
    if (dataNeige) {
        glGenTextures(1, &textureNeige);
        glBindTexture(GL_TEXTURE_2D, textureNeige);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthNeige,heightNeige, 0, GL_RGB, GL_UNSIGNED_BYTE, dataNeige);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(dataNeige);
    } else {
        std::cerr << "Erreur : Impossible de charger la texture !" << std::endl;
    }


    unsigned char *dataHeightMap = stbi_load("../textures/Heightmap_Rocky.png", &widthHeightMap, &heightHeightMap, &nrChannelsHeightMap, 0);
    if (dataHeightMap) {
        glGenTextures(1, &textureHeight);
        glBindTexture(GL_TEXTURE_2D, textureHeight);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthHeightMap, heightHeightMap, 0, GL_RGB, GL_UNSIGNED_BYTE, dataHeightMap);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(dataHeightMap);
    } else {
        std::cerr << "Erreur : Impossible de charger la heightMap !" << std::endl;
    }



    programID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");

    glUseProgram(programID);
    // Activation de la texture pour l'envoyer au shader

    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(programID, "textureGrass"), 0);

    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRock);
    glUniform1i(glGetUniformLocation(programID, "textureRock"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureNeige);
    glUniform1i(glGetUniformLocation(programID, "textureNeige"), 2);


    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureHeight);
    glUniform1i(glGetUniformLocation(programID, "heightMap"), 3);


    // Dé-liaison du VAO
    glBindVertexArray(0);
}



void draw() {
    glUseProgram(programID);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRock);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureNeige);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureHeight);
    
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void clearBuf() { 
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &vboTex);
        glDeleteBuffers(1, &ebo);
        glDeleteTextures(1, &texture);
        glDeleteTextures(1, &textureHeight);
        glDeleteTextures(1, &textureRock);
        glDeleteTextures(1, &textureNeige);

    }
    if (programID != 0) {
        glDeleteProgram(programID);
    }
}

    void computeMVP(mat4 &MVP){
        glm::mat4 model = glm::mat4();

        glm::mat4 view = glm::lookAt(
            camera_position,
            camera_target,
            camera_up
        );
        
        mat4 proj = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

 
        MVP = proj * view * model;

        
    }

    void updateOrbitalCamera()
    {
        vec3 terrain_centre = {0,0,0};

        if (orbitalMode) {
            rotationAngle += rotationSpeed * deltaTime;
            
            if (rotationAngle > 2 * M_PI) {
                rotationAngle -= 2 * M_PI;
            }  
            
            camera_position.x = terrain_centre.x + cos(rotationAngle);
            camera_position.y = 4; 
            camera_position.z = terrain_centre.z + sin(rotationAngle);
            
            camera_target = terrain_centre - camera_position;
            camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }


int main(void)
{
    // Initialise GLFW
    if(!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }


    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP1 - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
   

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);


    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    mat4 MVP ;
    computeMVP(MVP);

    GLint uniMvp = glGetUniformLocation(programID, "mvp");
    glUniformMatrix4fv(uniMvp, 1, GL_FALSE, value_ptr(MVP));

    // Compute and generate the surface geometry and texture associated
    computeGeometryAndTexture();

    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        updateOrbitalCamera();


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use our shader
        glUseProgram(programID);

        // Update MVP matrix
        computeMVP(MVP);

        GLint uniMvp = glGetUniformLocation(programID, "mvp");        
        glUniformMatrix4fv(uniMvp, 1, GL_FALSE, value_ptr(MVP));
        
        glUniform1i(glGetUniformLocation(programID, "textureGrass"), 0);
        glUniform1i(glGetUniformLocation(programID, "textureRock"), 1);
        glUniform1i(glGetUniformLocation(programID, "textureNeige"), 2);
        glUniform1i(glGetUniformLocation(programID, "heightMap"), 3);

        
        // Draw the surface
        draw();

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && 
            glfwWindowShouldClose(window) == 0);

    // Cleanup


    clearBuf();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }


    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!keyCPressed) {
            keyCPressed = true;
            orbitalMode = !orbitalMode;
            printf("Camera mode changed to: %s\n", orbitalMode ? "Orbital" : "Free");

            if (orbitalMode) {
                camera_position = glm::vec3(0.0f, 5.0f, 0.0f);
                camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
                camera_up = glm::vec3(0.0f, 0.0f, -1.0f); 
            } 
            else {
                camera_position = glm::vec3(0.0f, 2.0f, 5.0f);
                camera_target = glm::vec3(0.0f, 0.0f, 0.0f);
                camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
            }
        }
    } else {
        keyCPressed = false;
    }

    // Gestion des flèches pour modifier la vitesse de rotation en mode orbital
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        rotationSpeed += 0.1f * deltaTime;
        printf("Rotation speed: %.2f\n", rotationSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        rotationSpeed -= 0.1f * deltaTime;
        if (rotationSpeed < 0.0f) rotationSpeed = 0.0f;
        printf("Rotation speed: %.2f\n", rotationSpeed);
    }



    if (!orbitalMode) {
        float cameraSpeed = 2.5f * deltaTime;
        glm::vec3 direction = glm::normalize(camera_target - camera_position);
        glm::vec3 right = glm::normalize(glm::cross(direction, camera_up));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera_position += direction * cameraSpeed;
            camera_target += direction * cameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera_position -= direction * cameraSpeed;
            camera_target -= direction * cameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera_position -= right * cameraSpeed;
            camera_target -= right * cameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera_position += right * cameraSpeed;
            camera_target += right * cameraSpeed;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        if (!keyEqualPressed) {
            keyEqualPressed = true;
            nbSommetx += 1;
            nbSommety += 1;
            clearBuf();
            computeGeometryAndTexture();
            printf("res augmenté %d,%d\n", nbSommetx, nbSommety);
        }
    } else {
        keyEqualPressed = false;
    }
    

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        if (!key6Pressed) {
            key6Pressed = true;
            if (nbSommetx > 2 && nbSommety > 2) { 
                nbSommetx -= 1;
                nbSommety -= 1;
                clearBuf();
                computeGeometryAndTexture();
                printf("res diminué to %d,%d\n", nbSommetx, nbSommety);
            }
        }
    } else {
        key6Pressed = false;
    }
}




// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}