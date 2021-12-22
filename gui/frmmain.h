#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QWidget>
#include <QPoint>
#include <QMouseEvent>
#include <QMenu>
#include "gui/titlebar.h"

class MainUI;
class frmmain : public QWidget
{
    Q_OBJECT

public:
    explicit frmmain(QWidget *parent = 0);
    ~frmmain();

private:
    TitleBar *pTitleBar;
    MainUI *pMainWindow;
};

#endif // FRMMAIN_H
