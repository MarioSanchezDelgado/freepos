/*
 *     Module Name       : RS232Port.cpp
 *
 *     Type              : Microsoft C++ Object  
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
 * 23-Apr-2001 Added Modem status functions some are for future 
 *             usuage and not implemented yet                            M.D.B.
 * 26-Apr-2001 Added information about DBC structure RTS and DTR lines   M.D.B.
 * 19-Apr-2001 Added BlockingMode (To be tested if set on TRUE!)         J.D.M.
 * ----------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include <tchar.h>
#include <string>
#include "RS232Port.hpp"
#include "stri_tls.h"

/*---------------------------------------------------------------------------*/
/* RS232Port::RS232Port                                                      */
/*---------------------------------------------------------------------------*/
RS232Port::RS232Port(short PortNumber, short BlockingMode)
          :PortNo(PortNumber),
           BlockingMode(BlockingMode),
           Initialised(COMINT_FALSE),
           Baudrate(OS_DEPENDEND_BAUD_RATE_9600),
           Databits(OS_DEPENDEND_DATA_BITS_8),
           Stopbits(OS_DEPENDEND_STOP_BITS_1),
           Parity(OS_DEPENDEND_PARITY_NONE) 
{
  OSDepAddVars.WriteTimeOut=1000L;
  OSDepAddVars.ReadTimeOut=1000L;
  OSDepAddVars.PortHandle=0;
  memset(&(OSDepAddVars.ReadOverlap), 0, sizeof(OVERLAPPED));
  memset(&(OSDepAddVars.WriteOverlap), 0, sizeof(OVERLAPPED));
  ClearPort();
} /* RS232Port::RS232Port */

/*---------------------------------------------------------------------------*/
/* RS232Port::RS232Port                                                      */
/*---------------------------------------------------------------------------*/
RS232Port::RS232Port(short PortNumber, short BlockingMode, short Baudrate,
                     short Databits, short Stopbits, short Parity)
          :PortNo(PortNumber),
           BlockingMode(BlockingMode),
           Initialised(COMINT_FALSE),
           Baudrate(Baudrate),
           Databits(Databits),
           Stopbits(Stopbits),
           Parity(Parity) 
{
  OSDepAddVars.WriteTimeOut=1000L;
  OSDepAddVars.ReadTimeOut=1000L;
  OSDepAddVars.PortHandle=0;
  memset(&(OSDepAddVars.ReadOverlap), 0, sizeof(OVERLAPPED));
  memset(&(OSDepAddVars.WriteOverlap), 0, sizeof(OVERLAPPED));
  ClearPort();
} /* RS232Port::RS232Port */

/*---------------------------------------------------------------------------*/
/* RS232Port::~RS232Port                                                     */
/*---------------------------------------------------------------------------*/
RS232Port::~RS232Port() 
{
  if(OSDepAddVars.PortHandle) {
    CloseHandle(OSDepAddVars.PortHandle);
    OSDepAddVars.PortHandle=0;
  }
  if(OSDepAddVars.WriteOverlap.hEvent) {
    CloseHandle(OSDepAddVars.WriteOverlap.hEvent);
    OSDepAddVars.WriteOverlap.hEvent=0;
  }
  if(OSDepAddVars.ReadOverlap.hEvent) {
    CloseHandle(OSDepAddVars.ReadOverlap.hEvent);
    OSDepAddVars.ReadOverlap.hEvent=0;
  }
} /* RS232Port::~RS232Port */

