/*
 * BromException.cpp
 *
 *  Created on: Sep 2, 2011
 *      Author: MTK81019
 */

#include <string>
#include "m_exception.h"

MException::MException(const char* func, const char* file, int line)
    : func_(func), file_(file), line_(line)
{}

MException::~MException() throw(){}

std::string MException::context() const
{
    char buffer[512] = {0};
    sprintf(buffer,"(%s,%s,%d)",func_,file_,line_);
    return std::string(buffer);
}

StopException::StopException(
        const char* func, const char* file, int line)
    :MException(func,file,line){}

StopException::~StopException() throw(){}

APPException::APPException(
        int err_code,
        const std::string& message,
        const char* func, const char* file, int line)
    :MException(func,file,line),
      err_msg_(message),
      err_code_(err_code)
{}

APPException::~APPException() throw(){}

std::string APPException::err_msg() const
{
    if(err_msg_.length() <= 0)
    {
        return QString("err:%0").arg((err_code_)).toStdString();
    }

    return err_msg_;
}

mException::mException(int err_code,unsigned int msp_code,
                       const char* func, const char* file, int line)
    :MException(func,file,line), err_code_(err_code),msp_code_(msp_code)
{}

mException::mException(const mException& e):
    MException("", "", 0)
{
    this->err_code_ = e.err_code_;
    this->msp_code_ = e.msp_code_;
}

mException::~mException() throw(){}

std::string mException::err_msg() const
{
    char buff[512] = {0};
    return std::string(buff);
}



