/*
 *     Module Name       : COMM_TLS.C
 *
 *     Type              : POS ctree & communication functions to STNETP24
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
 * 21-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 * 11-Dec-2000 Added backlog for messages sent to stnetp12             R.N.B.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Changes for Article Finder.                               M.W.
 * --------------------------------------------------------------------------
 * 03-May-2002 Connection termination when till is not defined in the
 *             netnodes                                                J.D.M.
 * --------------------------------------------------------------------------
 * 02-Aug-2005 Search customer by Fiscal number.                         M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "stnetp24.h"
#include "DllMnp24.h"
#include "pos_recs.h"
#include "sll_mgr.h"
                                            /* standard include files      */
#include <time.h>
                                            /* POS toolset include files   */
#include "comm_tls.h"

          /* the size of the buffer is the max entity size from pos_recs.h */
             /* plus the offset length of the entity data in SC_CTREE_INFO */
#define COMMBUF_SIZE   (MAX_POS_ENT_SIZE+CORRLENGTH)

// local variables
static MESSAGE_STRUCT ms_send;
static MESSAGE_STRUCT ms_recv;
static MESSAGE_STRUCT ms_save;              /* to save one message         */

static BYTE  msbuff[COMMBUF_SIZE];
static BYTE  savebuff[COMMBUF_SIZE];             /* to save one message    */

static SLL ms_send_backlog;                          /* backlog of ms_send */
static SLL ms_send_data_backlog;    /* backlog for data portion of ms_send */

static SC_CTREE_INFO *cti = (SC_CTREE_INFO *)&msbuff[0];       /* maps cti */
static int  bo_stat = GONE;              /* socket connection status bo */
static int  cr_stat = GONE;              /* socket connection status cr */

// local defined (and hidden) functions
static short put_message(MESSAGE_STRUCT *);
static short get_message(MESSAGE_STRUCT *);
static short wait_get(short, short);
static short put_wait_get(short, short);
static short ctree_gate(short, short, short, short, void *, short, short);
static short wanted(short status, short type);
static short lan_request(short, short, REQUEST_STRUCT *, void *);
static void  chng_lan_status(MESSAGE_STRUCT *);
static short command_2_p24(short, int);
static short message_backlog(short, MESSAGE_STRUCT *);

void (*display)(int) = NULL;                    /* display status function */

/*-------------------------------------------------------------------------*/
/*                         comm_tls_init                                   */
/*-------------------------------------------------------------------------*/
void comm_tls_init(void (*func)(int))
{
  display = func;
  memset(&ms_save, 0, sizeof(MESSAGE_STRUCT));
  sll_init(&ms_send_backlog, sizeof(MESSAGE_STRUCT));
  sll_init(&ms_send_data_backlog, sizeof(msbuff));
} /* comm_tls_init */

/*-------------------------------------------------------------------------*/
/*                         comm_idle                                       */
/*-------------------------------------------------------------------------*/
void comm_idle()
{
                                            /* process messages in backlog */
  while (sll_read(&ms_send_backlog, 0, &ms_send) == SUCCEED) {
    sll_read(&ms_send_data_backlog, 0, msbuff);
    ms_send.data = msbuff;
    if (put_message(&ms_send)==FAIL || ms_send.action==P12BUSY) {
      break;
    }
    sll_remove_elem(&ms_send_backlog, 0);
    sll_remove_elem(&ms_send_data_backlog, 0);
  }

  stnetp24_comm_idle();
} /* comm_idle */

/*-------------------------------------------------------------------------*/
/*                         put_message                                     */
/*-------------------------------------------------------------------------*/
static short put_message(MESSAGE_STRUCT * ms)
{
  short status;

  status = stnetp24_pass_data(ms, STNETP24_USER_PUT);
  if (status == P12BUSY) {
    sll_add(&ms_send_backlog, ms);              /* try later (in comm_idle) */
    sll_add(&ms_send_data_backlog, ms->data);
    ms->action = P12BUSY;
    return(SUCCEED);
  }
  if (status != SUCCEED) {
    ms->action = status;
    return(FAIL);
  }
  return(SUCCEED);
} /* put_message */

/*-------------------------------------------------------------------------*/
/*                         get_message                                     */
/*-------------------------------------------------------------------------*/
static short get_message(MESSAGE_STRUCT * ms)
{
  ms->dest_id   = PS_ID;
  ms->signature = 0;
  ms->type      = 0;
  ms->action    = 0;
  ms->data      = msbuff;
  ms->length    = COMMBUF_SIZE;
  return(stnetp24_pass_data(ms, STNETP24_USER_GET));
} /* get_message */


