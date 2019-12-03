/*
 *     Module Name       : RQUE_TLS.C
 *
 *     Type              : Ramdrive QUEue routines for the communication
 *                         between pos_main en stnetp24
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

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
                                             /* POS.lib include files */
#include "erlg_tls.h"
#include "stnetp24.h"
#include "rque_tls.h"
#include "mem_mgr.h"

#define AREA_SIZE 200000                        /* Size of queue area */
#define RO_SIZE     2048
                                          /* local 'hidden' functions */
static int fwrit_queue(struct QUEUE *, void *, int);
static int fread_queue(struct QUEUE *, void *, int);

/*--------------------------------------------------------------------*/
/*                         create_queue                               */
/*--------------------------------------------------------------------*/
int create_queue(struct QUEUE *queue, _TCHAR *fname, int flush)
{
  queue->area = (BYTE *) mem_allocate(AREA_SIZE);
  if (queue->area == NULL) {
    err_shw(_T("RQUE_TLS Unable to allocate memory"));
    return(-1);
  }
  _tcscpy(queue->name, fname);
  queue->writ = 0;
  queue->read = 0;
  queue->oldread = 0;
  queue->recread = 0;
  queue->recwrit = 0;
  queue->flush= flush;
  return(0);
} /* create_queue */

/*--------------------------------------------------------------------*/
/*                         close_queue                                */
/*--------------------------------------------------------------------*/
void close_queue(struct QUEUE *queue)
{
  mem_free(queue->area);
  return;
} /* close_queue */

/*--------------------------------------------------------------------*/
/*                         flush_queue                                */
/*--------------------------------------------------------------------*/
int flush_queue(struct QUEUE *queue)
{
  if (queue->read >= queue->writ) {
    queue->oldread = 0;
    queue->read = 0;
    queue->writ = 0;
  }
  return(0);
} /* flush_queue */

/*--------------------------------------------------------------------*/
/*                         reorg_queue                                */
/*--------------------------------------------------------------------*/
int reorg_queue(struct QUEUE *queue)
{
  BYTE  buffer[RO_SIZE];
  long  len;
  int   rlen, cc;

  len = queue->writ - queue->read;
  if (len < RO_SIZE) {                        /* last block reached */
    rlen = (int)len;
  }
  else {
    rlen = RO_SIZE;
  }
  err_shw(_T("RQUE_TLS: reorg_queue() reorganizing %s-queue(%ld, %ld)"),
                                 queue->name, queue->writ, queue->read);
  queue->writ = 0;
  while (len > 0) {
    cc = fread_queue(queue, (void*)&buffer[0], rlen);
    if (cc < 0) {
      err_shw(_T("RQUE_TLS: reorg_queue() ERROR %d during read on reorganisation"));
      return(cc);
    }
    cc = fwrit_queue(queue, (void*)&buffer[0], rlen);
    if (cc < 0) {
      err_shw(_T("RQUE_TLS: reorg_queue() ERROR %d during write on reorganisation"));
      return(cc);
    }
    len -= rlen;
    if (len < RO_SIZE) {                        /* last block reached */
      rlen = (int)len;
    }
  }
  queue->read = 0;                                    /* point to 1st */
  queue->oldread = 0;
  cc = flush_queue(queue);
  err_shw(_T("RQUE_TLS: reorg_queue() reorganisation done(%ld, %ld)"),
                                                queue->writ, queue->read);
  return(cc);
} /* reorg_queue */

/*--------------------------------------------------------------------*/
/*                         write_queue                                */
/*--------------------------------------------------------------------*/
int write_queue(struct QUEUE *queue, MESSAGE_STRUCT *ms)
{
  int  cc;

  cc = fwrit_queue(queue, (void*)ms, QHEADSIZE);
  if (cc < 0) {
    return(cc);
  }
  cc = fwrit_queue(queue, (void*)ms->data, ms->length);
  if (cc < 0) {
    queue->writ -= QHEADSIZE;           /* put back on original value */
    return(cc);
  }
  queue->recwrit++;
  return(flush_queue(queue));           /* returns zero if succesful  */
} /* write_queue */

/*--------------------------------------------------------------------*/
/*                         read_queue                                 */
/*--------------------------------------------------------------------*/
int read_queue(struct QUEUE *queue, MESSAGE_STRUCT *ms)
{
  int            cc;
  MESSAGE_STRUCT msget;

  if (queue->read >= queue->writ) {
    return(NO_MESSAGE);
  }
  queue->oldread = queue->read;
  cc = fread_queue(queue, (void*)&msget, QHEADSIZE);
  if (cc < 0) {
    return(cc);
  }
  if (ms->length < msget.length) {       /* Msg too small for q-elmnt */
    queue->read -= QHEADSIZE;
    return(BUFFER_TOO_SMALL);            /* This is VERY VERY wrong   */
  }
  memcpy((void*)ms, (void*)&msget, QHEADSIZE);
  cc = fread_queue(queue, (void*)ms->data, ms->length);
  if (cc < 0) {
    queue->read -= QHEADSIZE;
    return(cc);
  }
  queue->recread++;
  if (queue->flush) {                       /* must be done automatic */
    cc = flush_queue(queue);
    if (cc < 0) {
      return(cc);
    }
  }
  return(ms->type);
} /* read_queue */

/*--------------------------------------------------------------------*/
/*                         fwrit_queue                                */
/*--------------------------------------------------------------------*/
static int fwrit_queue(struct QUEUE *queue, void *data, int len)
{
  if (queue->writ + len > AREA_SIZE) {
    return(QUEUE_FULL);
  }
  memmove(queue->area + queue->writ, data, len);
  queue->writ += len;
  return(0);
} /* fwrit_queue */

/*--------------------------------------------------------------------*/
/*                         fread_queue                                */
/*--------------------------------------------------------------------*/
static int fread_queue(struct QUEUE *queue, void *data, int len)
{
  if (queue->read + len > queue->writ) {
    return(ERROR_ON_QUEUE);
  }
  memmove(data, queue->area + queue->read, len);
  queue->read += len;
  return(0);
} /* fread_queue */
