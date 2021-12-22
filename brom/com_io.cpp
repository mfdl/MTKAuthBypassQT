#include "boot_rom.h"

using namespace std;

class sync_event
{
public:
    //idx 0 for R/W, idx 1 for interrupt thread.
    HANDLE r_evHandle[2];
    HANDLE w_evHandle[2];
};

bool boot_rom::connect_brom(int port_num)
{
    disconnect_brom();

    timeout = 30000;
    is_bootloader = FALSE;

    _hcom = INVALID_HANDLE_VALUE;
    ext = NULL;

    if(port_num == 0)
        return false;

    //QThread::msleep(500);

    if (_hcom != INVALID_HANDLE_VALUE)
        return false;

    QString port_name = QString("\\\\.\\com%0").arg(port_num);

    quint32 retry = 20;
    while(retry > 0)
    {

        _hcom = ::CreateFile(port_name.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE,
                             0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

        if (_hcom != INVALID_HANDLE_VALUE)
        {
            break;
        }

        qInfo() << "retry CreateFile.";
        --retry;
        Sleep(5);
    }

    if (_hcom == INVALID_HANDLE_VALUE)
    {
        send_log_fail("open com fail! please restart the program.");
        return false;
    }

    DWORD com_error = 0;
    ::ClearCommError(_hcom, &com_error, NULL);
    ::SetupComm(_hcom, 80*1024, 80*1024); //80K cache.
    DCB dcb = {0};

    GetCommState(_hcom, &dcb);

    dcb.BaudRate = 115200;
    dcb.fAbortOnError = FALSE;
    dcb.fBinary = 1;

    // Set 8/N/1
    dcb.ByteSize = 8;
    dcb.fParity = FALSE;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    // Disable H/W flow control
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDsrSensitivity = FALSE;

    // Disable S/W flow control
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    if (!::SetCommState(_hcom, &dcb))
    {
        qInfo("SetCommState failed.");
        return false;
    }

    ::ClearCommBreak(_hcom);
    ::EscapeCommFunction(_hcom, SETRTS);
    ::EscapeCommFunction(_hcom, SETDTR);
    ::PurgeComm(_hcom, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);

    COMMTIMEOUTS timeout = {0};

    timeout.ReadIntervalTimeout         = 0;
    //async read must set these zero. or GetOverlappedResult return immediately
    timeout.ReadTotalTimeoutMultiplier  = 0; //1;
    timeout.ReadTotalTimeoutConstant    = 0; //200;
    timeout.WriteTotalTimeoutMultiplier = 0;//1;
    timeout.WriteTotalTimeoutConstant   = 0;//200;

    ::SetCommTimeouts(_hcom, &timeout);

    ext = (void*)new sync_event();
    ((sync_event*)ext)->w_evHandle[0] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    ((sync_event*)ext)->w_evHandle[1] = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    ((sync_event*)ext)->r_evHandle[0] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    ((sync_event*)ext)->r_evHandle[1] = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    qInfo() << QString("%0 open complete.").arg(port_name);

    return true;
}

void boot_rom::disconnect_brom()
{
    ext = (PVOID)0x1;

    if (_hcom != INVALID_HANDLE_VALUE)
    {
        DWORD com_error = 0;
        ::ClearCommError(_hcom, &com_error, NULL);
        ::CloseHandle(_hcom);
        _hcom = INVALID_HANDLE_VALUE;
    }
    ext = NULL;

}

//return actual read bytes. timeout throw exception.
size_t boot_rom::brom_get(quint8* data, size_t length, quint32 timeout)
{
    if (!this->_hcom)
        return 0;

    quint32 xferd_read = 0;
    OVERLAPPED r_ov;

    while(xferd_read < length)
    {
        ::ZeroMemory(&r_ov,sizeof(OVERLAPPED));
        r_ov.hEvent = ((sync_event*)ext)->r_evHandle[0];

        if(!::ReadFile(_hcom, data+xferd_read, length-xferd_read, NULL, &r_ov))
        {
            if(GetLastError() == ERROR_IO_PENDING)
            {
                DWORD result = ::WaitForMultipleObjects(2, ((sync_event*)ext)->r_evHandle, FALSE, timeout);
                if(result == WAIT_OBJECT_0)
                {
                    xferd_read += r_ov.InternalHigh;
                }
                else if(result == WAIT_OBJECT_0 + 1)
                {
                    qInfo() << "Rx abort.";
                    ::CancelIo(_hcom);
                    disconnect_brom();
                    throw "boost::thread_interrupted()";
                    break;
                }
                else if(result == WAIT_TIMEOUT)
                {
                    ::CancelIo(_hcom);
                   // disconnect_brom();
                    qInfo() << QString("Rx timeout in %0").arg(timeout);
                   // throw QString("Rx timeout in %0").arg(timeout);
                    break;
                }
                else
                {
                    quint32 last_err = GetLastError();
                    disconnect_brom();
                    qInfo() << QString("Rx WaitForMultipleObjects error: %0").arg(last_err);
                    throw QString("Rx WaitForMultipleObjects error: %0").arg(last_err);
                    break;
                }
            }
            else
            {
                quint32 last_err = GetLastError();
                disconnect_brom();
                qInfo() << QString("Rx ReadFile error. code: %0").arg(last_err);
                throw QString("Rx ReadFile error. code: %0").arg(last_err);
                break;
            }
        }
        else
        {
            if(r_ov.Internal == ERROR_NO_MORE_ITEMS)
            {
                DWORD com_error = 0;
                ::ClearCommError(_hcom, &com_error, NULL);
                disconnect_brom();
                throw QString("Rx Clear COM error");
                qInfo() << "Rx Clear COM error" ;
            }
            xferd_read += r_ov.InternalHigh;
        }
    }
    if (xferd_read != length)
    {
        qInfo() << QString("Rx xferd length[%0] != expected length[%1]").arg(xferd_read).arg(length);
        //disconnect_brom();
        //throw QString("Rx xferd length[%0] != expected length[%1]").arg(xferd_read).arg(length);
        //BOOSTHROW_EXCEPTION(runtime_exception(string("Rx data incomplete.")));
    }

//    qbyte buff = QByteArray::fromRawData((char*)data, length);
//    qInfo() << "boot_rom::read:" << buff.toHex() << buff.size();

    return xferd_read;
}

//return actual send bytes. timeout throw exception.
size_t boot_rom::brom_send(quint8* data, size_t length, quint32 timeout)
{
    if (!this->_hcom)
        return 0;

    quint32 xferd_write = 0;
    OVERLAPPED w_ov;

    while(xferd_write < length)
    {
        ::ZeroMemory(&w_ov,sizeof(OVERLAPPED));
        w_ov.hEvent = ((sync_event*)ext)->w_evHandle[0];

        if (!::WriteFile(_hcom,data+xferd_write, length-xferd_write, NULL, &w_ov))
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                DWORD result = ::WaitForMultipleObjects(2, ((sync_event*)ext)->w_evHandle, FALSE, timeout);
                if(result == WAIT_OBJECT_0)
                {
                    xferd_write += w_ov.InternalHigh;
                }
                else if(result == WAIT_OBJECT_0 + 1)
                {
                    qInfo() << "Tx abort.";
                    ::CancelIo(_hcom);
                    disconnect_brom();
                    throw "boost::thread_interrupted()";
                    break;
                }
                else if(result == WAIT_TIMEOUT)
                {
                    ::CancelIo(_hcom);
                    //disconnect_brom();
                    qInfo() << QString("Tx timeout in %0").arg(timeout);
                    //throw QString("Tx timeout in %0").arg(timeout);
                    break;
                }
                else
                {
                    quint32 last_err = GetLastError();
                    disconnect_brom();
                    qInfo() << QString("Tx WaitForMultipleObjects error: %0").arg(last_err);
                    throw QString("Tx WaitForMultipleObjects error: %0").arg(last_err);
                    break;
                }
            }
            else
            {
                quint32 last_err = GetLastError();
                disconnect_brom();
                qInfo() << QString("Tx WriteFile error. code: %0").arg(last_err);
                throw QString("Tx WriteFile error. code: %0").arg(last_err);
                break;
            }
        }
        else
        {
            xferd_write += w_ov.InternalHigh;
        }
        //::FlushFileBuffers(_hcom);

    }
    if (xferd_write != length)
    {
        qInfo() << QString("Tx xferd_write length[%0] != expected length[%1]").arg(xferd_write).arg(length);
        //disconnect_brom();
        //throw QString("Tx xferd_write length[%0] != expected length[%1]").arg(xferd_write).arg(length);
        //BOOSTHROW_EXCEPTION(runtime_exception(string("Tx data incomplete.")));
    }

