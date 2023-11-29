#define YELLOW_VALUE 4
#define SKY_VALUE 5
#define BLUE_VALUE 6
#define ORANGE_VALUE 7

#define COLOR_A 100
#define COLOR_B 101
#define COLOR_C 102
#define COLOR_D 103
#define COLOR_E 104
#define COLOR_F 105
#define COLOR_G 106
#define COLOR_H 107
#define COLOR_I 108
#define COLOR_J 109
#define COLOR_K 110
#define COLOR_L 111
#define COLOR_M 112
#define COLOR_N 113
#define COLOR_O 114
#define COLOR_P 115
#define COLOR_Q 116
#define COLOR_R 117
#define COLOR_W 118
#define COLOR_X 119
#define COLOR_Y 120

HPEN g_hGridPen;
HBRUSH g_hTileBrush_gray; // 장애물 [회색]
HBRUSH g_hTileBrush_green; // 출발지 [녹색]
HBRUSH g_hTileBrush_red; // 목적지 [빨간색]

HBRUSH g_hTileBrush_yellow; // openList에 들어가 있는 것
HBRUSH g_hTileBrush_blue; //  OpenList에서 뽑힌 색 [파란색]
HBRUSH g_hTileBrush_orange; // 목적지 경로 [주황]

/* rand color value */
HBRUSH g_hTileBrush_a;
HBRUSH g_hTileBrush_b;
HBRUSH g_hTileBrush_c;
HBRUSH g_hTileBrush_d;
HBRUSH g_hTileBrush_e;
HBRUSH g_hTileBrush_f;
HBRUSH g_hTileBrush_g;
HBRUSH g_hTileBrush_h;
HBRUSH g_hTileBrush_i;
HBRUSH g_hTileBrush_j;
HBRUSH g_hTileBrush_k;
HBRUSH g_hTileBrush_l;
HBRUSH g_hTileBrush_m;
HBRUSH g_hTileBrush_n;
HBRUSH g_hTileBrush_o;
HBRUSH g_hTileBrush_p;
HBRUSH g_hTileBrush_q;
HBRUSH g_hTileBrush_r;
HBRUSH g_hTileBrush_w;
HBRUSH g_hTileBrush_x;
HBRUSH g_hTileBrush_y;

void Init_Brush();