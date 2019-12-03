/*
 *     Module Name       : MEM_MGR.H
 *
 *     Type              : Include file memory manager modules
 *                         
 *
 *     Author/Location   : Getronics, Distribution & Retail, Nieuwegein
 *
 *     Copyright Makro International AG
 *               Aspermonstrasse 24
 *               7006 CHUR
 *               Switzerland
 *
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 13-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 * 11-Dec-2000 Added sleep time to give_time_to_OS                     R.N.B.
 * --------------------------------------------------------------------------
 * 01-Oct-2002 Removed 'far' from the pointers                         J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef _mem_mgr_h_
#define _mem_mgr_h_

#ifdef __cplusplus
extern "C" {
#endif

/* ERROR DEFINITION                                                          */
#define MEM_UNAVAILABLE    -2750

/* FUNCTIONS                                                                 */
extern void *mem_allocate(unsigned int);
extern void *mem_reallocate(void *, unsigned int);
extern void mem_free(void *);

extern void give_time_to_OS(long, long);

#ifdef __cplusplus
}
#endif

#endif