/*-------------------------------------------------------------------------*/
/*                         wait_get                                        */
/*-------------------------------------------------------------------------*/
static short wait_get(short secs, short type)
{
  short  status, working;
  time_t start, now;                   /* to calculate max seconds to wait */

  working = FALSE;
  if (type == ms_save.type) {  /* this one is in the save buffer   */
    memcpy(&ms_recv, &ms_save, sizeof(MESSAGE_STRUCT));
    memcpy(msbuff, savebuff, ms_recv.length);
    memset(&ms_save, 0, sizeof(MESSAGE_STRUCT));
    ms_recv.data = msbuff;
    status = SUCCEED;
    goto end;
  }
  /*working = FALSE;*/
  time(&start);
  status = NO_MESSAGE;
  while (status == NO_MESSAGE) {
    time(&now);
    if ((short)(now - start) > secs) {
      goto time_out;
    }
    comm_idle();
    status = get_message(&ms_recv);
    if (status == NO_MESSAGE) {                         /* Try to be smart */
      get_network_status(&bo_stat, &cr_stat);
      switch (ms_send.dest_id) {
        case BO_ID:
          if (bo_stat != CONN) {
            goto time_out;
          }
          break;
        case CR_ID:
          if (cr_stat != CONN) {
            goto time_out;
          }
          break;
        default:
          /* Unknown destination id! */
          break;
      }
    }
    status = wanted(status, type);
    if (!working && now - start > 1) {
      working = TRUE;
      if (display) {
        display(YES);
      }
    }
  }
  status = SUCCEED;
  goto end;

time_out:
  status = TIME_OUT;

end:
  if ((working) && (display)) {
    display(NO);
  }
  return(status);
} /* wait_get */

/*-------------------------------------------------------------------------*/
/*                         put_wait_get                                    */
/*-------------------------------------------------------------------------*/
static short put_wait_get(short secs, short type)
{
  short  status;

  status = put_message(&ms_send);
  if (status != SUCCEED) {
    return(status);
  }
  if (secs == 0) {                            /* do NOT wait for an answer */
    return(SUCCEED);
  }
  return(wait_get(secs, type));
} /* put_wait_get */

/*-------------------------------------------------------------------------*/
/*                         ctree_gate                                      */
/*-------------------------------------------------------------------------*/
static short
ctree_gate(short  action,                    /* IO-action to perform        */
           short  type,                      /* record type                 */
           short  len,                       /* length of data area         */
           short  fno,                       /* CTREE-filenumber            */
           void  *data,                      /* data area                   */
           short  giveback,                  /* data send back up           */
           short  keyrel                     /* the keyrelation type        */
          )
{
  short status;

  ms_send.dest_id   = SC_ID;                /* always to 'storecontroller' */
  ms_send.signature = PS_ID;                /* always from pos_main        */
  ms_send.type      = type;
  ms_send.action    = action;
  ms_send.data      = msbuff;
  ms_send.length    = (short)(len + CORRLENGTH);
  cti->indexfilenum = fno;                  /* already mapped on msbuff    */
  cti->keyrel = keyrel;
  memcpy(&msbuff[CORRLENGTH], data, len);   /* where cti->data starts      */
  status = put_wait_get(MAXWAIT, type);
  while (status == TIME_OUT) {
    status = wait_get(MAXWAIT, type);
  }
  if (status == SUCCEED) {
    if (giveback) {                         /* retrieved data ?            */
      memcpy(data, &msbuff[CORRLENGTH], len);
    }
    status = cti->status;                   /* always mapped on msbuff     */
  }
  return(status);
} /* ctree_gate */

/*-------------------------------------------------------------------------*/
/*                         wanted                                          */
/*-------------------------------------------------------------------------*/
static short wanted(short status, short type)
{
  if (status == NO_MESSAGE) {               /* nothing really changed      */
    return(status);
  }
  if (status == type) {
    return(status);
  }
  switch (ms_recv.type) {
    case COMM_TYPE :
      chng_lan_status(&ms_recv);
      status = NO_MESSAGE;
      break;
    case CUST_TYPE :                                      /* save this one */
    case CUFI_TYPE :                                      /* save this one */
      memcpy(&ms_save, &ms_recv, sizeof(MESSAGE_STRUCT));
      memcpy(savebuff, msbuff, ms_recv.length);
      status = NO_MESSAGE;
      break;
    default        :
      status = NO_MESSAGE;                       /* filter everything else */
      break;
  }
  return(status);
} /* wanted */

/*-------------------------------------------------------------------------*/
/*                         pos_first_rec                                   */
/*-------------------------------------------------------------------------*/
short pos_first_rec(short type, short len, short fno, void *data)
{
  return(ctree_gate(FRST_ACTION, type, len, fno, data, TRUE, 0));
} /* pos_first_rec */

