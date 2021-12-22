#include "frmmain.h"
#include <QDebug>
#include <QVBoxLayout>
#include "mainui.h"

frmmain::frmmain(QWidget *parent) :
    QWidget(parent)
{
    //setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setWindowFlags(Qt::FramelessWindowHint);
    pTitleBar = new TitleBar(this);

    installEventFilter(pTitleBar);
    setWindowTitle(tr("MTK Auth Bypass Tool V13.0.0.0:16:12:2021"));
    setWindowIcon(QIcon(":/new/prefix1/mtksecbypass.ico"));

    pMainWindow = new MainUI(this);

    setFixedSize(566, 500);

    QVBoxLayout *pLayout = new QVBoxLayout();

    pLayout->addWidget(pTitleBar);
    pLayout->addWidget(pMainWindow);

    pLayout->setSpacing(0);
    pLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(pLayout);
}

frmmain::~frmmain()
{

}
