/*
 *     Module Name       : ST_VLINE.C
 *
 *     Type              : States Correction, ReadVoidLine
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
 * 28-Mar-2001 Bug fixed. Handling PageUp key in corr_handle_scroll_key  M.W.
 * --------------------------------------------------------------------------
 * 29-Mar-2001 Added OCIA2_DATA.                                       R.N.B.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <math.h>
                                            /* Pos (library) include files   */

#include "appbase.h"
#include "stri_tls.h"
#include "misc_tls.h"
                                            /* Toolsset include files.       */
#include "tm_mgr.h"
#include "inp_mgr.h"
#include "state.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"
#include "err_mgr.h"
#include "tot_mgr.h"
#include "mapkeys.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_errs.h"
#include "pos_recs.h"
#include "pos_txt.h"
#include "pos_inp.h"
#include "pos_func.h"
#include "pos_keys.h"
#include "pos_vfy.h"
#include "pos_dflt.h"
#include "pos_st.h"
#include "pos_scrl.h"
#include "pos_tot.h"
#include "WPos_mn.h"
#include "st_main.h"

static void  view_item2(TM_INDX);
static void  Correction_VW(void);
static short corr_handle_scroll_key(_TCHAR *, short);
static short corr_search_artno(_TCHAR *, short);
static void  ReadVoidLine_VW(void);
static short end_corr_mode(_TCHAR *, short);
static short commit_void_item(_TCHAR *, short);
static TM_INDX step_item(TM_INDX, short);


/*---------------------------------------------------------------------------*/
/*                             step_item                                     */
/*---------------------------------------------------------------------------*/
static TM_INDX step_item(TM_INDX base, short offset)
{
  TM_INDX strt_indx, last_indx_ok, temp_indx;

  /*                                                                       */
  /* Step offset times through the item-tree.                              */
  /*                                                                       */
  /* The sign of offset determines the direction and will skip voided      */
  /* items (also skip the active_item).                                    */
  /*                                                                       */

  tm_read_nth(TM_ITEM_NAME, (void*)&c_item, base);

  if (offset!=0) {                              /* Do some stepping        */
    strt_indx    = base;
    temp_indx    = strt_indx;
    last_indx_ok = strt_indx;
    while (offset!=0 && temp_indx>=0) {                            /* Step */
      if (offset<0) {
        temp_indx=tm_prev(TM_ITEM_NAME, (void*)&c_item);
      }
      else {
        temp_indx=tm_next(TM_ITEM_NAME, (void*)&c_item);
      }
      if (temp_indx>=0) {                  /* Check Begin/End of item-list */
        if (c_item.voided==FALSE) {        /* Check voided or not.         */
          if (offset<0) {
            ++offset;
          }
          else {
            --offset;
          }
          last_indx_ok=temp_indx;
        }
      }
    }
    return(last_indx_ok);
  }
  return(base);                                            /* offset==0    */
} /* step_item */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE    CORRECTION                                             */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* At this point it is certain that voiding a line is legal.                 */
/* An inverted bar can be moved by the arrow-keys.                           */
/* It's also possible to enter an article number to move the inverted bar.   */
/* Pressing 'NO' will cancel this state and returns to Invoicing_ST.         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void Correction_VW(void)
{
  last_item=TM_ROOT;                              /* Disable repeat-key.   */
  if (corr_item==TM_ROOT) {
    display_item=TM_ROOT;
    corr_item=tm_last(TM_ITEM_NAME, (void*)&c_item);
    /* To be shure that this item is not voided, step back and forward   */
    /* ( step_item() ensures steps over not voided items).               */
    corr_item=step_item(corr_item, -1);
    corr_item=step_item(corr_item,  1);
    view_item2(corr_item);
  }
  else {
    scrl_redraw_lines(INV_ART_LIST_WINDOW);
  }
  scrn_select_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[36],0,16);    /* USE SCROLL-KEYS OR ENTER AR  */
} /* Correction_VW */



static VERIFY_ELEMENT Correction_VFY[] =
{
  NO_KEY,          (void *)NULL,
  CLEAR_KEY,       vfy_clear_key,
  VOID_LINE_KEY,   (void *)NULL,
  ENTER_KEY,       corr_search_artno,
  OCIA1_DATA,      corr_search_artno,
  OCIA2_DATA,      corr_search_artno,
  LINE_UP_KEY,     corr_handle_scroll_key,
  LINE_DOWN_KEY,   corr_handle_scroll_key,
  PAGE_UP_KEY,     corr_handle_scroll_key,
  PAGE_DOWN_KEY,   corr_handle_scroll_key,
  OPEN_DRAWER_KEY, open_and_close_drawer,      /* Approval in state-engine. */
  UNKNOWN_KEY,     illegal_fn_key
};

