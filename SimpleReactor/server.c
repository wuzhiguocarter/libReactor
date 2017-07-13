﻿#include "reactor.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>

extern int g_efd;
extern event_t g_events[MAX_EVENTS+1];

int main(int argc, char *argv[])
{
    unsigned short port = SERV_PORT;

    if (argc == 2)
        port = atoi(argv[1]);

    g_efd = epoll_create(MAX_EVENTS+1);

    if (g_efd <= 0)
        printf("create efd in %s err %s\n", __func__, strerror(errno));

    initlistensocket(g_efd, port);

    /* 事件掩码 */
    struct epoll_event events[MAX_EVENTS+1];

    printf("server running:port[%d]\n", port);
    int checkpos = 0, i;
    while (1) {
        /* 超时验证，每次测试100个链接，不测试listeactive_cnt  
        当客户端60秒内没有和服务器通信，则关闭此客户端链接 */
        long now = time(NULL);
        for (i = 0; i < 100; i++, checkpos++) {
            if (checkpos == MAX_EVENTS)
                checkpos = 0;
            if (g_events[checkpos].status != 1)
                continue;
            long duration = now - g_events[checkpos].last_active;
            if (duration >= 60) {
                close(g_events[checkpos].fd);
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
                eventdel(g_efd, &g_events[checkpos]);
            }
        }

        // 等待事件
        int active_cnt  = epoll_wait(g_efd, events, MAX_EVENTS+1, 1000);
        if (active_cnt  < 0) {
            printf("epoll_wait error, exit\n");
            break;
        }
        // 处理事件
        for (i = 0; i < active_cnt ; i++) {
            event_t *ev = (event_t *)events[i].data.ptr;
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) {
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) {
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
        }
    }

    /* 退出前释放所有资源 */
    return 0;
}	