//    qbyte buff = QByteArray::fromRawData((char*)data, length);
//    qInfo() << "boot_rom::read:" << buff.toHex() << buff.size();

    return xferd_write;
}

///********************************xflash_api***********************************
int boot_rom::read8(quint8* data)
{
    return  brom_get(data, 1, timeout);
}
int boot_rom::read16(quint16* data)
{
    brom_get((quint8*)data, sizeof(quint16), timeout);
    *data = ntohs(*data);
    return sizeof(quint16);
}
int boot_rom::read32(quint32* data)
{
    brom_get((quint8*)data, sizeof(quint32), timeout);
    *data = ntohl(*data);
    return sizeof(quint32);
}
int boot_rom::read_buffer(quint8* data, quint32 length)
{
    return brom_get(data, length, timeout);
}
int boot_rom::write8(quint8 data)
{
    return brom_send(&data, 1, timeout);
}
int boot_rom::write16(quint16 data)
{
    data = htons(data);
    return brom_send((quint8*)&data, sizeof(quint16), timeout);
}
int boot_rom::write32(quint32 data, bool type)
{
    data = htonl(data);
    if(!type)
    {
        return brom_send((quint8*)&data, sizeof(quint32), timeout);
    }
    else
    {
        if(!brom_send((quint8*)&data, sizeof(quint32), timeout))
            return 0;

        quint32 ack = {};
        read32(&ack);
        return (ack == qFromBigEndian(data));
    }
}