/*---------------------------------------------------------------------------*/
/* RS232Port::Initialise                                                     */
/*---------------------------------------------------------------------------*/
short RS232Port::Initialise() 
{
  DCB    dcb={0};
  _TCHAR port_name[100];
  DWORD  FlagsAndAttributes;

  ClearPort();
  if(BlockingMode!=TRUE && BlockingMode != FALSE) {
    return COMINT_FAIL_INIT_RS232;
  }
  _stprintf(port_name, _T("COM%d"), PortNo);

  if(BlockingMode==FALSE) {
    FlagsAndAttributes=FILE_FLAG_OVERLAPPED|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING;
  }
  else {
    FlagsAndAttributes=FILE_FLAG_WRITE_THROUGH|FILE_FLAG_NO_BUFFERING;
  }
  OSDepAddVars.PortHandle = CreateFile(port_name,
                       GENERIC_READ|GENERIC_WRITE,
                       0,
                       0,
                       OPEN_EXISTING,
                       FlagsAndAttributes,
                       0);
  if (OSDepAddVars.PortHandle == INVALID_HANDLE_VALUE) {
    OSDepAddVars.PortHandle=0;
    return COMINT_FAIL_INIT_RS232;
  }

  /*-MDB-dcb-information-----------------------------------------------------*/
  /*  RTS FIELD:                                                             */
  /*                                                                         */
  /*  RTS_CONTROL_DISABLE   Disables the RTS line when the device is         */
  /*                        opened and leaves it disabled.                   */
  /*  RTS_CONTROL_ENABLE    Enables the RTS line when the device is          */
  /*                        opened and leaves it on.                         */
  /*  RTS_CONTROL_HANDSHAKE Enables RTS handshaking. The driver raises       */
  /*                        the RTS line when the "type-ahead" (input)       */
  /*                        buffer is less than one-half full and lowers     */
  /*                        the RTS line when the buffer is more than        */
  /*                        three-quarters full. If handshaking is enabled,  */
  /*                        it is an error for the application to adjust     */
  /*                        the line by using the EscapeCommFunction         */
  /*                        function.                                        */
  /*  RTS_CONTROL_TOGGLE    Specifies that the RTS line will be high if      */
  /*                        bytes are available for transmission. After      */
  /*                        all buffered bytes have been sent, the RTS       */
  /*                        line will be low.                                */
  /*                                                                         */  
  /*  DTR FIELD:                                                             */
  /*                                                                         */
  /*  DTR_CONTROL_DISABLE   Disables the DTR line when the device is opened  */
  /*                        and leaves it disabled.                          */
  /*  DTR_CONTROL_ENABLE    Enables the DTR line when the device is opened   */
  /*                        and leaves it on.                                */
  /*  DTR_CONTROL_HANDSHAKE Enables DTR handshaking. If handshaking is       */
  /*                        enabled, it is an error for the application to   */
  /*                        adjust the line by using the EscapeCommFunction  */
  /*                        function.                                        */
  /*-------------------------------------------------------------------------*/

  if (GetCommState(OSDepAddVars.PortHandle, &dcb)) {
    dcb.BaudRate          = Baudrate;
    dcb.Parity            = Parity; 
    dcb.ByteSize          = Databits;
    dcb.StopBits          = Stopbits;
    dcb.fOutxDsrFlow      = FALSE;
    dcb.fOutxCtsFlow      = FALSE;
    dcb.fOutX             = FALSE;
    dcb.fInX              = FALSE;
 
    /* Not completely tested in combination with CAT hardware handshaking see above. */
    dcb.fDtrControl       = DTR_CONTROL_HANDSHAKE;
    dcb.fRtsControl       = RTS_CONTROL_HANDSHAKE;
 
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fNull             = FALSE;
    dcb.fAbortOnError     = TRUE;

    SetCommState(OSDepAddVars.PortHandle, &dcb);
  }
  else {
    CloseHandle(OSDepAddVars.PortHandle);
    OSDepAddVars.PortHandle=0;
    return COMINT_FAIL_INIT_RS232;
  }

  Initialised=COMINT_TRUE;
  return COMINT_SUCCEED;
} /* RS232Port::Initialise */

/*---------------------------------------------------------------------------*/
/* RS232Port::DeInitialise                                                   */
/*---------------------------------------------------------------------------*/
short RS232Port::DeInitialise() 
{
  if(OSDepAddVars.PortHandle) {
    CloseHandle(OSDepAddVars.PortHandle);
    OSDepAddVars.PortHandle=0;
  }
  if(OSDepAddVars.WriteOverlap.hEvent) {
    CloseHandle(OSDepAddVars.WriteOverlap.hEvent);
    OSDepAddVars.WriteOverlap.hEvent=0;
  }
  if(OSDepAddVars.ReadOverlap.hEvent) {
    CloseHandle(OSDepAddVars.ReadOverlap.hEvent);
    OSDepAddVars.ReadOverlap.hEvent=0;
  }

  Initialised=COMINT_FALSE;
  return COMINT_SUCCEED;
} /* RS232Port::DeInitialise */

