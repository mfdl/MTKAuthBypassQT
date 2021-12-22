#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickableLabel : public QLabel {
    Q_OBJECT

signals :
void clicked();

public:
void mousePressEvent(QMouseEvent* event)
{
     Q_UNUSED(event)
     emit clicked();
}

using QLabel::QLabel;

};

#endif // CLICKABLELABEL_H