/*-------------------------------------------------------------------------*/
/*                         pos_start_rec                                   */
/*-------------------------------------------------------------------------*/
short pos_start_rec(short type, short len, short fno, void *data)
{
  return(ctree_gate(READ_ACTION, type, len, fno, data, TRUE, keyGTE));
} /* pos_start_rec */

/*-------------------------------------------------------------------------*/
/*                         pos_next_rec                                    */
/*-------------------------------------------------------------------------*/
short pos_next_rec(short type, short len, short fno, void *data)
{
  return(ctree_gate(NXTR_ACTION, type, len, fno, data, TRUE, keyEQL));
} /* pos_next_rec */

/*-------------------------------------------------------------------------*/
/*                         pos_next_rec_no_key                             */
/*-------------------------------------------------------------------------*/
short pos_next_rec_no_key(short type, short len, short fno, void *data)
{
  return(ctree_gate(NXTR_NO_KEY_ACTION, type, len, fno, data, TRUE, keyEQL));
} /* pos_next_rec_no_key */

/*-------------------------------------------------------------------------*/
/*                         pos_get_rec                                     */
/*-------------------------------------------------------------------------*/
short pos_get_rec(short type, short len, short fno, void *data, short relation)
{
  return(ctree_gate(READ_ACTION, type, len, fno, data, TRUE, relation));
} /* pos_get_rec */

/*-------------------------------------------------------------------------*/
/*                         pos_put_rec                                     */
/*-------------------------------------------------------------------------*/
short pos_put_rec(short type, short len, short fno, void *data)
{
  return(ctree_gate(INST_ACTION, type, len, fno, data, FALSE, 0));
} /* pos_put_rec */

/*-------------------------------------------------------------------------*/
/*                         pos_update_rec                                  */
/*-------------------------------------------------------------------------*/
short pos_update_rec(short type, short len, short ifno, short dfno, void *data)
{
  cti->status = dfno;                  /* TRICKY, but necessary for RWTREC */
  return(ctree_gate(UPDT_ACTION, type, len, ifno, data, FALSE, keyEQL));
} /* pos_update_rec */

/*-------------------------------------------------------------------------*/
/*                         pos_delete_rec                                  */
/*-------------------------------------------------------------------------*/
short pos_delete_rec(short type, short len, short ifno, short dfno, void *data)
{
  cti->status = dfno;                  /* TRICKY, but necessary for DELREC */
  return(ctree_gate(DELT_ACTION, type, len, ifno, data, FALSE, keyEQL));
} /* pos_delete_rec */

/*-------------------------------------------------------------------------*/
/*                         pos_upda_system                                 */
/*-------------------------------------------------------------------------*/
short pos_upda_system(short len, short fno, SYSTEM_DEF *data)
{
  return(pos_update_rec(SYST_TYPE, len, fno, fno, (void*)data));
} /* pos_upda_system */

/*-------------------------------------------------------------------------*/
/*                         pos_read_system                                 */
/*-------------------------------------------------------------------------*/
short pos_read_system(short len, short fno, SYSTEM_DEF *data)
{
  return(pos_first_rec(SYST_TYPE, len, fno, (void*)data));
} /* pos_read_system */

/*-------------------------------------------------------------------------*/
/*                         pos_upda_genvar                                 */
/*-------------------------------------------------------------------------*/
short pos_upda_genvar(short len, short fno, GENVAR_DEF *data)
{
  return(pos_update_rec(GENV_TYPE, len, fno, fno, (void*)data));
} /* pos_upda_system */

/*-------------------------------------------------------------------------*/
/*                         pos_read_genvar                                 */
/*-------------------------------------------------------------------------*/
short pos_read_genvar(short len, short fno, GENVAR_DEF *data)
{
  return(pos_first_rec(GENV_TYPE, len, fno, (void*)data));
} /* pos_read_system */

/*-------------------------------------------------------------------------*/
/*                         pos_read_customer                               */
/*-------------------------------------------------------------------------*/
  short
pos_read_customer(short len, short fno, POS_CUST_DEF *data)
{
  return(pos_get_rec(CUST_TYPE, len, fno, (void*)data, keyEQL));
} /* pos_read_customer */

