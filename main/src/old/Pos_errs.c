/*
 *     Module Name       : POS_ERRS.C
 *
 *     Type              : Application error structures and functions
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
 * 21-Sep-2001 Added Pending Invoice.                                    M.W.
 * --------------------------------------------------------------------------
 * 05-Dec-2001 Added Article Finder.                                     M.W.
 * --------------------------------------------------------------------------
 * 23-Jan-2002 Changes for Credit note. Ask for amount if offline.       M.W.
 * --------------------------------------------------------------------------
 * 15-Feb-2002 Added Max Cash Amount in Cashdrawer functionality         M.W.
 * --------------------------------------------------------------------------
 * 03-May-2002 Connection termination when till is not defined in the
 *             netnodes                                                J.D.M.
 * --------------------------------------------------------------------------
 * 20-Jan-2003 Added Backlog corrupt message.                            M.W.
 * --------------------------------------------------------------------------
 * 21-Feb-2003 Added error OPOS_SCANNER_READ_TIME_OUT.                 J.D.M.
 * --------------------------------------------------------------------------
 * 29-Apr-2003 Added error SCAN_KEYB_CONFLICT.                         J.D.M.
 * --------------------------------------------------------------------------
 * 10-Jun-2003 Added errors MSAM_PARSING_TIME_OUT and
 *             MSAM_TOO_MANY_ERR.                                      J.D.M.
 * -------------------------------------------------------------------------- 
 * 13-Jul-2003 Bugfix: INP_TOO_MANY_KEYS was not known outside the
 *             #ifdef NCR. This could result in a 'unknown error'
 *             message with a needed supervisor approval.              J.D.M.
 * -------------------------------------------------------------------------- 
 * 01-May-2007 Added PRN_SMALL_SEQ_FINISHED                            J.D.M.
 * --------------------------------------------------------------------------
 * 12-Ago-2011 Change for sales Turkey						            ACM -
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */
#include <stdio.h>

#include "stnetp24.h"                       /* Pos (library) include files   */
#include "comm_tls.h"
#include "stri_tls.h"

                                            /* Toolsset include files.       */
#include "inp_mgr.h"
#include "tm_mgr.h"
#include "mem_mgr.h"
#include "prn_mgr.h"
#include "bp_mgr.h"
#include "scrn_mgr.h"
#include "err_mgr.h"
#include "state.h"

#include "pos_tm.h"                         /* Application include files.    */
#include "pos_recs.h"
#include "WPos_mn.h"
#include "pos_errs.h"
#include "pos_inp.h"
#include "pos_keys.h"
#include "pos_com.h"
#include "pos_func.h"
#include "pos_txt.h"
#include "write.h"
#include "pos_bp1.h"

#include "mapkeys.h"

extern ERR_NAME err_current;            /* Declared in err_mgr.c             */

short  power_fail = 0;                  /* to short cut bp_i_inv_init        */

_TCHAR *error_extra_msg=NULL;

/*
 *    Error Wait Function  (Wait on key in list)
 */

static short wait_on_CLEAR(void)
{
  short keylist[] = { CLEAR_KEY, 0 };

  return rs_wait_for_key_in_set(keylist);
} /* wait_on_CLEAR */

short wait_for_any_input(void)
{
  rs_wait_for_any_input();

  return(SUCCEED);
} /* wait_for_any_input */

short wait_for_any_input_and_clear(void)
{
  rs_wait_for_any_input();
  inp_abort_data();

  return(SUCCEED);
} /* wait_for_any_input_and_clear */

short wait_for_key_in_L(void)
{
  rs_wait_for_key_in_L();

  return(SUCCEED);
} /* wait_for_key_in_L */

static short wait_on_yes_no(void)
{
  short key, keylist[] = { ENTER_KEY, NO_KEY, 0 };

  key=rs_wait_for_key_in_set(keylist);

  return( (key==ENTER_KEY) ? SUCCEED : FAIL );
} /* wait_on_yes_no */


static short do_rs_error_tone(ERROR2 *err)
{
  /* This function is introduced because 'ERROR2 beep = { rs_errror_tone };' */
  /* gives a warning.                                                        */

  rs_error_tone();

  return(SUCCEED);
} /* do_rs_error_tone */


ERROR2 beep =
{
  do_rs_error_tone
};


/*
 *    The General Error Message Display Function
 */

short err_and_status(ERROR1 *error)
{
  short status = SUCCEED;
  _TCHAR buffer[81];
  _TCHAR dummy[256]; /* ADAPTED FOR WINPOS */
  unsigned short save_window = scrn_get_current_window();

  inp_abort_data();                                 /* abort any input      */

  scrn_clear_window(ERROR_WINDOW_ROW1);              /* Select error window */

  if(err_current != ERR_NOT_EXIST) {                    /* if unknown error */
    _stprintf(buffer, _T("(%d) "),err_current);       /* display unknown error no */
    _tcscpy(dummy, buffer); /* ADAPTED FOR WINPOS */
    scrn_string_out(buffer,0,0);                        /* Display error no */
  }

  if( (_tcsstr(*error->message1, _T("%s")) != NULL) &&
       error_extra_msg && *error_extra_msg != _T('\0') ) {
    _stprintf(buffer, *error->message1,error_extra_msg);
    *error_extra_msg=0;
    scrn_string_out_curr_location(buffer);            /* Display the string */
    _tcscat(dummy, buffer); /* ADAPTED FOR WINPOS */
  }
  else {
    scrn_string_out_curr_location(*error->message1);    /* Display the str. */
    _tcscat(dummy, *error->message1); /* ADAPTED FOR WINPOS */
  }

  scrn_string_out(dummy,0,0); /* ADAPTED FOR WINPOS */

  scrn_select_window(ERROR_WINDOW_ROW2);       /* Select error window, row2 */
  scrn_string_out(*error->message2,0,0);              /* Display the string */

  rs_error_tone();                                  /* Sound the error tone */

  if (error->wait) {                         /* Use specified wait function */
    status = error->wait();                         /* If specified call fn */
    scrn_clear_window(ERROR_WINDOW_ROW1);                   /* Clear window */
    scrn_clear_window(ERROR_WINDOW_ROW2);                   /* Clear window */
  }
  scrn_select_window(save_window);                   /* Reselect old window */

  return status;
} /* err_and_status */


/*
 *    Display Error Window
 */

