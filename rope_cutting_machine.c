#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#define WINDOW_WIDTH 50
#define WINDOW_HEIGHT 56

#define SLEEP_TIME_MIN 12
#define SLEEP_TIME_MAX 19

char screen[WINDOW_HEIGHT][WINDOW_WIDTH + 2] = {{0}};
char doll[20][36] = {
    "          ___   .--.                ",
    "    .--.-\"   \"-' .- |               ",
    "   / .-,`          .'               ",
    "   \\   `           \\                ",
    "    '.            ! \\               ",
    "      |     !  .--.  |              ",
    "      \\        '--'  /   __         ",
    "      `-.     \\__,'.'. .`  `\\       ",
    "        \\`-.____.-' `\\      /       ",
    "       .`'-'._/-`     \\----'    _   ",
    "   ,-'`  /             |    _.-' `\\ ",
    " .'     /              |--'`     / |",
    "/      /\\              `         | |",
    "|   .\\/  \\      .--. __          \\ |",
    " '-'      '._       /  `\\         / ",
    "             `\\    '     |------'`  ",
    "               \\  |      |          ",
    "                \\        /          ",
    "                 '._  _.'           ",
    "                    ``              "
};

int loose = 0;
int win = 0;

int get_mil_time() {
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    return ((tm.wHour * 60 + tm.wMinute) * 60 + tm.wSecond ) * 1000 + tm.wMilliseconds;
}

void init_console() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        return;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        return;
    }

    /* 获取当前窗口句柄 */
    HWND wh = GetConsoleWindow();

    /* 调整窗口大小和位置 */
    MoveWindow(wh, 600, 0, WINDOW_WIDTH * 10, WINDOW_HEIGHT * 18, TRUE);

    return;
}

void set_cursor_position(short y, short x) {
    printf("%c8", 0x1B);
    printf("%c[%dC", 0x1B, x);
    printf("%c[%dB", 0x1B, y);
    return;
}

void draw_box() {
    int x, y;
    /* 画竖线 */
    for (y = 1; y < 56; y++) {
        screen[y][0] = '#';
        if (y < 32) screen[y][WINDOW_WIDTH-1] = '#';
    }

    /* 画横线 */
    for (x = 0; x < WINDOW_WIDTH; x++) {
        screen[0][x] = '=';
        screen[WINDOW_HEIGHT-1][x] = '=';
    }
    strncpy(screen[31], "#=====", strlen("|====="));
    strncpy(&screen[31][WINDOW_WIDTH-strlen("=====+")], "=====+", strlen("=====+"));

    /* 画角 */
    screen[0][0] = '+';
    screen[0][WINDOW_WIDTH - 1] = '+';
    screen[WINDOW_HEIGHT - 1][0] = '+';

    return;
}

void draw_rope() {
    int y;
    for (y = 1; y < 8; y++) {
        screen[y][22] = '|';
    }
    return;
}

void draw_doll() {
    int i;
    for (i = 0; i < 20; i++ ) {
        strncpy(&screen[i + 7][7], doll[i], 36);
    }
    return;
}

void draw_knife() {
    int x;
    screen[4][42] = '=';
    for (x = 43; x < WINDOW_WIDTH - 1; x++) {
        screen[4][x] = '-';
    }
    return;
}

void print_score() {
    set_cursor_position(56, 0);
    printf("Total %3d    Loose %3d    win %3d", loose + win, loose, win);
}

void print_result(char *message) {
    set_cursor_position(58, 0);
    printf("%-50s\n", message);
    return;
}

void init_screen() {
    printf("%c8", 0x1B);
    //set_cursor_position(0, 0);
    int x, y;
    for (y = 0; y < WINDOW_HEIGHT; y++) {
        for (x = 0; x < WINDOW_WIDTH; x++) {
            screen[y][x] = ' ';
        }
        screen[y][x] = '\n';
    }
    draw_box();
    draw_doll();
    draw_rope();
    draw_knife();
    for (y = 0; y < WINDOW_HEIGHT; y++) {
        printf(screen[y]);
    }
    for (x = 0; x < WINDOW_WIDTH; x++) {
        putchar(' ');
    }
    
    /* 打印成绩 */
    print_score();

    /* 打印帮助信息 */
    set_cursor_position(57, 0);
    printf("Press Enter to start, press Shift to next round");

    print_result(" ");
    return;
}

