#ifndef BACKGROUNDWORKER_H
#define BACKGROUNDWORKER_H

#include <QObject>
#include <QThread>
#include <QSharedPointer>
#include "m_cmd.h"
#include "m_tcb.h"
#include <iostream>
#include <list>

#include "m_defs.h"

class mController;

class mWorker : public QThread
{
    Q_OBJECT
public:
    //TODO: refine & unify the callback mechanism(such as std::bind...)
    typedef void (__stdcall * void_callback)();
	typedef void (__stdcall * void_callback_arg)(int *p_val);

    explicit mWorker(const APKey key,void_callback cb=NULL,
                              void_callback cb_ok= NULL, void_callback_arg cb_question=NULL,
							  QObject *parent = 0);

    ~mWorker();

    void SetCallback(TCB* cb) { current_cb = cb; }

    void AddJob(const QSharedPointer<mSetting> &job);

    void ClearJobQueue(){ qInfo() << "num of jobs:" << job_settings.size(); job_settings.clear(); }

    bool is_stopping() const { return stop_flag == AP_STOP_FLAG; }

public:
    bool boot_stop() const { return stop_flag; }

protected:
    virtual void run();

private:
    const APKey session_id_;

    TCB *current_cb;

    std::list<QSharedPointer<mSetting> > job_settings;

    int stop_flag;

    void_callback finished_cb;
    void_callback ok_cb;
	void_callback_arg question_cb;

    mController *control_;

signals:
    void signal_err_msg(int err_code, const std::string& err_msg);
    void signal_finish();

public slots:
    void slot_stop_executing();

};

#endif // BACKGROUNDWORKER_H
