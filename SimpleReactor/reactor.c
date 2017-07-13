// tab-size = 4
#include "reactor.h"

#include <stdlib.h>
#include <stdio.h> 
#include <stdio.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

// export global variable defs
int g_efd; 						// epoll_create返回的句柄 
event_t g_events[MAX_EVENTS+1];	// +1 最后一个用于 listen fd


// Event Ops
void eventset(event_t *ev, 
				int fd, 
				call_back cb, 
				void *arg)
{
    ev->fd = fd;
    ev->call_back = cb;
    ev->events = 0;
    ev->arg = arg;
    ev->status = 0;
    //memset(ev->buf, 0, sizeof(ev->buf));
    //ev->len = 0;
    ev->last_active = time(NULL);

    return;
}

void eventadd(int efd, int events, event_t *ev)	// epoll_ctl
{
    struct epoll_event epv = {0, {0}};
    int op;
    epv.data.ptr = ev;
    epv.events = ev->events = events;

    if (ev->status == 1) {
        op = EPOLL_CTL_MOD;
    } 
    else {
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }

    if (epoll_ctl(efd, op, ev->fd, &epv) < 0)
        printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
    else
        printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, op, events);

    return;
}

void eventdel(int efd, event_t *ev) // epoll_ctl
{
    struct epoll_event epv = {0, {0}};

    if (ev->status != 1)
        return;

    epv.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);

    return;
}

// tcp server and client initializations
void initlistensocket(int efd, short port)
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    // 设置监听事件
    // 设置监听描述符为非阻塞
    fcntl(lfd, F_SETFL, O_NONBLOCK);
    // 将该监听描述符的事件处理注册到event_t的一个实例g_events[MAX_EVENTS]
    eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
    // 将g_events[MAX_EVENTS]的EPOLLIN事件添加到epoll instance(epoll descriptor/handle)
    eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);

    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);

    bind(lfd, (struct sockaddr *)&sin, sizeof(sin));

    listen(lfd, 20);

    return;
}

// handlers
// 连接建立->可读->可写
void acceptconn(int lfd, int events, void *arg)
{
    struct sockaddr_in cin;
    socklen_t len = sizeof(cin);
    int cfd, i;

    if ((cfd = accept(lfd, (struct sockaddr *)&cin, &len)) == -1) {
        if (errno != EAGAIN && errno != EINTR) {
            /* 暂时不做出错处理 */
        }
        printf("%s: accept, %s\n", __func__, strerror(errno));
        return;
    }

    do {
        for (i = 0; i < MAX_EVENTS; i++) {
            if (g_events[i].status == 0)
                break;
        }

        if (i == MAX_EVENTS) {
            printf("%s: max connect limit[%d]\n", __func__, MAX_EVENTS);
            break;
        }

        int flag = 0;
        if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0)
        {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }

        eventset(&g_events[i], cfd, recvdata, &g_events[i]);
        eventadd(g_efd, EPOLLIN, &g_events[i]);
    } while(0);

    printf("new connect [%s:%d][time:%ld], pos[%d]\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active, i);

    return;
}

void recvdata(int fd, int events, void *arg)
{
    event_t *ev = (event_t *)arg;
    int len;

    len = recv(fd, ev->buf, sizeof(ev->buf), 0);

    // 删除读事件
    eventdel(g_efd, ev);

    if (len > 0) {
        ev->len = len;
        ev->buf[len] = '\0';
        printf("C[%d]:%s\n", fd, ev->buf);
        // 添加写事件
        eventset(ev, fd, senddata, ev);
        eventadd(g_efd, EPOLLOUT, ev);
    }
    else if (len == 0) {
        close(ev->fd);
        /* ev-g_events 地址相减得到偏移元素位置 */
        printf("[fd=%d] pos[%d], closed\n", fd, (int)(ev - g_events));
    }
    else {
        close(ev->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }

    return;
}

void senddata(int fd, int events, void *arg)
{
    event_t *ev = (event_t *)arg;
    int len;

    len = send(fd, ev->buf, ev->len, 0);
    //printf("fd=%d\tev->buf=%s\ttev->len=%d\n", fd, ev->buf, ev->len);
    //printf("send len = %d\n", len);

    // 删除写事件
    eventdel(g_efd, ev);
    if (len > 0) {
        printf("send[fd=%d], [%d]%s\n", fd, len, ev->buf);
        // 添加读事件
        eventset(ev, fd, recvdata, ev);
        eventadd(g_efd, EPOLLIN, ev);
    }
    else {
        close(ev->fd);
        printf("send[fd=%d] error %s\n", fd, strerror(errno));
    }

    return;
}
