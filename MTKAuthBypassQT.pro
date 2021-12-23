QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += -std=c++11

CONFIG += qt warn_off

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32:{
    DEFINES += "_CRT_SECURE_NO_WARNINGS"
}

#QMAKE_LFLAGS += -Wl,--large-address-aware

#get rid of auto generated debug/release folders
CONFIG -= debug_and_release debug_and_release_target

CONFIG(debug, debug|release) {DESTDIR = ProjectTemp/debug }
CONFIG(release, debug|release) {DESTDIR = ProjectTemp/release }

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.u

DESTDIR = output

win32:RC_FILE += MTKAuthBypassQT.rc
win32 {
CONFIG += embed_manifest_exe

#QMAKE_LFLAGS += /MANIFESTUAC:"level='requireAdministrator'uiAccess='false'"
}

INCLUDEPATH += $$PWD/mthread
INCLUDEPATH += $$PWD/comport
INCLUDEPATH += $$PWD/hiusb
INCLUDEPATH += $$PWD/gui

LIBS += -lsetupapi

FORMS += \
    gui/mainui.ui

HEADERS += \
    brom/boot_rom.h \
    brom/boot_rom_cmd.h \
    brom/m_defs.h \
    brom/meta_cmd.h \
    brom/usb_defs.h \
    comport/usbdevfinder.h \
    gui/app.h \
    gui/appinit.h \
    gui/clickablelabel.h \
    gui/frmmain.h \
    gui/iconhelper.h \
    gui/log/LogHandler.h \
    gui/log/Singleton.h \
    gui/mainui.h \
    gui/myhelper.h \
    gui/titlebar.h \
    mthread/m_callback.h \
    mthread/m_cmd.h \
    mthread/m_controller.h \
    mthread/m_defs.h \
    mthread/m_exception.h \
    mthread/m_glob.h \
    mthread/m_helper.h \
    mthread/m_tcb.h \
    mthread/m_worker.h

SOURCES += \
    brom/boot_rom.cpp \
    brom/com_io.cpp \
    brom/meta_mode.cpp \
    brom/vivo_demo.cpp \
    brom/bootrom_old.cpp \
    comport/usbdevfinder.cpp \
    gui/app.cpp \
    gui/appinit.cpp \
    gui/frmmain.cpp \
    gui/iconhelper.cpp \
    gui/log/LogHandler.cpp \
    gui/mainui.cpp \
    gui/titlebar.cpp \
    main.cpp \
    mthread/m_callback.cpp \
    mthread/m_cmd.cpp \
    mthread/m_controller.cpp \
    mthread/m_exception.cpp \
    mthread/m_glob.cpp \
    mthread/m_helper.cpp \
    mthread/m_worker.cpp

RESOURCES += \
    RES.qrc \
    qss.qrc
