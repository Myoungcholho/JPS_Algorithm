#include "framework.h"
#include "Win32JPS.h"
#include "mainDefind.h"


#define MAX_LOADSTRING 100
HINSTANCE hInst;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32JPS));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32JPS));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WIN32JPS);
    wcex.lpszClassName = L"GridTile";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    HWND hWnd = CreateWindowW(L"GridTile", L"GridTile", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        g_bDrag = true;
        {

            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int iTileX = xPos / GRID_SIZE;
            int iTileY = yPos / GRID_SIZE;

            if (g_Tile[iTileY][iTileX] == 1)
                g_bErase = true;
            else
                g_bErase = false;
        }
        break;

    case WM_LBUTTONUP:
        g_bDrag = false;
        break;

    case WM_MOUSEMOVE:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        if (g_bDrag)
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);

            int iTileX = xPos / GRID_SIZE;
            int iTileY = yPos / GRID_SIZE;

            /* 타일에 값 주기*/
            g_Tile[iTileY][iTileX] = tileValue;

            InvalidateRect(hWnd, NULL, true);
        }
    }
    break;

    case WM_CREATE:
        Init_Brush();

        _search_btn = CreateWindow(L"button", L"길찾기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1750, 100, 80, 25, hWnd, (HMENU)ROOTSEARCH, hInst, NULL);
        _reset_btn = CreateWindow(L"button", L"초기화", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1750, 130, 80, 25, hWnd, (HMENU)RESET, hInst, NULL);
        _start_btn = CreateWindow(L"button", L"출발점", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1750, 160, 80, 25, hWnd, (HMENU)STARTPOINT, hInst, NULL);
        _dest_btn = CreateWindow(L"button", L"도착점", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1750, 190, 80, 25, hWnd, (HMENU)DESTPOINT, hInst, NULL);
        _wall_btn = CreateWindow(L"button", L"장애물", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1750, 220, 80, 25, hWnd, (HMENU)WALL, hInst, NULL);
        _remove_btn = CreateWindow(L"button", L"지우기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 1750, 250, 80, 25, hWnd, (HMENU)REMOVE, hInst, NULL);
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case ROOTSEARCH:

            if (g_bRset)
                break;

            g_bRset = true;

            /* 1. 출발지 OpenList에 생성 후 push하기  */
            Init_startPoint();

            /* 2. 목적지 stTile 설정하기 */
            Init_destination();

            /* 3. findpath 함수로 길 찾기 */
            findPath(hWnd);

            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);

            break;
        case RESET:

            if (!g_bRset)
                break;

            g_bRset = false;
            road_draw = false;
            destpoint = nullptr;

            for (int i = 0; i < GRID_HEIGHT; ++i)
            {
                memset(g_Tile[i], 0, sizeof(char) * GRID_WIDTH);
            }

            while (openList.size() > 0)
            {
                stTile* deleteMemory = openList.front();
                openList.pop_front();
                MemoryCloseList.push_back(deleteMemory);
            }

            /* 출발지는 heap에 없으므로 delete시 오류 ,,
                예외적 1회 .. */
            MemoryCloseList.pop_front();
            while (MemoryCloseList.size() > 0)
            {
                stTile* deleteMemory = MemoryCloseList.front();
                MemoryCloseList.pop_front();
                delete deleteMemory;
            }
            InvalidateRect(hWnd, NULL, TRUE);

            break;

        case STARTPOINT:
            tileValue = STARTVALUE;
            break;

        case DESTPOINT:
            tileValue = DESTVALUE;
            break;

        case WALL:
            tileValue = WALLVALUE;
            break;

        case REMOVE:
            tileValue = 0;
            break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RenderObstacle(hdc);
        RenderGrid(hdc);
        DestLine(hdc);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:

        DeleteObject(g_hTileBrush_gray);
        DeleteObject(g_hTileBrush_green);
        DeleteObject(g_hTileBrush_red);
        DeleteObject(g_hTileBrush_yellow);
        DeleteObject(g_hTileBrush_blue);
        DeleteObject(g_hTileBrush_orange);
        DeleteObject(g_hGridPen);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void RenderGrid(HDC hdc)
{
    int iX = 0;
    int iY = 0;
    HPEN hOldPen = (HPEN)SelectObject(hdc, g_hGridPen);

    for (int iCntW = 0; iCntW <= GRID_WIDTH; iCntW++)
    {
        MoveToEx(hdc, iX, 0, NULL);
        LineTo(hdc, iX, GRID_HEIGHT * GRID_SIZE);
        iX += GRID_SIZE;
    }

    for (int iCntH = 0; iCntH <= GRID_HEIGHT; iCntH++)
    {
        MoveToEx(hdc, 0, iY, NULL);
        LineTo(hdc, GRID_WIDTH * GRID_SIZE, iY);
        iY += GRID_SIZE;
    }
    SelectObject(hdc, hOldPen);
}

void findPath(HWND hWnd)
{
    while (openList.size() > 0)
    {
        /* openList 정렬 */
        openList.sort(F_Ascending);

        stTile* Cur = openList.front();
        openList.pop_front();
        MemoryCloseList.push_back(Cur);

        /* openList에서 뽑힌 색은 BLUE */
        g_Tile[Cur->yPos][Cur->xPos] = BLUE_VALUE;

        /* 해당 stTile의 탐색한 곳의 Color */
        currentColor = (rand() % 21) + 100;

        /* 만약 목적지라면 ..*/
        if (Cur->xPos == dest.xPos && Cur->yPos == dest.yPos)
        {
            destpoint = Cur;
            road_draw = true;

            g_Tile[Cur->yPos][Cur->xPos] = DESTVALUE;

            while (x_begin != Cur->xPos || y_begin != Cur->yPos)
            {
                Cur = Cur->parent;
                g_Tile[Cur->yPos][Cur->xPos] = ORANGE_VALUE;

                if (Cur == &depart)
                {
                    g_Tile[Cur->yPos][Cur->xPos] = STARTVALUE;

                }
            }
            break;
        }

        if (Cur->parent == nullptr)
        {
            Right(Cur);
            Up(Cur);
            Left(Cur);
            Down(Cur);

            /* 이 녀석은 코너를 발견하면 그 자리로 돌아오고 그 자리에 노드를 만들어야 한다! */
            RightUp(Cur); 
            LeftUp(Cur);
            LeftDown(Cur);
            RightDown(Cur);
        }
        else
        {
            int _x = Cur->xPos;
            int _y = Cur->yPos;

            switch (check_Direction(Cur))
            {
                /* 직각 */
            case RR:
            {
                Right(Cur);
                if (!tile_Check(_x, _y - 1) && tile_Check(_x + 1, _y - 1))
                {
                    RightUp(Cur);
                }
                if (!tile_Check(_x, _y + 1) && tile_Check(_x + 1, _y + 1))
                {
                    RightDown(Cur);
                }
            }
            break;
            case DD:
            {
                Down(Cur);
                if (!tile_Check(_x - 1, _y) && tile_Check(_x - 1, _y+1 ))
                {
                    LeftDown(Cur);
                }
                if (!tile_Check(_x + 1, _y) && tile_Check(_x + 1, _y + 1))
                {
                    RightDown(Cur);
                }
            }
            break;
            case LL:
            {
                Left(Cur);
                if (!tile_Check(_x, _y - 1) && tile_Check(_x - 1, _y - 1))
                {
                    LeftUp(Cur);
                }
                if (!tile_Check(_x, _y + 1) && tile_Check(_x - 1, _y + 1))
                {
                    LeftDown(Cur);
                }
            }
            break;
            case UU:
            {
                Up(Cur);
                if (!tile_Check(_x - 1, _y) && tile_Check(_x - 1, _y - 1))
                {
                    LeftUp(Cur);
                }
                if (!tile_Check(_x + 1, _y) && tile_Check(_x + 1, _y - 1))
                {
                    RightUp(Cur);
                }
            }
            break;
                /* 대각 */
            case RU:
            {
                Right(Cur);
                Up(Cur);
                RightUp(Cur);

                if (!tile_Check(_x - 1, _y) && tile_Check(_x - 1, _y - 1))
                {
                    LeftUp(Cur);
                }
                if (!tile_Check(_x, _y + 1) && tile_Check(_x + 1, _y + 1))
                {
                    RightDown(Cur);
                }
            }
            break;
            case RD:
            {
                Right(Cur);
                Down(Cur);
                RightDown(Cur);

                if (!tile_Check(_x, _y - 1) && tile_Check(_x + 1, _y - 1))
                {
                    RightUp(Cur);
                }
                if (!tile_Check(_x - 1, _y) && tile_Check(_x - 1, _y + 1))
                {
                    LeftDown(Cur);
                }
            }
            break;
            case LD:
            {
                Left(Cur);
                Down(Cur);
                LeftDown(Cur);

                if (!tile_Check(_x, _y - 1) && tile_Check(_x - 1, _y - 1))
                {
                    LeftUp(Cur);
                }
                if (!tile_Check(_x + 1, _y) && tile_Check(_x + 1, _y + 1))
                {
                    RightDown(Cur);
                }
            }
            break;
            case LU:
            {
                Left(Cur);
                Up(Cur);
                LeftUp(Cur);

                if (!tile_Check(_x + 1, _y) && tile_Check(_x + 1, _y - 1))
                {
                    RightUp(Cur);
                }
                if (!tile_Check(_x, _y + 1) && tile_Check(_x - 1, _y + 1))
                {
                    LeftDown(Cur);
                }

            }
            break;
            } // switch 끝 
        } // else 끝

        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
        
    } // while 끝 


}

/* 직선 반경 */
void Right(stTile* tile)
{
    stTile* m_tile = tile;

    /* 기준 좌표 */
    int m_xPos = m_tile->xPos+1;
    int m_yPos = m_tile->yPos;
    
    /* 맵 사이즈 넘어가는지 , 장애물인지 , */
    while (m_xPos >= 0 && m_xPos < GRID_WIDTH && m_yPos >= 0 && m_yPos < GRID_HEIGHT && g_Tile[m_yPos][m_xPos] != 1)
    {
        /* openList에 있다면 break!! */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }

        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }


        /*목적지랑 겹친다면 .. */
        if (m_xPos == x_end && m_yPos == y_end)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos,new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);

            break;
        }

        /* 위나 아래 코너가 생긴다면 ..*/
        if((!tile_Check(m_xPos, m_yPos - 1) && tile_Check(m_xPos + 1, m_yPos - 1)) || 
            (!tile_Check(m_xPos, m_yPos + 1) && tile_Check(m_xPos + 1, m_yPos + 1)))
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile,&dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }

        g_Tile[m_yPos][m_xPos] = currentColor;
        
        
        ++m_xPos;
    } 
}

