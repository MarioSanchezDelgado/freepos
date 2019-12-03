/*
 *  Module Name       : FPOS_FUNC.C
 *  Type              : Utility Functions for the FreePOS Application
 *  Author/Location   : Mario Sanchez Delgado, Lima-Perú
 *  Copyright Daichin SoftTech International SA
 *            Lima
 *            Perú
 *
 * -----------------------------------------------------------------------
 *                            CHANGELOG
 * -----------------------------------------------------------------------
 * DATE        REASON                                             AUTHOR
 * -----------------------------------------------------------------------
 * 29-Oct-2019 Initial Release FreePOS                            MLSD 
 * ----------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <string.h>
#include <time.h>


#include "fpos_func.h"
#include "fpos_defs.h"


short err_invoke(short type)
{
  fprintf(stderr, "Error [%d]\n", type);

  return(OK);
}

short get_date_time(char * buffer, short size)
{
  time_t now = time(0);
  struct tm now_t;
  
  memset(&now_t, 0, sizeof(struct tm));

  now_t = *localtime(&now);
  
  strftime(buffer, size, "%Y-%m-d %I:%M:%S %p", &now_t);
  
  return(OK);
}
