#include <ncurses.h>
#include <locale.h>
#include <title.h>
#include <roomlist.h>
#include <connection.h>
#include <signal.h>
int serverfd;


int main(int argc,char* argv[]){
    setlocale(LC_ALL, ""); // 设置locale以支持中文
    initscr();
    start_color();//支持彩色
    raw(); // 禁用行缓冲
    noecho(); // 禁用回显
    curs_set(0); // 隐藏光标

    signal(SIGWINCH, handle_winch); // 当终端窗口大小改变时，调用handle_winch函数
    draw_title(); // 绘制欢迎标题
    serverfd = connect_to_server(); // 连接到服务器
    show_server_status(serverfd);
    
    refresh();


    if(serverfd == -1){
        getch();
        endwin();
        return 0;
    }

    getch();
    WINDOW* roomlist_win = init_roomlist_win(); // 初始化房间列表窗口
    
    endwin();
    return 0;
}