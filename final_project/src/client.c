#include <ncurses.h>
#include <locale.h>
#include <title.h>
#include <connection.h>
#include <signal.h>


int main(int argc,char* argv[]){
    setlocale(LC_ALL, ""); // 设置locale以支持中文
    initscr();
    start_color();//支持彩色
    raw(); // 禁用行缓冲
    noecho(); // 禁用回显
    curs_set(0); // 隐藏光标

    signal(SIGWINCH, handle_winch); // 当终端窗口大小改变时，调用handle_winch函数
    draw_title(); // 绘制欢迎标题
    connect_to_server(); // 连接到服务器
    show_server_status(sockfd);
    refresh(); 


    if(sockfd == -1){
        getch();
        endwin();
        return 0;
    }

    getch();
    roomlist_win = newwin(0, 0, 0, 0); // 初始化房间列表窗口
    send_request(get_roomlist());
    while(1){
        char input = wgetch(roomlist_win);
        if (input == 'c')
        {
            //创建房间
             // 创建房间
            wclear(roomlist_win);  // 清空窗口
            wrefresh(roomlist_win);  // 刷新窗口
            WINDOW* popup_win = newwin(5, 30, (LINES - 5) / 2, (COLS - 30) / 2);  // 创建一个新的窗口
            box(popup_win, 0, 0);  // 绘制边框
            mvwprintw(popup_win, 2, 1, "请输入房间名(最大10个字符):");  // 打印提示信息
            wrefresh(popup_win);  // 刷新窗口，显示提示信息

            char room_name[11];
            echo();  // 开启回显
            curs_set(1);  // 显示光标
            mvwgetnstr(popup_win, 3, 10, room_name, 10);  // 获取用户的输入
            curs_set(0);  // 隐藏光标
            noecho();  // 关闭回显

            if (strlen(room_name) == 0) {
                // 房间名为空
                mvwprintw(popup_win, 4, 1, "房间名不能为空");
                wrefresh(popup_win);
                getch();
                delwin(popup_win);
            }
            send_request(create_room(room_name));

            delwin(popup_win);  // 删除窗口
        }
        else if (input == 27)
        {
            //退出
            break;
        }
    }
    
    endwin();
    return 0;
}