/*---------------------------------------------------------------------------*/
/* RS232Port::WriteToPort                                                    */
/*---------------------------------------------------------------------------*/
short RS232Port::WriteToPort(BYTE* RawData, short DataLength) 
{
  return(WriteToPort(RawData, DataLength, OSDepAddVars.WriteTimeOut));
} /* RS232Port::WriteToPort */

/*---------------------------------------------------------------------------*/
/* RS232Port::WriteToPort                                                    */
/*---------------------------------------------------------------------------*/
short RS232Port::WriteToPort(BYTE* RawData, short DataLength, long WriteTimeOut) 
{
  if(!OSDepAddVars.PortHandle) {
    return COMINT_RS232_NO_PORT_HANDLE;
  }
  if(OSDepAddVars.WriteBusy==COMINT_FALSE&&*RawData) {
    memset(&(OSDepAddVars.WriteOverlap), 0, sizeof(OVERLAPPED));
    OSDepAddVars.WriteOverlap.hEvent=CreateEvent(0, TRUE, FALSE, 0);
  }
  if(!OSDepAddVars.WriteOverlap.hEvent) {
    ClearPortWrite();
    return COMINT_RS232_WRITE_NO_OVERLAP_EVENT;
  }
  if(!WriteFile(OSDepAddVars.PortHandle, RawData, DataLength,
                &(OSDepAddVars.nWritten),
                BlockingMode==FALSE ? &(OSDepAddVars.WriteOverlap) : NULL)) {
    if(GetLastError() != ERROR_IO_PENDING) {
      ClearPortWrite();
      return COMINT_RS232_WRITE_FAIL;
    }
    else {
      OSDepAddVars.WriteBusy=COMINT_TRUE;
    }
  }
  else {
    /* Write file completed immediately */
    if(OSDepAddVars.nWritten!=DataLength) { /* The operation timed out */
      /* We don't care about a timeout, if something is missing */
      /* let the other side of the port handle it.              */
      ClearPortWrite();
      return COMINT_SUCCEED;
    }
    else {
      ClearPortWrite();
      return COMINT_SUCCEED;
    }
  }

  if(OSDepAddVars.WriteBusy==COMINT_TRUE) {
    return(WaitForWriteIO(DataLength, WriteTimeOut));
  }
  else {
    ClearPortWrite();
    return COMINT_RS232_WRITE_FAIL;
  }
} /* RS232Port::WriteToPort */

/*---------------------------------------------------------------------------*/
/* RS232Port::ReadFromPort                                                   */
/*---------------------------------------------------------------------------*/
short RS232Port::ReadFromPort(BYTE* RawData, short DataLength) 
{
  return(ReadFromPort(RawData, DataLength, OSDepAddVars.ReadTimeOut));
} /* RS232Port::ReadFromPort */

/*---------------------------------------------------------------------------*/
/* RS232Port::ReadFromPort                                                   */
/*---------------------------------------------------------------------------*/
short RS232Port::ReadFromPort(BYTE* RawData, short DataLength, long ReadTimeOut) 
{
  if(!OSDepAddVars.PortHandle) {
    return COMINT_RS232_NO_PORT_HANDLE;
  }
  if(OSDepAddVars.ReadBusy==COMINT_FALSE) {
    memset(&(OSDepAddVars.ReadOverlap), 0, sizeof(OVERLAPPED));
    OSDepAddVars.ReadOverlap.hEvent=CreateEvent(0, TRUE, FALSE, 0);
  }
  if(!OSDepAddVars.ReadOverlap.hEvent) {
    ClearPortRead();
    return COMINT_RS232_READ_NO_OVERLAP_EVENT;
  }
  if(OSDepAddVars.ReadBusy==COMINT_FALSE) {
    if(!ReadFile(OSDepAddVars.PortHandle, RawData, DataLength, &(OSDepAddVars.nRead),
      BlockingMode==FALSE ? &(OSDepAddVars.ReadOverlap) : NULL)) {
      if(GetLastError() != ERROR_IO_PENDING) {
        ClearPortRead();
        return COMINT_RS232_READ_FAIL;
      }
      else {
        OSDepAddVars.ReadBusy=COMINT_TRUE;
      }
    }
    else {
      /* Read file completed immediately */
      if(OSDepAddVars.nRead!=DataLength) {
        /* The operation timed out */
        return COMINT_WAIT_TIMEOUT;
      }
      else {
        ClearPortRead();
        return COMINT_SUCCEED;
      }
    }
  }

  if(OSDepAddVars.ReadBusy==COMINT_TRUE) {
    return(WaitForReadIO(DataLength, ReadTimeOut));
  }
  else {
    ClearPortRead();
    return COMINT_RS232_READ_FAIL;
  }
} /* RS232Port::ReadFromPort */

