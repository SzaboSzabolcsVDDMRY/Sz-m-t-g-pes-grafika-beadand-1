#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;
float circleRadius = 50.0f;
glm::vec2 circlePos(SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);

// Mozgás alapból vízszintes
glm::vec2 circleVel(200.0f, 0.0f);

// Színek
glm::vec3 innerColorOrig(1.0f, 0.0f, 0.0f); // piros
glm::vec3 outerColorOrig(0.0f, 1.0f, 0.0f); // zöld
glm::vec3 innerColor = innerColorOrig;
glm::vec3 outerColor = outerColorOrig;

glm::vec3 bgColor(1.0f, 1.0f, 0.0f); // sárga háttér
glm::vec3 lineColor(0.0f, 0.0f, 1.0f); // kék vonal

// Kék vonal paraméterek
float lineY = SCR_HEIGHT / 2.0f;
const float lineLength = SCR_WIDTH / 3.0f;
const float lineWidth = 3.0f;

// Shader betöltés
std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) { std::cerr << "Nem sikerült: " << filepath << "\n"; return ""; }
    std::stringstream buffer; buffer << file.rdbuf(); return buffer.str();
}

// Shader ellenőrzés
void checkCompileErrors(GLuint shader, std::string type) {
    GLint success; GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) { glGetShaderInfoLog(shader, 1024, NULL, infoLog); std::cerr << type << " hiba:\n" << infoLog << "\n"; }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) { glGetProgramInfoLog(shader, 1024, NULL, infoLog); std::cerr << "Program hiba:\n" << infoLog << "\n"; }
    }
}

// Billentyű kezelése
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_S) {
            float alpha = glm::radians(25.0f);
            float speed = 200.0f;
            circleVel = glm::vec2(speed * cos(alpha), speed * sin(alpha));
        }
        if (key == GLFW_KEY_UP) { lineY += 5.0f; if (lineY > SCR_HEIGHT) lineY = SCR_HEIGHT; }
        if (key == GLFW_KEY_DOWN) { lineY -= 5.0f; if (lineY < 0.0f) lineY = 0.0f; }
    }
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Bouncing Circle", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) { std::cerr << "GLEW hiba\n"; return -1; }

    // Shader program
    std::string vertexCode = loadShaderSource("circle.vert");
    std::string fragmentCode = loadShaderSource("circle.frag");
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex); checkCompileErrors(vertex, "VERTEX");

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment); checkCompileErrors(fragment, "FRAGMENT");

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex);
    glAttachShader(shaderProgram, fragment);
    glLinkProgram(shaderProgram); checkCompileErrors(shaderProgram, "PROGRAM");
    glDeleteShader(vertex); glDeleteShader(fragment);

    // Quad
    float quadVertices[] = { -1,-1, 1,-1, 1,1, -1,1 };
    unsigned int quadIndices[] = { 0,1,2, 2,3,0 };
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO); glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    float lastFrame = glfwGetTime();
    static bool colorsSwapped = false;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Kör mozgása
        circlePos += circleVel * deltaTime;
        if (circlePos.x - circleRadius <= 0.0f || circlePos.x + circleRadius >= SCR_WIDTH) circleVel.x *= -1.0f;
        if (circlePos.y - circleRadius <= 0.0f || circlePos.y + circleRadius >= SCR_HEIGHT) circleVel.y *= -1.0f;

        // Érintkezés ellenőrzése teljes kék vonalhoz (x és y)
        bool touchesLine =
            (circlePos.x + circleRadius >= (SCR_WIDTH - lineLength) / 2.0f) &&
            (circlePos.x - circleRadius <= (SCR_WIDTH + lineLength) / 2.0f) &&
            (circlePos.y + circleRadius >= lineY - lineWidth / 2.0f) &&
            (circlePos.y - circleRadius <= lineY + lineWidth / 2.0f);

        // Színek cseréje, ha már nincs érintkezés
        if (!touchesLine && !colorsSwapped) { std::swap(innerColor, outerColor); colorsSwapped = true; }
        else if (touchesLine && colorsSwapped) { std::swap(innerColor, outerColor); colorsSwapped = false; }

        glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Kör rajzolása
        glUseProgram(shaderProgram);
        glUniform2f(glGetUniformLocation(shaderProgram, "uCircleCenter"), circlePos.x, circlePos.y);
        glUniform1f(glGetUniformLocation(shaderProgram, "uRadius"), circleRadius);
        glUniform3f(glGetUniformLocation(shaderProgram, "uInnerColor"), innerColor.r, innerColor.g, innerColor.b);
        glUniform3f(glGetUniformLocation(shaderProgram, "uOuterColor"), outerColor.r, outerColor.g, outerColor.b);
        glUniform3f(glGetUniformLocation(shaderProgram, "uBgColor"), bgColor.r, bgColor.g, bgColor.b);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Kék vonal
        glUseProgram(0);
        glLineWidth(lineWidth);
        glColor3f(lineColor.r, lineColor.g, lineColor.b);
        float x1 = ((SCR_WIDTH - lineLength) / 2.0f / SCR_WIDTH) * 2.0f - 1.0f;
        float x2 = ((SCR_WIDTH + lineLength) / 2.0f / SCR_WIDTH) * 2.0f - 1.0f;
        float yNDC = (lineY / SCR_HEIGHT) * 2.0f - 1.0f;
        glBegin(GL_LINES);
        glVertex2f(x1, yNDC);
        glVertex2f(x2, yNDC);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}