void Up(stTile* tile)
{
    stTile* m_tile = tile;

    /* 기준 좌표 */
    int m_xPos = m_tile->xPos;
    int m_yPos = m_tile->yPos -1 ;

    while (m_xPos >= 0 && m_xPos < GRID_WIDTH && m_yPos >= 0 && m_yPos < GRID_HEIGHT && g_Tile[m_yPos][m_xPos] != 1 )
    {
        /* openList에 있다면 break!! */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (m_xPos == x_end && m_yPos == y_end)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);

            break;
        }

        /* 왼쪽이나 오른쪽 코너가 생긴다면 */
        if ((!tile_Check(m_xPos-1, m_yPos ) && tile_Check(m_xPos-1 , m_yPos-1 )) ||
            (!tile_Check(m_xPos+1, m_yPos ) && tile_Check(m_xPos+1 , m_yPos-1)))
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }

        g_Tile[m_yPos][m_xPos] = currentColor;

        --m_yPos;
    }
}

void Left(stTile* tile)
{
    stTile* m_tile = tile;

    /* 기준 좌표 */
    int m_xPos = m_tile->xPos - 1;
    int m_yPos = m_tile->yPos;

    while (m_xPos >= 0 && m_xPos < GRID_WIDTH && m_yPos >= 0 && m_yPos < GRID_HEIGHT && g_Tile[m_yPos][m_xPos] != 1)
    {
        /* openList에 있다면 break!! */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (m_xPos == x_end && m_yPos == y_end)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);

            break;
        }

        /* 아래나 위 코너가 생긴다면 */
        if ((!tile_Check(m_xPos , m_yPos+1) && tile_Check(m_xPos-1 , m_yPos+1 )) ||
            (!tile_Check(m_xPos , m_yPos-1) && tile_Check(m_xPos -1, m_yPos -1)))
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }

        g_Tile[m_yPos][m_xPos] = currentColor;
        
        
        --m_xPos;
    }
}

