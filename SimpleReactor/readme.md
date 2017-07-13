# 一个简单的Reactor模式的C语言实现

---

## Features

- 封装epoll api
- 定义事件管理数据结构event_t
- 一个event_t对象对应一个描述符
- event_t对象的接口有eventset, eventadd, eventdel(调用epoll_ctl实现)
- 封装tcp建立连接、connfd可读、connfd写的回调函数

参考：[libevent深入浅出](https://aceld.gitbooks.io/libevent/)