/*-------------------------------------------------------------------------*/
/*                         lan_request                                     */
/*-------------------------------------------------------------------------*/
static short lan_request(short type, short secs, REQUEST_STRUCT *rqst, void *data)
{
  short status;

  ms_send.signature = PS_ID;                              /* from pos_main */
  ms_send.type      = RQST_TYPE;
  ms_send.action    = 0;
  ms_send.data      = msbuff;
  ms_send.length    = sizeof(REQUEST_STRUCT);
  memcpy(msbuff, rqst, ms_send.length);

  status = put_wait_get(secs, type);            /* only expected type back */
  if (status == SUCCEED &&
      data != NULL) {             /* NULL, indicates nothing expected back */
    memcpy(data, msbuff, ms_recv.length);       /* ms..length should be OK */
    status = ms_recv.action;                    /* 0: found, 1 NOT found   */
  }
  return(status);
} /* lan_request */

/*-------------------------------------------------------------------------*/
/*                         bo_again                                        */
/*-------------------------------------------------------------------------*/
short bo_again(short type, short secs, void *data)
{
  short status;

  status = wait_get(secs, type);                /* only expected type back */
  if (status == SUCCEED) {
    memcpy(data, msbuff, ms_recv.length);       /* ms..length should be OK */
    status = ms_recv.action;                    /* 0: found, 1 NOT found   */
  }
  return(status);
} /* bo_again */

/*-------------------------------------------------------------------------*/
/*                         cr_again                                        */
/*-------------------------------------------------------------------------*/
short cr_again(short type, short secs, void *data)
{
  short status;

  status = wait_get(secs, type);                /* only expected type back */
  if (status == SUCCEED) {
    memcpy(data, msbuff, ms_recv.length);       /* ms..length should be OK */
    status = ms_recv.action;                    /* 0: found, 1 NOT found   */
  }
  return(status);
} /* cr_again */

/*-------------------------------------------------------------------------*/
/*                         bo_request                                      */
/*-------------------------------------------------------------------------*/
short bo_request(short type, short secs, REQUEST_STRUCT *rqst, void *data)
{
  ms_send.dest_id = BO_ID;
  return(lan_request(type, secs, rqst, data));
} /* bo_request */

/*-------------------------------------------------------------------------*/
/*                         cr_request                                      */
/*-------------------------------------------------------------------------*/
short cr_request(short type, short secs, REQUEST_STRUCT *rqst, void *data)
{
  ms_send.dest_id = CR_ID;
  return(lan_request(type, secs, rqst, data));
} /* cr_request */

/*-------------------------------------------------------------------------*/
/*                         chng_lan_status                                 */
/*-------------------------------------------------------------------------*/
static void chng_lan_status(MESSAGE_STRUCT * ms)
{
#ifdef NOT_USED_ANYMORE
  if (ms->signature == BO_ID) {                    /* BackOffice           */
    bo_stat = *(short*)ms->data;
  }
  else {                                           /* Customer Reception   */
    cr_stat = *(short*)ms->data;
  }
#endif
} /* chng_lan_status */

/*-------------------------------------------------------------------------*/
/*                         get_bo_status                                   */
/*-------------------------------------------------------------------------*/
short get_bo_status(void)
{
  get_network_status(&bo_stat, &cr_stat);
  return(bo_stat);
} /* get_bo_status */

/*-------------------------------------------------------------------------*/
/*                         get_cr_status                                   */
/*-------------------------------------------------------------------------*/
short get_cr_status(void)
{
  get_network_status(&bo_stat, &cr_stat);
  return(cr_stat);
} /* get_cr_status */

/*-------------------------------------------------------------------------*/
/*                         command_2_p24                                   */
/*-------------------------------------------------------------------------*/
  static short
command_2_p24(short secs, int cmd)
{
  ms_send.dest_id   = SC_ID;                /* always to 'storecontroller' */
  ms_send.signature = PS_ID;                /* always from pos_main        */
  ms_send.type      = COMM_TYPE;
  ms_send.action    = 0;
  ms_send.data      = msbuff;
  ms_send.length    = sizeof(cmd);
  memcpy(msbuff, &cmd, ms_send.length);
  return(put_wait_get(secs, COMM_TYPE));
} /* command_2_p24 */

/*-------------------------------------------------------------------------*/
/*                         end_of_day_p24                                  */
/*-------------------------------------------------------------------------*/
short end_of_day_p24(void)
{
  return(command_2_p24(20 * MAXWAIT, EDAY));
} /* end_of_day_p24 */

/*-------------------------------------------------------------------------*/
/*                         powerfail_p24                                   */
/*-------------------------------------------------------------------------*/
short powerfail_p24(void)
{
  return(command_2_p24(5 * MAXWAIT, PFAL));
} /* powerfail_p24 */

/*-------------------------------------------------------------------------*/
/*                         cmd2p24                                         */
/*-------------------------------------------------------------------------*/
short cmd2p24(short cmd)
{
  return(command_2_p24(20 * MAXWAIT, cmd));
} /* cmd2p24 */