void Down(stTile* tile)
{
    stTile* m_tile = tile;

    /* 기준 좌표 */
    int m_xPos = m_tile->xPos;
    int m_yPos = m_tile->yPos+1;

    while (m_xPos >= 0 && m_xPos < GRID_WIDTH && m_yPos >= 0 && m_yPos < GRID_HEIGHT && g_Tile[m_yPos][m_xPos] != 1 )
    {
        /* openList에 있다면 break!! */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (m_xPos == x_end && m_yPos == y_end)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);

            break;
        }

        /* 왼쪽이나 오른쪽에 코너가 생긴다면 */
        if ((!tile_Check(m_xPos-1, m_yPos ) && tile_Check(m_xPos-1 , m_yPos+1 )) ||
            (!tile_Check(m_xPos+1, m_yPos ) && tile_Check(m_xPos+1 , m_yPos+1 )))
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
            
        g_Tile[m_yPos][m_xPos] = currentColor;
        

        ++m_yPos;
    }
}

/* 대각선 반경 */
void RightUp(stTile* tile)
{
    stTile* m_tile = tile;

    int m_xPos = m_tile->xPos+1;
    int m_yPos = m_tile->yPos-1;

    char Search_Check;

    while (m_xPos >= 0 && m_xPos < GRID_WIDTH && m_yPos >= 0 && m_yPos < GRID_HEIGHT && g_Tile[m_yPos][m_xPos] != 1 )
    {
        /* 대각선도 openList에 있는지 체크해야지 ?? */
        g_Tile[m_yPos][m_xPos] = currentColor;
        /*openList에 있었다면 .. */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (m_xPos == x_end && m_yPos == y_end)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);

            break;
        }

        /* 본인 자리 특수 경우 확인 */
        if ((!tile_Check(m_xPos -1, m_yPos) && tile_Check(m_xPos -1, m_yPos -1)) ||
            (!tile_Check(m_xPos , m_yPos+1) && tile_Check(m_xPos +1, m_yPos +1)))
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }

        Search_Check = Search_Right(m_xPos+1, m_yPos);

        if (Search_Check == true)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
        else if (Search_Check == 2)
        {
            break;
        }

        Search_Check = Search_Up(m_xPos, m_yPos-1);

        if (Search_Check == true)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
        else if (Search_Check == 2)
        {
            break;
        }

        ++m_xPos;
        --m_yPos;
    }
}

