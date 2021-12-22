#ifndef mController_H
#define mController_H

#include <QObject>
#include <QtCore/QThread>
#include <list>
#include "m_defs.h"
#include "m_cmd.h"
#include <QSharedPointer>
#include "m_glob.h"
#include "m_worker.h"

class MainUI;
class TCB;
class mWorker;

class CoreHandle{};

class mController : public QObject
{
    Q_OBJECT
public:
    explicit mController(QObject *parent, MainUI * window);
    ~mController();

    void QueueAJob(const QSharedPointer<mSetting> &job);

    void StartExecuting(TCB *callback);

    void StopByUser();

    void StopByTool();

    bool isStop();

    bool is_stop_by_user() const{return is_stop_by_user_;}

public:

    APKey NewAPKey();

    CoreHandle* GetHandle(const APKey key);

    void FreeHandle(const APKey key);

    unsigned int GetHandleCount() const { return handles_map.size(); }

    void FreeAll();

private:
    std::map<APKey, CoreHandle*> handles_map;
private:
    void CreateNewThread();

    MainUI *mWindow;
    APKey session_id;

    bool is_stop_by_user_;

    mWorker *m_worker;

signals:
    void signal_err_msg(int err_code, const std::string& err_msg);
    void signal_stop_working();
    void signal_conn_init();
    void signal_show_ok();

public slots:

};

#endif // mController_H
