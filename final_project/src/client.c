#include <ncurses.h>
#include <locale.h>
#include <title.h>
#include <connection.h>
#include <signal.h>
#include <ctype.h>

int main(int argc,char* argv[]){
    srand(time(NULL)); // 初始化随机数生成器
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

            wclear(popup_win);  // 清空窗口
            wrefresh(popup_win);  // 刷新窗口
            delwin(popup_win);  // 删除窗口
        }
        else if (input == 27)
        {
            //退出
            break;
        }else if (input == '\n'){
            //加入房间
            // 创建一个新的窗口，位置在房间列表窗口的旁边
            WINDOW* input_win = newwin(5, 30, 0, COLS - 30);
            box(input_win, 0, 0);  // 绘制边框
            mvwprintw(input_win, 2, 1, "请输入4位房间ID:");  // 打印提示信息
            wrefresh(input_win);  // 刷新窗口，显示提示信息

            char room_id[5];  // 用于存储用户输入的房间ID
            echo();  // 开启回显
            curs_set(1);  // 显示光标
            mvwgetnstr(input_win, 3, 10, room_id, 4);  // 获取用户的输入，限制输入4个字符

            char player_name[11];  // 用于存储用户输入的昵称
            wclear(input_win);  // 清空窗口
            box(input_win, 0, 0);  // 绘制边框
            mvwprintw(input_win, 2, 1, "请输入昵称(最大10个字符):");  // 打印提示信息
            wrefresh(input_win);  // 刷新窗口，显示提示信息
            mvwgetnstr(input_win, 3, 10, player_name, 10);  // 获取用户的输入


            curs_set(0);  // 隐藏光标
            noecho();  // 关闭回显

            bool flag = false;
            // 检查用户输入的是否是4位整数
            for (int i = 0; i < 4; i++) {
                if (!isdigit(room_id[i])) {
                    mvwprintw(input_win, 4, 1, "输入的房间ID必须是4位整数");
                    wrefresh(input_win);
                    flag = true;
                    break; 
                }
            }
        
            wclear(input_win);  // 清空窗口
            wrefresh(input_win);  // 刷新窗口  
            delwin(input_win);  // 删除窗口
            
            

            if (!flag) {
                uint16_t id = atoi(room_id);
                local_room_id = id;
                strcpy(local_player_name, player_name); 
                send_request(join_room(id, player_name));
            }   
        }
    }
    
    endwin();
    return 0;
}