int run() {
    short key_ret;
    int chr;
    int x = 42;
    int last_time, cur_time;
    int sleep_time;

    /* 监听 Enter 是否按下 */
    while (1) {
        key_ret = GetAsyncKeyState(VK_RETURN);
        if (key_ret != 0) break;
    }

    /* 在 Enter 松开之前，使剪刀前进 */
    last_time = get_mil_time();
    srand(last_time);
    sleep_time = ((double)rand() / RAND_MAX) * (SLEEP_TIME_MAX - SLEEP_TIME_MIN) + SLEEP_TIME_MIN;
    while (1) {
        Sleep(1);

        /* 检查 Enter 键是否还被按着 */
        key_ret = GetAsyncKeyState(VK_RETURN);
        if (key_ret == 0) break;

        /* 把控速度 */
        cur_time = get_mil_time();
        if (cur_time - last_time < sleep_time) continue;
        last_time = cur_time;

        /* 左止点 */
        if (x < 12) break;

        set_cursor_position(4, x);
        printf("=-");
        x--;
    }

    /* 15 把内不让赢 */
    if (x == 21 && win + loose <= 15) {
        Sleep(sleep_time);
        set_cursor_position(4, x);
        printf("=-");
        x--;
    }

    /* 判断是否剪到 */
    if (x == 21) {
        return 1;
    } else {
        return 0;
    }
}

void move_doll(int cur_y, int cur_x, int next_y, int next_x) {
    int i, x, y;
    for (y = cur_y; y < cur_y + 20; y++) {
        set_cursor_position(y, cur_x);
        printf("%36s", " ");
    }

    for (i = 0; i < 20; i++) {
        set_cursor_position(i + next_y, next_x);
        printf("%.*s", 36, doll[i]);
    }
    return;
}

void doll_fall() {
    int i, y;
    double sleep_time;

    /* 擦除线的下半部分 */
    for (y = 5; y < 8; y++) {
        set_cursor_position(y, 22);
        putchar(' ');
    }

    /* 下落 */
    for (i = 0; i < 28; i++) {
        /* 重力引擎，放大了重力加速度 */
        sleep_time = 1000 * (sqrt(0.001 * (i + 1)) - sqrt(0.001 * i));
        Sleep((int)sleep_time);
        move_doll(7 + i, 7, 8 + i, 7);
    }

    /* 第一次回弹，直接写死 */
    Sleep(8);
    move_doll(35, 7, 34, 7);
    Sleep(10);
    move_doll(34, 7, 33, 7);
    Sleep(13);
    move_doll(33, 7, 32, 7);
    Sleep(31);
    move_doll(32, 7, 31, 7);
    Sleep(31);
    move_doll(31, 7, 32, 7);
    Sleep(13);
    move_doll(32, 7, 33, 7);
    Sleep(10);
    move_doll(33, 7, 34, 7);
    Sleep(8);
    move_doll(34, 7, 35, 7);

    /* 第二次回弹，直接写死 */
    Sleep(13);
    move_doll(35, 7, 34, 7);
    Sleep(31);
    move_doll(34, 7, 33, 7);
    Sleep(31);
    move_doll(33, 7, 34, 7);
    Sleep(13);
    move_doll(34, 7, 35, 7);
    return;
}

void show_result(int result) {
    if (result == 0) {
        loose++;
        print_score();
        print_result("Fiald in this round !");
    }
    else {
        win++;
        print_score();
        print_result("Congratulations ! You Won !");
        doll_fall();
    }
    return;
}

int main (void) {
    int cut_result;
    short key_ret;

    /* 初始化窗口 */
    init_console();

    /* 记录当前光标位置 */
    printf("%c7", 0x1B);

    while (1) {
        /* 打印初始屏幕 */
        init_screen();

        /* 运行 */
        cut_result = run();

        /* 打印运行结果 */
        show_result(cut_result);

        /* 按 Shift 键重开 */
        while (1) {
            key_ret = GetAsyncKeyState(VK_LSHIFT);
            if (key_ret != 0) break;
            key_ret = GetAsyncKeyState(VK_RSHIFT);
            if (key_ret != 0) break;
        }
    }
    return 0;
}
