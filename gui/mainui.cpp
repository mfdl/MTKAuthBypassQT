#include "mainui.h"
#include "ui_mainui.h"
#include "m_cmd.h"
#include "m_tcb.h"

#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <cassert>
#include <QDesktopWidget>
#include <QStatusBar>
#include <QScreen>
#include <QClipboard>

#include "m_callback.h"
#include "m_controller.h"

#include <QDir>

#include "clickablelabel.h"

MainUI::MainUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainUI),
    loader(new mController(this, this)),
    callback(new mCallback(this, this))

{
    qDebug().nospace() << "ctor of " << this;

    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(238, 238, 238));
    setAutoFillBackground(true);
    setPalette(pal);

    QLabel *statusLabel = new QLabel(this);
    statusLabel->setMinimumSize(150, 20);
    statusLabel->setText(tr("https://www.facebook.com/mofadal.96/"));
    statusBar()->addWidget(statusLabel);
    setWindowIcon(QIcon(":/new/prefix1/mtksecbypass.ico"));

    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<QTextCursor>("QTextCursor");
    qRegisterMetaType<QAbstractItemDelegate::EndEditHint>("QAbstractItemDelegate::EndEditHint");
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
    qRegisterMetaType<QItemSelection>("QItemSelection");
    qRegisterMetaType<LogInfor>("LogInfor");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<Ext4FileInfo>("Ext4FileInfo");
    qRegisterMetaType<QTextCharFormat>("QTextCharFormat");

    qRegisterMetaType<MTKMetaNetworkData>("MTKMetaNetworkData");

    connect(this, SIGNAL(signal_OPTFinished()), SLOT(OPTFinished()));
    connect(this, SIGNAL(signal_OPTOK()), SLOT(OPTOK()));
    connect(this, SIGNAL(signal_OPTQuestion(int *)), SLOT(OPTQuestion(int *)), Qt::BlockingQueuedConnection);

    QObject::connect (ui->btn_stop, &QPushButton::clicked,  [=]()
    {
        loader->StopByUser();
    });
    QObject::connect (ui->btn_screen_shot, &QPushButton::clicked,  [=]()
    {
        take_screen_shot();
    });
    QObject::connect (ui->btn_disable_sec_boot, &QPushButton::clicked,  [=]()
    {
        PerformJob(DoMTKSecBypass, false);
    });

    QObject::connect (ui->btn_crash_pl, &QPushButton::clicked,  [=]()
    {
        PerformJob(DoMTKCrashPL, false);
    });

    QObject::connect (ui->btn_dump_pl, &QPushButton::clicked,  [=]()
    {
        PerformJob(DoMTKDumpPreloader, false);
    });

    QObject::connect (ui->btn_vivo_demo, &QPushButton::clicked,  [=]()
    {
        PerformJob(DoMTKVivoDemoRemove, false);
    });

    QObject::connect (ui->btn_reboot_meta, &QPushButton::clicked,  [=]()
    {
        PerformJob(DoMTKRebootMETA, false);
    });


    QObject::connect (ui->lb_mrx, &ClickableLabel::clicked,  [=]()
    {
        QDesktopServices::openUrl(QUrl("https://www.facebook.com/mofadal.96/"));
    });
}

void MainUI::UpdateLog(const QString &text, LogInfor info)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textEdit->setTextCursor(cursor);

    QFont orgfont = ui->textEdit->font();

    switch (info)
    {
        case LogOkay:
            ui->textEdit->setTextColor(QColor("#191970"));
            break;
        case LogInfo:
            ui->textEdit->setTextColor(QColor("#007fff"));
            break;
        case LogRed:
            ui->textEdit->setTextColor(QColor("#860111"));
            break;
        case LogBold:
            ui->textEdit->setTextColor(QColor("#121623"));//9f2d75
            break;
        case LogFail:
            ui->textEdit->setTextColor(QColor(Qt::red));
            break;
        default:
            ui->textEdit->setTextColor(QColor("#010203"));//Qt::black));
            break;
    }

    if(info == LogNormal)
    {
        ui->textEdit->setCurrentFont(orgfont);
        QTextCursor text_cursor = QTextCursor(ui->textEdit->document());
        text_cursor.movePosition(QTextCursor::End);
        ui->textEdit->insertPlainText(text);
        ui->textEdit->ensureCursorVisible();
    }
    else if(info == LogBold)
    {
        QFont font("Inconsolata", 9);
        font.setBold(true);
        ui->textEdit->setCurrentFont(font);
        QTextCursor text_cursor = QTextCursor(ui->textEdit->document());
        text_cursor.movePosition(QTextCursor::End);
        ui->textEdit->insertPlainText(text);
        ui->textEdit->insertPlainText("\n");
        ui->textEdit->ensureCursorVisible();
        ui->textEdit->setCurrentFont(orgfont);
    }
    else
    {
        ui->textEdit->setCurrentFont(orgfont);
        QTextCursor text_cursor = QTextCursor(ui->textEdit->document());
        text_cursor.movePosition(QTextCursor::End);
        ui->textEdit->insertPlainText(text);
        ui->textEdit->insertPlainText("\n");
        ui->textEdit->ensureCursorVisible();
    }
}

