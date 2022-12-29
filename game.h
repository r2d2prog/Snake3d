#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED
#include <glm/glm.hpp>
#include <cstdlib>
#include <vector>
#define GRID_COL_COUNT 4
#define GRID_ROW_COUNT 4
#define GRID_CELLS_TOTAL GRID_COL_COUNT * GRID_ROW_COUNT
#define GRID_CELL_SPACE 0.5
#define GRID_CELL_CENTER (float)GRID_CELL_SPACE / 2
#define GRID_CENTER GRID_ROW_COUNT / 2 * GRID_COL_COUNT + GRID_COL_COUNT / 2
#define Z_BEGIN -1.0
#define SCORE_INCREMENT 100
#define MIN_UPDATE_TIME 1/60
#define START_TIME_UPD 1.5 - (float)MIN_UPDATE_TIME
#define FOODS_PER_LEVEL 5
#define UPDATE_TIME_PER_LEVEL (START_TIME_UPD) * FOODS_PER_LEVEL  / ((float)GRID_CELLS_TOTAL - 1)

using glm::vec3;
using std::vector;
using std::pair;

typedef struct GLFWwindow GLFWwindow;
class Snake;

void SetMat4Uniform(unsigned int program, const char* source, void* matrix4);
void SetVec3Uniform(unsigned int program, const char* source, void* vector3);

class Game{
private:
    bool isLoad;
    bool isNeedRedraw;
    int status;
    unsigned int score;
    int wWidth;
    int wHeight;
    unsigned int level;
    float lastFrame;
    GLFWwindow* gameWindow;
    Snake* snake;
    pair<vec3,vec3> food;
    bool isFoodRedraw;
    void CreateGrid();
    void DrawGrid(void* mv, void* proj);
    void UpdateFood();
    void DrawFood(void* mv, void* proj);
public:
    Game(int width = 800, int height = 600);
    void ChangeSize(int width, int height);
    static void SetRandColor(vec3* color)
    {
        vec3 colors[] = {vec3(1.0,0.0,0.0),vec3(0.0,1.0,0.0),vec3(0.0,0.0,1.0),
                         vec3(1.0,0.67,0.11),vec3(1.0,0.92,0)};
        unsigned int select = std::rand() % (sizeof(colors)/ sizeof(vec3));
        *color = colors[select];
    }
    static vec3 CellToPos(unsigned int cell)
    {
        vec3 result(0);
        unsigned int row = cell / GRID_ROW_COUNT;
        unsigned col = cell - GRID_COL_COUNT * row;
        result.z = Z_BEGIN - GRID_CELL_CENTER - ((GRID_ROW_COUNT - row - 1) * GRID_CELL_SPACE);
        float xBegin =  GRID_CELL_CENTER - (float)GRID_COL_COUNT / 2 * GRID_CELL_SPACE;
        result.x = xBegin + col * GRID_CELL_SPACE;
        return result;
    }
    static int PosToCell(vec3 pos)
    {
        float beginX = (float)GRID_CELL_SPACE / 2 - (float)GRID_COL_COUNT / 2 * GRID_CELL_SPACE;
        float centerOfCell = beginX + GRID_COL_COUNT / 2 * GRID_CELL_SPACE;
        int colIndex = (int)((pos.x - centerOfCell) /  GRID_CELL_SPACE) + GRID_COL_COUNT / 2;
        int rowIndex = (int)(pos.z / GRID_CELL_SPACE) + GRID_ROW_COUNT + 1 ;
        if((colIndex < 0 || colIndex >= GRID_COL_COUNT) || (rowIndex < 0 || rowIndex >= GRID_ROW_COUNT ))
            return -1;
        return rowIndex * GRID_COL_COUNT + colIndex;
    }
    int LoadShader(const char* fileName, unsigned int);
    int AttachShaders(int vs,int fs, const char* key);
    void Update();
    void Render();
    ~Game();
};

class Snake
{
private:
    bool isNeedRedraw;
    vector<pair<unsigned int,vec3> > cells;
    vec3 dir;
    float vertices[144];
    unsigned int indices[36];
    void WriteVector(vec3 coords);
    void WriteVertices(vec3 offset);
public:
    Snake();
    inline vec3* GetDirection()
    {
        return &dir;
    }
    inline  vector<pair<unsigned int,vec3> >* GetCells()
    {
        return &cells;
    }
    bool UpdateKeyboard(GLFWwindow* gameWindow);
    int Update(const pair<vec3,vec3>* food);
    void DrawSnake(void* mv, void* proj);
    inline bool* SetRedraw()
    {
        return &isNeedRedraw;
    }
    ~Snake();
};
#endif // GAME_H_INCLUDED