short err_and_SUCCEED(ERROR1 *error)
{
  /* Return status of err_and_status(),                                     */
  /* does not ignore is by returning SUCCEED!                               */
  return err_and_status(error);         /* Use unknown error displ function */
} /* err_and_SUCCEED */


/*
 *    Display Error Window
 *    Also display err_current, set in err_invoke()
 */

short unknown_err_and_SUCCEED(ERROR1 *error)
{
  /* Return status of err_and_status(),                                     */
  /* does not ignore is by returning SUCCEED!                               */
  return err_and_status(error);         /* Use unknown error displ function */
} /* unknown_err_and_SUCCEED */


/*
 *    Display Error Window and check power fail status
 */

short err_and_check_pwr_fail(ERROR1 *error)
{
  short status;

  power_fail = 1;
  status=err_and_status(error);          /* Use normal error display function*/
  powerfail_p24();                       /* Tell P24 about powerfail         */
  rs_power_failed();

  bp_now(BP_INVOICE, BP_INV_INIT, 0);    /* Initialise invoice printer.      */
  if (printers_attached & PRINTER_SIZE_SMALL) {
    bp_now(BP_SMALL_INVOICE, BP_INV_INIT, 0);
  }
  power_fail = 0;

  return status;                         /* ... and then ignore the status  */
} /* err_and_check_pwr_fail */


/*
 *    ERROR MESSAGE STRUCTURES
 */

/*
 *  Unknown error, called by the err_mgr() is the error invoked
 *  is not in the known-error-list.
 */

