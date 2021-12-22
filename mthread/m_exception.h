/*
 * Exception.h
 *
 *  Created on: Sep 2, 2011
 *      Author: MTK81019
 */

#ifndef MEXCEPTION_H_
#define MEXCEPTION_H_

#include <iostream>
#include <m_defs.h>

//MException**********************************************************************
class MException : public std::exception
{
public:
    MException(const char* func, const char* file, int line);
    virtual ~MException() throw();

    std::string context() const;

    virtual int err_code() const = 0;
    virtual std::string err_msg() const = 0;

protected:
    const char* func_;
    const char* file_;
    int line_;
};
//APPException**********************************************************************
class APPException : public MException
{
public:
    APPException(int err_code, const std::string& message, const char* func="", const char* file="", int line=0);
    virtual ~APPException() throw();

    virtual std::string err_msg() const;
    int err_code() const { return err_code_; }

private:
    std::string err_msg_;
    int err_code_;
};
//StopException**********************************************************************
class StopException: public MException
{
public:
    StopException(const char* func="", const char* file="", int line=0);
    virtual ~StopException() throw();

    virtual int err_code() const {return 0;}
    virtual std::string err_msg() const { return "DUMMY"; }
};

//mException**********************************************************************
class mException : public MException
{
public:
    mException(int err_code,unsigned int msp_code=0,
               const char* func="", const char* file="", const int line=0);
    mException(const mException& e);
    virtual ~mException() throw();

    int err_code() const { return err_code_; }
    virtual std::string err_msg() const;

private:
    mException & operator=(const mException &rhs);

protected:
    int err_code_;
    unsigned int msp_code_;
};

#define MRX_ERROR 999
//main_Exception**********************************************************************

#define THROW_APP_EXCEPTION(x, y) \
    do{ throw APPException(x, y,  __FUNCTION__, __FILE__, __LINE__); } while(0);

#define THROW_M_EXCEPTION(x, y) \
    do{ throw mException(x, y, __FUNCTION__, __FILE__, __LINE__); } while(0);

#define THROW_M_EXCEPTION_EX(e) \
    do{ throw mException(e); } while(0);

#define THROW_USER_STOP \
    do{ throw StopException(__FUNCTION__, __FILE__, __LINE__); } while(0);

//#define mExceptionTHROW \
//        do{ throw mException(ret, 0, __FUNCTION__, __FILE__, __LINE__); } while(0)

#define mExceptionTHROW \
    do{ get_mlib()->ResetWithThrow(ret); throw mException(MRX_ERROR, 0, __FUNCTION__, __FILE__, __LINE__); } while(0)

#define mExceptionTHROWA \
    do{ ResetWithThrow(ret, false); throw mException(MRX_ERROR, 0, __FUNCTION__, __FILE__, __LINE__); } while(0)

#define mExceptionTHROWB \
    do{ throw mException(ret, 0, __FUNCTION__, __FILE__, __LINE__); } while(0)

#endif /* MEXCEPTION_H_ */