void LeftUp(stTile* tile)
{
    stTile* m_tile = tile;

    int m_xPos = m_tile->xPos - 1;
    int m_yPos = m_tile->yPos - 1;

    char Search_Check;

    while (m_xPos >= 0 && m_xPos < GRID_WIDTH && m_yPos >= 0 && m_yPos < GRID_HEIGHT && g_Tile[m_yPos][m_xPos] != 1 )
    {
        /* 대각선도 openList에 있는지 체크해야지 ?? */
        g_Tile[m_yPos][m_xPos] = currentColor;
        /*openList에 있었다면 .. */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (m_xPos == x_end && m_yPos == y_end)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);

            break;
        }

         /* 본인 자리 특수 경우 확인 */
        if ((!tile_Check(m_xPos+1, m_yPos) && tile_Check(m_xPos+1, m_yPos-1)) ||
            (!tile_Check(m_xPos, m_yPos+1) && tile_Check(m_xPos-1, m_yPos+1)))
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }


        Search_Check = Search_Left(m_xPos-1, m_yPos);

        if (Search_Check == true)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
        else if (Search_Check == 2)
        {
            break;
        }

        Search_Check = Search_Up(m_xPos, m_yPos-1);

        if (Search_Check == true)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
        else if (Search_Check == 2)
        {
            break;
        }
        --m_xPos;
        --m_yPos;
    }
}

void LeftDown(stTile* tile)
{
    stTile* m_tile = tile;
    int m_xPos = m_tile->xPos -1;
    int m_yPos = m_tile->yPos +1;

    char Search_Check;

    while (m_xPos >= 0 && m_xPos < GRID_WIDTH && m_yPos >= 0 && m_yPos < GRID_HEIGHT && g_Tile[m_yPos][m_xPos] != 1 )
    {
        /* 대각선도 openList에 있는지 체크해야지 ?? */
        g_Tile[m_yPos][m_xPos] = currentColor;

        /*openList에 있었다면 .. */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (m_xPos == x_end && m_yPos == y_end)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);

            break;
        }

        /* 본인 자리 특수 경우 확인 */
        if ((!tile_Check(m_xPos, m_yPos-1) && tile_Check(m_xPos-1, m_yPos-1)) ||
            (!tile_Check(m_xPos+1, m_yPos) && tile_Check(m_xPos+1, m_yPos+1)))
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }


        Search_Check = Search_Left(m_xPos-1, m_yPos);
        if (Search_Check == true)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
        else if (Search_Check == 2)
        {
            break;
        }

        Search_Check = Search_Down(m_xPos, m_yPos+1);

        if (Search_Check == true)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
        else if (Search_Check == 2)
        {
            break;
        }

        --m_xPos;
        ++m_yPos;
    }
}

