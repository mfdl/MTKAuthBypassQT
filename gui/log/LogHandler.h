#ifndef LOGHANDLER_H
#define LOGHANDLER_H
#include "Singleton.h"
struct LogHandlerPrivate;
class LogHandler {
    SINGLETON(LogHandler) // 使用单例模式
public:
    void release(); // 释放资源
    void installMessageHandler(); // 给 Qt 安装消息处理函数
private:
    LogHandlerPrivate *d;
};
#endif // LOGHANDLER_H