#ifndef MainUI_H
#define MainUI_H

#include <QMainWindow>
#include <QSharedPointer>
#include <QTableWidgetItem>
#include <QTableWidget>
#include <QTime>
#include <QTimer>
#include "control.h"
#include <QMouseEvent>

#include "m_cmd.h"
#include "m_controller.h"
#include "m_defs.h"

namespace Ui {
class MainUI;
}

class CheckHeader;
class mCallback;
class mController;

class MainUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainUI(QWidget *parent = nullptr);
    ~MainUI();

    void InitGUI();

signals:
public slots:
    void slot_set_prg_val(int val);
    void slot_set_prg_format(qstr txt);
    void slot_get_log(const QString &text, LogInfor info);

private:
    void take_screen_shot();
public:
    void ResetStatus();

    void UpdateLog(const QString &text, LogInfor info = LogNormal);

signals:
    void signal_OPTFinished();
    void signal_OPTOK();
    void signal_OPTQuestion(int *p_val);

public slots:
    static void __stdcall OPTFinished();
    static void __stdcall OPTOK();
    static void __stdcall OPTQuestion(int *p_val);

    void reset_ui();

public:
    Ui::MainUI *gui() const;
    mController *Loader() const;
    mCallback *CallBack() const;

    void PerformJob(mThreadFunc func, bool reboot);

    bool CheckTableItems();

    void calculateElapsedTime();

public:
    QElapsedTimer etimer;
    QElapsedTimer stimer;
private:
    Ui::MainUI *ui;
    mController *loader;
    mCallback *callback;
};

#endif // MainUI_H
