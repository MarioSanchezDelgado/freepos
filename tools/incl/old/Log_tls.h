/*
 *     Module Name       : LOG_TLS.H
 *
 *     Type              : Include file for logging to file
 *
 *     Author/Location   : Pedro Meuwissen, Getronics Nieuwegein
 * 
 *     Copyright Makro International AG
 *               Aspermonstrasse 24
 *               7006 CHUR
 *               Switzerland
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE       REASON                                         CALL #   AUTHOR
 * --------------------------------------------------------------------------
 * 04-Oct-2000 Initial Release                                         J.D.M.
 * 25-Oct-2000 Made MAX_FILE_NO and MAX_NO_DPRINTFS flexible through
 *             the function dinit().                                   J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef __LOG_TLS_H__
#define __LOG_TLS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern short dinit(_TCHAR *, short, double);
extern void dprintf(_TCHAR *, long, _TCHAR *, ...);

#ifdef __cplusplus
}
#endif

#endif

