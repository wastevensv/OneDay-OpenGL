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

float bg_vertices[] = {
//  Position            Texture
    -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, // Top left
     0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // Top right
     0.5f, -0.5f,  0.0f, 1.0f, 1.0f, // Bottom right
    -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, // Boottom left

//  Position            Texture
     0.0f,  1.0f,  0.0f, 0.0f, 0.0f, // Topmost
     0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // Leftmost
     0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // Bottommost
     0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // Rightmost
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
    
    // Create Vertex Array Object 2
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    
    // Create Vertex Buffer Object 2
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bg_vertices), bg_vertices, GL_STATIC_DRAW);
    

    // Create element array 2
    GLuint ebo;
    glGenBuffers(1, &ebo);

    GLuint bg_elements[] = {
        0, 1, 2,
        0, 3, 2,
        4, 5, 6,
        4, 7, 6
    };

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(bg_elements), bg_elements, GL_STATIC_DRAW);

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertexSource = readFile("3dshader.vert").c_str();

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

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragmentSource = readFile("3dshader.frag").c_str();

    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    GLint fStatus;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fStatus);
    if(fStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
        std::cerr << buffer << std::endl;
        return 4;
    }

    // Link vertex and fragment shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), 0);

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE,
                 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));

    // Create texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    int width, height;
    unsigned char* image =
                SOIL_load_image("img.png", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                                  GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Setup view
    glm::mat4 view = glm::lookAt(
                   glm::vec3(1.0f, 1.0f, 1.0f),
                   glm::vec3(0.0f, 0.0f, 0.0f),
                   glm::vec3(0.5f, 0.0f, 0.0f)
                   );
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    // Setup transformation
    GLint uniModel = glGetUniformLocation(shaderProgram, "model");

    // Setup projection
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    glEnable(GL_DEPTH_TEST);
    // --- Main Loop ---
    while(!glfwWindowShouldClose(window))
    {
        // Reset
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       

        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        // Capture frame
        cv::Mat cameraFrame;
        stream1.read(cameraFrame);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, cameraFrame.cols, cameraFrame.rows, 0, GL_RGB,
                                  GL_UNSIGNED_BYTE, cameraFrame.data);

        // Draw Spinny
        glm::mat4 model;
        model = glm::rotate(
                model,
                time * glm::radians(180.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
            );
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
       
        // Draw stationary
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                                  GL_UNSIGNED_BYTE, image);
    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        glm::mat4 model2; 
        glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model2));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6*sizeof(float)));
        

        //Display
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup/Shutdown ---
    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);

    glDeleteVertexArrays(1, &vao);
    
    SOIL_free_image_data(image);
    //glDeleteBuffers(1, &fg_ebo);
    //glDeleteBuffers(1, &fg_vbo);

    //glDeleteVertexArrays(1, &fg_vao);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

