/*
******************************************************************
** 类名称:   IconHelper
** 功能概述: 单例模式设定自定义标题栏的最大化，最小化，和还原图标
**          使用fontawesome-webfont.ttf字体文件实现最小化
**          最大化关闭等按钮文字，图形文字，很好很强大
** 参数说明:
** 作成日期: 20170415
** 作成者:   liudianwu
** 修改日期:
** 修改者:
******************************************************************
*/


#include "iconhelper.h"

IconHelper* IconHelper::_instance = 0;
IconHelper::IconHelper(QObject*):
    QObject(qApp)
{
    int fontId = QFontDatabase::addApplicationFont(":/new/prefix1/fontawesome-webfont.ttf");
    QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
    iconFont = QFont(fontName);
}

void IconHelper::SetIcon(QLabel* lab, QChar c, int size)
{
    iconFont.setPointSize(size);
    lab->setFont(iconFont);
    lab->setText(c);
}

void IconHelper::SetIcon(QPushButton* btn, QChar c, int size)
{
    iconFont.setPointSize(size);
    btn->setFont(iconFont);
    btn->setText(c);
}
