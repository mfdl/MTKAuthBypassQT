#include "app.h"
#include <QSettings>
#include <QFile>
#include <QDate>
#include <QTextStream>
#include <QDir>
#include <QApplication>
#include "qdatetime.h"
//#include "myhelper.h"

int App::IconMain = 0xf17b;
int App::IconMenu = 0xf0d7;
int App::IconMin = 0xf068;
int App::IconMax = 0xf079;
int App::IconNormal = 0xf0b2;
int App::IconClose = 0xf00d;

bool App::UseTray = false;
bool App::SaveLog = false;
bool App::UseStyle = true;
QString App::StyleName = ":/images/style.qss";
int App::FontSize = 9;
QString App::FontName = "Microsoft YaHei";

QString App::Title = "general app framework";

void App::ReadConfig()
{
	if (!CheckConfig()) {
		return;
	}

    QString fileName = QString("%1/%2.ini").arg(AppPath).arg(AppName);
	QSettings set(fileName, QSettings::IniFormat);

	set.beginGroup("AppConfig");
	App::UseTray = set.value("UseTray").toBool();
	App::SaveLog = set.value("SaveLog").toBool();
	App::UseStyle = set.value("UseStyle").toBool();
	App::StyleName = set.value("StyleName").toString();
	App::FontSize = set.value("FontSize").toInt();
	App::FontName = set.value("FontName").toString();
	set.endGroup();

	set.beginGroup("MainConfig");
	App::Title = set.value("Title").toString();
	set.endGroup();
}

void App::WriteConfig()
{
    QString fileName = QString("%1/%2.ini").arg(AppPath).arg(AppName);
	QSettings set(fileName, QSettings::IniFormat);

	set.beginGroup("AppConfig");
	set.setValue("UseTray", App::UseTray);
	set.setValue("SaveLog", App::SaveLog);
	set.setValue("UseStyle", App::UseStyle);
	set.setValue("StyleName", App::StyleName);
	set.setValue("FontSize", App::FontSize);
	set.setValue("FontName", App::FontName);
	set.endGroup();

	set.beginGroup("MainConfig");
	set.setValue("Title", App::Title);
	set.endGroup();
}

void App::NewConfig()
{
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
	App::Title = App::Title.toLatin1();
#endif
	WriteConfig();
}

bool App::CheckConfig()
{
    QString fileName = QString("%1/%2.ini").arg(AppPath).arg(AppName);

	//如果配置文件大小为0,则以初始值继续运行,并生成配置文件
	QFile file(fileName);

	if (file.size() == 0) {
		NewConfig();
		return false;
	}

	//如果配置文件不完整,则以初始值继续运行,并生成配置文件
	if (file.open(QFile::ReadOnly)) {
		bool ok = true;

		while (!file.atEnd()) {
			QString line = file.readLine();
			line = line.replace("\r", "");
			line = line.replace("\n", "");
			QStringList list = line.split("=");

			if (list.count() == 2) {
				if (list.at(1) == "") {
					ok = false;
					break;
				}
			}
		}

		if (!ok) {
			NewConfig();
			return false;
		}
	} else {
		NewConfig();
		return false;
	}

	return true;
}

void App::WriteError(QString str)
{
	QString fileName = QString("%1/log/%2_Error_%3.txt").arg(AppPath).arg(AppName).arg(QDATE);
	QFile file(fileName);
	file.open(QIODevice::WriteOnly | QIODevice::Append | QFile::Text);
	QTextStream stream(&file);
	stream << DATETIME << "  " << str << "\n";
}

void App::WriteStartTime()
{
	QString fileName = QString("%1/log/%2_Start_%3.txt").arg(AppPath).arg(AppName).arg(QDATE);
	QFile file(fileName);
	QString str = QString("start time : %1").arg(DATETIME);
	file.open(QFile::WriteOnly | QIODevice::Append | QFile::Text);
	QTextStream stream(&file);
	stream << str << "\n";
}

void App::NewDir(QString dirName)
{
	//如果路径中包含斜杠字符则说明是绝对路径
	//windows系统 路径字符带有 :/  linux系统路径字符带有 /
	if (!dirName.contains(":/") && !dirName.startsWith("/")) {
		dirName = QString("%1/%2").arg(AppPath).arg(dirName);
	}

	QDir dir(dirName);

	if (!dir.exists()) {
		dir.mkpath(dirName);
	}
}
