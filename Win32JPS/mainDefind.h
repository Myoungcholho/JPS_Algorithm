#include<windowsx.h>
#include<list>
#include "BRUSH_define.h"

using namespace std;

#define GRID_SIZE 32
#define GRID_WIDTH 16
#define GRID_HEIGHT 16
#define ROOTSEARCH 0x1001
#define RESET 0x1002
#define STARTPOINT 0x1003
#define STARTVALUE 2
#define DESTPOINT 0x1004
#define DESTVALUE 3
#define WALL 0x1005
#define WALLVALUE 1
#define REMOVE 0x1006

char g_Tile[GRID_HEIGHT][GRID_WIDTH];
bool g_TileInvalid[GRID_HEIGHT][GRID_WIDTH];

bool g_bErase = false;
bool g_bDrag = false;
bool g_bRset = false;

void RenderGrid(HDC hdc);
void RenderObstacle(HDC hdc);

struct stTile
{
	int xPos;
	int yPos;
	int G; /* 이동 거리 */
	int H; /* 목표 까지 거리 */
	int F; /* G + H */
	stTile* parent;
	
};

list<stTile*> openList;
list<stTile*> MemoryCloseList;

/* 버튼 */
HWND        _search_btn; 
HWND        _reset_btn;
HWND        _start_btn;
HWND        _dest_btn;
HWND        _wall_btn;
HWND        _remove_btn;

/* int TileValue */
int tileValue;

/* 출발지 /목적지 */
stTile depart;
stTile dest;
int x_begin, y_begin; // 시작 위치 
int x_end, y_end; // 끝 위치

void Init_startPoint();
void Init_destination();

int Manhattan_Cal(stTile* _start, stTile* _end);
bool F_Ascending(stTile* lhs, stTile* rhs);

void Screen_print(HWND hWnd);

/* JPS */

#define UU	101
#define RU	102
#define RR	103
#define RD	104
#define DD	105
#define LD	106
#define LL	107
#define LU	108

int currentColor;

void findPath(HWND hWnd);

void Right(stTile* tile);
void Up(stTile* tile);
void Left(stTile* tile);
void Down(stTile* tile);
void RightUp(stTile* tile);
void LeftUp(stTile* tile);
void LeftDown(stTile* tile);
void RightDown(stTile* tile);

/* 대각선 탐색 시 이용 */
char Search_Right(int _xpos, int _ypos);
char Search_Up(int _xpos, int _ypos);
char Search_Left(int _xpos, int _ypos);
char Search_Down(int _xpos, int _ypos);

int check_Direction(stTile* Cur);
bool tile_Check(int m_xpos, int m_ypos);
int G_Cal(int current_xpos, int current_ypos, int parent_xpos, int parent_ypos,int parent_G);

/* 마지막 선 그리기*/
HPEN h_red_pen;
bool road_draw;
stTile* destpoint = nullptr;
void DestLine(HDC hWnd);