/* Input controler defined in state: Invoicing_ST                            */

static PROCESS_ELEMENT Correction_PROC[] =
{
  NO_KEY,       end_corr_mode,
  UNKNOWN_KEY,  (void *)NULL
};


static CONTROL_ELEMENT Correction_CTL[] =
{
  NO_KEY,        &Invoicing_ST,
  VOID_LINE_KEY, &ReadVoidLine_ST,
  UNKNOWN_KEY,   &Correction_ST
};


extern STATE_OBJ Correction_ST =
{
  ST_CORRECTION,
  Correction_VW,
  no_DFLT,
  &Dartno14KO14n,                        /* defined in Invoicing_ST         */
  Correction_VFY,
  Input_UVW,
  Correction_PROC,
  Correction_CTL
};



/*---------------------------------------------------------------------------*/
/*                          corr_handle_scroll_key                           */
/*---------------------------------------------------------------------------*/
static short corr_handle_scroll_key(_TCHAR *data, short key)
{
  TM_INDX new_indx;

  switch(key) {
    case LINE_UP_KEY :
      new_indx=step_item(corr_item, -1);
      if (new_indx>=0 && new_indx!=corr_item) {
        corr_item=new_indx;
        view_item2(corr_item);
      }
      else {
        rs_error_tone();
      }
      break;
    case LINE_DOWN_KEY :
      new_indx=step_item(corr_item, 1);
      if (new_indx>=0 && new_indx!=corr_item) {
        corr_item=new_indx;
        view_item2(corr_item);
      }
      else {
        rs_error_tone();
      }
      break;
    case PAGE_UP_KEY :
      new_indx=step_item(corr_item, (short)(scrl_get_numrows(INV_ART_LIST_WINDOW)*-1));
      if (new_indx>=0 && new_indx!=corr_item) {
        corr_item=new_indx;
        view_item2(corr_item);
      }
      else {
        rs_error_tone();
      }
      break;
    case PAGE_DOWN_KEY :
      new_indx=step_item(corr_item, scrl_get_numrows(INV_ART_LIST_WINDOW));
      if (new_indx>=0 && new_indx!=corr_item) {
        corr_item=new_indx;
        view_item2(corr_item);
      }
      else {
        rs_error_tone();
      }
      break;
    default:
      break;
  }

  return(UNKNOWN_KEY);
} /* corr_handle_scroll_key */


/*---------------------------------------------------------------------------*/
/*                              view_item2                                   */
/*---------------------------------------------------------------------------*/
void view_item2(TM_INDX item_indx)
{
  /*                                                                       */
  /* scrl_goto_line() makes sure all lines within the current item are     */
  /* displayed within the visible part of the scrolling window.            */
  /*                                                                       */

  scrl_goto_line(INV_ART_LIST_WINDOW, SCRL_INV_ART_LINE, item_indx);
  scrl_goto_line(INV_ART_LIST_WINDOW, SCRL_INV_DEPOSIT_LINE, item_indx);
  scrl_goto_line(INV_ART_LIST_WINDOW, SCRL_INV_DISCNT_LINE, item_indx);
  scrl_redraw_lines(INV_ART_LIST_WINDOW);
} /* view_item2 */


