#include <windows.h>
#include <vector>
#include <ctime>
#include <cstdlib>


struct Point {
    int x, y;
};

const int width = 20;
const int height = 20;

std::vector<Point> snake;
Point food;
int score = 0;
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir = STOP;

HWND hwnd;
HDC hdc;

void InitGame() {
    dir = STOP;
    snake.clear();
    snake.push_back({ width / 2, height / 2 });
    food = { rand() % width, rand() % height };
    score = 0;
}

void Draw() {
    HBRUSH blackBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, blackBrush);

    RECT rect = { 0, 0, width * 20, height * 20 };
    FillRect(hdc, &rect, blackBrush);

    DeleteObject(blackBrush);

    HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
    SelectObject(hdc, redBrush);
    Rectangle(hdc, food.x * 20, food.y * 20, (food.x + 1) * 20, (food.y + 1) * 20);
    DeleteObject(redBrush);

    HBRUSH greenBrush = CreateSolidBrush(RGB(0, 255, 0));
    SelectObject(hdc, greenBrush);
    for (const auto& segment : snake) {
        Rectangle(hdc, segment.x * 20, segment.y * 20, (segment.x + 1) * 20, (segment.y + 1) * 20);
    }
    DeleteObject(greenBrush);

    wchar_t scoreText[50];
    swprintf_s(scoreText, L"Score: %d", score);
    TextOutW(hdc, 10, 10, scoreText, wcslen(scoreText));
}

void MoveSnake() {
    Point newHead = snake.front();

    switch (dir) {
    case LEFT:  newHead.x--; break;
    case RIGHT: newHead.x++; break;
    case UP:    newHead.y--; break;
    case DOWN:  newHead.y++; break;
    default: return;
    }

    if (newHead.x < 0 || newHead.x >= width || newHead.y < 0 || newHead.y >= height) {
        MessageBoxW(hwnd, L"Game Over!", L"Snake", MB_OK);
        InitGame();
        return;
    }

    for (const auto& segment : snake) {
        if (segment.x == newHead.x && segment.y == newHead.y) {
            MessageBoxW(hwnd, L"Game Over!", L"Snake", MB_OK);
            InitGame();
            return;
        }
    }

    snake.insert(snake.begin(), newHead);

    if (newHead.x == food.x && newHead.y == food.y) {
        score += 10;
        food = { rand() % width, rand() % height };
    }
    else {
        snake.pop_back();
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdcPaint = BeginPaint(hwnd, &ps);
        Draw();
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_LEFT:  if (dir != RIGHT) dir = LEFT; break;
        case VK_RIGHT: if (dir != LEFT) dir = RIGHT; break;
        case VK_UP:    if (dir != DOWN) dir = UP; break;
        case VK_DOWN:  if (dir != UP) dir = DOWN; break;
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI GameLoop(LPVOID lpParam) {
    srand(static_cast<unsigned int>(time(nullptr)));
    InitGame();

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"SnakeGame";
    RegisterClassW(&wc);

    hwnd = CreateWindowExW(0, L"SnakeGame", L"Snake Game", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, width * 20 + 16, height * 20 + 39,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    hdc = GetDC(hwnd);

    MSG msg;
    while (true) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                ReleaseDC(hwnd, hdc);
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        MoveSnake();
        InvalidateRect(hwnd, NULL, TRUE);
        Sleep(100);
    }

    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, GameLoop, NULL, 0, NULL);
    }
    return TRUE;
}
