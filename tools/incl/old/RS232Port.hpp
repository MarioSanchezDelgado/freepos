/*
 *     Module Name       : RS232Port.hpp
 *
 *     Type              : 
 *                         
 *
 *     Author/Location   : J.R.F. De Maeijer, Nieuwegein
 *
 *     Copyright         : 2000, Getronics, Distribution & Retail, Nieuwegein
 *
 * ----------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * ----------------------------------------------------------------------------
 * DATE        REASON                                                    AUTHOR
 * ----------------------------------------------------------------------------
 * 29-Nov-2000 Initial Release                                           J.D.M.
 * ----------------------------------------------------------------------------
 * 23-Apr-2001 Added extra modem status functions                        M.D.B.
 * ----------------------------------------------------------------------------
 */

#ifndef __RS232Port_hpp__
#define __RS232Port_hpp__

#include <windows.h>
#include <windowsx.h> 

#include <tchar.h>
#pragma hdrstop

typedef struct {
  HANDLE     PortHandle;
  short      ReadBusy;
  long       ReadTimeOut;
  OVERLAPPED ReadOverlap;
  DWORD      nRead;
  short      WriteBusy;
  long       WriteTimeOut;
  OVERLAPPED WriteOverlap;
  DWORD      nWritten;
  OVERLAPPED Status;
  COMSTAT    Error;
} OS_DEPENDEND_ADDITIONAL_RS232_PORT_VARIABLES;

#define OS_DEPENDEND_BAUD_RATE_110      CBR_110
#define OS_DEPENDEND_BAUD_RATE_300      CBR_300
#define OS_DEPENDEND_BAUD_RATE_600      CBR_600
#define OS_DEPENDEND_BAUD_RATE_1200     CBR_1200
#define OS_DEPENDEND_BAUD_RATE_2400     CBR_2400
#define OS_DEPENDEND_BAUD_RATE_4800     CBR_4800
#define OS_DEPENDEND_BAUD_RATE_9600     CBR_9600
#define OS_DEPENDEND_BAUD_RATE_14400    CBR_14400
#define OS_DEPENDEND_BAUD_RATE_19200    CBR_19200
#define OS_DEPENDEND_BAUD_RATE_38400    CBR_38400
#define OS_DEPENDEND_BAUD_RATE_56000    CBR_56000
#define OS_DEPENDEND_BAUD_RATE_57600    CBR_57600
#define OS_DEPENDEND_BAUD_RATE_115200   CBR_115200
#define OS_DEPENDEND_BAUD_RATE_128000   CBR_128000
#define OS_DEPENDEND_BAUD_RATE_256000   CBR_256000

#define OS_DEPENDEND_DATA_BITS_4        4
#define OS_DEPENDEND_DATA_BITS_5        5
#define OS_DEPENDEND_DATA_BITS_6        6
#define OS_DEPENDEND_DATA_BITS_7        7
#define OS_DEPENDEND_DATA_BITS_8        8

#define OS_DEPENDEND_PARITY_NONE        NOPARITY
#define OS_DEPENDEND_PARITY_ODD         ODDPARITY
#define OS_DEPENDEND_PARITY_EVEN        EVENPARITY
#define OS_DEPENDEND_PARITY_MARK        MARKPARITY
#define OS_DEPENDEND_PARITY_SPACE       SPACEPARITY

#define OS_DEPENDEND_STOP_BITS_1        ONESTOPBIT
#define OS_DEPENDEND_STOP_BITS_1_5      ONE5STOPBITS
#define OS_DEPENDEND_STOP_BITS_2        TWOSTOPBITS

enum RS232RETURNVALUES {
  COMINT_SUCCEED,
  COMINT_RS232_NO_PORT_HANDLE,
  COMINT_FAIL_INIT_RS232,
  COMINT_RS232_WRITE_NO_OVERLAP_EVENT,
  COMINT_RS232_WRITE_FAIL,
  COMINT_RS232_WRITE_WAIT_NO_OVERLAP_EVENT,
  COMINT_RS232_WRITE_WAIT_FAIL,
  COMINT_RS232_READ_FAIL,
  COMINT_RS232_READ_NO_OVERLAP_EVENT,
  COMINT_RS232_READ_WAIT_NO_OVERLAP_EVENT,
  COMINT_RS232_READ_WAIT_FAIL,
  COMINT_WAIT_TIMEOUT,
  COMINT_TRUE,
  COMINT_FALSE
};

/*---------------------------------------------------------------------------*/
/* CLASS RS232Port                                                           */
/*---------------------------------------------------------------------------*/
class RS232Port {
  public:
    /* Constructors */
    RS232Port(short, short);
    RS232Port(short, short, short, short, short, short);
    /* Destructor */
    ~RS232Port();
    /* Member Functions */
    short   Initialise();
    short   DeInitialise();
    short   WriteToPort(BYTE*, short);
    short   WriteToPort(BYTE*, short, long);
    short   ReadFromPort(BYTE*, short);
    short   ReadFromPort(BYTE*, short, long);
    _TCHAR* RetrieveString();
    short   ClearPort();
    short   ClearToSend();
    short   RLSD();
    short   Error(); 
  private:
    /* Member Functions */
    short WaitForReadIO(short, long);
    short WaitForWriteIO(short, long);
    short ClearPortRead();
    short ClearPortWrite();
    /* Member Variables */
    short PortNo;
    short BlockingMode;
    short Initialised;
    short Baudrate;
    short Databits;
    short Stopbits;
    short Parity;
    OS_DEPENDEND_ADDITIONAL_RS232_PORT_VARIABLES  OSDepAddVars;
};

#endif
