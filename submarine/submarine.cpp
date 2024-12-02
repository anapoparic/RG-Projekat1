#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#define CRES 360
#define M_PI 3.141592653589793
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <cstdlib>  
#include <ctime> 
#include <map>

#include <GL/glew.h>   
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // For transformations like glm::rotate
#include <glm/gtc/type_ptr.hpp>  
#include "stb_image.h"
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std;
using namespace glm;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ GLOBALS +++++++++++++++++++++++++++++++++++++++++++++++++

GLenum err;

//Window 
GLFWwindow* window;
float aspectRatio;
void initializeWindow();

//Shaders
unsigned int sonarShader;
unsigned int textureShader;
unsigned int dotShader;
unsigned int progressShader;
unsigned int textShader;
unsigned int lightShader;
unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);

//Textures
static unsigned loadImageToTexture(const char* filePath);
void setupTexture(unsigned int texture);
float lastTimeTextureToggled = 0.0f;
bool toggleTexture = false;

//Click
void mouseClickCallback(GLFWwindow* window, int button, int action, int mods);

//Data
unsigned int sonarVAO;
unsigned int sonarVBO;

unsigned int linesVAO;
unsigned int linesVBO;

unsigned int needleVAO;
unsigned int needleVBO;

unsigned int smallerCirclesVAO[4];  
unsigned int smallerCirclesVBO[4];

unsigned int circleTextureVAO;
unsigned int circleTextureVBO;
unsigned int circleTextureEBO;

unsigned int dotVAO;
unsigned int dotVBO;

unsigned int progressBarVAO;
unsigned int progressBarVBO;

unsigned int textVAO;
unsigned int textVBO;

unsigned int warningMessageVAO;
unsigned int warningMessageVBO;
unsigned int warningMessageEBO;

unsigned int lightVAO;
unsigned int lightVBO;

//SONAR 
float currentTime = 0.0f;
float dotSpawnRate = 0.8f;
float lastDotSpawnTime = 0.0f;

struct Dot {
    vec2 position;
    float timeRemaining;
};

vector<Dot> activeDots;
void createDot();

//STOP TEXTURE GLOBALS
vector<unsigned int> indices;
bool stopClicked = true;
float stopTime = 0.0f;
float needleAngle = 0.0f;

//PROGRESS BAR
const int SEGMENTS = 100; 
float progress = 0.0f;
float oxygen = 1.0f;

float lastTimeOxygenDecremented = 0.0f;
bool isOxygenLow = false;

bool isWPressed = false;
bool isSPressed = false;
void updateProgressBar(float offsetX, float progress, bool interpolateColor);

//FREE TYPE
struct Character {
    GLuint TextureID; // Texture ID for the glyph
    fvec2 Size;  // Size of glyph
    fvec2 Bearing; // Offset from the baseline to the top-left of the character
    int Advance;  // Horizontal offset to advance to the next character
};

map<GLchar, Character> Characters; // Holds the characters for the font
void renderText(const char* text, float x, float y, float scale);
int maxChars = 128;

FT_Library ft;
FT_Face face;
void initFreeType();
GLuint loadTextureFromFreeType(FT_Bitmap bitmap);



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ SET METHODS +++++++++++++++++++++++++++++++++++++++++++++++++

void setSonarVAO();

