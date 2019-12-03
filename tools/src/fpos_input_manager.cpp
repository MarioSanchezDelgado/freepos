/*
 *     Module Name       : FPOS_INPUT_MANAGER.C
 *     Type              : Main entry of the FreePOS Application
 *     Author/Location   : Mario Sanchez Delgado, Lima-Perú
 *     Copyright Daichin International SA
 *               Lima
 *               Perú
 *
 * --------------------------------------------------------------------------
 *                            CHANGELOG
 * --------------------------------------------------------------------------
 * DATE        REASON                                                  AUTHOR
 * --------------------------------------------------------------------------
 * 29-Oct-2019 Initial Release FreePOS                                 MLSD 
 * -------------------------------------------------------------------------- 
 */

#include <stdio.h>
#include <iostream>
#include <string.h>
using namespace std;

#include <unistd.h>

#include "fpos_defs.h"
#include "fpos_input_mgr.h"



short inp_get_data(INPUT_CONTROLLER *control, char *data)
{
  static unsigned short index;        /* Process the array of functions   */
  short  status;
//  short  ConflictDetected=FALSE;
  static char temp_data[BUFFER_LENGTH]={'\0'};

  index = 0;
  status = INP_NO_DATA;

//  if (lpScanner1 && (control->device_mask & OCIA1_MASK)) {
//    lpScanner1->SetDeviceEnabled(TRUE);
//  }
//  if (lpScanner2 && (control->device_mask & OCIA2_MASK)) {
//    lpScanner2->SetDeviceEnabled(TRUE);
//  }

//  if (*data) {
//    display_data(control->display,data);/* show format string and data   */
//  }

  do {
//    /* Give OS some time... */
//    give_time_to_OS(PM_REMOVE, 0);
//
//    status = array_of_func[index](data,control);/* Call the inp function */
//
//    if(IsScannerData(status)) {
//      Sleep(10);
//      /* In case there is a lagging time when ALL I/O is coming from OPOS      */
//      /* (See comments below!!!).                                              */
//      /* Larger sleep times will cause more I/O to be rejected, but reduces or */
//      /* eliminates the risk that I/O is interpreted in the wrong order.       */
//
//      /* Check if there is also keyboard data (we can use a destructive read). */
      *temp_data='\0';
//      ConflictDetected=IsKeyboardData(pickup_keyboard(temp_data,control));
//      /* Also if temp_data is filled we have a conflict */
//      if(*temp_data) {
//        ConflictDetected=TRUE;
//      }
//    }
//    else if(IsKeyboardData(status)) {
//
//      Sleep(40);
//      /* It appears that on a Celeron processor 400 MHz the OPOS I/O always       */
//      /* arrives around 13 milliseconds later than direct Windows I/O. This must  */
//      /* be compensated to determine if there is data from both I/O channels at   */
//      /* the same time. So we need a little sleep here. To be on the safe side we */
//      /* will sleep a little bit longer than 13 milliseconds. On faster           */
//      /* processors this lagging effect will only become less. If also the        */
//      /* keyboard data is retrieved through OPOS and not directly from Windows    */
//      /* there should be no lagging effect and the sleep should not be necessary. */
//      /* Larger sleep times will cause more I/O to be rejected, but reduces or    */
//      /* eliminates the risk that I/O is interpreted in the wrong order.          */
//      /* Elimination of this problem can only be done completely if the           */
//      /* application is going to be rewritten to an event driven application.     */
//
//      /* Check if there is also scanner1 data (we can use a destructive read). */
//      *temp_data=_T('\0');
//      ConflictDetected=IsScannerData(pickup_ocia1(temp_data,control));
//      /* Also if temp_data is filled we have a conflict */
//      if(*temp_data) {
//        ConflictDetected=TRUE;
//      }
//
//      /* Check if there is also scanner2 data (we can use a destructive read). */
//      if(ConflictDetected!=TRUE) {
//        *temp_data=_T('\0');
//        ConflictDetected=IsScannerData(pickup_ocia2(temp_data,control));
//        /* Also if temp_data is filled we have a conflict */
//        if(*temp_data) {
//          ConflictDetected=TRUE;
//        }
//      }
//    }
//
//    if(ConflictDetected) {
//      ConflictDetected = FALSE;
//      if(check_for_state_abort(data,control)==INP_STATE_ABORT_REQUEST) {
//        /* Leave immediatly! */
//        status = INP_STATE_ABORT_REQUEST;
//      }
//      else {
//        /* Invoke an error and remove all data and input. */
//        display_data(control->display, "");/* clear the data display line */
//        err_invoke(SCAN_KEYB_CONFLICT);
//        inp_abort_data();
//        *data=0;
//        if (lpScanner1 && (control->device_mask & OCIA1_MASK)) {
//          lpScanner1->SetDeviceEnabled(TRUE);
//        }
//        if (lpScanner2 && (control->device_mask & OCIA2_MASK)) {
//          lpScanner2->SetDeviceEnabled(TRUE);
//        }
//        status = INP_NO_DATA;
//      }
//    }
//
//    index ++;                        /* Prepare for next function        */
//    if (!array_of_func[index]) {
//      index = 0;
//    }
    cin >> temp_data;
    if(*temp_data){
      sprintf(data, "%s", temp_data);
      status = OK;
    }
    else
      status = INP_NO_DATA;

    sleep(40);

  } while (status == INP_NO_DATA);

  return status;                     /* Return function key pressed      */
} /* inp_get_data */

