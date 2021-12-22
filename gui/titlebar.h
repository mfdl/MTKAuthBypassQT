#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QMouseEvent>
#include "iconhelper.h"

namespace Ui {
class TitleBar;
}

class TitleBar : public QFrame
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = 0);
    ~TitleBar();

private slots:
    void onClicked();

public:
    void mouseDoubleClickEvent(QMouseEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *event);

private:
    QLabel *m_pIconLabel;
    QLabel *m_pTitleLabel;
    QPushButton *m_btn_min;
    QPushButton *m_btn_max;
    QPushButton *m_btn_close;
private:
    bool m_bPressed;
    QPoint m_point;

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // TITLEBAR_H
