/*
 *     Module Name       : WindowsVersion.cpp
 *
 *     Type              : Functionality concerning Windows versions
 *                         
 *
 *     Author/Location   : J.R.F. De Maeijer, Nieuwegein
 *
 *     Copyright         : 2002, Getronics, Retail, Nieuwegein
 *
 * ----------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * ----------------------------------------------------------------------------
 * DATE        REASON                                                    AUTHOR
 * ----------------------------------------------------------------------------
 * 02-Jul-2002 Initial Release                                           J.D.M.
 * ----------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "WindowsVersion.hpp"

static short WindowsVersion=WIN_UNKNOWN_VERSION;

static BOOL  Init();

/*---------------------------------------------------------------------------*/
/* GetWindowsVersion                                                         */
/*---------------------------------------------------------------------------*/
short GetWindowsVersion() {
  static short FirstTime=TRUE;

  if(FirstTime==TRUE) {
    FirstTime=FALSE;
    Init();
  }

  return WindowsVersion;
} /* GetWindowsVersion */

/*---------------------------------------------------------------------------*/
/* Init                                                                      */
/*---------------------------------------------------------------------------*/
static BOOL Init() {
  OSVERSIONINFOEX osvi;
  BOOL bOsVersionInfoEx;
  static short FirstTime=TRUE;

  // Try calling GetVersionEx using the OSVERSIONINFOEX structure,
  // which is supported on Windows 2000.
  //
  // If that fails, try using the OSVERSIONINFO structure.

  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
#ifndef NOT_YET_SUPPORTED
  if( bOsVersionInfoEx ) { /* J.D.M.: Force use of OSVERSIONINFO */
    bOsVersionInfoEx = !bOsVersionInfoEx;
  }
#endif

  if( !bOsVersionInfoEx ) {
    // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
    osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) {
      return FALSE;
    }
  }

  switch (osvi.dwPlatformId) {
    case VER_PLATFORM_WIN32_NT:
      // Test for the product.
      if ( osvi.dwMajorVersion <= 4 ) {
        //_stprintf(_T("Microsoft Windows NT "));
        WindowsVersion=WIN_NT;
      }
      if ( osvi.dwMajorVersion == 5 ) {
        //_stprintf(_T("Microsoft Windows 2000 "));
        WindowsVersion=WIN_2000;
      }
      // Test for workstation versus server.
      if( bOsVersionInfoEx ) {
#ifdef NOT_YET_SUPPORTED
        if ( osvi.wProductType == VER_NT_WORKSTATION ) {
          //_stprintf(_T("Professional "));
          switch(WindowsVersion) {
            case WIN_NT:
              WindowsVersion=WIN_NT_WORKSTATION;
              break;
            case WIN_2000:
              WindowsVersion=WIN_2000_PROFESSIONAL;
              break;
            default:
              break;
          }
        }
        if ( osvi.wProductType == VER_NT_SERVER ) {
          //_stprintf"_T(Server "));
          switch(WindowsVersion) {
            case WIN_NT:
              WindowsVersion=WIN_NT_SERVER;
              break;
            case WIN_2000:
              WindowsVersion=WIN_2000_SERVER;
              break;
            default:
              break;
          }
        }
        if ( osvi.wProductType == VER_NT_DOMAIN_CONTROLLER ) {
          switch(WindowsVersion) {
            case WIN_NT:
              break;
            case WIN_2000:
              WindowsVersion=WIN_2000_DOMAIN_CONTROLLER;
              break;
            default:
              break;
          }
        }
#else
        //_stprintf(_T("???????? "));
#endif
      }
      else {
        HKEY hKey;
        char szProductType[80];
        DWORD dwBufLen;
        RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      _T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
                      0, KEY_QUERY_VALUE, &hKey );
        RegQueryValueEx( hKey, _T("ProductType"), NULL, NULL,
                        (LPBYTE) szProductType, &dwBufLen);
        RegCloseKey( hKey );
        if ( lstrcmpi( _T("WINNT"), (_TCHAR*)szProductType) == 0 ) {
          //_stprintf(_T("Workstation "));
          switch(WindowsVersion) {
            case WIN_NT:
              WindowsVersion=WIN_NT_WORKSTATION;
              break;
            case WIN_2000:
              WindowsVersion=WIN_2000_PROFESSIONAL;
              break;
            default:
              break;
          }
        }
        if ( lstrcmpi( _T("SERVERNT"), (_TCHAR*)szProductType) == 0 ) {
          //_stprintf(_T("Server "));
          switch(WindowsVersion) {
            case WIN_NT:
              WindowsVersion=WIN_NT_SERVER;
              break;
            case WIN_2000:
              WindowsVersion=WIN_2000_SERVER;
              break;
            default:
              break;
          }
        }
        //if ( lstrcmpi( _T("??DOMAINCONTROLLER??"), szProductType) == 0 ) {
        //  switch(WindowsVersion) {
        //    case WIN_NT:
        //      break;
        //    case WIN_2000:
        //      WindowsVersion=WIN_2000_DOMAIN_CONTROLLER;
        //      break;
        //    default:
        //      break;
        //  }
        //}
      }

      // Display version, service pack (if any), and build number.
      //_stprintf(_T("version %d.%d %s (Build %d)\n",
      //              osvi.dwMajorVersion,
      //              osvi.dwMinorVersion,
      //              osvi.szCSDVersion,
      //              osvi.dwBuildNumber & 0xFFFF));
      break;
    case VER_PLATFORM_WIN32_WINDOWS:
      if ((osvi.dwMajorVersion > 4) || 
         ((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0))) {
        //_stprintf(_T("Microsoft Windows 98 "));
        WindowsVersion=WIN_98;
      } 
      else {
        //_stprintf(_T("Microsoft Windows 95 "));
        WindowsVersion=WIN_95;
      }
      break;
     case VER_PLATFORM_WIN32s:
      //_stprintf(_T("Microsoft Win32s "));
      WindowsVersion=WIN_31;
      break;
  }

  return TRUE; 
} /* Init */

#ifndef _LIB
/*---------------------------------------------------------------------------*/
/* main                                                                      */
/*---------------------------------------------------------------------------*/
void main() {
  switch(GetWindowsVersion()) {
    case WIN_31:
      _stprintf(_T("WIN_31\n"));
      break;
    case WIN_95:
      _stprintf(_T("WIN_95\n"));
      break;
    case WIN_98:
      _stprintf(_T("WIN_98\n"));
      break;
    case WIN_NT:
      _stprintf(_T("WIN_NT\n"));
      break;
    case WIN_NT_WORKSTATION:
      _stprintf(_T("WIN_NT_WORKSTATION\n"));
      break;
    case WIN_NT_SERVER:
      _stprintf(_T("WIN_NT_SERVER\n"));
      break;
    case WIN_2000:
      _stprintf(_T("WIN_2000\n"));
      break;
    case WIN_2000_PROFESSIONAL:
      _stprintf(_T("WIN_2000_PROFESSIONAL\n"));
      break;
    case WIN_2000_DOMAIN_CONTROLLER:
      _stprintf(_T("WIN_2000_DOMAIN_CONTROLLER\n"));
      break;
    case WIN_2000_SERVER:
      _stprintf(_T("WIN_2000_DOMAIN_SERVER\n"));
      break;
    case WIN_UNKNOWN_VERSION:
    default:
      _stprintf(_T("WIN_UNKNOWN_VERSION\n"));
      break;
  }
}
#endif
