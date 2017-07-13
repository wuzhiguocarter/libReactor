#ifndef _REACTOR_H
#define _REACTOR_H

// constants
#define MAX_EVENTS  1024
#define BUFLEN 128
#define SERV_PORT   8080

// typedefs
typedef struct myevent_s event_t;
struct myevent_s {
    int fd;                 //listenfd, connfd
    int events;             //EPOLLIN  EPLLOUT
    void *arg;              //指向自己结构体指针
    void (*call_back)(int fd, int events, void *arg);
    int status;
    char buf[BUFLEN];
    int len;
    long last_active;
};

typedef void (*call_back)(int, int, void *);

// export global variable
extern int g_efd;
extern event_t g_events[MAX_EVENTS+1];

// export functions
/* Event Ops*/
void eventset(event_t *ev, int fd, call_back cb, void *arg);
void eventadd(int efd, int events, event_t *ev);
void eventdel(int efd, event_t *ev);

/* tcp server and client initializations*/
void initlistensocket(int efd, short port);	// for tcp server
void initconnectsocket(int efd, short port); // for tcp client

/* IO Event Handlers*/
void acceptconn(int lfd, int events, void *arg);
void recvdata(int fd, int events, void *arg);
void senddata(int fd, int events, void *arg);
#endif