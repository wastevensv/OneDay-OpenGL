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

float bg_vertices[] = {
//  Position            Texture
     0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // Near Right
    -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // Far Right
     0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // Far Left
     
     0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // Near Right
     1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // Near Left
     0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // Far Left
};

float fg_vertices[] = {
//  Position            Texture
    -1.0f,  0.0f,  1.0f, 1.0f, 1.0f, // Top Right
     0.0f, -1.0f,  1.0f, 0.0f, 1.0f, // Top Left
     0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // Bottom Left

    -1.0f,  0.0f,  1.0f, 1.0f, 1.0f, // Top Right
    -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // Bottom Right
     0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // Bottom Left
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
    
    
    // Create Vertex Array Object
    GLuint fg_vao;
    glGenVertexArrays(1, &fg_vao);
    glBindVertexArray(fg_vao);
    
    // Create Vertex Buffer Object
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

    // Create Vertex Array Object
    GLuint bg_vao;
    glGenVertexArrays(1, &bg_vao);
    glBindVertexArray(bg_vao);

    // Create Vertex Buffer Object
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


    // Create element array
//    GLuint ebo;
//    glGenBuffers(1, &ebo);

//    GLuint bg_elements[] = {
//        0, 1, 2,
//        0, 3, 2,
//        
//        4, 5, 6,
//        4, 7, 6,
//    };

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bg_elements), bg_elements, GL_STATIC_DRAW);

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexSource = readFile("3dvert.glsl").c_str();

    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint vStatus;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vStatus);
    if(vStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
        std::cerr << buffer << std::endl;
        return 3;
    }

    // Compile color fragment shader
    GLuint colorFrag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentSource = readFile("colorfrag.glsl").c_str();

    glShaderSource(colorFrag, 1, &fragmentSource, NULL);
    glCompileShader(colorFrag);
    GLint fStatus;
    glGetShaderiv(colorFrag, GL_COMPILE_STATUS, &fStatus);
    if(fStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(colorFrag, 512, NULL, buffer);
        std::cerr << buffer << std::endl;
        return 4;
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

    // Make color shader program.
    GLuint colorShaderProgram = glCreateProgram();
    glAttachShader(colorShaderProgram, vertexShader);
    glAttachShader(colorShaderProgram, colorFrag);
    glBindFragDataLocation(colorShaderProgram, 0, "outColor");
    glLinkProgram(colorShaderProgram);
    glUseProgram(colorShaderProgram);

    GLint fg_posAttrib = glGetAttribLocation(colorShaderProgram, "position");
    

    GLint colorPosAttrib = glGetAttribLocation(colorShaderProgram, "position");
    glEnableVertexAttribArray(colorPosAttrib);
    glVertexAttribPointer(colorPosAttrib, 3, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), 0);

    GLint colorTexAttrib = glGetAttribLocation(colorShaderProgram, "texcoord");
    glEnableVertexAttribArray(colorTexAttrib);
    glVertexAttribPointer(colorTexAttrib, 2, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
    
    // Make blank shader program.
    GLuint blankShaderProgram = glCreateProgram();
    glAttachShader(blankShaderProgram, vertexShader);
    glAttachShader(blankShaderProgram, blankFrag);
    glBindFragDataLocation(blankShaderProgram, 0, "outColor");
    glLinkProgram(blankShaderProgram);
    glUseProgram(blankShaderProgram);

    GLint blankPosAttrib = glGetAttribLocation(blankShaderProgram, "position");
    glEnableVertexAttribArray(blankPosAttrib);
    glVertexAttribPointer(blankPosAttrib, 3, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), 0);

    GLint blankTexAttrib = glGetAttribLocation(blankShaderProgram, "texcoord");
    glEnableVertexAttribArray(blankTexAttrib);
    glVertexAttribPointer(blankTexAttrib, 2, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
    
    // Setup view
    glm::mat4 view = glm::lookAt(
                   glm::vec3( 1.0f,  1.0f,  1.0f),
                   glm::vec3( 0.0f,  0.0f,  0.5f),
                   glm::vec3( 0.0f,  0.0f,  1.0f)
                   );
    GLint uniView = glGetUniformLocation(colorShaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    // Setup transformation
    GLint uniModel = glGetUniformLocation(colorShaderProgram, "model");

    // Setup projection
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
    GLint uniProj = glGetUniformLocation(colorShaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
    
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
        glBindVertexArray(fg_vao);
        model = glm::mat4();
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Draw Backboard
        //glUseProgram(colorShaderProgram);
        glBindVertexArray(bg_vao);
        model = glm::mat4();
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        //Display
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup/Shutdown ---
    glDeleteProgram(colorShaderProgram);
    glDeleteShader(colorFrag);
    glDeleteShader(vertexShader);

//    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &fg_vbo);

    glDeleteVertexArrays(1, &fg_vao);
    
//    SOIL_free_image_data(image);
    //glDeleteBuffers(1, &fg_ebo);
    glDeleteBuffers(1, &bg_vbo);

    glDeleteVertexArrays(1, &bg_vao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