void setSonarVAO() {
    float vertices[CRES * 2];
    for (int i = 0; i < CRES; i++) {
        float angle = (2.0f * M_PI * i) / CRES;
        vertices[i * 2] = 0.5f * cos(angle); // X
        vertices[i * 2 + 1] = 0.5f * sin(angle); // Y
    }
    glGenVertexArrays(1, &sonarVAO);
    glGenBuffers(1, &sonarVBO);

    glBindVertexArray(sonarVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sonarVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setLinesVAO();

void setLinesVAO() {
    float vertices[] = {
        -0.5f, 0.0f,   //Horizontal
         0.5f, 0.0f,   
         0.0f, -0.5f,  // Vertical
         0.0f, 0.5f    
    };

    glGenVertexArrays(1, &linesVAO);
    glGenBuffers(1, &linesVBO);

    glBindVertexArray(linesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, linesVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setSmallerCirclesVAO();

void setSmallerCirclesVAO() {
   
    float radii[4] = { 0.1f, 0.2f, 0.3f, 0.4f };  
    float vertices[CRES * 2];  

    for (int j = 0; j < 4; j++) {  
        for (int i = 0; i < CRES; i++) {
            float angle = (2.0f * M_PI * i) / CRES;
            vertices[i * 2] = radii[j] * cos(angle);  // X coordinate
            vertices[i * 2 + 1] = radii[j] * sin(angle);  // Y coordinate
        }

        glGenVertexArrays(1, &smallerCirclesVAO[j]);
        glGenBuffers(1, &smallerCirclesVBO[j]);

        glBindVertexArray(smallerCirclesVAO[j]);
        glBindBuffer(GL_ARRAY_BUFFER, smallerCirclesVBO[j]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setNeedleVAO();

void setNeedleVAO() {
    float angle1 = 0.45f;
    float angle2 = -0.45f;
    float radius = 0.5f;    

    float numVertices = 100; // smooth arc
    vector<float> vertices; 

    // Center vertex with maximum red intensity
    vertices.push_back(0.0f); // x
    vertices.push_back(0.0f); // y
    vertices.push_back(1.0f); 

    // Generate vertices along the arc
    for (float i = 0; i <= numVertices; i++) {
        float t = i / numVertices; 
        float angle = angle1 + t * (angle2 - angle1);
        float x = radius * cos(angle);
        float y = radius * sin(angle);

        float fadeFactor = pow(1.0f - t, 2.0f);  //fade from center to edge
     
        vertices.push_back(x);          
        vertices.push_back(y);         
        vertices.push_back(fadeFactor); 
    }
    glGenVertexArrays(1, &needleVAO);
    glGenBuffers(1, &needleVBO);

    glBindVertexArray(needleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, needleVBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);     
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setCircleTextureVAO();

void setCircleTextureVAO() {
    float radius = 0.07f; 
    float textureCenterX = 0.5f, textureCenterY = 0.5f; // Texture center for UV mapping

    vector<float> vertices;

    // Center vertex
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(textureCenterX);
    vertices.push_back(textureCenterY);

    // Generate circle vertices
    for (int i = 0; i <= CRES; ++i) {
        float angle = (2.0f * M_PI * i) / CRES;
        float x = radius * cos(angle) / aspectRatio;
        float y = radius * sin(angle);
        float u = textureCenterX + 0.5f * cos(angle); // Map circle to texture coordinates
        float v = textureCenterY + 0.5f * sin(angle);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(u);
        vertices.push_back(v);

        if (i > 0) {
            indices.push_back(0); //Center         
            indices.push_back(i); // Point on edge
            indices.push_back(i + 1); // Next point on edge
        }
    }

    indices.back() = 1;

    glGenVertexArrays(1, &circleTextureVAO);
    glGenBuffers(1, &circleTextureVBO);
    glGenBuffers(1, &circleTextureEBO);

    glBindVertexArray(circleTextureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleTextureVBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleTextureEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setDotVAO();

void setDotVAO() {
    float vertices[32 * 2]; 
    float dotSize = 0.008f;

    for (float i = 0; i < 32; ++i) {
        float angle = 2.0f * M_PI * i / 32.0f;
        vertices[int(i) * 2] = dotSize * cos(angle) / aspectRatio; 
        vertices[int(i) * 2 + 1] = dotSize * sin(angle);           
    }

    glGenVertexArrays(1, &dotVAO);
    glGenBuffers(1, &dotVBO);

    glBindVertexArray(dotVAO);
    glBindBuffer(GL_ARRAY_BUFFER, dotVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);  
}

void setProgressBarVAO(float offsetX, float r, float g, float b);

void setProgressBarVAO(float offsetX, float r, float g, float b) {
    float vertices[SEGMENTS * 4 * 5];
    float length = 1.0f;

    int vertexIndex = 0;
    float angleStep = length * 2.0f * M_PI / (SEGMENTS - 1);

    for (int i = 0; i < SEGMENTS; ++i) {
        float angle = i * angleStep;

        float outerX = cos(angle);
        float outerY = sin(angle);
        float innerX = cos(angle);
        float innerY = sin(angle);

        // Outer vertex
        vertices[vertexIndex++] = outerX + offsetX;
        vertices[vertexIndex++] = outerY;
        vertices[vertexIndex++] = r;
        vertices[vertexIndex++] = g;
        vertices[vertexIndex++] = b;

        // Inner vertex
        vertices[vertexIndex++] = innerX + offsetX;
        vertices[vertexIndex++] = innerY;
        vertices[vertexIndex++] = r;
        vertices[vertexIndex++] = g;
        vertices[vertexIndex++] = b;
    }

    glBindVertexArray(progressBarVAO);
    glBindBuffer(GL_ARRAY_BUFFER, progressBarVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setTextVAO();

void setTextVAO() {
    const char* charactersToRender = "0123456789m%";

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * maxChars, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);             
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); 
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    for (int i = 0; charactersToRender[i] != '\0'; ++i) {
        if (FT_Load_Char(face, charactersToRender[i], FT_LOAD_RENDER | FT_LOAD_TARGET_LCD)) {
            std::cerr << "Failed to load glyph for character: " << charactersToRender[i] << std::endl;
            continue; // Skip to the next character
        }

        GLuint texture = loadTextureFromFreeType(face->glyph->bitmap);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters[charactersToRender[i]] = character;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void setWarningMessageVAO();

void setWarningMessageVAO() {
    float width = 0.25f;
    float height = 0.25f;

    // (x, y, u, v)
    float vertices[] = {
        -width / 2, -height / 2, 0.0f, 0.0f, // Bottom-left
        width / 2, -height / 2, 1.0f, 0.0f, // Bottom-right
        width / 2, height / 2, 1.0f, 1.0f, // Top-right
        -width / 2, height / 2, 0.0f, 1.0f  // Top-left
    };

    indices.clear();
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);

    glGenVertexArrays(1, &warningMessageVAO);
    glGenBuffers(1, &warningMessageVBO);
    glGenBuffers(1, &warningMessageEBO);

    glBindVertexArray(warningMessageVAO);
    glBindBuffer(GL_ARRAY_BUFFER, warningMessageVBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, warningMessageEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setLightVAO();

void setLightVAO() {
    float vertices[] = {
                            //R,    G,    B,    A)
         0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 1.0f, // Top (Fully Red)
        -2.0f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left (Transparent Red)
         2.0f, -0.5f, 0.0f,   1.0f, 0.0f, 0.0f, 0.0f  // Bottom-right (Transparent Red)
    };

    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



void initSonar() {
    glUseProgram(sonarShader); 

    // Get the window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    aspectRatio = (float)width / (float)height;
    mat4 projection = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f);
    int projectionLoc = glGetUniformLocation(sonarShader, "uProjection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ RENDER METHODS +++++++++++++++++++++++++++++++++++++++++++++++++

void renderSonar() {
    glUseProgram(sonarShader);

    vec3 darkGreen(0.0f, 0.0941f, 0.0f);
    vec3 lightGreen(0.0f, 0.5f, 0.0f);
    vec3 black(0.0f, 0.0f, 0.0f);
    vec3 white(1.0f, 1.0f, 1.0f);

    vec3 currentDarkColor = stopClicked ? black : darkGreen;
    vec3 currentLightColor = stopClicked ? white : lightGreen;
    vec3 currentColor = stopClicked ? black : (darkGreen * 0.5f + lightGreen * 0.5f);

    if (stopClicked) {
        float deltaTime = glfwGetTime() - stopTime; 
        currentTime = deltaTime;
    }
    else {
        float pulse = (sin(currentTime) + 1.0f) / 2.0f; 
        currentColor = currentDarkColor * (1.0f - pulse) + currentLightColor * pulse;

        // Update the needle's angle
        needleAngle = fmod(-currentTime * 45.0f, 360.0f) * float(M_PI / 180.0f); 
    }

    // Sonar circle
    mat4 identityMatrix = mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(sonarShader, "uModel"), 1, GL_FALSE, value_ptr(identityMatrix));
    glUniform3f(glGetUniformLocation(sonarShader, "uColor"), currentColor.r, currentColor.g, currentColor.b);
    glUniform3f(glGetUniformLocation(sonarShader, "uBaseColor"), currentColor.r, currentColor.g, currentColor.b);
    glBindVertexArray(sonarVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, CRES);

    //Needle
    if (!stopClicked) {
        mat4 needleRotation = rotate(mat4(1.0f), needleAngle, vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(sonarShader, "uModel"), 1, GL_FALSE, value_ptr(needleRotation));
        glUniform3f(glGetUniformLocation(sonarShader, "uColor"), stopClicked ? white.r : 1.0f, stopClicked ? white.g : 0.0f, stopClicked ? white.b : 0.0f);
        glUniform3f(glGetUniformLocation(sonarShader, "uBaseColor"), stopClicked ? black.r : 0.0f, stopClicked ? black.g : 0.5f, stopClicked ? black.b : 0.0f);
        glBindVertexArray(needleVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 101);
    }
    
    // Smaller circles
    for (int i = 0; i < 4; i++) {
        glUniform3f(glGetUniformLocation(sonarShader, "uColor"), 0.0f, 0.8314f, 0.0f);
        glBindVertexArray(smallerCirclesVAO[i]);
        glDrawArrays(GL_LINE_LOOP, 0, CRES);
    }

    //Lines
    glUniformMatrix4fv(glGetUniformLocation(sonarShader, "uModel"), 1, GL_FALSE, value_ptr(identityMatrix));
    glUniform3f(glGetUniformLocation(sonarShader, "uBaseColor"), 0.0f, 0.8314f, 0.0f);
    glUniform3f(glGetUniformLocation(sonarShader, "uColor"), 0.0f, 0.8314f, 0.0f);
    glBindVertexArray(linesVAO);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
}

void renderTexture(unsigned int shader, unsigned int texture, unsigned int VAO, vec2 position) {
    glUseProgram(shader);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    mat4 model = mat4(1.0f);
    model = translate(model, vec3(position, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, value_ptr(model));
    glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

void renderDots() {
    glUseProgram(dotShader);

    if (currentTime - lastDotSpawnTime > dotSpawnRate) {
        if (activeDots.size() < 5) { 
            createDot(); 
        }
        lastDotSpawnTime = currentTime;
    }

    for (auto it = activeDots.begin(); it != activeDots.end();) {
        it->timeRemaining -= 0.001f; 
        if (it->timeRemaining <= 0) {
            it = activeDots.erase(it); 
            continue;
        }

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(it->position, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(dotShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(dotVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 32);
        glBindVertexArray(0);

        ++it;
    }
}

void renderProgressBar() {
    glUseProgram(progressShader);

    setProgressBarVAO(-0.7f, 0.0f, 1.5f, 1.0f);
    updateProgressBar(-0.7f, oxygen, true);
    glBindVertexArray(progressBarVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, SEGMENTS * 2);
    glBindVertexArray(0);

    setProgressBarVAO(0.7f, 1.0f, 1.0f, 1.0f);
    updateProgressBar(0.7f, progress, false);
    glBindVertexArray(progressBarVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, SEGMENTS * 2);
    glBindVertexArray(0);

    glUseProgram(0);
}

void setupText(const char* text, float value, const vec3& color, float offsetX) {
    char textBuffer[10];
    snprintf(textBuffer, sizeof(textBuffer), "%.0f%s", value, text);

    glUseProgram(textShader);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform3f(glGetUniformLocation(textShader, "textColor"), color.r, color.g, color.b);

    float textWidth = 0.0f;
    for (int i = 0; textBuffer[i] != '\0'; ++i) {
        Character ch = Characters[textBuffer[i]];
        textWidth += (ch.Advance >> 5);  // Add character width (in pixels)
    }

    float centerX = offsetX;
    float centerY = 0.0f;  
    float textScale = 0.0004f;  
    float startX = centerX - (textWidth * textScale / 2.0);
    float startY = 0.0f;

    renderText(textBuffer, startX, startY, textScale);

    glUseProgram(0);
    glDisable(GL_BLEND);  
}

void renderText(const char* text, float x, float y, float scale) {
    glUseProgram(textShader);
    glBindVertexArray(textVAO);

    float uniformWidth = 70.0f; 
    float uniformHeight = 100.0f; 

    float startX = x; 

    for (int i = 0; text[i] != '\0'; ++i) {
        Character ch = Characters[text[i]];
        float w = uniformWidth * scale;
        float h = uniformHeight * scale;

        float xpos = startX;
        float ypos = y - (uniformHeight - ch.Bearing.y) * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos,     ypos,       0.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f }
        };

        // Upload vertex data to VBO
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        mat4 model = mat4(1.0f);
        model = translate(model, vec3(0.0f, 0.0f, 0.0f)); 
        glUniformMatrix4fv(glGetUniformLocation(textShader, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        startX += ((ch.Advance >> 6) * 0.001) + 0.01f;  // Use character advance for spacing
    }

    glBindVertexArray(0);
    glUseProgram(0);
}


void renderLight() {
    glUseProgram(lightShader);

    mat4 model = mat4(1.0f); 
    model = translate(model, vec3(0.0f, 0.3f, 0.0f)); 
    glUniformMatrix4fv(glGetUniformLocation(lightShader, "model"), 1, GL_FALSE, value_ptr(model));

    glBindVertexArray(lightVAO); 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLES, 0, 3); 

    glBindVertexArray(0); 
}

void mouseClickCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float xNDC = (xpos / width) * 2.0f - 1.0f;
        float yNDC = 1.0f - (ypos / height) * 2.0f;

        if (xNDC >= 0.83f && xNDC <= 0.93f && yNDC >= 0.75f && yNDC <= 0.85f) {
            stopClicked = !stopClicked;
            stopTime = glfwGetTime();
            if (stopClicked) {
                std::cout << "STOP button clicked! Sonar stopped." << std::endl;
            }
            else {
                std::cout << "START button clicked! Sonar resumed." << std::endl;
            }
        }
    }
}

void createDot() {
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI; // Random angle [0, 2pi)
    float radius = sqrt(static_cast<float>(rand()) / RAND_MAX) * 0.27f; // Random radius 

    float x = radius * cos(angle);
    float y = radius * sin(angle); 

    Dot newDot;
    newDot.position = vec2(x, y); 
    newDot.timeRemaining = 0.02f; 
    activeDots.push_back(newDot);
}

void updateProgressBar(float offsetX, float progress, bool interpolateColor) {
    float stripVertices[SEGMENTS * 4 * 5];
    int vertexIndex = 0;

    for (int i = 0; i < SEGMENTS; ++i) {
        float angle = (i * 2.0f * M_PI / (SEGMENTS - 1)) - M_PI / 2;
        float outerX = 0.4f * cos(angle) / aspectRatio;
        float outerY = 0.4f * sin(angle);
        float innerX = 0.35f * cos(angle) / aspectRatio;
        float innerY = 0.35f * sin(angle);

        // Set position data
        stripVertices[vertexIndex++] = outerX + offsetX;
        stripVertices[vertexIndex++] = outerY;
        stripVertices[vertexIndex++] = 1.0f;
        stripVertices[vertexIndex++] = 1.0f;
        stripVertices[vertexIndex++] = 1.0f;

        stripVertices[vertexIndex++] = innerX + offsetX;
        stripVertices[vertexIndex++] = innerY;
        stripVertices[vertexIndex++] = 1.0f;
        stripVertices[vertexIndex++] = 1.0f;
        stripVertices[vertexIndex++] = 1.0f;

        int colorOffset = 2;  // Color components are at +2, +3, +4
        float progressRatio = (float)i / (SEGMENTS - 1);

        if (progressRatio <= progress) {
            float red, green, blue;

            if (interpolateColor) {
                // Interpolate from blue to red
                red = 1.0f - progressRatio;
                green = 0.0f;
                blue = progressRatio;
            }
            else {
                // Darken blue
                red = 0.0f;
                green = 0.0f;
                blue = 1.0f - progressRatio;
            }

            stripVertices[vertexIndex - 10 + colorOffset] = red;
            stripVertices[vertexIndex - 9 + colorOffset] = green;
            stripVertices[vertexIndex - 8 + colorOffset] = blue;

            stripVertices[vertexIndex - 5 + colorOffset] = red;
            stripVertices[vertexIndex - 4 + colorOffset] = green;
            stripVertices[vertexIndex - 3 + colorOffset] = blue;
        }
        else {
            // Reset to white
            stripVertices[vertexIndex - 10 + colorOffset] = 1.0f;
            stripVertices[vertexIndex - 9 + colorOffset] = 1.0f;
            stripVertices[vertexIndex - 8 + colorOffset] = 1.0f;

            stripVertices[vertexIndex - 5 + colorOffset] = 1.0f;
            stripVertices[vertexIndex - 4 + colorOffset] = 1.0f;
            stripVertices[vertexIndex - 3 + colorOffset] = 1.0f;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, progressBarVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(stripVertices), stripVertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ MAIN +++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
    double lastFrameTime = 0.0;
    const double targetFrameRate = 1.0 / 60.0;
    double fpsCurrentTime = 0.0;

    initializeWindow();
    initFreeType();


    if (!window) {
        return -1; 
    }

    glClearColor(0.5, 0.5, 0.5, 1.0);

    //Create shaders
    sonarShader = createShader("sonar.vert", "sonar.frag");
    textureShader = createShader("texture.vert", "texture.frag");
    dotShader = createShader("dot.vert", "dot.frag");
    progressShader = createShader("progress.vert", "progress.frag");
    textShader = createShader("texture.vert", "text.frag");
    lightShader = createShader("light.vert", "light.frag");

    initSonar();

    //Set-up VAO
    glGenVertexArrays(1, &progressBarVAO);
    glGenBuffers(1, &progressBarVBO);
    setSonarVAO();
    setLinesVAO();
    setSmallerCirclesVAO();
    setNeedleVAO();
    setDotVAO();
    setWarningMessageVAO();
    setCircleTextureVAO();
    setTextVAO();
    setLightVAO();

    //TEXTURES
    unsigned startStopTexture = loadImageToTexture("res/start-stop.png");
    setupTexture(startStopTexture);

    unsigned int warningTexture = loadImageToTexture("res/warning.png");
    setupTexture(warningTexture);

    unsigned int enoughOxygenTexture = loadImageToTexture("res/enough-oxygen-message.png");
    setupTexture(enoughOxygenTexture);

    unsigned int redWarningTexture = loadImageToTexture("res/red-warning.png");
    setupTexture(redWarningTexture);

    unsigned int lampOffTexture = loadImageToTexture("res/lamp-off.png");
    setupTexture(lampOffTexture);

    unsigned int lampOnTexture = loadImageToTexture("res/lamp-on.png");
    setupTexture(lampOnTexture);

    unsigned int indexTexture = loadImageToTexture("res/index.png");
    setupTexture(indexTexture);

    unsigned int depthTexture = loadImageToTexture("res/depth.png");
    setupTexture(depthTexture);

    unsigned int oxygenTexture = loadImageToTexture("res/oxygen.png");
    setupTexture(oxygenTexture);

    while (!glfwWindowShouldClose(window)) 
    {
        fpsCurrentTime = glfwGetTime();
        if (fpsCurrentTime - lastFrameTime >= targetFrameRate) {
            lastFrameTime = fpsCurrentTime;
            
            currentTime = glfwGetTime();

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }

            // Screen cleaning
            glClear(GL_COLOR_BUFFER_BIT);


            //RENDERING
            renderSonar();
            if (!stopClicked) {
                renderDots();
            }
            renderProgressBar();
            //Depth text
            if (progress > 1.0f) {
                progress = 1.0f;
            }
            if (progress <= 0.0f) {
                progress = 0.0f;
            }
            setupText("m", progress * 250.0f, vec3(8.0f / 255.0f, 14.0f / 255.0f, 44.0f / 255.0f), 0.7f);
            //Oxygen text
            if (oxygen <= 0.0f) {
                oxygen = 0.0f;
            }
            if (oxygen >= 1.0f) {
                oxygen = 1.0f;
            }
            setupText("%", oxygen * 100.0f, vec3(0.0f, 0.0f, 1.0f), -0.7f);


            renderTexture(textureShader, startStopTexture, circleTextureVAO, vec2(0.88f, 0.8f));
            renderTexture(textureShader, lampOffTexture, circleTextureVAO, vec2(0.0f, 0.8f));
            renderTexture(textureShader, indexTexture, warningMessageVAO, vec2(-0.7f, -0.8f));
            renderTexture(textureShader, depthTexture, circleTextureVAO, vec2(0.7f, 0.6f));
            renderTexture(textureShader, oxygenTexture, circleTextureVAO, vec2(-0.7f, 0.6f));

            if (!stopClicked) {

                //W KEY PRESSED
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                    isWPressed = true;
                    if (isWPressed && progress < 1.0f) {
                        progress += 0.005f;
                    }
                }
                else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) {
                    isWPressed = false;
                }

                //S KEY PRESSED
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                    isSPressed = true;
                    if (isSPressed && progress > 0.0f) {
                        progress -= 0.005f;
                        progress = std::max(0.0f, progress); // progress is never negative
                    }
                }

                //INCREASE OXYGEN
                if (progress == 0.0f) {
                    if (oxygen < 1.0f) {
                        if (currentTime - lastTimeOxygenDecremented >= 1.0f) {
                            oxygen += 0.01f;
                            lastTimeOxygenDecremented = currentTime;
                        }
                    }
                }
                else {
                    if (oxygen > 0.0f) {
                        if (currentTime - lastTimeOxygenDecremented >= 1.0f) {
                            oxygen -= 0.01f;
                            lastTimeOxygenDecremented = currentTime;
                        }
                    }
                }

                if (oxygen <= 0.25f) {
                    isOxygenLow = true;
                }
                else if (oxygen > 0.75f) {
                    isOxygenLow = false;
                }

                if (isOxygenLow) {
                    renderLight();
                    renderTexture(textureShader, lampOnTexture, circleTextureVAO, vec2(0.0f, 0.8f));

                    if (currentTime - lastTimeTextureToggled >= 1.0f) {
                        toggleTexture = !toggleTexture;
                        lastTimeTextureToggled = currentTime;
                    }
                    if (toggleTexture) {
                        renderTexture(textureShader, warningTexture, warningMessageVAO, vec2(0.7f, -0.8f));
                    }
                    else {
                        renderTexture(textureShader, redWarningTexture, warningMessageVAO, vec2(0.7f, -0.8f));
                    }
                }
                if (oxygen > 0.75f) {
                    renderTexture(textureShader, enoughOxygenTexture, warningMessageVAO, vec2(0.7f, -0.8f));
                }

            }
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    // Cleanup
    glDeleteBuffers(1, &sonarVBO);
    glDeleteVertexArrays(1, &sonarVAO);

    glDeleteBuffers(1, &dotVBO);
    glDeleteVertexArrays(1, &dotVAO);

    glDeleteVertexArrays(1, &linesVAO);
    glDeleteBuffers(1, &linesVBO);

    for (int i = 0; i < 4; i++) {
        glDeleteVertexArrays(1, &smallerCirclesVAO[i]);
        glDeleteBuffers(1, &smallerCirclesVBO[i]);
    }

    glDeleteBuffers(1, &needleVBO);
    glDeleteVertexArrays(1, &needleVAO);

    glDeleteBuffers(1, &circleTextureVBO);
    glDeleteVertexArrays(1, &circleTextureVAO);

    glDeleteBuffers(1, &progressBarVBO);
    glDeleteVertexArrays(1, &progressBarVAO);

    glDeleteBuffers(1, &textVBO);
    glDeleteVertexArrays(1, &textVAO);

    glDeleteBuffers(1, &warningMessageVBO);
    glDeleteVertexArrays(1, &warningMessageVAO);

    glDeleteBuffers(1, &lightVBO);
    glDeleteVertexArrays(1, &lightVAO);

    glDeleteProgram(sonarShader);
    glDeleteProgram(textureShader);
    glDeleteProgram(dotShader);
    glDeleteProgram(progressShader);
    glDeleteProgram(textShader);
    glDeleteProgram(lightShader);

    //OK - terminate program
    glfwTerminate();
    return 0;
}

void initializeWindow() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cout << "GLFW library loading failed! :(\n";
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Get the primary monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        std::cerr << "Failed to get the primary monitor.\n";
        glfwTerminate();
    }

    // Get the monitor's video mode (resolution and refresh rate)
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
    if (!videoMode) {
        std::cerr << "Failed to get the video mode of the monitor.\n";
        glfwTerminate();
    }

    // Create a fullscreen window
    window = glfwCreateWindow(
        videoMode->width,    // Screen width
        videoMode->height,   // Screen height
        "Submarine",
        primaryMonitor,      // Fullscreen on the primary monitor
        nullptr
    );

    if (window == nullptr) {
        std::cout << "Window loading failed! :(\n";
        glfwTerminate();
    }

    glfwSetMouseButtonCallback(window, mouseClickCallback);

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW loading failed! :'(\n";
        glfwTerminate();
    }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ TEXTURE METHODS ++++++++++++++++++++++++++++++++++++++++++++++++++++++

static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}

void setupTexture(unsigned int texture) {
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
}
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ FREE TYPE ++++++++++++++++++++++++++++++++++++++++++++++++++++++

void initFreeType() {
    if (FT_Init_FreeType(&ft)) {
        std::cout << "Could not initialize FreeType library" << std::endl;
        return;
    }

    if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face)) {
        std::cout << "Could not load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);  
}


GLuint loadTextureFromFreeType(FT_Bitmap bitmap) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,                     // FreeType provides grayscale bitmaps
        bitmap.width,
        bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        bitmap.buffer               // Raw glyph bitmap data
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture
    return texture;
}


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++ SHADER METHODS ++++++++++++++++++++++++++++++++++++++++++++++++++++++

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Successfully read file from path\"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Error reading file from path \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str(); 

    int shader = glCreateShader(type); 

    int success; 
    char infoLog[512]; 
    glShaderSource(shader, 1, &sourceCode, NULL); 
    glCompileShader(shader); 

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); 
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog); 
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{

    unsigned int program; 
    unsigned int vertexShader; 
    unsigned int fragmentShader; 

    program = glCreateProgram(); 

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); 
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); 
    glValidateProgram(program); 

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); 
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "The unified shader has an error! Error: \n";
        std::cout << infoLog << std::endl;
    }
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