void MainUI::slot_set_prg_format(qstr txt)
{
    if(Loader()->isStop())
        return;

    ui->pbar->setFormat(txt);
}

void MainUI::slot_set_prg_val(int val)
{
    if(Loader()->isStop())
        return;

    ui->pbar->setValue(val);
}
void MainUI::slot_get_log(const QString &text, LogInfor info)
{
    UpdateLog(text, info);
}

void MainUI::take_screen_shot()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap pixmap  = screen->grabWindow(QApplication::activeWindow()->winId());
    QString rpath = QDir::currentPath();//mCallback::get_instance()->mPath();
    if(pixmap.isNull())
        return;

    QString save_path =  QString("%0").arg(rpath) + qstr("/%1_screenshot.png")
            .arg(qdt::currentDateTime().toString("yyyy_MM_dd hh_mm_ss"))
            .replace(" ","_");

    pixmap.save(save_path);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString("%0").arg(rpath)));
}

vd MainUI::calculateElapsedTime()
{
    int secs = etimer.elapsed() / 1000;
    int mins = (secs / 60) % 60;
    secs = secs % 60;
    QString MIN = QString("%0").arg(mins, 2, 10, qla1ch('0'));
    QString SEC = QString("%0").arg(secs, 2, 10, qla1ch('0'));
    slot_set_prg_format(QString("Elapsed time : [%1:minutes:%2:seconds]").arg(MIN).arg(SEC));
}

void MainUI::PerformJob(mThreadFunc func, bool reboot)
{
    QString out_thread_dir = {};
    etimer.start();
    ui->textEdit->clear();
    ui->btn_stop->setEnabled(true);

    ui->btn_disable_sec_boot->setEnabled(false);
    ui->btn_dump_pl->setEnabled(false);
    ui->btn_crash_pl->setEnabled(false);
    ui->btn_vivo_demo->setEnabled(false);

    ui->pbar->setValue(0);
    ui->pbar->setFormat("%p%");

    mThreadOPT opt;
    opt.option = func;
    opt.reboot = reboot;
    opt.savepath = out_thread_dir;

    QSharedPointer<m_setting> cmd(new m_setting());
    cmd->setOpt(opt);
    Loader()->QueueAJob(cmd);
    Loader()->StartExecuting(new MCB<MainUI>(this, &MainUI::reset_ui));
}

void __stdcall MainUI::OPTFinished()
{
    qInfo("MainUI::OPTFinished");
}

void __stdcall MainUI::OPTOK()
{
    qInfo("MainUI::OPTOK");
}

void __stdcall MainUI::OPTQuestion(int *p_val)
{
    QString num;num.sprintf("%s", p_val);
    qInfo() << QString("MainUI::OPTQuestion:%0").arg(num);
}

void MainUI::reset_ui()
{
    calculateElapsedTime();

    ui->btn_stop->setEnabled(false);
    ui->btn_disable_sec_boot->setEnabled(true);
    ui->btn_dump_pl->setEnabled(true);
    ui->btn_crash_pl->setEnabled(true);
    ui->btn_vivo_demo->setEnabled(true);
    qInfo() << "MainUI::reset_ui";
}

void MainUI::ResetStatus()
{
    //reset prg val . ui , etc
    qInfo() << "ResetStatus from mControler";
    slot_set_prg_format("%p%");
    reset_ui();
}

Ui::MainUI *MainUI::gui() const
{
    return ui;
}

mController *MainUI::Loader() const
{
    return this->loader;
}

mCallback *MainUI::CallBack() const
{
    return this->callback;
}
MainUI::~MainUI()
{
    delete ui;
    ui = NULL;
}
