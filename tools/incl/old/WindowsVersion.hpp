/*
 *     Module Name       : WindowsVersion.hpp
 *
 *     Type              : Header file Functionality concerning Windows
 *                         versions
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

#ifndef __WindowsVersion_hpp__
#define __WindowsVersion_hpp__

#ifdef __cplusplus
extern "C" {
#endif

extern short GetWindowsVersion();

enum WIN_VERSION {
/* 00 */  WIN_UNKNOWN_VERSION
/* 01 */ ,WIN_31
/* 02 */ ,WIN_95
/* 03 */ ,WIN_98
/* 04 */ ,WIN_NT
/* 05 */ ,WIN_NT_WORKSTATION
/* 06 */ ,WIN_NT_SERVER
/* 07 */ ,WIN_2000
/* 08 */ ,WIN_2000_PROFESSIONAL
/* 09 */ ,WIN_2000_DOMAIN_CONTROLLER
/* 10 */ ,WIN_2000_SERVER
};

#ifdef __cplusplus
}
#endif

#endif