void RightDown(stTile* tile)
{
    stTile* m_tile = tile;
    int m_xPos = m_tile->xPos + 1;
    int m_yPos = m_tile->yPos + 1;

    char Search_Check;

    while (m_xPos >= 0 && m_xPos < GRID_WIDTH && m_yPos >= 0 && m_yPos < GRID_HEIGHT && g_Tile[m_yPos][m_xPos] != 1 )
    {
        /* 대각선도 openList에 있는지 체크해야지 ?? */
        g_Tile[m_yPos][m_xPos] = currentColor;

        /*openList에 있었다면 .. */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == m_xPos && (*iter)->yPos == m_yPos)
            {
                break;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (m_xPos == x_end && m_yPos == y_end)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);

            break;
        }

         /* 본인 자리 특수 경우 확인 */
        if ((!tile_Check(m_xPos , m_yPos-1) && tile_Check(m_xPos +1, m_yPos-1 )) ||
            (!tile_Check(m_xPos -1, m_yPos) && tile_Check(m_xPos -1, m_yPos+1 )))
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }

        Search_Check = Search_Right(m_xPos+1, m_yPos);

        if (Search_Check == true)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
        else if (Search_Check == 2)
        {
            break;
        }

        Search_Check = Search_Down(m_xPos, m_yPos+1);

        if (Search_Check == true)
        {
            stTile* new_tile = new stTile;
            new_tile->xPos = m_xPos;
            new_tile->yPos = m_yPos;
            new_tile->parent = m_tile;
            new_tile->G = G_Cal(new_tile->xPos, new_tile->yPos, new_tile->parent->xPos, new_tile->parent->yPos, new_tile->parent->G);
            new_tile->H = Manhattan_Cal(new_tile, &dest);
            new_tile->F = new_tile->G + new_tile->H;

            openList.push_back(new_tile);
            g_Tile[m_yPos][m_xPos] = YELLOW_VALUE;

            break;
        }
        else if (Search_Check == 2)
        {
            break;
        }

        ++m_xPos;
        ++m_yPos;
    }
}

/* 매개변수로 부터 -> 탐색 대각선 탐색 시 이용  */
char Search_Right(int _xpos, int _ypos)
{
    
    /* 벽에 부딪히거나, 밖으로 나가거나 */
    while (_xpos >= 0 && _xpos < GRID_WIDTH && _ypos >= 0 && _ypos < GRID_HEIGHT && g_Tile[_ypos][_xpos] != 1 )
    {
        g_Tile[_ypos][_xpos] = currentColor;

        /* tile이 만들어진 곳이었다면 [closeList도 뒤져야 하나?] 정리가 안되네 */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == _xpos && (*iter)->yPos == _ypos)
            {
                return 2;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == _xpos && (*iter)->yPos == _ypos)
            {
                return 2;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (_xpos == x_end && _ypos == y_end)
        {
            return true;
        }

        /* 위나 아래 코너가 생긴다면 ..*/
        if ((!tile_Check(_xpos, _ypos - 1) && tile_Check(_xpos + 1, _ypos - 1)) ||
            (!tile_Check(_xpos, _ypos + 1) && tile_Check(_xpos + 1, _ypos + 1)))
        {
            return true;
        }

        ++_xpos;
    }
    return false;
}

char Search_Up(int _xpos, int _ypos)
{
    
    /* 벽에 부딪히거나, 밖으로 나가거나 */
    while (_xpos >= 0 && _xpos < GRID_WIDTH && _ypos >= 0 && _ypos < GRID_HEIGHT && g_Tile[_ypos][_xpos] != 1)
    {
        g_Tile[_ypos][_xpos] = currentColor;

        /* tile이 만들어진 곳이었다면 [closeList도 뒤져야 하나?] 정리가 안되네 */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == _xpos && (*iter)->yPos == _ypos)
            {
                return 2;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == _xpos && (*iter)->yPos == _ypos)
            {
                return 2;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (_xpos == x_end && _ypos == y_end)
        {
            return true;
        }

        /* 왼쪽이나 오른쪽 코너가 생긴다면 */
        if ((!tile_Check(_xpos - 1, _ypos) && tile_Check(_xpos - 1, _ypos - 1)) ||
            (!tile_Check(_xpos + 1, _ypos) && tile_Check(_xpos + 1, _ypos - 1)))
        {
            return true;
        }

        --_ypos;
    }

    return false;
}

