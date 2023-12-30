//存放对首页的相关操作
#include <ncurses.h>
#include <string.h>

void draw_title(){

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);

    char* title[] = {
        "  GGGG  RRRRR   AAA  SSSSS SSSSS EEEEE DDDDD  GGGG  EEEEE ",
        " G    G R    R A   A S     S     E     D    D G   G E   ",
        " G      RRRRR  AAAAA SSSSS SSSSS EEEEE D    D G     EEEEE ",
        " G   GG R  R   A   A     S     S E     D    D G  GG E    ",
        "  GGGGG R   RR A   A SSSSS SSSSS EEEEE DDDDD  GGGGG EEEEE "
    };

    int start_x = (COLS - strlen(title[0])) / 2;
    int start_y = LINES / 2 - sizeof(title) / sizeof(title[0]) / 2;

    for (int i = 0; i < sizeof(title) / sizeof(title[0]); i++) {
        for (int j = 0; j < strlen(title[i]); j++) {
            attron(COLOR_PAIR((title[i][j] - 'A' + 1) % 7 + 1));
            mvaddch(start_y + i, start_x + j, title[i][j]);
            attroff(COLOR_PAIR((title[i][j] - 'A' + 1) % 7 + 1));
        }
    }

    char* creator = "制作人：20217022曹方泽";
    mvwprintw(stdscr, LINES - 12, (COLS - strlen(creator)) / 2 + 5, "%s", creator);
    
};

void handle_winch(int sig){
    endwin();
    clear();
    draw_title();
    refresh();
};

void show_server_status(int server_fd){
    if(server_fd == -1){
        char* error = "连接服务器失败：按下任意键退出";
        mvwprintw(stdscr, LINES - 10, (COLS - strlen(error)) / 2 + 9, "%s", error);
    }
    else{
        char* success = "连接服务器成功: 按下任意键开始游戏";
        mvwprintw(stdscr, LINES - 10, (COLS - strlen(success)) / 2 + 9, "%s", success);
    }
};