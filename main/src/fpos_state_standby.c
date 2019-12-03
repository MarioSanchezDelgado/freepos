/*
 *     Module Name       : FPOS_STATE_STANDBY.C
 *     Type              : States PosStandBy, EdpMode
 *     Author/Location   : Getronics, Distribution & Retail, Nieuwegein
 *     Copyright Makro International AG
 *               Aspermonstrasse 24
 *               7006 CHUR
 *               Switzerland
 * --------------------------------------------------------------------------
 *                            MODIFICATION HISTORY
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 13-Dec-1999 Initial Release WinPOS                                  J.D.M.
 * --------------------------------------------------------------------------
 */

#include <stdio.h>
#include "fpos_screen.h"
#include "fpos_state.h"
#include "fpos_states.h"
#include "fpos_string_tools.h"
#include "fpos_input.h"
#include "fpos_input_mgr.h"
#include "fpos_default.h"

#include "fpos_func.h"
#include "MapKeys.h"

#include "fpos_errors.h"

short illegal_fn_key(char *data, short key)
{
  err_invoke(ILLEGAL_FUNCTION_KEY_ERROR);

  return(UNKNOWN_KEY);
} /* illegal_fn_key */

short proc_init_environment_records(char *data, short key)
{
  return(key);
} /* proc_init_environment_records */


void PosStandBy_VW()
{
  standby_window();  
}

extern INPUT_CONTROLLER DstK1nLlnsU =
{
  (INPUT_DISPLAY *)&dsp_keypos,
  KEYLOCK_S_MASK | KEYLOCK_N_MASK | KEYLOCK_X_MASK,
  1,
  1,
  (VERIFY_KEY *)&numeric
};

static void PosStandBy_UVW(void)
{
  /* Nothing to do yet */
  return;
} /* PosStandBy_UVW */

static PROCESS_ELEMENT PosStandBy_PROC[] =
{
  KEYLOCK_NORMAL, proc_init_environment_records,
  UNKNOWN_KEY,    0,
};

static CONTROL_ELEMENT PosStandBy_CTL[] =
{
  UNKNOWN_KEY,        &PosStandBy_ST
};

static VERIFY_ELEMENT PosStandBy_VFY[] =
{                              
  UNKNOWN_KEY,        illegal_fn_key
};

extern STATE_OBJ PosStandBy_ST =
{                         
  ST_POS_STAND_BY,
  PosStandBy_VW,
  no_DFLT_2,
  &DstK1nLlnsU,
  PosStandBy_VFY,
  PosStandBy_UVW,
  PosStandBy_PROC,
  PosStandBy_CTL
};
