#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <streambuf>

#define BG_ID 0
#define FG_ID 1

#define POS_ID 0
#define TEX_ID 1

float fg_vertices[] = {
//  Position             Texture
     0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // Near Right
    -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // Far Right
     0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // Far Left
     
     0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // Near Right
     1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // Near Left
     0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // Far Left
};

float bg_vertices[] = {
//  Position             Texture
    -1.0f,  0.0f,  1.0f, 1.0f, 1.0f, // Top Right
     0.0f, -1.0f,  1.0f, 0.0f, 1.0f, // Top Left
     0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // Bottom Left

    -1.0f,  0.0f,  1.0f, 1.0f, 1.0f, // Top Right
    -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // Bottom Right
     0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // Bottom Left
};

float axis_verticies[] = {
// Position              Color
     0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f, // Origin
     1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f, // X Line
    
     0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  0.0f, // Origin
     0.0f,  1.0f,  0.0f, 0.0f, 1.0f,  0.0f, // Y Line
    
     0.0f,  0.0f,  0.0f, 0.0f, 0.0f,  1.0f, // Origin
     0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  1.0f, // Z Line
};

std::string readFile(const char *filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if(!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

int main()
{
    auto t_start = std::chrono::high_resolution_clock::now();
    
    // --- OpenCV Init ---
    cv::VideoCapture stream1(0);   //0 is the id of video device.0 if you have only one camera.

    if (!stream1.isOpened()) { //check if video device has been initialised
            std::cerr << "cannot open camera" << std::endl;
    }
         
    // --- GLFW Init ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // --- Create window ---
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr); // Windowed
    if(window == NULL) return 2;
    glfwMakeContextCurrent(window);
    
    // --- GLEW/GL Init ---
    glewExperimental = GL_TRUE;
    glewInit();

    // --- Application Specific Setup ---
    
    
    // Create FG Vertex Array Object
    GLuint fg_vao;
    glGenVertexArrays(1, &fg_vao);
    glBindVertexArray(fg_vao);
    
    // Create FG Vertex Buffer Object
    GLuint fg_vbo;
    glGenBuffers(1, &fg_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, fg_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fg_vertices), fg_vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(POS_ID);
    glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), 0);

    glEnableVertexAttribArray(TEX_ID);
    glVertexAttribPointer(TEX_ID, 2, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));

    // Create BG Vertex Array Object
    GLuint bg_vao;
    glGenVertexArrays(1, &bg_vao);
    glBindVertexArray(bg_vao);

    // Create BG Vertex Buffer Object
    GLuint bg_vbo;
    glGenBuffers(1, &bg_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, bg_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bg_vertices), bg_vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(POS_ID);
    glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), 0);

    glEnableVertexAttribArray(TEX_ID);
    glVertexAttribPointer(TEX_ID, 2, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));


    // Compile color vertex shader
    GLuint prettyVert = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexSource = readFile("3dvert.glsl").c_str();

    glShaderSource(prettyVert, 1, &vertexSource, NULL);
    glCompileShader(prettyVert);
    GLint cvStatus;
    glGetShaderiv(prettyVert, GL_COMPILE_STATUS, &cvStatus);
    if(cvStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(prettyVert, 512, NULL, buffer);
        std::cerr << buffer << std::endl;
        return 3;
    }

    // Compile color fragment shader
    GLuint prettyFrag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSource = readFile("prettyfrag.glsl").c_str();

    glShaderSource(prettyFrag, 1, &fragmentSource, NULL);
    glCompileShader(prettyFrag);
    GLint fStatus;
    glGetShaderiv(prettyFrag, GL_COMPILE_STATUS, &fStatus);
    if(fStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(prettyFrag, 512, NULL, buffer);
        std::cerr << buffer << std::endl;
        return 4;
    }

    // Compile blank vertex shader
    GLuint blankVert = glCreateShader(GL_VERTEX_SHADER);
    const char *blankVertexSource = readFile("3dvert.glsl").c_str();

    glShaderSource(blankVert, 1, &blankVertexSource, NULL);
    glCompileShader(blankVert);
    GLint bvStatus;
    glGetShaderiv(blankVert, GL_COMPILE_STATUS, &bvStatus);
    if(bvStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(blankVert, 512, NULL, buffer);
        std::cerr << buffer << std::endl;
        return 3;
    }

    // Compile blank fragment shader
    GLuint blankFrag = glCreateShader(GL_FRAGMENT_SHADER);
    fragmentSource = readFile("simplefrag.glsl").c_str();

    glShaderSource(blankFrag, 1, &fragmentSource, NULL);
    glCompileShader(blankFrag);
    glGetShaderiv(blankFrag, GL_COMPILE_STATUS, &fStatus);
    if(fStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(blankFrag, 512, NULL, buffer);
        std::cerr << buffer << std::endl;
        return 5;
    }

    // Make blank shader program.
    GLuint colorShaderProgram = glCreateProgram();
    glAttachShader(colorShaderProgram, prettyVert);
    glAttachShader(colorShaderProgram, prettyFrag);
    glBindFragDataLocation(colorShaderProgram, 0, "outColor");
    glLinkProgram(colorShaderProgram);
    glUseProgram(colorShaderProgram);
    
    // Make blank shader program.
    GLuint blankShaderProgram = glCreateProgram();
    glAttachShader(blankShaderProgram, blankVert);
    glAttachShader(blankShaderProgram, blankFrag);
    glBindFragDataLocation(blankShaderProgram, 0, "outColor");
    glLinkProgram(blankShaderProgram);
    glUseProgram(blankShaderProgram);

    // Setup view
    glm::mat4 view = glm::lookAt(
                   glm::vec3( 1.0f,  1.0f,  1.0f),
                   glm::vec3( 0.0f,  0.0f,  0.5f),
                   glm::vec3( 0.0f,  0.0f,  1.0f)
                   );
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
    glm::mat4 model = glm::mat4();

    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "view" ), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "proj" ), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "view" ), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "proj" ), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // Create texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);


    glEnable(GL_DEPTH_TEST);
    // --- Main Loop ---
    while(!glfwWindowShouldClose(window))
    {
        // Reset
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       

        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        // Capture frame
        cv::Mat cameraFrame;
        stream1.read(cameraFrame);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cameraFrame.cols, cameraFrame.rows, 0, GL_BGR,
                                  GL_UNSIGNED_BYTE, cameraFrame.data);
        glGenerateMipmap(GL_TEXTURE_2D);


        glm::mat4 model;
        
        // Draw Baseboard
        glUseProgram(blankShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(fg_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Draw Backboard
        glUseProgram(colorShaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(bg_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        //Display
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup/Shutdown ---
    glDeleteProgram(colorShaderProgram);
    glDeleteProgram(blankShaderProgram);
    
    glDeleteShader(prettyFrag);
    glDeleteShader(prettyVert);

    glDeleteShader(blankVert);
    glDeleteShader(blankFrag);

    glDeleteBuffers(1, &fg_vbo);
    glDeleteBuffers(1, &bg_vbo);

    glDeleteVertexArrays(1, &fg_vao);
    glDeleteVertexArrays(1, &bg_vao);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

