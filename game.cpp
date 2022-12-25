#include "game.h"
#include "graphicsData.h"
#include <chrono>
#include <thread>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

using glm::mat4;
using glm::perspective;
using namespace std;

void Resize(GLFWwindow* window, int width, int height)
{
    ((Game*)glfwGetWindowUserPointer(window))->ChangeSize(width, height);
}


void SetMat4Uniform(unsigned int program, const char* source, void* matrix4)
{
    unsigned int location = glGetUniformLocation(program,source);
    glUniformMatrix4fv(location,1,GL_FALSE,(GLfloat*)matrix4);
}

void SetVec3Uniform(unsigned int program, const char* source, void* vector3)
{
    unsigned int location = glGetUniformLocation(program,source);
    glUniform3fv(location,1,(GLfloat*)vector3);
}


void Snake::DrawSnake(void* mv, void* proj)
{
    auto snakeVao = GraphicsData::Data()["snakeVao"].first;
    auto program = GraphicsData::Data()["sProgram"].first;
    vec3 color(0.82f, 0.82f, 0.82f);
    mat4 model(1.0f);
    model = glm::translate(model,position);
    glUseProgram(program);
    SetMat4Uniform(program,"model",&model);
    SetMat4Uniform(program,"modelView",mv);
    SetMat4Uniform(program,"projection",proj);
    SetVec3Uniform(program,"color",&color);
    glBindVertexArray(snakeVao);
    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
}

Snake::Snake()
{
    float rowCenter = GRID_ROW_COUNT * GRID_CELL_SPACE / 2;
    float centerX = (GRID_COL_COUNT & 1) == 0 ? GRID_CELL_SPACE / 2 : 0;
    float centerZ = (GRID_ROW_COUNT & 1) == 0 ?  Z_BEGIN - rowCenter - GRID_CELL_SPACE / 2: Z_BEGIN - rowCenter;
    float offset = 0.03f;
    float cubeSide = GRID_CELL_SPACE - offset * 2;
    float cubeHalfSide = cubeSide / 2;
    float radius = sqrt(2) * cubeSide / 2;
    vec3 pos = vec3(0.0f,cubeHalfSide,cubeHalfSide);
    GraphicsData::CreateFace(vertices,indices, 0, radius,pos, XY);
    pos.z = -pos.z;
    GraphicsData::CreateFace(vertices,indices, 24, radius,pos, XY, false);
    pos.z = 0.0f; pos.x = -cubeHalfSide;
    GraphicsData::CreateFace(vertices,indices, 48, radius,pos, ZY, false);
    pos.x = cubeHalfSide;
    GraphicsData::CreateFace(vertices,indices, 72, radius,pos, ZY);
    pos = vec3(0.0f, 0.0f, 0.0f);
    GraphicsData::CreateFace(vertices,indices, 96, radius,pos, XZ, false);
    pos.y = 2 * cubeHalfSide;
    GraphicsData::CreateFace(vertices,indices, 120, radius,pos, XZ);
    unsigned int snakeVao, snakeVbo, snakeEbo;
    glGenVertexArrays(1,&snakeVao);
    glGenBuffers(1,&snakeVbo);
    glGenBuffers(1,&snakeEbo);
    glBindVertexArray(snakeVao);
    glBindBuffer(GL_ARRAY_BUFFER,snakeVbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,snakeEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices), indices,GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    GraphicsData::Data()["snakeVao"] = std::make_pair(snakeVao, BUFFER);
    GraphicsData::Data()["snakeVbo"] = std::make_pair(snakeVbo, BUFFER);
    GraphicsData::Data()["snakeEbo"] = std::make_pair(snakeEbo, BUFFER);
    position = vec3(centerX, 0.0f, centerZ);
    dir = vec3(0.0f, 0.0f, -GRID_CELL_SPACE);
}

Snake::~Snake()
{
    auto snakeVao = GraphicsData::Data()["snakeVao"].first;
    auto snakeVbo = GraphicsData::Data()["snakeVbo"].first;
    auto snakeEbo = GraphicsData::Data()["snakeEbo"].first;
    glDeleteVertexArrays(1,&snakeVao);
    glDeleteBuffers(1,&snakeVbo);
    glDeleteBuffers(1,&snakeEbo);
}

