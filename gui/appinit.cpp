#include "appinit.h"
#include "app.h"
#include "myhelper.h"
//#include "savelog.h"
#include <QDebug>
#include <QtCore>
#include <QtWidgets>
#include "gui/log/LogHandler.h"
#include "gui/log/Singleton.h"

AppInit *AppInit::self = 0;
AppInit::AppInit(QObject *parent) : QObject(parent)
{
}

bool AppInit::eventFilter(QObject *obj, QEvent *evt)
{
    QWidget *w = (QWidget *)obj;
    if (!w->property("CanMove").toBool()) {
        return QObject::eventFilter(obj, evt);
    }

    static QPoint mousePoint;
    static bool mousePressed = false;

    QMouseEvent *event = static_cast<QMouseEvent *>(evt);
    if (event->type() == QEvent::MouseButtonPress) {
        if (event->button() == Qt::LeftButton) {
            mousePressed = true;
            mousePoint = event->globalPos() - w->pos();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        mousePressed = false;
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        if (mousePressed && (event->buttons() && Qt::LeftButton)) {
            w->move(event->globalPos() - mousePoint);
            return true;
        }
    }

    return QObject::eventFilter(obj, evt);
}

void AppInit::start()
{
    //myHelper::setCode();
    App::ReadConfig();    

    if (App::SaveLog) {
        App::NewDir("log");
        Singleton<LogHandler>::getInstance().installMessageHandler();
    }
    if (App::UseStyle) {
        //qApp->installEventFilter(this);
        myHelper::setStyle(App::StyleName);
    }
}
