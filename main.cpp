#include <QApplication>
#include <QWidget>
#include "frmmain.h"
#include "gui/appinit.h"
#include "gui/app.h"
#include <QStyleFactory>
#include "m_callback.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("mtksecbypass");
    a.setApplicationVersion("7.0.0.1");
    a.setWindowIcon(QIcon(":/new/prefix1/mtksecbypass.ico"));

    QFont font(QFont("Microsoft Yahei", 7));
    font.setBold(true);
    a.setFont(font);
    a.setStyle(QStyleFactory::create("Fusion"));
   // a.setStyleSheet(mCallback::ReadFile(":/qss/darkblue.css"));
    //    AppInit::Instance()->start();
    frmmain w;
    w.show();

    return a.exec();
}