Game::Game(int width, int height)
{
    isLoad = false;
    if(!glfwInit())
    {
        printf("Error: Can`t initailize GLFW\n");
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
    gameWindow = glfwCreateWindow(width,height,"SNAKE3D(WORK IN PROGRESS)",NULL,NULL);
    if(!gameWindow){
        printf("Error: Cant`t create GLFW window\n");
        glfwTerminate();
        return;
    }
    glfwSetWindowSizeCallback(gameWindow,Resize);
    glfwSetWindowUserPointer(gameWindow,this);
    glfwMakeContextCurrent(gameWindow);
    if(glewInit() != GLEW_OK){
        printf("Error: Cant`t initialize Glew\n");
        return;
    }
    glfwGetFramebufferSize(gameWindow,&wWidth,&wHeight);
    glViewport(0,0,wWidth,wHeight);
    glEnable(GL_DEPTH_TEST);
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(gameWindow,(mode->width >> 1) - (wWidth >> 1), (mode->height >> 1) - (wHeight >> 1));
    auto vShader = GraphicsData::LoadShader("shaders/vertex.vs", GL_VERTEX_SHADER,"gridVShader");
    auto fShader = GraphicsData::LoadShader("shaders/fragment.fs",GL_FRAGMENT_SHADER,"gridFShader");
    auto vsShader = GraphicsData::LoadShader("shaders/vsnake.vs", GL_VERTEX_SHADER,"snakeVShader");
    auto fsShader = GraphicsData::LoadShader("shaders/fsnake.fs", GL_FRAGMENT_SHADER,"snakeFShader");
    if(!vShader || !fShader || !vsShader || !fsShader)
        return;
    auto program =  AttachShaders(vShader, fShader, "gridProgram");
    auto sProgram =  AttachShaders(vsShader , fsShader, "sProgram");
    if(!program || !sProgram)
        return;
    CreateGrid();
    snake = new Snake();
    glClearColor(0.0f,0.59f,0.59f,1.0f);
    printf("Hi it`s Snake3D game\n");
    level = 1;
    isLoad = true;
}

void* Game::ReadFile(void* inFile){
    FILE* file = (FILE*)inFile;
    char buffer[MAX_BUFFER];
    unsigned size = 0;
    while(!feof(file))
        buffer[size++] = fgetc(file);
    if(!size)
        return 0;
    buffer[size] = '\0';
    void* memory = malloc(size+1);
    memcpy(memory,buffer,size+1);
    return memory;
}

void Game::CreateGrid()
{
    if(GRID_COL_COUNT >= 2 && GRID_ROW_COUNT >= 2)
    {
        unsigned cols = GRID_COL_COUNT - 1;
        unsigned rows = GRID_ROW_COUNT - 1;
        unsigned colsVertices = cols << 1;
        unsigned rowsVertices = rows << 1;
        glGenVertexArrays(1,&gridVao);
        glGenBuffers(1,&gridVbo);
        vec3 data[4 + colsVertices + rowsVertices];
        float xMiddle = (float)GRID_COL_COUNT *  GRID_CELL_SPACE / 2;
        float zBegin = Z_BEGIN;
        float zEnd = zBegin - (float)GRID_ROW_COUNT * GRID_CELL_SPACE;
        float xBegin = -xMiddle + GRID_CELL_SPACE;
        data[0] = vec3(-xMiddle, 0.0f, zBegin);
        data[1] = vec3(-xMiddle, 0.0f, zEnd);
        data[2] = vec3(xMiddle, 0.0f, zEnd);
        data[3] = vec3(xMiddle, 0.0f, zBegin);
        for(unsigned x = 0; x < colsVertices; x+=2, xBegin += GRID_CELL_SPACE)
        {
            data[4 + x] = vec3(xBegin, 0.0f, zBegin);
            data[4 + x + 1] = vec3(xBegin, 0.0f, zEnd);
        }
        zBegin -= GRID_CELL_SPACE;
        for(unsigned z = 0; z < rowsVertices; z+=2, zBegin -= GRID_CELL_SPACE)
        {
            data[4 + cols * 2 + z] = vec3(-xMiddle, 0.0f, zBegin);
            data[4 + cols * 2 + z + 1] = vec3(xMiddle, 0.0f, zBegin);
        }
        glBindVertexArray(gridVao);
        glBindBuffer(GL_ARRAY_BUFFER,gridVbo);
        glBufferData(GL_ARRAY_BUFFER,sizeof(data),data,GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        isNeedRedraw = true;
    }
}

void Game::ChangeSize(int width, int height)
{
    wWidth = width;
    wHeight = height;
    glViewport(0,0,width,height);
    isNeedRedraw = true;
}

void Game::DrawGrid(void* mv,void* proj)
{
    auto program = GraphicsData::Data()["gridProgram"].first;
    vec3 color(0.0f, 1.0f, 0.0f);
    mat4 model(1.0f);
    glUseProgram(program);
    SetMat4Uniform(program,"model",&model);
    SetMat4Uniform(program,"modelView",mv);
    SetMat4Uniform(program,"projection",proj);
    SetVec3Uniform(program,"color",&color);
    glBindVertexArray(gridVao);
    glDrawArrays(GL_LINE_LOOP,0,4);
    glDrawArrays(GL_LINES, 4, 2 * (GRID_ROW_COUNT + GRID_COL_COUNT - 2));
}

int Game::AttachShaders(int vs,int fs, const char* key)
{
    int program = glCreateProgram();
    if(vs && fs){
        glAttachShader(program,vs);
        glAttachShader(program,fs);
        glLinkProgram(program);
        GLint succes;
        glGetProgramiv(program,GL_LINK_STATUS,&succes);
        if(!succes){
            char infoLog[512];
            glGetProgramInfoLog(program,512,NULL,infoLog);
            cout<< "Error on link program with message:\n" << infoLog;
            isLoad = false;
            return 0;
        }
        GraphicsData::Data()[key] = std::make_pair(program, PROGRAMM);
    }else{
        glDeleteProgram(program);
        isLoad = false;
        program = 0;
    }
    return program;
}

void Game::Update()
{
    static float startTick = 0.0f;
    while(!glfwWindowShouldClose(gameWindow) && isLoad){
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        if(deltaTime + 0.001f < MIN_UPDATE_TIME)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        lastFrame = currentFrame;
        UpdateKeyboard(snake->GetPosition(),snake->GetDirection());
        if(currentFrame - startTick >= START_TIME_UPD - ((level - 1) * 0.25))
        {
            startTick = currentFrame;
            *(snake->GetPosition()) += *snake->GetDirection();
            isNeedRedraw = true;
        }
        Render();
        glfwPollEvents();
    }
}

void Game::UpdateKeyboard(vec3* position, vec3* dir)
{
    if(glfwGetKey(gameWindow,GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
    	glfwSetWindowShouldClose(gameWindow, GL_TRUE);
    	isLoad = false;
    }
    if(glfwGetKey(gameWindow,GLFW_KEY_UP) == GLFW_PRESS)
        *dir = vec3(0.0f, 0.0f, -GRID_CELL_SPACE);
    else if(glfwGetKey(gameWindow,GLFW_KEY_DOWN) == GLFW_PRESS)
        *dir = vec3(0.0f, 0.0f, GRID_CELL_SPACE);
    else if(glfwGetKey(gameWindow,GLFW_KEY_LEFT) == GLFW_PRESS)
        *dir = vec3(-GRID_CELL_SPACE, 0.0f, 0.0f);
    else if(glfwGetKey(gameWindow,GLFW_KEY_RIGHT) == GLFW_PRESS)
        *dir = vec3(GRID_CELL_SPACE, 0.0f, 0.0f);
}

void Game::Render()
{
    if(isNeedRedraw)
    {
        mat4 modelView = lookAt(vec3(0.0f,4.0f,-1.0f),vec3(0.0f,0.0f,-3.0f),vec3(0.0f,1.0f,0.0f));
        mat4 proj = perspective(glm::radians(80.0f), (float)wWidth/wHeight, 0.01f, 50.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
        DrawGrid(&modelView,&proj);
        snake->DrawSnake(&modelView,&proj);
        glfwSwapBuffers(gameWindow);
        isNeedRedraw = false;
    }
}

Game::~Game()
{
    delete snake;
    cout<<"End game Snake3D, bye!\n";
    glDeleteVertexArrays(1,&gridVao);
    glDeleteBuffers(1,&gridVbo);
    auto vS = GraphicsData::Data()["gridVShader"].first;
    auto fS = GraphicsData::Data()["gridFShader"].first;
    auto vsS = GraphicsData::Data()["snakeVShader"].first;
    auto fsS = GraphicsData::Data()["snakeFShader"].first;
    auto gProgram = GraphicsData::Data()["gridProgram"].first;
    auto sProgram = GraphicsData::Data()["sProgram"].first;
    glDeleteShader(vS);
    glDeleteShader(fS);
    glDeleteShader(vsS);
    glDeleteShader(fsS);
    glDetachShader(gProgram, vS);
    glDetachShader(gProgram, fS);
    glDetachShader(sProgram, vsS);
    glDetachShader(sProgram, fsS);
    glfwTerminate();
}