static ERROR1 err_unknown =
{
  unknown_err_and_SUCCEED,
  &err_msg_TXT[4],                                        /* Unknown error   */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

/*
 *    Power Failure Has Occurred
 *       PRN_POWER_FAILURE
 *       RS_DRAWER_FAILURE
 *    - Exit when CLEAR_KEY hit, then check PF status to clear it
 */

static ERROR1 err_pwr_fail =
{
  err_and_check_pwr_fail,
  &err_msg_TXT[5],                                         /* POWER FAILURE */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/*
 *    Journal Paper is low
 *       PRN_JOURNAL_PAPER_OUT
 *    - Exit when CLEAR_KEY hit.
 */

static ERROR1 err_no_paper =
{
  err_and_SUCCEED,
  &err_msg_TXT[6],                                     /* JOURNAL PAPER LOW */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/*
 *    No slip is detected at the slip station, force insertion
 *       PRN_SLIP_OUT
 *       PRN_INSERT_SLIP
 *    - Open the Platen Exit when CLEAR_KEY hit.
 */

static ERROR1 err_no_slip =
{
  err_and_SUCCEED,
  &err_msg_TXT[7],                              /* PLEASE INSERT CHECK/SLIP */
  &err_msg_TXT[74],
  wait_on_yes_no
};

/*
 *    A slip is detected at the slip station, force removal.
 *       PRN_REMOVE_SLIP
 *       BP_REMOVE_SLIP_ERROR
 *    - Open the Platen Exit when slip is gone.
 */

static ERROR1 err_slip =
{
  err_and_SUCCEED,
  &err_msg_TXT[8],                              /* PLEASE REMOVE CHECK/SLIP */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/*
 *    The printer motor has collided with the paper.
 *       PRN_NOT_CONFIGURED
 *       PRN_UNDEFINED_ERROR
 *       PRN_FATAL_ERROR
 *       PRN_NOT_OPEN
 *       PRN_NOT_QUEUED
 *    -  Exit when CLEAR_KEY hit.
 */

static ERROR1 err_prn_gen =
{
  err_and_SUCCEED,
  &err_msg_TXT[10],                              /* GENERAL PRINTER FAILURE */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_prn_cover =
{
  err_and_SUCCEED,
  &err_msg_TXT[93],                              /* COVER IS OPEN           */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_off_line =
{
  err_and_SUCCEED,
  &err_msg_TXT[10],                              /* GENERAL PRINTER FAILURE */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/*
 *    The MSR returned an error.
 *       INP_MSR_ERROR
 *    -  Exit when input becomes available from the sequencer.
 */

static ERROR1 err_msr =
{
  err_and_SUCCEED,
  &err_msg_TXT[11],                                    /* CARD READ FAILURE */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_kbd =
{
  err_and_SUCCEED,
  &err_msg_TXT[13],                                     /* KEYBOARD FAILURE */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

/*
 *    The OCIA returned an error.
 *       INP_OCIA_ERROR
 *    -  Exit when input becomes available from the sequencer.
 */

static ERROR1 err_ocia =
{
  err_and_SUCCEED,
  &err_msg_TXT[14],                                    /* ITEM READ FAILURE */
  &err_msg_TXT[0],
  (void *)NULL
};

/*
 *       INP_TOO_MANY_KEYS
 *    -  The input contains too many keys
 */

static ERROR1 inp_too_many_keys =
{
  err_and_SUCCEED,
  &err_msg_TXT[86],                                    /* TOO MANY KEYS     */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/*
 *    The MSR initialization returned an error.
 *       INP_MSR_FAILURE
 *    -  Exit when CLEAR_KEY hit.
 */

static ERROR1 err_msr_not_avail =
{
  err_and_SUCCEED,
  &err_msg_TXT[16],                             /* MSR DEVICE NOT AVAILABLE */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/*
 *    The OCIA port 1 initialization returned an error.
 *       INP_OCIA1_FAILURE
 *    -  Exit when CLEAR_KEY hit.
 */

static ERROR1 err_ocia1_not_avail =
{
  err_and_SUCCEED,
  &err_msg_TXT[17],                            /* OCIA PORT 1 NOT AVAILABLE */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/*********/
/*
 *    The OCIA port 2 initialization returned an error.
 *       INP_OCIA2_FAILURE
 *    -  Exit when CLEAR_KEY hit.
 */

static ERROR1 err_ocia2_not_avail =
   {
   err_and_SUCCEED,               
   &err_msg_TXT[92],                            /* OCIA PORT 2 NOT AVAILABLE */
   &err_msg_TXT[1],
   wait_on_CLEAR
   };
/*********/


/*
 *    The Console Driver initialization returned an error.
 *       INP_KEYLOCK_FAILURE
 *    -  Exit when CLEAR_KEY hit.
 */

static ERROR1 err_keylock =
{
  err_and_SUCCEED,
  &err_msg_TXT[18],                                      /* KEYLOCK FAILURE */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/*
 *    Terminal Retail devices (Misc Driver) are inoperable.
 *       RS_HANDLE_ERROR
 *    -  Exit when the keylock goes to the 'S" and then the 'N' positions
 */

static ERROR1 err_retail_devices =
{
  err_and_SUCCEED,
  &err_msg_TXT[20],                 /* RETAIL DEVICE, INITIALIZATION FAILURE */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

/*
 *    Terminal Software Power Down has failed.
 *       RS_NO_POWER_DOWN
 *    -  Exit when the keylock goes to the 'S" and then the 'N' positions
 */

static ERROR1 err_rs_pwr =
{
  err_and_SUCCEED,
  &err_msg_TXT[21],                          /* SOFTWARE POWER DOWN FAILURE */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

/*
 *    Terminal Ram Disk could not be formatted.
 *       RS_FORMAT_FAILURE
 *    -  Exit when the keylock goes to the 'S" and then the 'N' positions
 */

static ERROR1 err_rs_format =
{
  err_and_SUCCEED,
  &err_msg_TXT[22],                              /* RAM DISK FORMAT FAILURE */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

/*
 *    Terminal Ram Memory limits have been exceeded.
 *       MEM_UNAVAILABLE
 *    -  Exit when the keylock goes to the 'S" and then the 'N' positions
 */

static ERROR1 err_memory =
{
  err_and_SUCCEED,
  &err_msg_TXT[15],                              /* OUT OF MEMORY CONDITION */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

/*
 *    POS Errors
 */

static ERROR1 err_to_less_foreign =
{
  err_and_SUCCEED,
  &err_msg_TXT[83],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_paytype_not_same_select =
{
  err_and_SUCCEED,
  &err_msg_TXT[82],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_data_corrupt =
{
  err_and_SUCCEED,
  &err_msg_TXT[81],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_cust_delete_passport =
{
   err_and_SUCCEED,               
   &err_msg_TXT[94],
   &err_msg_TXT[1],
   wait_on_CLEAR
};

static ERROR1 err_before_copy_invoice =
{
  err_and_SUCCEED,
  &err_msg_TXT[80],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_after_copy_invoice =
{
  err_and_SUCCEED,
  &err_msg_TXT[79],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_no_other_payments =
{
  err_and_SUCCEED,
  &err_msg_TXT[78],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_approved_item =
{
  err_and_SUCCEED,
  &err_msg_TXT[77],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

/********/
static ERROR1 err_validate_cheque =
{
   err_and_SUCCEED,               
   &err_msg_TXT[90],
   &err_msg_TXT[74],
   wait_on_yes_no
};
/*********/


static ERROR1 err_cashier2fast =
{
  err_and_SUCCEED,
  &err_msg_TXT[76],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_too_many_shifts =
{
  err_and_SUCCEED,
  &err_msg_TXT[75],
  &err_msg_TXT[3],
  wait_on_yes_no
};

static ERROR1 err_accept_credit_card =
{
  err_and_SUCCEED,
  &err_msg_TXT[73],
  &err_msg_TXT[74],
  wait_on_yes_no
};

static ERROR1 err_no_amount_on_credit_cards =
{
  err_and_SUCCEED,
  &err_msg_TXT[72],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_no_disc_on_r2c_barcd =
{
  err_and_SUCCEED,
  &err_msg_TXT[71],
  &err_msg_TXT[1],
  wait_on_CLEAR
};


static ERROR1 err_no_discount_on_deposit =
{
  err_and_SUCCEED,
  &err_msg_TXT[70],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_value_too_large =
{
  err_and_SUCCEED,
  &err_msg_TXT[69],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_shift_recovered_not_closed =
{
  err_and_SUCCEED,
  &err_msg_TXT[68],
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

static ERROR1 err_cancel_till_lift_refill =
{
  err_and_SUCCEED,
  &err_msg_TXT[67],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_remove_invoice_msg =
{
  err_and_SUCCEED,
  &appr_msg_TXT[5],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_init_environment_error =
{
  err_and_SUCCEED,
  &err_msg_TXT[39],
  &err_msg_TXT[25],                                /* Turn key to L         */
  wait_for_key_in_L
};

static ERROR1 err_illegal_system_date =
{
  err_and_SUCCEED,
  &err_msg_TXT[51],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_cust_unknown =
{
  err_and_SUCCEED,
  &err_msg_TXT[43],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_invalid_cust_on_pos =
{
   err_and_SUCCEED,
   &err_msg_TXT[87],
   &err_msg_TXT[0],
   wait_for_super_appr_or_NO
};

static ERROR1 err_cust_illegal_passport_num =
{
  err_and_SUCCEED,
  &err_msg_TXT[66],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_cust_passport_invalid =
{
  err_and_SUCCEED,
  &err_msg_TXT[28],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_cust_passport_expired =
{
  err_and_SUCCEED,
  &err_msg_TXT[65],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_cust_blocked =
{
  err_and_SUCCEED,
  &err_msg_TXT[54],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_cfee_return =
{
  err_and_SUCCEED,
  &err_msg_TXT[113],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_cfee_overrule_once =
{
  err_and_SUCCEED,
  &err_msg_TXT[84],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};
static ERROR1 err_cfee_overrule_one_year =
{
  err_and_SUCCEED,
  &err_msg_TXT[85],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_ocia_not_legal =
{
  err_and_SUCCEED,
  &err_msg_TXT[64],                                /* ocia not legal to use */
  &err_msg_TXT[0],
  (void *)NULL
};

static ERROR1 err_exceeds_cheque_limit =
{
  err_and_SUCCEED,
  &err_msg_TXT[63],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_cust_no_cheques_allowed =
{
  err_and_SUCCEED,
  &err_msg_TXT[31],                                        /* No cheques al */
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

/********/
static ERROR1 war_cust_no_cheques_allowed =
{
   err_and_SUCCEED,               
   &err_msg_TXT[91],                                       /* No cheques al */
   &err_msg_TXT[1],
   wait_on_CLEAR
};
/********/

static ERROR1 err_amnt_paid_exceed_limit =
{
  err_and_SUCCEED,
  &err_msg_TXT[29],                                        /* Amnt exceeds  */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_first_enter_amount =
{
  err_and_SUCCEED,
  &err_msg_TXT[30],                                        /* First amnt    */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_payment_not_present =
{
  err_and_SUCCEED,
  &err_msg_TXT[32],                                        /* Illegal paymw */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_illegal_cliename_perception =
{
  err_and_SUCCEED,
  &err_msg_TXT[149],                                        /* Illegal paymw */
  &err_msg_TXT[1],
  wait_on_CLEAR
};




//DOCUMENT_DOC_ERR
static ERROR1 err_document_doc_err =
{
  err_and_SUCCEED,
  &err_msg_TXT[148],                                        /* DOCUMENT_DOC_ERR */
  &err_msg_TXT[1],
  wait_on_CLEAR
};



static ERROR1 err_total_not_possible =
{
  err_and_SUCCEED,
  &err_msg_TXT[33],                                        /* tot not possi */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_no_last_item =
{
  err_and_SUCCEED,
  &err_msg_TXT[34],                                        /* No last item  */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_invalid_artno =
{
  err_and_SUCCEED,
  &err_msg_TXT[35],                                        /* Invalid artno */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_invalid_discno =
{
  err_and_SUCCEED,
  &err_msg_TXT[38],                                        /* Invalid disc. */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_qty_too_large =
{
  err_and_SUCCEED,
  &err_msg_TXT[36],                                        /* Qty exceeds   */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_field_not_empty =
{
  err_and_SUCCEED,
  &err_msg_TXT[37],                                        /* field not emp */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_illegal_function_key_error =
{
  err_and_SUCCEED,
  &err_msg_TXT[40],                                        /* Illegal func  */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_pos_too_many_keys =
{
  err_and_SUCCEED,
  &err_msg_TXT[41],                                        /* Too many keys */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_zero_not_legal_error =
{
  err_and_SUCCEED,
  &err_msg_TXT[42],                                        /* Zero not lega */
  &err_msg_TXT[1],
  wait_on_CLEAR
};


static ERROR1 test_key =                        /* For debug purpose only!   */
{
  err_and_SUCCEED,
  &err_msg_TXT[24],                                        /* Test key      */
  &err_msg_TXT[0],
  wait_for_any_input
};

static ERROR1 err_invalid_menu_option =
{
  err_and_SUCCEED,
  &err_msg_TXT[27],                                        /* INVALID MNU-OP*/
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_invalid_input =
{
  err_and_SUCCEED,
  &err_msg_TXT[2],                                         /* INVALID INPUT */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_not_ruc_customer =
{
  err_and_SUCCEED,
  &err_msg_TXT[122],                                         /*	NOT RUC CUSTOMER */
  &err_msg_TXT[1],
  wait_on_CLEAR
};
/*jcp*/
static ERROR1 err_turkey_voucher =
{
  err_and_SUCCEED,
  &err_msg_TXT[123],                                        
  &err_msg_TXT[1],
  wait_on_CLEAR
};
/*JCP*/
static ERROR1 err_turkey_scan =
{
  err_and_SUCCEED,
  &err_msg_TXT[124],                                        
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/* 12-Ago-2011 acm - { */
static ERROR1 err_turkey_vale_scan =
{
  err_and_SUCCEED,
  &err_msg_TXT[124],                                        
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_weight_turkey_mustbe7 =
{
  err_and_SUCCEED,
  &err_msg_TXT[125],                                        
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_one_vale_by_invoice =
{
  err_and_SUCCEED,
  &err_msg_TXT[126],    /* 0126  ,_T("Solo se permite un vale de Pavo por Boleta/Factura")*/
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_turkey_not_applicable =
{
  err_and_SUCCEED,
  &err_msg_TXT[127],    /* 0127  ,_T("Articulo Pavo no corresponde con el Vale ingresado")*/
  &err_msg_TXT[1],
  wait_on_CLEAR
};


static ERROR1 err_article_mustbe_turkey =
{
  err_and_SUCCEED,
  &err_msg_TXT[128],    /*  0128  ,_T("El articulo a ingresar debe ser un codigo de Pavo") */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

/* 12-Ago-2011 acm - } */


/* 25-Set-2012 acm - { */

    static ERROR1 err_code_queue_busting_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[129], /* 0129  ,_T("Codigo QueueBusting es invalido, verifique por favor")  */  
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

    static ERROR1 err_conection_queue_busting_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[130], /* 0130  ,_T("Falla de conexion con el Servidor QueueBusting")        */
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

     static ERROR1 err_article_queue_busting_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[131], /* 0131  ,_T("Error al leer articulo %s en proceso QueueBusting")     */
      &err_msg_TXT[1],
      wait_on_CLEAR
    };


     static ERROR1 err_format_queue_busting_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[132], /* 0132  ,_T("QB:Error en formato de datos QueueBusting")*/
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

     static ERROR1 err_rows_big_queue_busting_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[133], /* 0133  ,_T("QB:Filas recibidas por el servidor no puede ser mayor que 500")*/
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

     static ERROR1 err_rows_queue_busting_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[134], /* 0134  ,_T("QB:Filas recibidas por el servidor difiere al esperado")*/
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

     static ERROR1 err_socket_created_queue_busting_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[135], /* 0135  ,_T("QB:No se puede crear el socket en el POS")*/
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

     static ERROR1 err_hostname_queue_busting_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[136], /* 0136  ,_T("QB:No se peude leer hostname por nombre")*/
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

     static ERROR1 err_queue_busting_notfound =
    {
      err_and_SUCCEED,
      &err_msg_TXT[137], /* 0137  ,_T("QB:No se encontro codigo Queue Busting")*/
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

     static ERROR1 err_queue_busting_sql_invalid =
    {
      err_and_SUCCEED,
      &err_msg_TXT[138], /* 0138  ,_T("QB:Error al procesar commando SQL")*/
      &err_msg_TXT[1],
      wait_on_CLEAR
    };

      /* 0137  ,_T("QB:No se encontro codigo Queue Busting")*/
      /* 0138  ,_T("QB:Error al procesar commando SQL")*/


/* 25-Set-2011 acm - } */


static ERROR1 err_cash_pincd_mistake1 =
{
  err_and_SUCCEED,
  &err_msg_TXT[44],                                      /* Invalid pincode */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_cash_pincd_mistake2 =
{
  err_and_SUCCEED,
  &err_msg_TXT[45],                                      /* Inv. pinc,lst   */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_cash_pincd_mistake3 =
{
  err_and_SUCCEED,
  &err_msg_TXT[46],                                      /* Inv.pincd,call  */
  &err_msg_TXT[3],
  wait_for_supervisor_approval                           /* S -> N-key      */
};

static ERROR1 err_no_items_to_void =
{
  err_and_SUCCEED,
  &err_msg_TXT[47],                                      /* Inv.pincd,call  */
  &err_msg_TXT[1],
  wait_on_CLEAR
};


static ERROR1 err_discnt_amnt_too_large =
{
  err_and_SUCCEED,
  &err_msg_TXT[48],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_illegal_time_value =
{
  err_and_SUCCEED,
  &err_msg_TXT[49],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_illegal_date_value =
{
  err_and_SUCCEED,
  &err_msg_TXT[50],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_cashier_not_available =
{
  err_and_SUCCEED,
  &err_msg_TXT[60],
  &err_msg_TXT[1],
  wait_on_CLEAR
};


static ERROR1 err_too_many_invoice_lines =
{
  err_and_SUCCEED,
  &err_msg_TXT[52],
  &err_msg_TXT[62],
  wait_for_any_input_and_clear
};

static ERROR1 err_article_blocked =
{
  err_and_SUCCEED,
  &err_msg_TXT[53],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_price_too_large =
{
  err_and_SUCCEED,
  &err_msg_TXT[55],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_illegal_barcode =
{
  err_and_SUCCEED,
  &err_msg_TXT[56],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_lift_amnt_exceed_till_amnt =
{
  err_and_SUCCEED,
  &err_msg_TXT[57],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_cashier_already_logon =
{
  err_and_SUCCEED,
  &err_msg_TXT[58],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_cashier_not_allowed =
{
  err_and_SUCCEED,
  &err_msg_TXT[59],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_unknown_cashierno =
{
  err_and_SUCCEED,
  &err_msg_TXT[60],
  &err_msg_TXT[0],
  wait_for_super_appr_or_NO
};

static ERROR1 err_no_training =
{
  err_and_SUCCEED,
  &err_msg_TXT[61],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_net_config =
{
  err_and_SUCCEED,
  &err_msg_TXT[107],
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

static ERROR1 err_net_no_shift =
{
  err_and_SUCCEED,
  &err_msg_TXT[108],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_msam_syntax_err =
{
  err_and_SUCCEED,
  &err_msg_TXT[88],                              /* MULTISAM SYNTAX ERROR */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

static ERROR1 err_msam_distribute_fail =
{
  err_and_SUCCEED,
  &err_msg_TXT[89],                              /* MULTISAM SYNTAX ERROR */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

static ERROR1 err_validate_save_invoice =
{
   err_and_SUCCEED,               
   &err_msg_TXT[95],
   &err_msg_TXT[74],
   wait_on_yes_no
};

static ERROR1 err_overwrite_saved_invoice =
{
   err_and_SUCCEED,               
   &err_msg_TXT[96],
   &err_msg_TXT[74],
   wait_on_yes_no
};

static ERROR1 err_get_pending_invoice =
{
   err_and_SUCCEED,               
   &err_msg_TXT[97],
   &err_msg_TXT[74],
   wait_on_yes_no
};

static ERROR1 err_query_parse_error =
{
   err_and_SUCCEED,               
   &err_msg_TXT[98],
   &err_msg_TXT[1],
   wait_on_CLEAR
};

static ERROR1 err_query_too_many =
{
   err_and_SUCCEED,               
   &err_msg_TXT[99],
   &err_msg_TXT[1],
   wait_on_CLEAR
};

static ERROR1 err_query_no_records =
{
   err_and_SUCCEED,               
   &err_msg_TXT[100],
   &err_msg_TXT[1],
   wait_on_CLEAR
};

static ERROR1 err_opos_keyb =
{
   err_and_SUCCEED,               
   &err_msg_TXT[101],
   &err_msg_TXT[1],
   wait_on_CLEAR
};

static ERROR1 err_no_connection =
{
  err_and_SUCCEED,
  &err_msg_TXT[102],                                /* VOUCHER ERROR */
  &err_msg_TXT[3],
  wait_for_super_appr_or_NO
};

static ERROR1 err_voucher_not_known =
{
  err_and_SUCCEED,
  &err_msg_TXT[103],                                /* VOUCHER ERROR */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_voucher_blocked =
{
  err_and_SUCCEED,
  &err_msg_TXT[104],                                /* VOUCHER ERROR */
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

static ERROR1 err_voucher_length =
{
  err_and_SUCCEED,
  &err_msg_TXT[105],                                /* VOUCHER ERROR */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_unblock_voucher =
{
  err_and_SUCCEED,
  &err_msg_TXT[106],                                /* VOUCHER ERROR */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_exceed_max_start_float =
{
  err_and_SUCCEED,
  &err_msg_TXT[109],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_exceed_max_amount_cashdrawer =
{
  err_and_SUCCEED,
  &err_msg_TXT[110],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_backlog_corrupt =
{
  err_and_SUCCEED,
  &err_msg_TXT[111],
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

static ERROR1 err_double_line_on_invoice =
{
  err_and_SUCCEED,
  &err_msg_TXT[112],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_opos_scanner_read_time_out =
{
   err_and_SUCCEED,               
   &err_msg_TXT[114],
   &err_msg_TXT[1],
   wait_on_CLEAR
};

static ERROR1 err_scan_keyb_conflict =
{
  err_and_SUCCEED,
  &err_msg_TXT[115],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_msam_parsing_time_out =
{
  err_and_SUCCEED,
  &err_msg_TXT[116],
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

static ERROR1 err_msam_too_many_err =
{
  err_and_SUCCEED,
  &err_msg_TXT[117],
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};

static ERROR1 err_exceed_max_amount_donation =
{
  err_and_SUCCEED,
  &err_msg_TXT[118],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_exceed_change_value_donation =
{
  err_and_SUCCEED,
  &err_msg_TXT[119],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_cust_fisc_no_error =
{
  err_and_SUCCEED,
  &err_msg_TXT[120],
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 prn_err_small_seq_finished =
{
  err_and_SUCCEED,
  &err_msg_TXT[121],
  &err_msg_TXT[3],
  wait_for_supervisor_approval
};


static ERROR1 err_field_not_horeca =
{
  err_and_SUCCEED,
  &err_msg_TXT[139],                                        /* */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_article_not_horeca =
{
  err_and_SUCCEED,
  &err_msg_TXT[140],                                        /* */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_vigencia_not_horeca =
{
  err_and_SUCCEED,
  &err_msg_TXT[141],                                        /* */
  &err_msg_TXT[1],
  wait_on_CLEAR
};


static ERROR1 err_invalid_valepavo =
{
  err_and_SUCCEED,
  &err_msg_TXT[142],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_valepavo_conection =
{
  err_and_SUCCEED,
  &err_msg_TXT[143],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_valepavo_used =
{
  err_and_SUCCEED,
  &err_msg_TXT[144],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_valepavo_unknown =
{
  err_and_SUCCEED,
  &err_msg_TXT[145],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_valepavo_notfound =
{
  err_and_SUCCEED,
  &err_msg_TXT[146],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_valepavo_bd =
{
  err_and_SUCCEED,
  &err_msg_TXT[147],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_epos_connect =
{
  err_and_SUCCEED,
  &err_msg_TXT[162],                                        /* EPOS error */
  &err_msg_TXT[1],
  wait_on_CLEAR
};




////////////////////////////////////////


static ERROR1 err_anticipo_conection =
{
  err_and_SUCCEED,
  &err_msg_TXT[150],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_anticipo_used =
{
  err_and_SUCCEED,
  &err_msg_TXT[151],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_anticipo_unknown =
{
  err_and_SUCCEED,
  &err_msg_TXT[152],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_anticipo_notfound =
{
  err_and_SUCCEED,
  &err_msg_TXT[153],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_anticipo_bd =
{
  err_and_SUCCEED,
  &err_msg_TXT[154],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};



static ERROR1 err_anticipo_percepcion =
{
  err_and_SUCCEED,
  &err_msg_TXT[155],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};
static ERROR1 err_anticipo_igv =
{
  err_and_SUCCEED,
  &err_msg_TXT[156],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};
static ERROR1 err_anticipo_total =
{
  err_and_SUCCEED,
  &err_msg_TXT[157],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_anticipo_total2 =
{
  err_and_SUCCEED,
  &err_msg_TXT[158],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};

static ERROR1 err_anticipo_doc_fac_type =
{
  err_and_SUCCEED,
  &err_msg_TXT[159],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};
static ERROR1 err_anticipo_doc_bol_type =
{
  err_and_SUCCEED,
  &err_msg_TXT[160],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};
static ERROR1 err_anticipo_fis_no =
{
  err_and_SUCCEED,
  &err_msg_TXT[161],                                        /* Invalid valepavo */
  &err_msg_TXT[1],
  wait_on_CLEAR
};



///////////////////////////////////////////


/*
 *    Set up POS error handlers.
 */

void pos_init_errors(void)
{
  err_init();

  /*
   *    PRINTER ERRORS DEFINED IN PRN_MGR.H
   */

  err_set(PRN_METHOD_ERR, &err_prn_gen);
  err_set(PRN_OFFLINE, &err_prn_gen);
  err_set(PRN_TIMEOUT, &err_prn_gen);
  err_set(PRN_BUSY, &err_prn_gen);
  err_set(PRN_COVER_OPEN, &err_prn_cover);
  err_set(PRN_UNKNOWN_ERR, &err_prn_gen);
  err_set(PRN_INIT_ERROR, &err_prn_gen);
  err_set(PRN_NO_PAPER, &err_no_paper);
  err_set(PRN_REMOVE_SLIP, &err_slip);
  err_set(PRN_NOT_EMPTY, &err_slip);
  err_set(PRN_INSERT_SLIP, &err_no_slip);
  err_set(PRN_JOURNAL_PAPER_OUT,&err_no_paper);
  err_set(PRN_FATAL_ERROR, &err_prn_gen);

/*
 *    INPUT MANAGER ERRORS DEFINED IN INP_MGR.H
 */

  err_set(INP_KBD_ERROR, &err_kbd);
  err_set(INP_POWER_FAILURE,&err_pwr_fail);
  err_set(INP_TOO_MANY_KEYS, &inp_too_many_keys);
#ifdef NCR
  err_set(INP_MSR_ERROR, &err_msr);
  err_set(INP_OCIA_ERROR, &err_ocia);
  err_set(INP_MSR_FAILURE, &err_msr_not_avail);
  err_set(INP_OCIA1_FAILURE, &err_ocia1_not_avail);
  err_set(INP_OCIA2_FAILURE, &err_ocia2_not_avail);
  err_set(INP_KEYLOCK_FAILURE, &err_keylock);
  err_set(INP_CANNOT_BACKSPACE, &beep);
#endif

/*
 *    RETAIL SPECIFIC TOOLS ERRORS DEFINED IN RS_TOOLS.H
 */

  err_set(RS_HANDLE_ERROR, &err_retail_devices);
  err_set(RS_DRAWER_FAILURE, &err_pwr_fail);
#ifdef NCR
  err_set(RS_NO_POWER_DOWN, &err_rs_pwr);
  err_set(RS_FORMAT_FAILURE, &err_rs_format);
#endif

/*
 *    MEMORY MANAGER
 */

  err_set(MEM_UNAVAILABLE, &err_memory);


/*
 *    POS Errors
 */
  err_set(TO_LESS_FOREIGN, &err_to_less_foreign);
  err_set(PAYTYPE_NOT_SAME_SELECT, &err_paytype_not_same_select);
  err_set(DATA_CORRUPT, &err_data_corrupt);
  err_set(BEFORE_COPY_INVOICE, &err_before_copy_invoice);
  err_set(AFTER_COPY_INVOICE, &err_after_copy_invoice);
  err_set(NO_OTHER_PAYMENTS, &err_no_other_payments);

  err_set(APPROVED_ITEM, &err_approved_item);

  err_set(CASHIER2FAST, &err_cashier2fast);
  err_set(TOO_MANY_SHIFTS, &err_too_many_shifts);
  err_set(ACCEPT_CREDIT_CARD, &err_accept_credit_card);
  err_set(NO_AMOUNT_ON_CREDIT_CARDS, &err_no_amount_on_credit_cards);

  err_set(NO_DISC_ON_R2C_BARCD, &err_no_disc_on_r2c_barcd);
  err_set(NO_DISCOUNT_ON_DEPOSIT, &err_no_discount_on_deposit);

  err_set(VALUE_TOO_LARGE, &err_value_too_large);
  err_set(SHIFT_RECOVERED_NOT_CLOSED, &err_shift_recovered_not_closed);

  err_set(CANCEL_TILL_LIFT_REFILL, &err_cancel_till_lift_refill);
  err_set(REMOVE_INVOICE_MSG, &err_remove_invoice_msg);
  err_set(ILLEGAL_SYSTEM_DATE , &err_illegal_system_date);
  err_set(INIT_ENVIRONMENT_ERROR, &err_init_environment_error);

  err_set(CUST_NO_FACTURA, &err_cust_illegal_passport_num);  /*JCP*/
  err_set(CUST_ILLEGAL_PASSPORT_NUMBER, &err_cust_illegal_passport_num);
  err_set(CUST_PASSPORT_INVALID, &err_cust_passport_invalid);
  err_set(CUST_PASSPORT_EXPIRED, &err_cust_passport_expired);
  err_set(CUST_BLOCKED, &err_cust_blocked);
  err_set(CUST_UNKNOWN, &err_cust_unknown);
  err_set(INVALID_GENV_CD, &err_invalid_cust_on_pos);

  err_set(INP_OCIA_NOT_LEGAL,&err_ocia_not_legal);
  err_set(EXCEEDS_CHEQUE_LIMIT, &err_exceeds_cheque_limit);
  err_set(DISCNT_AMNT_TOO_LARGE, &err_discnt_amnt_too_large);
  err_set(ILLEGAL_TIME_VALUE, &err_illegal_time_value);
  err_set(ILLEGAL_DATE_VALUE, &err_illegal_date_value);
  err_set(CASHIER_NOT_AVAILABLE, &err_cashier_not_available);
  err_set(TOO_MANY_INVOICE_LINES, &err_too_many_invoice_lines);
  err_set(ARTICLE_BLOCKED, &err_article_blocked);
  err_set(CUST_BLOCKED, &err_cust_blocked);
  err_set(PRICE_TOO_LARGE, &err_price_too_large);
  err_set(ILLEGAL_BARCODE, &err_illegal_barcode);
  err_set(LIFT_AMNT_EXCEED_TILL_AMNT, &err_lift_amnt_exceed_till_amnt);
  err_set(CASHIER_ALREADY_LOGON, &err_cashier_already_logon);
  err_set(CASHIER_NOT_ALLOWED, &err_cashier_not_allowed);
  err_set(UNKNOWN_CASHIERNO, &err_unknown_cashierno);

  err_set(CASH_PINCD_MISTAKE3, &err_cash_pincd_mistake3);
  err_set(CASH_PINCD_MISTAKE2, &err_cash_pincd_mistake2);
  err_set(CASH_PINCD_MISTAKE1, &err_cash_pincd_mistake1);
  err_set(NO_ITEMS_TO_VOID, &err_no_items_to_void);
  err_set(CUST_NO_CHEQUES_ALLOWED, &err_cust_no_cheques_allowed);
  err_set(WARN_NO_CHEQUES_ALLOWED, &war_cust_no_cheques_allowed);
  err_set(AMNT_PAID_EXCEED_LIMIT, &err_amnt_paid_exceed_limit);
  err_set(FIRST_ENTER_AMOUNT, &err_first_enter_amount);
  err_set(PAYMENT_NOT_PRESENT, &err_payment_not_present);



  

  err_set(TOTAL_NOT_POSSIBLE, &err_total_not_possible);
  err_set(NO_LAST_ITEM, &err_no_last_item);
  err_set(INVALID_ARTNO, &err_invalid_artno);
  err_set(INVALID_DISCNO, &err_invalid_discno);
  err_set(QTY_TOO_LARGE, &err_qty_too_large);
  err_set(FIELD_NOT_EMPTY, &err_field_not_empty);
  err_set(ILLEGAL_FUNCTION_KEY_ERROR, &err_illegal_function_key_error);
  err_set(POS_TOO_MANY_KEYS, &err_pos_too_many_keys);
  err_set(ZERO_NOT_LEGAL_ERROR, &err_zero_not_legal_error);
  err_set(CUST_DELETE_PASSPORT, &err_cust_delete_passport);

  err_set(NOT_RUC_CUSTOMER_ERROR, &err_not_ruc_customer);   /*jcp*/
  err_set(INVALID_KEY_ERROR, &err_invalid_input);
  err_set(ERR_NOT_EXIST, &err_unknown);
  err_set(TEST_KEY, &test_key);
  err_set(INVALID_MENU_OPTION, &err_invalid_menu_option);
  err_set(NO_TRAINING, &err_no_training);
  err_set(CFEE_OVERRULE_ONCE, &err_cfee_overrule_once);
  err_set(CFEE_OVERRULE_ONE_YEAR, &err_cfee_overrule_one_year);

  err_set(VALIDATE_CHEQUE, &err_validate_cheque);
  err_set(VALIDATE_SAVE_INVOICE, &err_validate_save_invoice);
  err_set(OVERWRITE_SAVED_INVOICE, &err_overwrite_saved_invoice);
  err_set(GET_PENDING_INVOICE, &err_get_pending_invoice);

  err_set(QUERY_PARSE_ERROR, &err_query_parse_error);
  err_set(QUERY_TOO_MANY, &err_query_too_many);
  err_set(QUERY_ZERO_RECORDS, &err_query_no_records);

  err_set(OPOS_KEYB_ERROR, &err_opos_keyb);
  err_set(OPOS_SCANNER_READ_TIME_OUT, &err_opos_scanner_read_time_out);

  err_set(NO_CONNECTION, &err_no_connection);
  err_set(VOUCHER_NOT_KNOWN, &err_voucher_not_known);
  err_set(VOUCHER_BLOCKED, &err_voucher_blocked);
  err_set(VOUCHER_LENGTH, &err_voucher_length);
  err_set(UNBLOCK_VOUCHER_ERROR, &err_unblock_voucher);

  err_set(MSAM_SYNTAX_ERR, &err_msam_syntax_err);
  err_set(MSAM_DISTR_ERR, &err_msam_distribute_fail);

  err_set(NET_CONFIG_ERROR, &err_net_config);
  err_set(NET_CFG_NO_SHIFT, &err_net_no_shift);

  err_set(EXCEED_MAX_START_FLOAT, &err_exceed_max_start_float);
  err_set(EXCEED_MAX_AMOUNT_CASHDRAWER, &err_exceed_max_amount_cashdrawer);

  err_set(BACKLOG_CORRUPT, &err_backlog_corrupt);
  err_set(DOUBLE_LINE_ON_INVOICE, &err_double_line_on_invoice);
  err_set(CFEE_RETURN, &err_cfee_return);

  err_set(SCAN_KEYB_CONFLICT, &err_scan_keyb_conflict);
  err_set(MSAM_PARSING_TIME_OUT, &err_msam_parsing_time_out);
  err_set(MSAM_TOO_MANY_ERR, &err_msam_too_many_err);

  err_set(EXCEED_MAX_AMOUNT_DONATION, &err_exceed_max_amount_donation);
  err_set(EXCEED_CHANGE_VALUE_DONATION, &err_exceed_change_value_donation);

  err_set(CUST_FISC_NO_ERROR, &err_cust_fisc_no_error);
  err_set(PRN_SMALL_SEQ_FINISHED, &prn_err_small_seq_finished);

  err_set(NOT_VALID_VOUCHER_TURKEY, &err_turkey_voucher);  /*JCP*/

  err_set(NOT_VALID_TURKEY, &err_turkey_scan);  /*JCP*/

  err_set(NOT_VALID_TURKEY_VALE, &err_turkey_vale_scan);  /*12-Ago-2011 acm -*/
  err_set(WEIGHT_TURKEY_MUSTBE7, &err_weight_turkey_mustbe7);  /*12-Ago-2011 acm -*/

  err_set(ONE_VALE_BY_INVOICE, &err_one_vale_by_invoice);  /*12-Ago-2011 acm -*/
  err_set(TURKEY_NOT_APPLICABLE, &err_turkey_not_applicable);  /*12-Ago-2011 acm -*/
  err_set(ARTICLE_MUSTBE_TURKEY, &err_article_mustbe_turkey);  /*12-Ago-2011 acm -*/

/*25-Set-2012 acm - {*/
  err_set(QB_CODE_QUEUE_BUSTING_INVALID,        &err_code_queue_busting_invalid);  
  err_set(QB_CONECTION_QUEUE_BUSTING_ERROR,     &err_conection_queue_busting_invalid);  
  err_set(QB_ARTICLE_QUEUE_BUSTING_INVALID,     &err_article_queue_busting_invalid);  

  err_set(QB_FORMAT_QUEUE_BUSTING_INVALID,      &err_format_queue_busting_invalid);  
  err_set(QB_ROWS_BIG_QUEUE_BUSTING_INVALID,    &err_rows_big_queue_busting_invalid); 
  err_set(QB_ROWS_QUEUE_BUSTING_INVALID,        &err_rows_queue_busting_invalid);  
  err_set(QB_SOCKET_CREATED_QUEUE_BUSTING_INVALID,      &err_socket_created_queue_busting_invalid);  
  err_set(QB_HOSTNAME_QUEUE_BUSTING_INVALID,            &err_hostname_queue_busting_invalid);  

  err_set(QB_QUEUE_BUSTING_NOTFOUND,            &err_queue_busting_notfound);
  err_set(QB_QUEUE_BUSTING_SQL_INVALID,         &err_queue_busting_sql_invalid);  



 /*25-Set-2012 acm - }*/

  err_set(FIELD_NOT_HORECA   ,  &err_field_not_horeca  );// v3.4.8 acm -
  err_set(ARTICLE_NOT_HORECA ,  &err_article_not_horeca);// v3.4.8 acm -
  err_set(VIGENCIA_NOT_HORECA , &err_vigencia_not_horeca);// v3.4.8 acm -
  err_set(INVALID_VALEPAVO    ,  &err_invalid_valepavo  );// v3.4.8 acm -

  err_set(VALEPAVO_CONECTION_ERROR  ,  &err_valepavo_conection  );// v3.4.8 acm -
  err_set(VALEPAVO_USED             ,  &err_valepavo_used       );// v3.4.8 acm -

  err_set(VALEPAVO_NOTFOUND  ,  &err_valepavo_unknown  );// v3.4.8 acm -
  err_set(VALEPAVO_UNKNOWN   ,  &err_valepavo_notfound       );// v3.4.8 acm -

  err_set(VALEPAVO_BD_ERR    ,  &err_valepavo_bd     );// v3.4.8 acm -

  err_set(DOCUMENT_DOC_ERR,   &err_document_doc_err);


  err_set(ILLEGAL_CLIENAME_PERCEPTION, &err_illegal_cliename_perception);





  err_set(ANTICIPO_CONECTION_ERROR  ,  &err_anticipo_conection  );// v3.4.8 acm -
  err_set(ANTICIPO_USED             ,  &err_anticipo_used       );// v3.4.8 acm -
  err_set(ANTICIPO_NOTFOUND         ,  &err_anticipo_unknown  );// v3.4.8 acm -
  err_set(ANTICIPO_UNKNOWN          ,  &err_anticipo_notfound       );// v3.4.8 acm -
  err_set(ANTICIPO_BD_ERR           ,  &err_anticipo_bd     );// v3.4.8 acm -


  err_set(ANTICIPO_IGV_ERR         ,  &err_anticipo_igv         );// v3.4.8 acm -
  err_set(ANTICIPO_TOTAL_ERR       ,  &err_anticipo_total       );// v3.4.8 acm -
  err_set(ANTICIPO_PERCEPCION_ERR  ,  &err_anticipo_percepcion  );// v3.4.8 acm -

  err_set(ANTICIPO_TOTAL2_ERR       ,  &err_anticipo_total2     );// v3.4.8 acm -

  err_set(ANTICIPO_DOC_TYPE_FAC_ERR ,  &err_anticipo_doc_fac_type);// v3.4.8 acm -
  err_set(ANTICIPO_DOC_TYPE_BOL_ERR ,  &err_anticipo_doc_bol_type);// v3.4.8 acm -
  err_set(ANTICIPO_FIS_NO_ERR ,        &err_anticipo_fis_no);// v3.4.8 acm -

  err_set(EPOS_CONNECT_ERROR ,        &err_epos_connect); //epos

  pos_reinit_errors();
} /* pos_init_errors */


void pos_reinit_errors(void)
{
  err_set(MEM_UNAVAILABLE, &err_memory);
} /* pos_reinit_errors */