int boot_rom::write_buffer(quint8* data, quint32 length)
{
    return brom_send(data, length, timeout);
}
///********************************mrx_edit***********************************//
int boot_rom::write_ptr(char *data, quint32 length)
{
    return brom_send((quint8*)data, length, timeout);
}
int boot_rom::read_ptr(char *data, quint32 length)
{
    return brom_get((quint8*)&data, length, timeout);
}
int boot_rom::bootrom_sv5_read_ptr(QByteArray data, quint32 length)
{
    return brom_get((quint8*)data.data(), length, timeout);
}
///********************************mrx_edit***********************************//
int boot_rom::write_pattern(QByteArray buff)
{
    return brom_send((quint8*)buff.data(), buff.length(), timeout);
}
int boot_rom::read_pattern(QByteArray &buff, quint32 length)
{
    buff = QByteArray{static_cast<int>(length), Qt::Uninitialized};
    int len = brom_get((quint8*)buff.data(), length, timeout);
    return len;
}

//unsigned int boot_rom::ReadPattern(PATTERN_INFO_S * pPattern, const unsigned int kPatternNum)
//{
//	DWORD  dwReadLen;
//	char szRevBuf[MAX_REVBUF_SIZE];
//	char szFlowPool[MAX_PATTERN_LEN];
//	unsigned int uPoolSize;

//	//Check parameters
//	if( (NULL == pPattern) ||(1 > kPatternNum))
//	{
//		printf("Incorrect parameters!(NULL pointer or wrong pattern number)\n");
//		return S_INVALID_ARGUMENTS;
//	}

//	uPoolSize = MAX_PATTERN_LEN;

//	memset(szRevBuf, 0, MAX_REVBUF_SIZE);
//	memset(szFlowPool, 0, MAX_PATTERN_LEN);


//	try
//	{
//		for(unsigned int i = 0; i < m_uRetryTime; i++)
//		{
//			//if(BOOT_STOP == *m_pStopFlag)
//			//{
//			//    printf("PreloaderCmd::ReadPattern(): STOP!");
//			//    return S_STOP;
//			//}


//			memset(szRevBuf, 0, MAX_REVBUF_SIZE);
//			dwReadLen = 0;
//			if(brom_get(szRevBuf, &dwReadLen))
//			{
//				printf("ReadComm Read fail!\n");
//				return S_PL_READ_FAIL;
//			}
//			else if(0 != dwReadLen)
//			{
//				//printf("PreloaderCmd::ReadPattern(): Dump data: ===%s===\n", szRevBuf);
//				//printf("PreloaderCmd::ReadPattern(): Dump data size: %u\n", dwReadLen);

//				for(unsigned int k = 0; k < dwReadLen; k++)
//				{
//					memcpy(szFlowPool, szFlowPool+1, uPoolSize-1);
//					memcpy(szFlowPool+uPoolSize-1, szRevBuf+k, 1);

//					for(unsigned int s = 0; s < kPatternNum; s++ )
//					{
//						if(0 == memcmp(szFlowPool + (uPoolSize - pPattern[s].m_uLen), pPattern[s].m_szPattern, pPattern[s].m_uLen))
//						{
//							//printf("Pattern matched: %s\n", pPattern[s].m_szPattern);
//							return pPattern[s].m_uStatus;
//						}
//					}
//				}
//			}

//			Sleep(m_quinterval);
//		}

//		printf("Read timeout! Retry time(%u)\n", m_uRetryTime);
//		return S_PL_READIMEOUT;
//	}
//	catch(...)
//	{

//		printf( "Unknown exception! Err(%u)\n", GetLastError());

//		return S_UNDEFINED_ERROR;
//	}
//}
