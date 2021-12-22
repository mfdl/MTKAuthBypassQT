#include "m_worker.h"
#include "m_exception.h"
#include "m_controller.h"
#include "mainui.h"
#include "ui_mainui.h"
#include "m_callback.h"

mWorker::mWorker(const APKey key,
                 void_callback cb,
                 void_callback cb_ok,
                 void_callback_arg cb_question,
                 QObject *parent) :
    QThread(parent),
    session_id_(key),
    current_cb(NULL),
    stop_flag(AP_STOP_FLAG),
    finished_cb(cb),
    ok_cb(cb_ok),
    question_cb(cb_question)
{
    qDebug().nospace() << "ctor of " << this;
}

mWorker::~mWorker()
{
    qDebug().nospace() << "dtor of " << this;

    this->terminate();
    this->wait();

    if (current_cb != NULL)
    {
        delete current_cb;
        current_cb = NULL;
    }
}

void mWorker::AddJob(const QSharedPointer<mSetting> &job)
{
    job->set_stop_flag(&stop_flag);
    job_settings.push_back(job);
}

void mWorker::run()
{
    stop_flag = 0;
    bool isOK = true;
    try
    {
        for(std::list<QSharedPointer<mSetting> >::const_iterator it = job_settings.begin();
            it != job_settings.end(); ++it)
        {
            QSharedPointer<CMD> cmd = (*it)->CreateCommand(session_id_);
            cmd->exec();
        }
    }
    catch(const StopException& /*user_stop*/)
    {
        qInfo("User stopped.");
        send_log_red("stopped by user request!");
        isOK = false;
    }
    catch(const mException& brom_exp)
    {
        if(is_stopping())
        {
            //user stop
            qInfo("User stopped.");
            send_log_red("stopped by user request!");
        }
        else
        {
            QString err_msg = 0;//mCallback::get_err_msg(brom_exp.err_code());
            qInfo("IO Exception! (%d)(%s)(%s)", brom_exp.err_code(), err_msg.toStdString().c_str(),
                  brom_exp.context().c_str());

            QString exception = brom_exp.context().c_str();
            if(exception.contains(","))
            {
                QString func = exception.split(',').at(0);
                QString clas = exception.split(',').at(1);
                QString line = exception.split(',').at(2);

                send_log_succes(QString("● ReprtInfo:{%0]}").arg(clas));
                send_log_normal("● ReprtInfo:{function(");
                send_log_succes(QString("%0:codeline:%1)]}").arg(func, line));
            }
//                                                                                                          brom_exp.context().c_str());

            emit signal_err_msg(brom_exp.err_code(), brom_exp.err_msg());
        }

        isOK = false;
    }
    catch(const APPException& app_exp)
    {
        qInfo("App Exception! (%s)(%s)", app_exp.err_msg().c_str(),
              app_exp.context().c_str());

        QString full_err = QString("(%0)(%1)\n(%2)").arg(app_exp.err_code()).arg(app_exp.err_msg().c_str(),
                                                                                                    app_exp.context().c_str());

        send_log_red(full_err.trimmed());

        emit signal_err_msg(app_exp.err_code(), app_exp.err_msg());

        isOK = false;
    }
    catch(const MException& base_exp)
    {
        qInfo("Base Exception! (%s)(%s)", base_exp.err_msg().c_str(),
              base_exp.context().c_str());

        qInfo("Base Exception! (%s)(%s)", base_exp.err_msg().c_str(),
              base_exp.context().c_str());

        QString full_err = QString("(%0)(%1)\n(%2)").arg(base_exp.err_code()).arg(base_exp.err_msg().c_str(),
                                                                                                      base_exp.context().c_str());

        send_log_red(full_err.trimmed());

        emit signal_err_msg(-1, base_exp.err_msg());

        isOK = false;
    }
    catch(const std::exception &std_exp)
    {
        qInfo("Uncaught std::exception : %s", std_exp.what());

        send_log_red(QString("Uncaught std::exception : %0").arg(std_exp.what()));

        isOK = false;
    }
    catch(const QUnhandledException& exp)
    {
        qInfo("QUnhandledException! (%s)", exp.what());

        send_log_red(QString("Uncaught QUnhandledException : %0").arg(exp.what()));

        isOK = false;
    }
    catch(const QString &str_exp)
    {
        send_log_fail(QString("\n%0").arg(str_exp));

        isOK = false;
    }
    catch(...)
    {
        qInfo("Uncaught std::exception!");

        send_log_red("Uncaught std::exception!");

        isOK = false;
    }
    //    control_->set_loop_dl_ok(isOK);

    if(current_cb != NULL)
    {
        current_cb->Execute();
    }

    if(finished_cb != NULL)
    {
        finished_cb();
    }

    if(ok_cb != NULL && isOK)
    {
        ok_cb();
    }

    ClearJobQueue();
}

void mWorker::slot_stop_executing()
{
    stop_flag = AP_STOP_FLAG;
    QString stop_0 = QString().sprintf("0x%08x", stop_flag).toLower();
    QString stop_1 = QString().setNum(stop_flag).toLower();
    qInfo() << QString("mWorker::slot_stop_executing:%0:%1").arg(stop_0, stop_1);
}