char Search_Left(int _xpos, int _ypos)
{
    
    /* 벽에 부딪히거나, 밖으로 나가거나 */
    while (_xpos >= 0 && _xpos < GRID_WIDTH && _ypos >= 0 && _ypos < GRID_HEIGHT && g_Tile[_ypos][_xpos] != 1 )
    {
        /* 앞서 벽인지 확인했기 때문에 벽을 바꿀 일은 없음 */
        /* end 는 바꿔도 상관없음 값을 따로 저장해놈 */
        /* 만약 openList에 있는 값을 건들였다면.. 색은 바뀌겠지만 밑에서 확인함*/
        g_Tile[_ypos][_xpos] = currentColor;

        /* tile이 만들어진 곳이었다면 [closeList도 뒤져야 하나?] 정리가 안되네 */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == _xpos && (*iter)->yPos == _ypos)
            {
                return 2;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == _xpos && (*iter)->yPos == _ypos)
            {
                return 2;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (_xpos == x_end && _ypos == y_end)
        {
            return true;
        }

        /* 위나 아래가 코너라면 */
        if ((!tile_Check(_xpos , _ypos -1 ) && tile_Check(_xpos -1 , _ypos-1 )) ||
            (!tile_Check(_xpos , _ypos+1) && tile_Check(_xpos-1 , _ypos +1)))
        {
            return true;
        }

        --_xpos;
    }

    return false;
}

char Search_Down(int _xpos, int _ypos)
{
    
    /* 벽에 부딪히거나, 밖으로 나가거나 */
    while (_xpos >= 0 && _xpos < GRID_WIDTH && _ypos >= 0 && _ypos < GRID_HEIGHT && g_Tile[_ypos][_xpos] != 1)
    {
        g_Tile[_ypos][_xpos] = currentColor;

        /* tile이 만들어진 곳이었다면 [closeList도 뒤져야 하나?] 정리가 안되네 */
        list<stTile*>::iterator iter;
        for (iter = openList.begin(); iter != openList.end(); ++iter)
        {
            if ((*iter)->xPos == _xpos && (*iter)->yPos == _ypos)
            {
                return 2;
            }
        }
        /* closeList에 있다면 break!! */
        for (iter = MemoryCloseList.begin(); iter != MemoryCloseList.end(); ++iter)
        {
            if ((*iter)->xPos == _xpos && (*iter)->yPos == _ypos)
            {
                return 2;
            }
        }
        /*목적지랑 겹친다면 .. */
        if (_xpos == x_end && _ypos == y_end)
        {
            return true;
        }

        /* 왼쪽이나 오른쪽이 코너라면 */
        if ((!tile_Check(_xpos-1, _ypos) && tile_Check(_xpos-1, _ypos+1)) ||
            (!tile_Check(_xpos+1, _ypos) && tile_Check(_xpos+1, _ypos+1)))
        {
            return true;
        }

        ++_ypos;
    }
    return false;
}

int G_Cal(int current_xpos, int current_ypos, int parent_xpos, int parent_ypos, int parent_G)
{
    int base_xpos, base_ypos;
    int ed_xpos, ed_ypos;
    int x_move, y_move;

    /* base가 큰값 ed가 작은 값*/
    //2		10			10		2
    base_xpos = (parent_xpos < current_xpos ? current_xpos : parent_xpos);
    //2		10			10		2
    ed_xpos = (parent_xpos > current_xpos ? current_xpos : parent_xpos);

    //4		1			1		4
    base_ypos = (parent_ypos < current_ypos ? current_ypos : parent_ypos);
    //4		1			1		4
    ed_ypos = (parent_ypos > current_ypos ? current_ypos : parent_ypos);

    x_move = base_xpos - ed_xpos;
    y_move = base_ypos - ed_ypos;

    int _G = parent_G;
    int _abs = x_move - y_move;
    _abs = abs(_abs);
    int _max = max(x_move, y_move);

    for (int i = 0; i < _max - _abs; ++i)
    {
        _G += 14;
    }

    for (int i = 0; i < _abs; ++i)
    {
        _G += 10;
    }

    return _G;
}
int check_Direction(stTile* Cur)
{
    int p_xpos, p_ypos;
    int c_xpos, c_ypos;

    p_xpos = Cur->parent->xPos;
    p_ypos = Cur->parent->yPos;

    c_xpos = Cur->xPos;
    c_ypos = Cur->yPos;

    if (p_xpos == c_xpos)
    {
        if(p_ypos < c_ypos)
            return DD;
        if(p_ypos > c_ypos)
            return UU;
    }
    else if(p_xpos < c_xpos)
    {
        /* RR RD RU 로 확정 */
        if (p_ypos < c_ypos)
            return RD;
        if (p_ypos == c_ypos)
            return RR;
        if (p_ypos > c_ypos)
            return RU;
    }
    else // ( p_xpos > c_xpos)
    {
        /* LL LD LU 로 확정 */
        if (p_ypos < c_ypos)
            return LD;
        if (p_ypos == c_ypos)
            return LL;
        if (p_ypos > c_ypos)
            return LU;
    }
}
bool tile_Check(int m_xpos, int m_ypos)
{
    /* 장외라면 볼 것도 없이 장애물이 있다고 판단. */
    if (m_xpos == -1)
        return false;
    if (m_ypos == -1)
        return false;
    if (m_xpos == GRID_WIDTH)
        return false;
    if (m_ypos == GRID_HEIGHT)
        return false;

    /* 빈 곳이라면 1[true] 장애물이 있다면 0[false]*/
    if (g_Tile[m_ypos][m_xpos] == 1)
        return false;
    else
        return true;
}

