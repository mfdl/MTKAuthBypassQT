#include "titlebar.h"
#include <QHBoxLayout>
#include <QApplication>

TitleBar::TitleBar(QWidget *parent) :
    QFrame(parent)
{
    setFixedHeight(23);

    m_bPressed = false;

    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(0xff, 0xcc, 0xd4));//#00bcd4/#1D2025
    setAutoFillBackground(true);
    setPalette(pal);

    m_pIconLabel = new QLabel(this);
    m_pTitleLabel = new QLabel(this);

    QFont font;
    font.setPointSize(11);
    font.setFamily(("Tahoma"));
    font.setBold(true);

    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::white);
    pe.setColor(QPalette::Shadow, Qt::yellow);

    font.setBold(true);
    m_pTitleLabel->setFont(font);
    m_pTitleLabel->setPalette(pe);

    m_btn_min = new QPushButton(this);
    //    m_btn_max = new QPushButton(this);
    m_btn_close = new QPushButton(this);

    m_pIconLabel->setFixedSize(20,20);
    m_pIconLabel->setScaledContents(true);

    m_pTitleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //    m_pTitleLabel->setAlignment(Qt::AlignHCenter);
    m_btn_min->setFixedSize(22, 22);
    //    m_btn_max->setFixedSize(22, 22);
    m_btn_close->setFixedSize(22, 22);

    m_pTitleLabel->setObjectName("whiteLabel");
    m_btn_min->setObjectName("minimizeButton");
    m_btn_close->setObjectName("closeButton");

    m_btn_min->setToolTip("Minimize");
    m_btn_close->setToolTip("Close");

    m_btn_close->setStyleSheet("QPushButton{background-color:transparent;"
                                      "color: white; border: none;"
                                      "border-style: outset;}"
                                      "QPushButton:hover{background-color:rgb(255,100,100); color: white;}"
                                      "QPushButton:pressed{background-color:rgb(255,100,100);}"
                                      );

    //    m_btn_max->setStyleSheet("QPushButton{background-color:transparent;"
    //                                         "color: white; border: none;"
    //                                         "border-style: outset;}"
    //                                         "QPushButton:hover{background-color:rgb(100,100,100); color: white;}"
    //                                         "QPushButton:pressed{background-color:rgb(100,100,100);}"
    //                                        );

    m_btn_min->setStyleSheet("QPushButton{background-color:transparent;"
                                         "color: white; border: none;"
                                         "border-style: outset;}"
                                         "QPushButton:hover{background-color:rgb(100,100,100); color: white;}"
                                         "QPushButton:pressed{background-color:rgb(100,100,100);}"
                                         );

    //    IconHelper::Instance()->SetIcon((QLabel)m_btn_close, ":/res/themes/icons/res/themes/icons/close.png");
    //    IconHelper::Instance()->SetIcon(m_btn_max, ":/res/themes/icons/res/themes/icons/max.png");
    //    IconHelper::Instance()->SetIcon(m_btn_min, ":/res/themes/icons/res/themes/icons/min.png");

    IconHelper::Instance()->SetIcon(m_btn_close, QChar(0xf057), 12);
    //    IconHelper::Instance()->SetIcon(m_btn_max, QChar(0xf079), 12);
    IconHelper::Instance()->SetIcon(m_btn_min, QChar(0xf146), 12);

    QHBoxLayout *pLayout = new QHBoxLayout(this);
    pLayout->addWidget(m_pIconLabel);
    pLayout->addSpacing(5);
    pLayout->addWidget(m_pTitleLabel);
    pLayout->addWidget(m_btn_min);
    //    pLayout->addWidget(m_btn_max);
    pLayout->addWidget(m_btn_close);
    pLayout->setSpacing(0);
    pLayout->setContentsMargins(5, 0, 5, 0);
    setLayout(pLayout);

    connect(m_btn_min, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    // connect(m_btn_max, SIGNAL(clicked(bool)), this, SLOT(onClicked()));
    connect(m_btn_close, SIGNAL(clicked(bool)), this, SLOT(onClicked()));

    this->setStyleSheet("QFrame{border: none;"
                        "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                        "stop: 0 #a6a6a6, stop: 0.08 #7f7f7f,"
                        "stop: 0.39999 #717171, stop: 0.4 #626262,"
                        "stop: 0.9 #4c4c4c, stop: 1 #333333);"
                        "border-bottom-right-radius: 10px;"
                        "border-bottom-left-radius:10px;}"

                       "QPushButton {"
                       "color: #333;"
                       "border: 2px solid #555;"
                       "border-radius: 11px;"
                       "padding: 5px;"
                       "background: qradialgradient(cx: 0.3, cy: -0.4,"
                       "fx: 0.3, fy: -0.4,"
                       "radius: 1.35, stop: 0 #fff, stop: 1 #888);}"

                       "QPushButton:hover {"
                       "background: qradialgradient(cx: 0.3, cy: -0.4,"
                       "fx: 0.3, fy: -0.4,"
                       "radius: 1.35, stop: 0 #fff, stop: 1 #bbb);}"

                       "QPushButton:pressed {"
                       "background: qradialgradient(cx: 0.4, cy: -0.1,"
                       "fx: 0.4, fy: -0.1,"
                       "radius: 1.35, stop: 0 #fff, stop: 1 #ddd);}");
}

TitleBar::~TitleBar()
{

}

void TitleBar::onClicked()
{
    QPushButton *pButton = qobject_cast<QPushButton *>(sender());
    QWidget *pWindow = this->window();
    if (pWindow->isTopLevel())
    {
        if (pButton == m_btn_min)
        {
            pWindow->showMinimized();
        }
        else if (pButton == m_btn_close)
        {
            pWindow->close();
        }
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    // emit m_btn_max->clicked();
}

bool TitleBar::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
        case QEvent::WindowTitleChange:
        {
            QWidget *pWidget = qobject_cast<QWidget *>(obj);
            if (pWidget)
            {
                m_pTitleLabel->setText(pWidget->windowTitle());
                return true;
            }
        }
        case QEvent::WindowIconChange:
        {
            QWidget *pWidget = qobject_cast<QWidget *>(obj);
            if (pWidget)
            {
                QIcon icon = pWidget->windowIcon();
                m_pIconLabel->setPixmap(icon.pixmap(m_pIconLabel->size()));
                return true;
            }
        }
        case QEvent::WindowStateChange:
        case QEvent::Resize:
            return true;
        default:
            break;
    }
    return QWidget::eventFilter(obj, event);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bPressed = true;
        m_point = event->pos();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    QWidget *pWindow = this->window();
    //pWindow = parentWidget();
    if (m_bPressed)
        pWindow-> move(event->pos() - m_point + pWindow->pos());
}
void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_bPressed = false;
}
