#-------------------------------------------------
#
# Project created by QtCreator 2022-06-08T09:25:46
# del unattended
# blacklist intel_powerclamp
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ethercatMaster
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /home/chang/桌面/defaultrt/ethercat-master/include
LIBS += /home/chang/桌面/defaultrt/ethercat-master/lib/*.o

CONFIG += c++11

SOURCES += \
    axiswid.cpp \
    ethercat/ethercatwid.cpp \
    ethercat/slaveConfWid/axis_setting_wid.cpp \
    ethercat/slaveConfWid/slave_assign_wid.cpp \
    ethercat/slaveConfWid/slave_index_wid.cpp \
    ethercat/slaveConfWid/slave_pdo_wid.cpp \
    ethercat/slaveConfWid/slave_startup_wid.cpp \
    ethercat/slave_conf_dg.cpp \
        main.cpp \
        ethercatmaster.cpp \
    etcHeader/null.cpp \
    etcHeader/etcheader.cpp \
    etcHeader/jmc_moveabs.cpp \
    etcHeader/jmc_movereal.cpp \
    etcHeader/jmc_axis_cmd.cpp \
    etcHeader/jmc_movevel.cpp \
    etcHeader/jmc_home.cpp \
    etcHeader/jmc_stop.cpp \
    etcHeader/jmc_setpos.cpp \
    etcHeader/jmc_reset.cpp \
    etcHeader/jmc_power.cpp \
    ethercatmaster-axis.cpp \
    ethercatmaster-pthread.cpp \
    etcHeader/velo_planing.cpp \
    velplan/s7_vel_plan.cpp \
    ethercatmaster-udp.cpp \
    etcHeader/jmc_wtmc_tc.cpp

HEADERS += \
    axiswid.h \
    ethercat/ethercatwid.h \
    ethercat/slaveConfWid/axis_setting_wid.h \
    ethercat/slaveConfWid/nullnwe.h \
    ethercat/slaveConfWid/qt59to514.h \
    ethercat/slaveConfWid/slave_assign_wid.h \
    ethercat/slaveConfWid/slave_index_wid.h \
    ethercat/slaveConfWid/slave_pdo_wid.h \
    ethercat/slaveConfWid/slave_startup_wid.h \
    ethercat/slave_conf_dg.h \
        ethercatmaster.h \
    etcHeader/etcheader.h \
    etcHeader/jmc_axis_cmd.h \
    velplan/s7_vel_plan.h

FORMS += \
        axiswid.ui \
        ethercat/ethercatwid.ui \
        ethercat/slaveConfWid/axis_setting_wid.ui \
        ethercat/slaveConfWid/slave_assign_wid.ui \
        ethercat/slaveConfWid/slave_index_wid.ui \
        ethercat/slaveConfWid/slave_pdo_wid.ui \
        ethercat/slaveConfWid/slave_startup_wid.ui \
        ethercat/slave_conf_dg.ui \
        ethercatmaster.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS +=-lpthread
LIBS +=-lrt
QT += sql
