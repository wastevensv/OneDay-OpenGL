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

#define POS_ID 0
#define TEX_ID 1

#define POS_OBJ_ID 2

#define COLOR_ID 3

bool loadOBJ(
   const char * path,
       std::vector < glm::vec3 > & out_vertices,
       std::vector < glm::vec2 > & out_uvs,
       std::vector < glm::vec3 > & out_normals
    );



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

float axis_vertices[] = {
// Position              Color
     0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f, // Origin
     0.5f,  0.0f,  0.0f, 1.0f, 0.0f,  0.0f, // X Line
    
     0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  0.0f, // Origin
     0.0f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, // Y Line
    
     0.0f,  0.0f,  0.0f, 0.0f, 0.0f,  1.0f, // Origin
     0.0f,  0.0f,  0.5f, 0.0f, 0.0f,  1.0f, // Z Line
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

    // Create Axis Vertex Array Object
    GLuint axis_vao;
    glGenVertexArrays(1, &axis_vao);
    glBindVertexArray(axis_vao);

    // Create Axis Vertex Buffer Object
    GLuint axis_vbo;
    glGenBuffers(1, &axis_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertices), axis_vertices, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(POS_ID);
    glVertexAttribPointer(POS_ID, 3, GL_FLOAT, GL_FALSE,
                 6*sizeof(GLfloat), 0);

    glEnableVertexAttribArray(COLOR_ID);
    glVertexAttribPointer(COLOR_ID, 3, GL_FLOAT, GL_FALSE,
                 6*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));


    // Read our .obj file
    std::vector< glm::vec3 > obj_vertices;
    std::vector< glm::vec2 > obj_uvs;
    std::vector< glm::vec3 > obj_normals; // Won't be used at the moment.
    bool res = loadOBJ("../model.obj", obj_vertices, obj_uvs, obj_normals);

    GLuint obj_vao;
    glGenVertexArrays(1, &obj_vao);
    glBindVertexArray(obj_vao);
    
    GLuint obj_vbo;
    glGenBuffers(1, &obj_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, obj_vbo);
    glBufferData(GL_ARRAY_BUFFER, obj_vertices.size() * sizeof(glm::vec3), &obj_vertices[0], GL_STATIC_DRAW);

    //glEnableVertexAttribArray(POS_OBJ_ID);
    //glVertexAttribPointer(POS_OBJ_ID, 3, GL_FLOAT, GL_FALSE,
    //             sizeof(glm::vec3), 0);


    // Compile color vertex shader
    GLuint colorVert = glCreateShader(GL_VERTEX_SHADER);
    const char *cvSource = readFile("3dvert.glsl").c_str();

    glShaderSource(colorVert, 1, &cvSource, NULL);
    glCompileShader(colorVert);
    GLint cvStatus;
    glGetShaderiv(colorVert, GL_COMPILE_STATUS, &cvStatus);
    if(cvStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(colorVert, 512, NULL, buffer);
        std::cerr << "colorvert shader: " << buffer << std::endl;
        return 7;
    }

    // Compile color fragment shader
    GLuint colorFrag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* cfSource = readFile("colorfrag.glsl").c_str();

    glShaderSource(colorFrag, 1, &cfSource, NULL);
    glCompileShader(colorFrag);
    GLint cfStatus;
    glGetShaderiv(colorFrag, GL_COMPILE_STATUS, &cfStatus);
    if(cfStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(colorFrag, 512, NULL, buffer);
        std::cerr << "colorfrag shader: " << buffer << std::endl;
        return 4;
    }

    // Compile pretty vertex shader
    GLuint prettyVert = glCreateShader(GL_VERTEX_SHADER);
    const char *pvSource = readFile("3dvert.glsl").c_str();

    glShaderSource(prettyVert, 1, &pvSource, NULL);
    glCompileShader(prettyVert);
    GLint pvStatus;
    glGetShaderiv(prettyVert, GL_COMPILE_STATUS, &pvStatus);
    if(pvStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(prettyVert, 512, NULL, buffer);
        std::cerr << "prettyvert shader: " << buffer << std::endl;
        return 8;
    }

    // Compile pretty fragment shader
    GLuint prettyFrag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* pfSource = readFile("prettyfrag.glsl").c_str();

    glShaderSource(prettyFrag, 1, &pfSource, NULL);
    glCompileShader(prettyFrag);
    GLint pfStatus;
    glGetShaderiv(prettyFrag, GL_COMPILE_STATUS, &pfStatus);
    if(pfStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(prettyFrag, 512, NULL, buffer);
        std::cerr << "prettyfrag shader: " << buffer << std::endl;
        return 7;
    }

    // Compile blank vertex shader
    GLuint blankVert = glCreateShader(GL_VERTEX_SHADER);
    const char *bvSource = readFile("3dvert.glsl").c_str();

    glShaderSource(blankVert, 1, &bvSource, NULL);
    glCompileShader(blankVert);
    GLint bvStatus;
    glGetShaderiv(blankVert, GL_COMPILE_STATUS, &bvStatus);
    if(bvStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(blankVert, 512, NULL, buffer);
        std::cerr <<  "blankvert shader: " << buffer << std::endl;
        return 9;
    }

    // Compile blank fragment shader
    GLuint blankFrag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* bfSource = readFile("simplefrag.glsl").c_str();

    glShaderSource(blankFrag, 1, &bfSource, NULL);
    glCompileShader(blankFrag);
    GLint bfStatus;
    glGetShaderiv(blankFrag, GL_COMPILE_STATUS, &bfStatus);
    if(bfStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(blankFrag, 512, NULL, buffer);
        std::cerr <<  "blankfrag shader: " << buffer << std::endl;
        return 10;
    }

    // Compile notex vertex shader
    GLuint notexVert = glCreateShader(GL_VERTEX_SHADER);
    const char *nvSource = readFile("3dnotex.glsl").c_str();

    glShaderSource(notexVert, 1, &nvSource, NULL);
    glCompileShader(notexVert);
    GLint nvStatus;
    glGetShaderiv(notexVert, GL_COMPILE_STATUS, &nvStatus);
    if(nvStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(notexVert, 512, NULL, buffer);
        std::cerr << "notexvert shader: " << buffer << std::endl;
        return 6;
    }

    // Compile notex fragment shader
    GLuint notexFrag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* nfSource = readFile("notexfrag.glsl").c_str();

    glShaderSource(notexFrag, 1, &nfSource, NULL);
    glCompileShader(notexFrag);
    GLint nfStatus;
    glGetShaderiv(notexFrag, GL_COMPILE_STATUS, &nfStatus);
    if(nfStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(notexFrag, 512, NULL, buffer);
        std::cerr << "notexfrag shader: " << buffer << std::endl;
        return 5;
    }

    // Make color shader program.
    GLuint colorShaderProgram = glCreateProgram();
    glAttachShader(colorShaderProgram, colorVert);
    glAttachShader(colorShaderProgram, colorFrag);
    glBindFragDataLocation(colorShaderProgram, 0, "outColor");
    glLinkProgram(colorShaderProgram);
    glUseProgram(colorShaderProgram);
    
    // Make pretty shader program.
    GLuint prettyShaderProgram = glCreateProgram();
    glAttachShader(prettyShaderProgram, prettyVert);
    glAttachShader(prettyShaderProgram, prettyFrag);
    glBindFragDataLocation(prettyShaderProgram, 0, "outColor");
    glLinkProgram(prettyShaderProgram);
    glUseProgram(prettyShaderProgram);
    
    // Make blank shader program.
    GLuint blankShaderProgram = glCreateProgram();
    glAttachShader(blankShaderProgram, blankVert);
    glAttachShader(blankShaderProgram, blankFrag);
    glBindFragDataLocation(blankShaderProgram, 0, "outColor");
    glLinkProgram(blankShaderProgram);
    glUseProgram(blankShaderProgram);

    // Make notex shader program.
    GLuint notexShaderProgram = glCreateProgram();
    glAttachShader(notexShaderProgram, notexVert);
    glAttachShader(notexShaderProgram, notexFrag);
    glBindFragDataLocation(notexShaderProgram, 0, "outColor");
    glLinkProgram(notexShaderProgram);
    glUseProgram(notexShaderProgram);

    // Setup view
    glm::mat4 view = glm::lookAt(
                   glm::vec3( 1.0f,  1.0f,  1.0f),
                   glm::vec3( 0.0f,  0.0f,  0.5f),
                   glm::vec3( 0.0f,  0.0f,  1.0f)
                   );
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
    glm::mat4 model = glm::mat4();

    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "view" ), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "proj" ), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(colorShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "view" ), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "proj" ), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    
    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "view" ), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "proj" ), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix4fv(glGetUniformLocation(notexShaderProgram, "view" ), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(notexShaderProgram, "proj" ), 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(notexShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

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
        
        // Draw Axis
        glUseProgram(colorShaderProgram);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.25f,-0.25f,0.0f));
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(axis_vao);
        axis_vertices[6] = sin(time)/2;
        axis_vertices[19] = sin(time)/2;
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_vertices), axis_vertices);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);

        // Draw Backboard
        glUseProgram(prettyShaderProgram);
        model = glm::mat4();
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        
        glBindVertexArray(bg_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        // Draw Baseboard
        glUseProgram(blankShaderProgram);
        model = glm::mat4();
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(glGetUniformLocation(blankShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(fg_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Draw Object
//        glUseProgram(notexShaderProgram);
//        model = glm::mat4();
//        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
//        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
//        glUniformMatrix4fv(glGetUniformLocation(prettyShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

//        glBindVertexArray(obj_vao);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//        glBindVertexArray(0);
        
        //Display
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup/Shutdown ---
    glDeleteProgram(colorShaderProgram);
    glDeleteProgram(prettyShaderProgram);
    glDeleteProgram(blankShaderProgram);
    
    glDeleteShader(colorFrag);
    glDeleteShader(colorVert);

    glDeleteShader(prettyFrag);
    glDeleteShader(prettyVert);

    glDeleteShader(blankVert);
    glDeleteShader(blankFrag);

    glDeleteBuffers(1, &axis_vbo);
    glDeleteBuffers(1, &fg_vbo);
    glDeleteBuffers(1, &bg_vbo);

    glDeleteVertexArrays(1, &axis_vao);
    glDeleteVertexArrays(1, &fg_vao);
    glDeleteVertexArrays(1, &bg_vao);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

bool loadOBJ(
        const char * path,
        std::vector < glm::vec3 > & out_vertices,
        std::vector < glm::vec2 > & out_uvs,
        std::vector < glm::vec3 > & out_normals
    ) {
    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    std::vector< glm::vec3 > temp_vertices;
    std::vector< glm::vec2 > temp_uvs;
    std::vector< glm::vec3 > temp_normals;

    FILE * file = fopen(path, "r");
    if( file == NULL ){
       printf("Impossible to open the file !\n");
       return false;
    }
    while( 1 ){
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.
        if ( strcmp( lineHeader, "v" ) == 0 ){
             glm::vec3 vertex;
             fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
             temp_vertices.push_back(vertex);
        } else if ( strcmp( lineHeader, "vt" ) == 0 ){
             glm::vec2 uv;
             fscanf(file, "%f %f\n", &uv.x, &uv.y );
             temp_uvs.push_back(uv);
        } else if ( strcmp( lineHeader, "vn" ) == 0 ){
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        } else if ( strcmp( lineHeader, "f" ) == 0 ){
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9){
            printf("File can't be read by our simple parser : ( Try exporting with other options\n");
            return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices    .push_back(uvIndex[0]);
            uvIndices    .push_back(uvIndex[1]);
            uvIndices    .push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }
    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++ ){
        unsigned int vertexIndex = vertexIndices[i];
        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
        out_vertices.push_back(vertex);
    }
}
