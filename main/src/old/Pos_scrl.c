/*
 *     Module Name       : POS_SCRL.C
 *
 *     Type              : Scroll windows functions
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
 * 05-Dec-2001 Added Article Finder.                                     M.W.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <stdio.h>
                                            /* Pos (library) include files   */
#include "stri_tls.h"
#include "appbase.h"
                                            /* Toolsset include files.       */
#include "fmt_tool.h"
#include "tm_mgr.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "inp_mgr.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_recs.h"
#include "pos_scrl.h"
#include "pos_inp.h"
#include "pos_func.h"
#include "WPos_mn.h"
#include "pos_txt.h"
#include "pos_keys.h"

static void build_inv_arti_header(TM_INDX);
static void build_inv_arti_line(TM_INDX);
static void build_inv_disc_line(TM_INDX);
static void build_inv_depo_line(TM_INDX);
static void build_art_find_line(TM_INDX);

/*---------------------------------------------------------------------------*/
/*                          init_scrl                                        */
/*---------------------------------------------------------------------------*/
short init_scrl(void)
{
  short status = SUCCEED;

  scrl_init();
  status |= scrl_add_fn(build_inv_arti_header, SCRL_INV_ART_HEADER);
  status |= scrl_add_fn(build_inv_arti_line,   SCRL_INV_ART_LINE);
  status |= scrl_add_fn(build_inv_disc_line,   SCRL_INV_DISCNT_LINE);
  status |= scrl_add_fn(build_inv_depo_line,   SCRL_INV_DEPOSIT_LINE);
  status |= scrl_add_fn(build_art_find_line,   SCRL_ART_FIND_LINE);

  if (status == FAIL) {
    return(-1);
  }

  return(SUCCEED);
} /* init_scrl */


/*---------------------------------------------------------------------------*/
/*                          build_inv_arti_header                            */
/*---------------------------------------------------------------------------*/
static void build_inv_arti_header(TM_INDX indx)
{
  /* Group_id is not used, the information to display is not in the tdm-   */
  /* structure but in the array: _TCHAR *scrn_inv_TXT[] which is assigned   */
  /* in txt_eng.c                                                          */
  scrn_string_out_curr_location(scrn_inv_TXT[6]);

  return;
} /* build_inv_arti_header */