/* 공용 */
bool F_Ascending(stTile* lhs, stTile* rhs)
{
    return lhs->F < rhs->F;
}

int Manhattan_Cal(stTile* _start, stTile* _end)
{
    int base_xpos, base_ypos;
    int ed_xpos, ed_ypos;

    base_xpos = (_start->xPos < _end->xPos ? _end->xPos : _start->xPos);
    ed_xpos = (_start->xPos > _end->xPos ? _end->xPos : _start->xPos);
    
    base_ypos = (_start->yPos < _end->yPos ? _end->yPos : _start->yPos);
    ed_ypos = (_start->yPos > _end->yPos ? _end->yPos : _start->yPos);

    /* Manhattan distance */
    int Manhattan = 0;

    while (base_xpos != ed_xpos) {
        ++ed_xpos;
        Manhattan += 10;
    }

    while (base_ypos != ed_ypos) {
        ++ed_ypos;
        Manhattan += 10;
    }

    return Manhattan;
}

void RenderObstacle(HDC hdc)
{
    int iX = 0;
    int iY = 0;
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush_gray);
    SelectObject(hdc, GetStockObject(NULL_PEN));

    for (int iCntW = 0; iCntW < GRID_WIDTH; iCntW++)
    {
        for (int iCntH = 0; iCntH < GRID_HEIGHT; iCntH++)
        {
            if (g_Tile[iCntH][iCntW] == WALLVALUE)
            {
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

        }
    }

    
    for (int iCntW = 0; iCntW < GRID_WIDTH; iCntW++)
    {
        for (int iCntH = 0; iCntH < GRID_HEIGHT; iCntH++)
        {
            
            if (g_Tile[iCntH][iCntW] == STARTVALUE)
            {
                SelectObject(hdc, g_hTileBrush_green);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == DESTVALUE)
            {
                SelectObject(hdc, g_hTileBrush_red);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == YELLOW_VALUE)
            {
                SelectObject(hdc, g_hTileBrush_yellow);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == BLUE_VALUE)
            {
                SelectObject(hdc, g_hTileBrush_blue);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == ORANGE_VALUE)
            {
                SelectObject(hdc, g_hTileBrush_orange);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

        }
    }

    /* currentColor */
    for (int iCntW = 0; iCntW < GRID_WIDTH; iCntW++)
    {
        for (int iCntH = 0; iCntH < GRID_HEIGHT; iCntH++)
        {
            if (g_Tile[iCntH][iCntW] == COLOR_A)
            {
                SelectObject(hdc, g_hTileBrush_a);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_B)
            {
                SelectObject(hdc, g_hTileBrush_b);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_C)
            {
                SelectObject(hdc, g_hTileBrush_c);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_D)
            {
                SelectObject(hdc, g_hTileBrush_d);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_E)
            {
                SelectObject(hdc, g_hTileBrush_e);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_F)
            {
                SelectObject(hdc, g_hTileBrush_f);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_G)
            {
                SelectObject(hdc, g_hTileBrush_g);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_H)
            {
                SelectObject(hdc, g_hTileBrush_h);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_I)
            {
                SelectObject(hdc, g_hTileBrush_i);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_J)
            {
                SelectObject(hdc, g_hTileBrush_j);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_K)
            {
                SelectObject(hdc, g_hTileBrush_k);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_L)
            {
                SelectObject(hdc, g_hTileBrush_l);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_M)
            {
                SelectObject(hdc, g_hTileBrush_m);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_N)
            {
                SelectObject(hdc, g_hTileBrush_n);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_O)
            {
                SelectObject(hdc, g_hTileBrush_o);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_P)
            {
                SelectObject(hdc, g_hTileBrush_p);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_Q)
            {
                SelectObject(hdc, g_hTileBrush_q);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_R)
            {
                SelectObject(hdc, g_hTileBrush_r);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_W)
            {
                SelectObject(hdc, g_hTileBrush_w);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_X)
            {
                SelectObject(hdc, g_hTileBrush_x);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }

            if (g_Tile[iCntH][iCntW] == COLOR_Y)
            {
                SelectObject(hdc, g_hTileBrush_y);
                iX = iCntW * GRID_SIZE;
                iY = iCntH * GRID_SIZE;
                Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
            }
        }
    }


    SelectObject(hdc, hOldBrush);

}

void Init_startPoint()
{
    for (int i = 0; i < GRID_HEIGHT; ++i)
    {
        for (int j = 0; j < GRID_WIDTH; ++j)
        {
            if (g_Tile[i][j] == STARTVALUE)
            {
                depart.xPos = j;
                depart.yPos = i;

                x_begin = j;
                y_begin = i;
                openList.push_back(&depart);
                return;
            }
        }
    }


}

void Init_destination()
{
    for (int i = 0; i < GRID_HEIGHT; ++i)
    {
        for (int j = 0; j < GRID_WIDTH; ++j)
        {
            if (g_Tile[i][j] == DESTVALUE)
            {
                dest.xPos = j;
                dest.yPos = i;

                x_end = j;
                y_end = i;

                return;
            }
        }
    }
}

void Init_Brush()
{
    g_hGridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    g_hTileBrush_gray = CreateSolidBrush(RGB(100, 100, 100));
    g_hTileBrush_green = CreateSolidBrush(RGB(100, 255, 100));
    g_hTileBrush_red = CreateSolidBrush(RGB(255, 0, 0));
    g_hTileBrush_yellow = CreateSolidBrush(RGB(255, 255, 0));
    g_hTileBrush_blue = CreateSolidBrush(RGB(137, 207, 240)); 
    g_hTileBrush_orange = CreateSolidBrush(RGB(255, 94, 00));

    /* change tile color , [102] 2~22 http://www.n2n.pe.kr/lev-1/color.htm */
    g_hTileBrush_a = CreateSolidBrush(RGB(102, 102, 0));
    g_hTileBrush_b = CreateSolidBrush(RGB(102, 102, 51));
    g_hTileBrush_c = CreateSolidBrush(RGB(102, 102, 153));
    g_hTileBrush_d = CreateSolidBrush(RGB(102, 102, 204));
    g_hTileBrush_e = CreateSolidBrush(RGB(102, 102, 255));
    g_hTileBrush_f = CreateSolidBrush(RGB(102, 153, 102));
    g_hTileBrush_g = CreateSolidBrush(RGB(102, 204, 102));
    g_hTileBrush_h = CreateSolidBrush(RGB(102, 255, 102));
    g_hTileBrush_i = CreateSolidBrush(RGB(102, 0, 102));
    g_hTileBrush_j = CreateSolidBrush(RGB(102, 51, 102));
    g_hTileBrush_k = CreateSolidBrush(RGB(102, 51, 153));
    g_hTileBrush_l = CreateSolidBrush(RGB(102, 51, 204));
    g_hTileBrush_m = CreateSolidBrush(RGB(102, 51, 255));
    g_hTileBrush_n = CreateSolidBrush(RGB(102, 51, 0));
    g_hTileBrush_o = CreateSolidBrush(RGB(102, 51, 51));
    g_hTileBrush_p = CreateSolidBrush(RGB(102, 0, 0));
    g_hTileBrush_q = CreateSolidBrush(RGB(102, 255, 255));
    g_hTileBrush_r = CreateSolidBrush(RGB(102, 204, 204));
    g_hTileBrush_w = CreateSolidBrush(RGB(102, 153, 153));
    g_hTileBrush_x = CreateSolidBrush(RGB(102, 153, 204));
    g_hTileBrush_y = CreateSolidBrush(RGB(102, 153, 255));

    h_red_pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
}

void DestLine(HDC hdc)
{
    /* 이놈은 없어도 됨,, */
    if (!road_draw)
        return;

    if (destpoint == nullptr)
        return;

    int half_grid = GRID_SIZE / 2;

    int iX = destpoint->xPos * GRID_SIZE + half_grid;
    int iY = destpoint->yPos * GRID_SIZE + half_grid;
    

    MoveToEx(hdc, iX, iY, NULL);

    SelectObject(hdc, h_red_pen);
    while (destpoint->xPos != x_begin || destpoint->yPos != y_begin)
    {
        destpoint = destpoint->parent;
        iX = destpoint->xPos * GRID_SIZE + half_grid;
        iY = destpoint->yPos * GRID_SIZE + half_grid;
        LineTo(hdc, iX , iY );
    }
}