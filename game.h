#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED
#include <unordered_set>
#include <glm/glm.hpp>
#define MAX_BUFFER 4096
#define GRID_COL_COUNT 16
#define GRID_ROW_COUNT 16
#define GRID_CELLS_TOTAL GRID_COL_COUNT * GRID_ROW_COUNT
#define GRID_CELL_SPACE 0.5
#define GRID_CELL_CENTER (float)GRID_CELL_SPACE / 2
#define GRID_CENTER GRID_ROW_COUNT / 2 * GRID_COL_COUNT + GRID_COL_COUNT / 2
#define Z_BEGIN -1.0
#define START_TIME_UPD 1.75

using glm::vec3;
using std::unordered_set;
const float MIN_UPDATE_TIME = (float)1000/60;

typedef struct GLFWwindow GLFWwindow;
class Snake;

void SetMat4Uniform(unsigned int program, const char* source, void* matrix4);
void SetVec3Uniform(unsigned int program, const char* source, void* vector3);

class Game{
private:
    bool isLoad;
    bool isNeedRedraw;
    int wWidth;
    int wHeight;
    unsigned int level;
    unsigned int gridVao;
    unsigned int gridVbo;
    float lastFrame;
    GLFWwindow* gameWindow;
    Snake* snake;
    void* ReadFile(void* inFile);
    void CreateGrid();
    void DrawGrid(void* mv, void* proj);
public:
    Game(int width = 800, int height = 600);
    void ChangeSize(int width, int height);
    int LoadShader(const char* fileName, unsigned int);
    int AttachShaders(int vs,int fs, const char* key);
    void Update();
    void Render();
    void UpdateKeyboard(vec3* dir);
    ~Game();
};

class Snake
{
private:
    bool isNeedRedraw;
    unordered_set<unsigned int> cells;
    vec3 position;
    vec3 dir;
    float vertices[144];
    unsigned int indices[36];
    void WriteVector(vec3 coords);
    void WriteVertices(vec3 offset);
public:
    Snake();
    int PosToCell(vec3 pos);
    vec3 CellToPos(unsigned int cell);
    inline vec3* GetPosition()
    {
        return &position;
    }
    inline vec3* GetDirection()
    {
        return &dir;
    }
    void DrawSnake(void* mv, void* proj);
    inline bool* SetRedraw()
    {
        return &isNeedRedraw;
    }
    ~Snake();
};
#endif // GAME_H_INCLUDED