short r2l_display(TEMPLATE_DISPLAY1 *disp, char *data)
{
  char buffer[MAX_FORMAT_LENGTH+1];
  char *dpointer, *fpointer, *index;

  strncpy(buffer,disp->format,MAX_FORMAT_LENGTH);
  buffer[MAX_FORMAT_LENGTH] = '\0';

  if(strlen(buffer)) {   /* to prevent reading and writing beyond boundaries */
    fpointer = buffer + strlen(buffer);
    index = fpointer - 1;
    if(*index!='-') {
      index=(char *)0;                  /* No minus in format-string     */
    }
    else {
      *index='~';                         /* Clear minus, set it if in data */
    }

    dpointer = data + strlen(data);

    while (fpointer > buffer) { /* work right to left until the format is exhausted */
      fpointer--;
      if (dpointer > data) {
        dpointer--;
        if ((*dpointer=='-') && (index!=(char *)0)) {
            *index='-';                                        /* minus found */
            if (index != fpointer) {
              fpointer++;
            }
        }
        else {
          if (strchr(disp->cover,*fpointer)) {
            *fpointer = *(dpointer);
          }
          else {
            dpointer++;
          }

        }
      }
    }
    if(index != (char *)0) {
      if(*index=='~') {
        *index='+';
      }
    }
    //scrn_select_window(disp->window);
    //scrn_string_out(buffer,disp->row,disp->col);
  }
  else { /* just display the format string as is */
    //scrn_select_window(disp->window);
    //scrn_string_out(buffer,disp->row,disp->col);
  }

  return SUCCEED;
}  /* r2l_display */



short l2r_display(TEMPLATE_DISPLAY1 *disp, char *data)
{
  char buffer[MAX_FORMAT_LENGTH+1];
  char *fpointer;
  char *index;
  short len_buf,
        len_data = strlen(data);


  strncpy(buffer,disp->format,MAX_FORMAT_LENGTH);
  buffer[MAX_FORMAT_LENGTH] = '\0';
  len_buf = strlen(buffer);

  //scrn_select_window(disp->window);

  if(len_buf && len_data) { /* to prevent reading and writing beyond boundaries */
    fpointer = buffer;
    index = buffer + len_buf - 1;            /* Determine minus position      */
    if(*index!='-') {
      index=(char *)0;                  /* No minus in format-string     */
    }
    else {
      *index='~';                         /* Clear minus, set it if in data */
    }

    if (len_data > len_buf) {
      if (*data=='-' && index!=(char *)0) {
        *index='-';
      }
      data += len_data - len_buf;
    }

    while (*fpointer) {
      if (*data=='-' && index!=(char *)0) {
        *index='-';
        if (index == fpointer) {
          fpointer++;
        }
        data++;
      }
      else {
        if (*data) {
          if (strchr(disp->cover,*fpointer)) {
            *fpointer = *data++;
          }
        }
        else {
          *fpointer = '\0';
          break;
        }
        fpointer++;
      }
    }
    if(index != (char *)0) {
      if(*index=='~') {
        *index='+';
      }
    }

    //scrn_string_out(disp->format,disp->row,disp->col); /* this is needed to get the */
    //scrn_string_out(buffer,disp->row,disp->col);       /* caret on the correct pos  */
  }
  else { /* just display the format string as is */
    //scrn_string_out(buffer,disp->row,disp->col);
    //scrn_string_out(""),disp->row,disp->col; /* this is needed to get the */
                                                 /* caret on the correct pos  */
  }
  return SUCCEED;
}



short r2l_password(TEMPLATE_PASSWORD *disp, char *data)
{
  TEMPLATE_DISPLAY1 tmp_display;
  char buffer[MAX_FORMAT_LENGTH+1];
  short length;

  length = strlen(data);
  memset(buffer, disp->password_char, MAX_FORMAT_LENGTH*sizeof(char));
  buffer[MAX_FORMAT_LENGTH] = '\0';                 /* Make password string    */
  if (length < MAX_FORMAT_LENGTH) {
    buffer[length] = '\0';
  }
  memcpy(&tmp_display, disp, sizeof(TEMPLATE_DISPLAY1));
  r2l_display(&tmp_display, buffer);

  return(SUCCEED);
} /* r2l_password */




