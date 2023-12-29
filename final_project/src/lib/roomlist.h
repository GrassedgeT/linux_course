#include <ncurses.h>


WINDOW* init_roomlist_win() {
    WINDOW *win = newwin(0, 0, 0, 0); // 创建一个新窗口，其大小与终端窗口相同
    box(win, 0, 0);
    wrefresh(win);
    wgetch(win);
    delwin(win);
    return win;
}