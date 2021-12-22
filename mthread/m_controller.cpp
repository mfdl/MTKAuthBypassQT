#include "m_controller.h"
#include "mainui.h"
#include "m_tcb.h"
#include "m_worker.h"
#include <QMetaType>

//mController implementation...
mController::mController(QObject *parent, MainUI *window) :
    QObject(parent),
    mWindow(window),
    is_stop_by_user_(false),
    m_worker(NULL)
{
    qDebug().nospace() << "ctor of " << this;

    session_id = NewAPKey();

    CreateNewThread();
}

APKey mController::NewAPKey()
{
    APKey new_key = qrand();
    handles_map.insert(std::pair<APKey, CoreHandle*>(new_key, new CoreHandle()));
    return new_key;
}

CoreHandle *mController::GetHandle(const APKey key)
{
    if(handles_map.find(key) != handles_map.end())
    {
        return handles_map[key];
    }
    return NULL;
}

void mController::FreeHandle(const APKey key)
{
    if(handles_map.find(key) != handles_map.end())
    {
        delete handles_map[key];
        handles_map.erase(key);
    }
}

void mController::FreeAll()
{
    for(std::map<int, CoreHandle*>::const_iterator it
        = handles_map.begin(); it!= handles_map.end(); ++it)
    {
        delete it->second;
    }
    handles_map.clear();
}

mController::~mController()
{
    qDebug().nospace() << "dtor of " << this;

    if(m_worker != NULL)
    {
        delete m_worker;
        m_worker = NULL;
    }
}

void mController::QueueAJob(const QSharedPointer<mSetting> &job)
{
    Q_ASSERT(m_worker != NULL);
    m_worker->AddJob(job);
}

void mController::StartExecuting(TCB *callback)
{
    Q_ASSERT(m_worker != NULL);
    is_stop_by_user_ = false;
    m_worker->SetCallback(callback);
    m_worker->start();
}

void mController::StopByUser()
{
    Q_ASSERT(m_worker!= NULL);

    qInfo("emit stop signal to background thread.");

    emit signal_stop_working();
    is_stop_by_user_ = true;

    qInfo("wait for thread finishing...");
    m_worker->wait();

    mWindow->ResetStatus();

    CreateNewThread();
}

bool mController::isStop()
{
    return m_worker->is_stopping();
}

void mController::StopByTool()
{
    qInfo("emit stop signal to background thread.");
    emit signal_stop_working();
    qInfo("wait for thread finishing...");
    m_worker->wait();
}

void mController::CreateNewThread()
{
    qInfo("creating a new working thread.");
    if(m_worker != NULL)
    {
        Q_ASSERT(m_worker->isFinished());

        m_worker->disconnect();
        m_worker->deleteLater();
    }

    m_worker = new mWorker(session_id, mWindow->OPTFinished, mWindow->OPTOK, mWindow->OPTQuestion);

    connect(this,SIGNAL(signal_stop_working()),
            m_worker, SLOT(slot_stop_executing()), Qt::DirectConnection);
    connect(m_worker, SIGNAL(signal_err_msg(int, const std::string&)),
            this,SIGNAL(signal_err_msg(int, const std::string&)));
}