/*---------------------------------------------------------------------------*/
/*                          build_inv_arti_line                              */
/*---------------------------------------------------------------------------*/
static void build_inv_arti_line(TM_INDX i_indx)
{
  TM_ITEM_GROUP h_item;
  _TCHAR buffer[83];
  _TCHAR cbuffer[20];

  if (i_indx==C_ITEM) {
    memcpy(&h_item, &c_item, sizeof(TM_ITEM_GROUP));
  }
  else {
    tm_read_nth(TM_ITEM_NAME, (void*)&h_item, i_indx);
  }
  if (h_item.arti.display_status==0) {
    h_item.arti.display_status=ARTICLE_OK;
  }

  ch_memset(buffer, _T(' '), sizeof(buffer));
  /* Inverse text if article is to be voided                                 */
  if (i_indx==corr_item && corr_item!=TM_ROOT) {
    buffer[0]=_T('\1');                       /* inverse-video                   */
  }
  else {
    buffer[0]=_T('\2');                       /* normal-video                    */
  }

//  buffer[81]=_T('\2');                        /* end-of-line, normal video       */
  buffer[82]=_T('\0');                        /* end-of-buffer                   */

  if (h_item.arti.base.bar_cd[0]==_T('\0')) {
    ftoa(h_item.arti.base.art_no, 13, cbuffer);
    format_string(&string_artno14, cbuffer);
  }
  else {
    format_string(&string_artno14, h_item.arti.base.bar_cd);
  }
  strcpy_no_null(buffer+1, string_artno14.result);

  if (h_item.arti.base.vat_no>=0) {
    *(buffer+49)=(_TCHAR)((short)_T('0')+h_item.arti.base.vat_no);
  }

  if (h_item.arti.base.arti_perception_ind==1)
    strcpy_no_null(buffer+15, "^");// acm -

  strcpy_no_null(buffer+16, h_item.arti.base.descr);

  *cbuffer=_T('\0');          /* If weight is being entered, display 0.000 or 0 */
  if (h_item.arti.art_ind==ART_IND_WEIGHT) {
    if (h_item.arti.display_status!=WWEIGHT_ART &&
      h_item.arti.display_status!=PWEIGHT_ART) {
      ftoa(h_item.arti.base.qty*1000.0, 9, cbuffer);
    }
    format_string(&string_qty9, cbuffer);
    strcpy_no_null(buffer+50, string_qty9.result);
  }
  else {
    if (h_item.arti.display_status!=WWEIGHT_ART &&
      h_item.arti.display_status!=PWEIGHT_ART) {
      ftoa(h_item.arti.base.qty, 9, cbuffer);
    }
    format_string(&string_qty5, cbuffer);
    strcpy_no_null(buffer+50, string_qty5.result);
  }

  if (h_item.arti.display_status==PRICE_ART) {  /* If price is being entered,*/
    _tcscpy(cbuffer,_T("0"));                        /* display 0.                */
  }
  else {
    ftoa_price(h_item.arti.base.price, 18, cbuffer);
  }

  if (*cbuffer==_T('-')) {                          /* Price can be negative,    */
    format_string(&string_price11_2,cbuffer+1); /* but display allways       */
  }
  else {                                        /* positive!                 */
    format_string(&string_price11_2,cbuffer);
  }
  strcpy_no_null(buffer+59, string_price11_2.result);

  if (h_item.arti.display_status!=ARTICLE_OK) { /* Display goods-value only  */
    _tcscpy(cbuffer,empty);                      /* if line is complete.      */
  }
  else {
    ftoa_price(h_item.arti.base.goods_value, 18, cbuffer);
  }

  format_string(&string_price11_2, cbuffer);
  strcpy_no_null(buffer+70, string_price11_2.result);

  scrn_string_out_curr_location(buffer);

  return;
} /* build_inv_arti_line */


/*---------------------------------------------------------------------------*/
/*                          build_inv_disc_line                              */
/*---------------------------------------------------------------------------*/
static void build_inv_disc_line(TM_INDX i_indx)
{
  TM_ITEM_GROUP h_item;
  _TCHAR buffer[83];
  _TCHAR cbuffer[20];

  if (i_indx==C_ITEM) {
    memcpy(&h_item, &c_item, sizeof(TM_ITEM_GROUP));
  }
  else {
    tm_read_nth(TM_ITEM_NAME, (void*)&h_item, i_indx);
  }

  ch_memset(buffer, _T(' '), sizeof(buffer));
  /* Inverse text if article is to be voided                                 */
  if (i_indx==corr_item && corr_item!=TM_ROOT) {
    buffer[0]=_T('\1');                       /* inverse-video                   */
  }
  else {
    buffer[0]=_T('\2');                       /* normal-video                    */
  }
//  buffer[81]=_T('\2');                        /* end-of-line, normal video       */
  buffer[82]=_T('\0');                        /* end-of-buffer                   */

  strcpy_no_null(buffer+16, h_item.disc.base.descr);

  if (h_item.disc.base.vat_no>=0) {
    *(buffer+49)=(_TCHAR)((short)_T('0')+h_item.disc.base.vat_no);
  }

  ftoa(h_item.disc.base.qty, 8, cbuffer);
  format_string(&string_qty4, cbuffer);
  strcpy_no_null(buffer+51, string_qty4.result);

  ftoa_price(h_item.disc.base.price, 18, cbuffer);

  if (*cbuffer==_T('-')) {                          /* Price can be negative,    */
    format_string(&string_price11_2,cbuffer+1); /* but display allways       */
  }
  else {                                        /* positive!                 */
    format_string(&string_price11_2,cbuffer);
  }
  strcpy_no_null(buffer+59, string_price11_2.result);

  ftoa_price(h_item.disc.base.goods_value, 18, cbuffer);

  format_string(&string_price11_2, cbuffer);
  strcpy_no_null(buffer+70, string_price11_2.result);

  scrn_string_out_curr_location(buffer);

  return;
} /* build_inv_disc_line */


