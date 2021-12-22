#ifndef MCMD_H
#define MCMD_H

#include "m_defs.h"
#include <iostream>
#include <QSharedPointer>

class CMD
{
public:
    CMD(APKey key):key_(key){}

    virtual ~CMD(){}

    virtual void exec() = 0;

protected:
    APKey key_;
};

class m_cmd : public CMD
{
public:
    m_cmd(APKey key);

    virtual ~m_cmd();

    virtual void exec();

    mThreadOPT opt() const;
    void setOpt(const mThreadOPT &opt);

protected:
    mThreadOPT _opt;
};

class mSetting : public QObject
{
public:
    mSetting() {}

    virtual ~mSetting();

    virtual QSharedPointer<CMD> CreateCommand(APKey key) = 0;

    virtual mThreadOPT opt() const = 0;

    virtual void set_stop_flag(int *stop_flag) = 0;

};

class m_setting : public mSetting
{
public:
    m_setting();

    virtual QSharedPointer<CMD> CreateCommand(APKey key);

    virtual void set_stop_flag(int *stop_flag);

    mThreadOPT opt() const;
    void setOpt(const mThreadOPT &opt);

protected:
    mThreadOPT _opt;
};

#endif // MCMD_H
