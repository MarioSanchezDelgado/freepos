/*
 *     Module Name       : ERR_MGR.H
 *
 *     Type              : Include file error manager
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
 * 15-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 */

#ifndef _ERR_MGR_H
#define _ERR_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/* MACROS                                                                     */
#define ERR_NOT_EXIST      -401                    /* Error numbers           */
#define ERR_NAME           short

/* TYPEDEF'S                                                                  */
typedef struct err_def {
  short (*err_fn)(struct err_def *);
} ERR_HANDLER;

typedef struct error_list {
  ERR_NAME name;
  ERR_HANDLER *handler;
  struct error_list *next;
} ERR_TBL;

/* PROTOTYPES                                                                 */
extern short err_invoke(ERR_NAME);
extern short err_set(ERR_NAME, void *);
extern short err_init(void);
extern void  err_unset_all(void);

#ifdef __cplusplus
}
#endif

#endif