/*---------------------------------------------------------------------------*/
/*                            corr_search_artno                              */
/*---------------------------------------------------------------------------*/
static short corr_search_artno(_TCHAR *data, short key)
{
  _TCHAR buffer[19];
  short found;
  TM_INDX new_indx, pre_indx;

  /*                                                                       */
  /* If data is not empty (contains artno), the article number is looked   */
  /* up (nearest article) and highlighted.                                 */
  /* If data is empty, key is returned.                                    */
  /*                                                                       */

  if (*data) {
    found=FALSE;
    /* First search from current minus one, to top                         */
    pre_indx=corr_item;
    new_indx=step_item(corr_item, -1);
    while (!found && new_indx>=0 && new_indx!=pre_indx) {
      /* A previous item found, check artno or barcode.                    */

      tm_read_nth(TM_ITEM_NAME, (void*)&c_item, new_indx);
      if (_tcslen(data)<7) {
        ftoa(c_item.arti.base.art_no, 8, buffer);
      }
      else {
        _tcscpy(buffer, c_item.arti.base.bar_cd);
      }

      if (_tcscmp(buffer,data) == 0) {
        corr_item=new_indx;
        view_item2(corr_item);
        found=TRUE;
      }
      else {
        pre_indx=new_indx;
        new_indx=step_item(new_indx, -1);
      }
    }

    /*                                                                   */
    /* Second, from end to current item, or not at all if already        */
    /* found. If no other exist, it finds itself.                        */
    /* To be sure that this item is not voided, step back and forward.   */
    /* ( step_item() steps over voided items).                           */
    /*                                                                   */
    if (!found) {
      pre_indx=tm_last(TM_ITEM_NAME, (void*)&c_item);
      pre_indx=step_item(pre_indx, -1);
      pre_indx=step_item(pre_indx,  1);
      new_indx=pre_indx;
      do {
        tm_read_nth(TM_ITEM_NAME, (void*)&c_item, new_indx);
        if (_tcslen(data)<7) {
          ftoa(c_item.arti.base.art_no, 8, buffer);
        }
        else {
          _tcscpy(buffer, c_item.arti.base.bar_cd);
        }

        if (_tcscmp(buffer,data) == 0) {
          corr_item=new_indx;
          view_item2(corr_item);
          found=TRUE;
        }
        else {
          pre_indx=new_indx;
          new_indx=step_item(new_indx, -1);
        }
      } while (!found && new_indx>=0 && new_indx!=pre_indx);
    }
  }
  else {                                          /* data is empty         */
    return(UNKNOWN_KEY);
  }

  if (!found) {                                   /* article not found     */
    tm_read_nth(TM_ITEM_NAME, (void*)&c_item, corr_item);
    err_invoke(INVALID_ARTNO);
    *data=_T('\0');
  }

  return(UNKNOWN_KEY);
} /* corr_search_artno */


/*---------------------------------------------------------------------------*/
/* STATE-STRUCTURE   READ VOID LINE                                          */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Item to be voided is now highlighted (corr_item).                         */
/* Ask for confirmation.                                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
static void ReadVoidLine_VW(void)
{
  scrn_select_window(INV_ART_INPUT_WINDOW);
  scrn_string_out(input_TXT[37],0,3); /* VOID ITEM Y/N?                    */
} /* ReadVoidLine_VW */


static VERIFY_ELEMENT ReadVoidLine_VFY[] =
{
  NO_KEY,          (void *)NULL,
  ENTER_KEY,       (void *)NULL,
  LINE_UP_KEY,     (void *)NULL,
  LINE_DOWN_KEY,   (void *)NULL,
  PAGE_UP_KEY,     (void *)NULL,
  PAGE_DOWN_KEY,   (void *)NULL,
  OPEN_DRAWER_KEY, open_and_close_drawer,      /* Approval in state-engine. */
  UNKNOWN_KEY,     illegal_fn_key
};


static PROCESS_ELEMENT ReadVoidLine_PROC[] =
{
  ENTER_KEY,       commit_void_item,
  UNKNOWN_KEY,     (void *)NULL
};


static CONTROL_ELEMENT ReadVoidLine_CTL[] =
{
  NO_KEY,          &Correction_ST,
  ENTER_KEY,       &Invoicing_ST,
  LINE_UP_KEY,     &Correction_ST,
  LINE_DOWN_KEY,   &Correction_ST,
  PAGE_UP_KEY,     &Correction_ST,
  PAGE_DOWN_KEY,   &Correction_ST,
  UNKNOWN_KEY,     &ReadVoidLine_ST
};


extern STATE_OBJ ReadVoidLine_ST =
{
  ST_READ_VOID_LINE,
  ReadVoidLine_VW,
  no_DFLT,
  &DYN1K1n,
  ReadVoidLine_VFY,
  Input_UVW,
  ReadVoidLine_PROC,
  ReadVoidLine_CTL
};


/*---------------------------------------------------------------------------*/
/*                          end_corr_mode                                    */
/*---------------------------------------------------------------------------*/
static short end_corr_mode(_TCHAR *data, short key)
{
  unsigned short save_window;

  /*                                                                       */
  /* End the correction mode. Redraw the scroll window causing the         */
  /* inverse display to be canceled.                                       */
  /*                                                                       */

  save_window=scrn_get_current_window();
  corr_item=TM_ROOT;
  scrl_redraw_lines(INV_ART_LIST_WINDOW);
  scrn_select_window(save_window);

  return(key);
} /* end_corr_mode */


/*---------------------------------------------------------------------------*/
/*                          commit_void_item                                 */
/*---------------------------------------------------------------------------*/
static short commit_void_item(_TCHAR *data, short key)
{
  do_void_item(corr_item);
  return(end_corr_mode(empty, key));
} /* commit_void_item */