/*---------------------------------------------------------------------------*/
/* RS232Port::WaitForWriteIO                                                 */
/*---------------------------------------------------------------------------*/
short RS232Port::WaitForWriteIO(short DataLength, long WriteTimeOut) 
{
  DWORD Result;

  if(!OSDepAddVars.WriteOverlap.hEvent) {
    return COMINT_RS232_WRITE_WAIT_NO_OVERLAP_EVENT;
  }
  Result=WaitForSingleObject(OSDepAddVars.WriteOverlap.hEvent, WriteTimeOut);
  switch(Result) {
    case WAIT_OBJECT_0:
      if(!GetOverlappedResult(OSDepAddVars.PortHandle,
          &(OSDepAddVars.WriteOverlap), &(OSDepAddVars.nWritten), FALSE)) {
        ClearPortWrite();
        return COMINT_RS232_WRITE_WAIT_FAIL;
      }
      else {
        /* Write finished */
        if(OSDepAddVars.nWritten!=DataLength) { /* The operation timed out */
          /* We don't care about a timeout, if something is missing */
          /* let the other side of the port handle it.              */
          ClearPortWrite();
          return COMINT_SUCCEED;
        }
        else {
          ClearPortWrite();
          return COMINT_SUCCEED;
        }
      }
      break;
    case WAIT_TIMEOUT:
      /* We don't care about a timeout, if something is missing   */
      /* let the other side of the port handle it (Crc checking). */
      ClearPortWrite();
      return COMINT_WAIT_TIMEOUT;
      //return COMINT_SUCCEED;
      break;
    default:
      ClearPortWrite();
      return COMINT_RS232_WRITE_WAIT_FAIL;
      break;
  }
} /* RS232Port::WaitForWriteIO */

/*---------------------------------------------------------------------------*/
/* RS232Port::WaitForReadIO                                                  */
/*---------------------------------------------------------------------------*/
short RS232Port::WaitForReadIO(short DataLength, long ReadTimeOut) 
{
  DWORD Result;

  if(!OSDepAddVars.ReadOverlap.hEvent) {
    return COMINT_RS232_READ_WAIT_NO_OVERLAP_EVENT;
  }
  Result=WaitForSingleObject(OSDepAddVars.ReadOverlap.hEvent, ReadTimeOut);
  switch(Result) {
    case WAIT_OBJECT_0:
      if(!GetOverlappedResult(OSDepAddVars.PortHandle,
          &(OSDepAddVars.ReadOverlap), &(OSDepAddVars.nRead), FALSE)) {
        ClearPortRead();
        return COMINT_RS232_READ_WAIT_FAIL;
      }
      else {
        /* Read finished */
        if(OSDepAddVars.nRead!=DataLength) {
          /* The operation timed out */
          return COMINT_WAIT_TIMEOUT;
        }
        else {
          ClearPortRead();
          return COMINT_SUCCEED;
        }
      }
      break;
    case WAIT_TIMEOUT:
      return COMINT_WAIT_TIMEOUT;
      break;
    default:
      ClearPortRead();
      return COMINT_RS232_READ_WAIT_FAIL;
      break;
  }
} /* RS232Port::WaitForReadIO */

/*---------------------------------------------------------------------------*/
/* RS232Port::ClearPort                                                      */
/*---------------------------------------------------------------------------*/
short RS232Port::ClearPort() 
{
  ClearPortWrite();
  ClearPortRead();

  return COMINT_SUCCEED;
} /* RS232Port::ClearPort */

/*---------------------------------------------------------------------------*/
/* RS232Port::ClearPortWrite                                                 */
/*---------------------------------------------------------------------------*/
short RS232Port::ClearPortWrite() 
{
  if(OSDepAddVars.WriteOverlap.hEvent) {
    CloseHandle(OSDepAddVars.WriteOverlap.hEvent);
  }
  memset(&(OSDepAddVars.WriteOverlap), 0, sizeof(OVERLAPPED));
  OSDepAddVars.nWritten=0;
  OSDepAddVars.WriteBusy=COMINT_FALSE;

  return COMINT_SUCCEED;
} /* RS232Port::ClearPortWrite */