/*---------------------------------------------------------------------------*/
/*                          build_inv_depo_line                              */
/*---------------------------------------------------------------------------*/
static void build_inv_depo_line(TM_INDX i_indx)
{
  TM_ITEM_GROUP h_item;
  _TCHAR buffer[83];
  _TCHAR cbuffer[20];

  if (i_indx==C_ITEM) {
    memcpy(&h_item, &c_item, sizeof(TM_ITEM_GROUP));
  }
  else {
    tm_read_nth(TM_ITEM_NAME, (void*)&h_item, i_indx);
  }

  ch_memset(buffer, _T(' '), sizeof(buffer));
  /* Inverse text if article is to be voided                                 */
  if (i_indx==corr_item && corr_item!=TM_ROOT) {
    buffer[0]=_T('\1');                       /* inverse-video                   */
  }
  else {
    buffer[0]=_T('\2');                       /* normal-video                    */
  }
//  buffer[81]=_T('\2');                        /* end-of-line, normal video       */
  buffer[82]=_T('\0');                        /* end-of-buffer                   */

  if (h_item.depo.base.bar_cd[0]==_T('\0')) {
    ftoa(h_item.depo.base.art_no, 13, cbuffer);
    format_string(&string_artno14, cbuffer);
  }
  else {
    format_string(&string_artno14, h_item.depo.base.bar_cd);
  }
  strcpy_no_null(buffer+1, string_artno14.result);

  if (h_item.depo.base.vat_no>=0) {
    *(buffer+49)=(_TCHAR)((short)_T('0')+h_item.depo.base.vat_no);
  }

  strcpy_no_null(buffer+16, h_item.depo.base.descr);

  *cbuffer=_T('\0');          /* If weight is being entered, display 0.000 or 0 */
  ftoa(h_item.depo.base.qty, 8, cbuffer);
  format_string(&string_qty4, cbuffer);
  strcpy_no_null(buffer+51, string_qty4.result);

  ftoa_price(h_item.depo.base.price, 18, cbuffer);
  if (*cbuffer==_T('-')) {                          /* Price can be negative,    */
    format_string(&string_price11_2,cbuffer+1); /* but display allways       */
  }
  else {                                        /* positive!                 */
    format_string(&string_price11_2,cbuffer);
  }
  strcpy_no_null(buffer+59, string_price11_2.result);

  ftoa_price(h_item.depo.base.goods_value, 18, cbuffer);

  format_string(&string_price11_2, cbuffer);
  strcpy_no_null(buffer+70, string_price11_2.result);

  scrn_string_out_curr_location(buffer);

  return;
} /* build_inv_depo_line */

/*---------------------------------------------------------------------------*/
/*                          build_art_find_line                              */
/*---------------------------------------------------------------------------*/
static void build_art_find_line(TM_INDX i_indx)
{
  TM_ARTF_GROUP h_item;
  _TCHAR buffer[83];
  _TCHAR mailno[10];
  _TCHAR cbuffer[20];
  _TCHAR video_mode;

  tm_read_nth(TM_ARTF_NAME, (_TCHAR *)&h_item, i_indx);

  ch_memset(buffer, _T('\0'), sizeof(buffer));

  if (i_indx==selected_item && selected_item!=TM_ROOT) {
    video_mode=_T('\1');                       /* inverse-video                   */
  }
  else {
    video_mode=_T('\2');                       /* normal-video                    */
  }

  ftoa_price(h_item.sell_pr, 18, cbuffer);
  format_string(&string_price11_2, cbuffer);

  *mailno = _T('\0');
  if (h_item.mmail_no > 0) {
    _stprintf(mailno, _T("%4d"), h_item.mmail_no);
  }

  _stprintf(buffer, _T("%c%-33.33s %10.10s %6d%3.3s %4.4s %10d"),
    video_mode,
    h_item.descr,
    string_price11_2.result,
    h_item.cont_sell_unit,
    h_item.pack_type,
    mailno,
    h_item.art_no);

  scrn_string_out_curr_location(buffer);

  return;
} /* build_art_find_line */
