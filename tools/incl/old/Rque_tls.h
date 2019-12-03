/*
 *     Module Name       : RQUE_TLS.H
 *
 *     Type              : Include file ramdrive QUEue routines for the
 *                         communication between pos_main en stnetp24
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
 * 13-Dec-1999 Initial Release WinPOS                                  P.M.
 * --------------------------------------------------------------------------
 */

#ifndef _rque_tls_h_
#define _rque_tls_h_

#ifdef __cplusplus
extern "C" {
#endif

#define QHEADSIZE   5 * sizeof(short)            /* not the data part */

struct QUEUE {                           /* internal queue for MS_DOS */
  BYTE   *area;
  _TCHAR  name[4];
  long    writ;
  long    read;
  long    oldread;
  long    recread;
  long    recwrit;
  int     flush;                         /* indicates automatic flush */
};

extern int  create_queue(struct QUEUE *, _TCHAR *, int);
extern void close_queue (struct QUEUE *);
extern int  flush_queue (struct QUEUE *);
extern int  reorg_queue (struct QUEUE *);
extern int  write_queue(struct QUEUE *, MESSAGE_STRUCT *);
extern int  read_queue (struct QUEUE *, MESSAGE_STRUCT *);

#ifdef __cplusplus
}
#endif

#endif