/*---------------------------------------------------------------------------*/
/* RS232Port::ClearPortRead                                                  */
/*---------------------------------------------------------------------------*/
short RS232Port::ClearPortRead() 
{
  if(OSDepAddVars.ReadOverlap.hEvent) {
    CloseHandle(OSDepAddVars.ReadOverlap.hEvent);
  }
  memset(&(OSDepAddVars.ReadOverlap), 0, sizeof(OVERLAPPED));
  OSDepAddVars.nRead=0;
  OSDepAddVars.ReadBusy=COMINT_FALSE;

  return COMINT_SUCCEED;
} /* RS232Port::ClearPortRead */

/*---------------------------------------------------------------------------*/
/* RS232Port::CleartoSend (not tested / implemented)                         */
/*---------------------------------------------------------------------------*/
short RS232Port::ClearToSend() 
{
  unsigned long status;
  short RetVal; 

  RetVal = GetCommModemStatus(OSDepAddVars.PortHandle, &status);
  if ( RetVal && ( status & MS_CTS_ON )) {
    return COMINT_TRUE;
  }
  else {
    return COMINT_FALSE;
  }
} /* RS232Port::ClearToSend */

/*---------------------------------------------------------------------------*/
/* RS232Port::RLSD (not tested / implemented )                               */
/*---------------------------------------------------------------------------*/
short RS232Port::RLSD()    //The RLSD (receive-line-signal-detect) signal is on. 
{    
  unsigned long status;
  short RetVal; 

  RetVal = GetCommModemStatus(OSDepAddVars.PortHandle, &status);
  if ( RetVal && ( status & MS_RLSD_ON )) {
    return COMINT_TRUE;
  }
  else {
    return COMINT_FALSE;
  }
} /* RS232Port::RLSD */

/*---------------------------------------------------------------------------*/
/* RS232Port::Error (not used )                                              */
/*---------------------------------------------------------------------------*/
short RS232Port::Error() 
{   
  unsigned long Error;
  short RetVal; 

  /*                                                                         */ 
  /* Errors:                                                                 */
  /*                                                                         */
  /* CE_RXOVER     0x0001   Receive Queue overflow                           */
  /* CE_OVERRUN    0x0002 x Receive Overrun Error                            */
  /* CE_RXPARITY   0x0004 x Receive Parity Error                             */
  /* CE_FRAME      0x0008 x Receive Framing error                            */
  /* CE_BREAK      0x0010 x Break Detected                                   */
  /* CE_TXFULL     0x0100   TX Queue is full                                 */
  /* CE_PTO        0x0200   LPTx Timeout                                     */
  /* CE_IOE        0x0400   LPTx I/O Error                                   */
  /* CE_DNS        0x0800   LPTx Device not selected                         */
  /* CE_OOP        0x1000   LPTx Out-Of-Paper                                */
  /* CE_MODE       0x8000   Requested mode unsupported                       */
  /*        TOTAL: 0x001E                                                    */
  /*                                                                         */

  RetVal = ClearCommError(OSDepAddVars.PortHandle, &Error, &OSDepAddVars.Error);
  if ( RetVal && ( Error & 0x01E)) {
    return COMINT_TRUE;
  }
  else {
    return COMINT_FALSE;
  }
} /* RS232Port::Error */

/*---------------------------------------------------------------------------*/
/* RetrieveString (not used )                                                */
/*---------------------------------------------------------------------------*/
_TCHAR* RS232Port::RetrieveString() 
{
  static _TCHAR BufferIn[1000];
  unsigned char Character[2];
  short  RetVal;
  char tmpbuffer[1000*sizeof(_TCHAR)];
  
  memset(tmpbuffer, 0, sizeof(BufferIn));
  Character[1]=0;
  do {
    RetVal= ReadFromPort(Character, 1);
    if(RetVal==COMINT_SUCCEED) {
      strcat((char *)tmpbuffer, (char*)Character);
    }
  } while (RetVal!=COMINT_WAIT_TIMEOUT && RetVal==COMINT_SUCCEED && *Character);
  _tcscpy(BufferIn, AnsiToUnicode(tmpbuffer));
  
  return BufferIn;
} /* RS232Port::RetrieveString */
