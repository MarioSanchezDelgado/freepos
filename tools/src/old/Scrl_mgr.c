/*
 *     Module Name       : SCRL_MGR.C
 *
 *     Type              : Scroll manager
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
 */
 
#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include "mem_mgr.h"
#include "scrn_mgr.h"
#include "scrl_mgr.h"

static short index = -1;
static short scroll_init_ok = FALSE;
static SCROLL_WINDOW scroll_wnd[ NR_OF_SCROLL_WINDOWS ];

#define SCRL_MAX_FUNCTIONS  10
static void (*scrl_functions[SCRL_MAX_FUNCTIONS])();

short find_scroll_window(unsigned short);
ELEM *find_nth_scroll_line(short);

static void disp_scroll_lines(unsigned short, short, short, ELEM *);


/*******************************************************************************
Function    : scrl_init
Description : Initialises the scroll list
INPUT       : None
OUTPUT      : None
*******************************************************************************/

void scrl_init(void)
{
  short i;
  for (i=0; i < SCRL_MAX_FUNCTIONS; i++) {
    scrl_functions[i] = (void *)NULL;
  }
} /* scrl_init */


/*******************************************************************************
Function    : scrl_add_fn
Description : Adds a scroll function pointer plus index to the scroll function
        list
INPUT       : - addres of a scroll function
        - index in scroll function list
OUTPUT      : (short) SUCCEED if ok, FAIL if index out of bounds or element
        already in use
*******************************************************************************/

short scrl_add_fn(void (*fn)(), short index)
{
  if (index >= SCRL_MAX_FUNCTIONS || scrl_functions[index] != (void *)NULL) {
    return(FAIL);
  }
  scrl_functions[index] = fn;

  return(SUCCEED);
} /* scrl_add_fn */


/*******************************************************************************
Function    : scrl_init_windows
Description : Initialize scroll window structure
INPUT       : None
OUTPUT      : - short  SUCCEED
*******************************************************************************/

short scrl_init_windows(void)
{ 
  short i;

  if ( scroll_init_ok == FALSE ) {                     /* not yet initialized */
    for ( i = 0; i < NR_OF_SCROLL_WINDOWS ; i++ ) { 
      /* initialize scroll-window array */
      scroll_wnd[i].window_nr = NO_WINDOW;
      scroll_wnd[i].top = -1;
      scroll_wnd[i].bottom = -1;
      scroll_wnd[i].line = NULL;                   /* pointer to scroll lines */
      scroll_wnd[i].header = NULL;                 /* pointer to header lines */
    }
    scroll_init_ok = TRUE;                      
  } /* if */

  return ( SUCCEED );
} /* scroll_init_windows */


/*******************************************************************************
Function    : scrl_define_window
Description : Initialize scroll window structure
INPUT       : - unsigned short window_nr    window number
OUTPUT      : - short   MEM_UNAVAILABLE, could not be enough memory allocated
      SUCCEED, otherwise
*******************************************************************************/

short scrl_define_window(unsigned short window_nr)
{
  short status, i;
 
  status = SUCCEED; 
  if ( scroll_init_ok ) {               
    /* scroll-window array is initialized so now look for a place to put the new
     defined scroll window */
    for (i = 0; i<NR_OF_SCROLL_WINDOWS && scroll_wnd[i].window_nr!=NO_WINDOW;i++) {
      ;
    }
    if ( i != NR_OF_SCROLL_WINDOWS ) {       /* is scroll-window array full ? */
      status = SUCCEED; 
      index = i;
      scroll_wnd[index].window_nr = window_nr;
      /* top will be nth added scroll line which is first displayed in window */
      scroll_wnd[index].top = 0;      
    /* bottom will be mth added scroll line which is last displayed in window */
      scroll_wnd[index].bottom = 0;
      scroll_wnd[index].line = (SCRL_SET *) mem_allocate( sizeof(SCRL_SET) );
      if ( scroll_wnd[index].line == NULL ) {
        status = MEM_UNAVAILABLE;
      }
      else {          /* initialize structure for linked list of scroll lines */
        scroll_wnd[index].line->nr_element = 0;
        scroll_wnd[index].line->first = NULL;
        scroll_wnd[index].line->last = NULL;
      }
      scroll_wnd[index].header = (SCRL_SET *) mem_allocate( sizeof(SCRL_SET) );
      if ( scroll_wnd[index].header == NULL ) {
        status = MEM_UNAVAILABLE;
      }
      else {          /* initialize structure for linked list of header lines */
        scroll_wnd[index].header->nr_element = 0;
        scroll_wnd[index].header->first = NULL;
        scroll_wnd[index].header->last = NULL;
      }
    }
  }

  return ( status );

} /* scrl_define_window */


/*******************************************************************************
Function    : scrl_close_windows
Description : Free scroll window structures
INPUT       : None
OUTPUT      : - short  SUCCEED
*******************************************************************************/

short scrl_close_windows(void)
{
  short i;
  short status = SUCCEED;

  if ( scroll_init_ok ) {
    for ( i = 0; i < NR_OF_SCROLL_WINDOWS ; i++) {   /* close scroll windows  */
      if ( scroll_wnd[i].window_nr != NO_WINDOW ) { 
        /* if 'NO_WINDOW' then there is no memory allocated */
        status = scrl_delete(scroll_wnd[i].window_nr); /* delete linked lists */
        if ( status == SUCCEED ) {
          if ( scroll_wnd[i].line != NULL ) {
            mem_free(scroll_wnd[i].line);      /* free scroll lines structure */
          }
          if ( scroll_wnd[i].header != NULL ) {
            mem_free(scroll_wnd[i].header);    /* free header lines structure */
          }
        }
      }
      scroll_wnd[i].window_nr = NO_WINDOW;
    }
  }

  return ( SUCCEED );
} /* scrl_close_windows */


/*******************************************************************************
Function    : scrl_add_line
Description : Add a line to the scroll window
INPUT       : - unsigned short window_nr    window number
        - short function_nr         display function number
        - short elemid              identification of a scroll line
OUTPUT      : - short   MEM_UNAVAILABLE, could not be enough memory allocated
      SUCCEED, otherwise
*******************************************************************************/

short scrl_add_line(unsigned short window_nr,short function_nr,short elemid)
{
  short status = FAIL;
  ELEM *new, *scroll_line;

  if ( BNDRY_CHK_OK && scroll_init_ok ) {            /* check if window is ok */
    status = find_scroll_window( window_nr );                  /* find window */
    if ( status == SUCCEED ) {
      scroll_line = scroll_wnd[index].line->first;
      while ( (scroll_line != NULL) && status == SUCCEED ) {
        if (scroll_line->function_nr == function_nr && 
            scroll_line->elemid == elemid) {
          status = -1;                      /* check if line is already added */
        }
        scroll_line = scroll_line->next;
      }
      if ( status == SUCCEED ) {
        new = (ELEM *) mem_allocate( sizeof(ELEM) );          /* add new line */
        if ( new != NULL ) {
          new->elemid = elemid;
          new->function_nr = function_nr;
          new->next = NULL;
          if ( scroll_wnd[index].line->first == NULL ) {
            scroll_wnd[index].line->first = new;            /* set first line */
          }
          else {
            scroll_wnd[index].line->last->next = new;
          }
          scroll_wnd[index].line->last = new;                /* set last line */
          scroll_wnd[index].line->nr_element++;                   /* one more */
          
          if ( (scroll_wnd[index].bottom - scroll_wnd[index].top + 1) + 
                scroll_wnd[index].header->nr_element >=
                screen_wnd[window_nr].num_of_rows )  {
            /* window is full with scroll lines */
            if (scroll_wnd[index].bottom == scroll_wnd[index].line->nr_element - 1) {
            /* the last displayed line in the window was the last element of the list */
              scrl_line_down(window_nr);
            }
            else {
            /* the displayed lines in the window are not the last added scroll
              lines so all the lines in the window are redrawn */
              scroll_wnd[index].bottom = scroll_wnd[index].line->nr_element;
              scroll_wnd[index].top = scroll_wnd[index].line->nr_element - 
                screen_wnd[window_nr].num_of_rows + 
                scroll_wnd[index].header->nr_element + 1;
              scrl_redraw_lines(window_nr);
            }
          }
          else {
            if ( scroll_wnd[index].top == 0 ) {           /* first added line */
              scroll_wnd[index].top++;
            }
            scroll_wnd[index].bottom++;
            /* just add line in window which is not yet full */
            scrl_update_line(window_nr, function_nr, elemid);
          }
        }
        else {
          status = MEM_UNAVAILABLE;
        }
      }
    }
  }

  return ( status );

} /* scrl_add_line */

/*******************************************************************************
Function    : scrl_append_line
Description : Add a line to the scroll window without redrawing the sreen
INPUT       : - unsigned short window_nr    window number
        - short function_nr         display function number
        - short elemid              identification of a scroll line
OUTPUT      : - short   MEM_UNAVAILABLE, could not be enough memory allocated
      SUCCEED, otherwise
*******************************************************************************/

short scrl_append_line(unsigned short window_nr,short function_nr,short elemid)
{
  short status = FAIL;
  ELEM *new_elem;

  if ( BNDRY_CHK_OK && scroll_init_ok ) {            /* check if window is ok */
    status = find_scroll_window( window_nr );                  /* find window */
    if ( status == SUCCEED ) {
      new_elem = (ELEM *) mem_allocate( sizeof(ELEM) );          /* add new line */
      if ( new_elem != NULL ) {
        new_elem->elemid = elemid;
        new_elem->function_nr = function_nr;
        new_elem->next = NULL;
        if ( scroll_wnd[index].line->first == NULL ) {
          scroll_wnd[index].line->first = new_elem;            /* set first line */
        }
        else {
          scroll_wnd[index].line->last->next = new_elem;
        }
        scroll_wnd[index].line->last = new_elem;                /* set last line */
        scroll_wnd[index].line->nr_element++;                   /* one more */

        if ( (scroll_wnd[index].bottom - scroll_wnd[index].top + 1) + 
              scroll_wnd[index].header->nr_element >=
              screen_wnd[window_nr].num_of_rows )  {
          ;
        }
        else {
          if ( scroll_wnd[index].top == 0 ) {           /* first added line */
            scroll_wnd[index].top++;
          }
          scroll_wnd[index].bottom++;
          /* just add line in window which is not yet full */
        }
      }
      else {
        status = MEM_UNAVAILABLE;
      }
    }
  }

  return ( status );

} /* scrl_append_line */

/*******************************************************************************
Function    : scrl_insert_line
*******************************************************************************/

short scrl_insert_line(unsigned short window_nr, short ref_fn,
                       short ref_id, short ins_fn, short ins_id)
{
  ELEM  *new, *scrl_line;
  short  status;

  if (!scroll_init_ok) {
    return(FAIL);
  }
  status = find_scroll_window(window_nr);
  if (status != SUCCEED) {
    return(status);
  }
  scrl_line = scroll_wnd[index].line->first;
  while (scrl_line != NULL) {
    if (scrl_line->function_nr == ref_fn && scrl_line->elemid == ref_id) {
      break;
    }
    scrl_line = scrl_line->next;
  }
  if (scrl_line == NULL) {
    return(FAIL);
  }
  new = (ELEM *)mem_allocate(sizeof(ELEM));           /* add new line */
  if (new == NULL) {
    return(MEM_UNAVAILABLE);
  }
  new->elemid = ins_id;
  new->function_nr = ins_fn;
  new->next = scrl_line->next;           /* could be NULL, no problem */
  scrl_line->next = new;                 /* now it's inserted         */
  if (new->next == NULL) {               /* the last, NEVER the first */
    scroll_wnd[index].line->last = new;              /* set last line */
  }
  scroll_wnd[index].line->nr_element++;                   /* one more */
  if ((scroll_wnd[index].bottom - scroll_wnd[index].top + 1) +
       scroll_wnd[index].header->nr_element >=
       screen_wnd[window_nr].num_of_rows) {            /* full screen */
    scrl_goto_line(window_nr, ins_fn, ins_id);
  }
  else {
    scroll_wnd[index].bottom++;
  }
  scrl_redraw_lines(window_nr);
  return(SUCCEED);
} /* scrl_insert_line */


/*******************************************************************************
Function    : scrl_delete_line
Description : Delete a line from the scroll window
INPUT       : - unsigned short window_nr  window number
        - short function_nr         display function number
        - short elemid              Scroll line to be deleted
OUTPUT      : - short  Indication scroll window not known
           SUCCEED, otherwise
*******************************************************************************/

short scrl_delete_line(unsigned short window_nr,short function_nr,short elemid)
{
  short status, i;
  ELEM  *delete_line, *prev_line;
  
  status = FAIL; 
  if ( BNDRY_CHK_OK && scroll_init_ok ) {
    status = find_scroll_window( window_nr );   
    if ( status == SUCCEED ) {
      i = 1;
      delete_line = scroll_wnd[index].line->first;
      prev_line = NULL;
      while ( (delete_line != NULL) && 
        (delete_line->function_nr != function_nr || 
         delete_line->elemid != elemid) ) {    /* search line to delete */
        prev_line = delete_line;
        delete_line = delete_line->next;
        i++;
      }
   
      if ( delete_line != NULL ) {
        if ( delete_line == scroll_wnd[index].line->last ) {
          scroll_wnd[index].line->last = prev_line; /* prev line will be last */
        }
        if ( delete_line == scroll_wnd[index].line->first ) {
          scroll_wnd[index].line->first = delete_line->next;
        }
        if ( prev_line != NULL ) {           /* delete the line from the list */
          prev_line->next = delete_line->next;
        }
        mem_free ( delete_line );       /* free memory allocated for the line */
        scroll_wnd[index].line->nr_element--;

        if ( scroll_wnd[index].line->nr_element == 0 ) { /* one and only line */
          scroll_wnd[index].top = 0;
          scroll_wnd[index].bottom = 0;
          /* clear top, bottom indication and the displayed line in the window */
          scrn_clear_rest_of_line(window_nr, scroll_wnd[index].header->nr_element,0);
        }
        else {
        /* depending on where the deleted line was placed in the list the top
          and bottom indication need to be adjusted */
          if ( i < scroll_wnd[index].top ||    /* line before displayed lines */
            scroll_wnd[index].bottom > scroll_wnd[index].line->nr_element) {
            /* all lines are displayed in the window */
            scroll_wnd[index].top--;      /* adjust top and bottom indication */
            scroll_wnd[index].bottom--;
            if ( scroll_wnd[index].top == 0 ) {
              scroll_wnd[index].top = 1;/* least value top if there are lines */
              scrn_clear_rest_of_line(window_nr,
                (short)(scroll_wnd[index].header->nr_element + scroll_wnd[index].bottom), 0);
              /* clear deleted line in window */
            }
            /* deleted line is displayed in window */
            if ( !(i < scroll_wnd[index].top) ) {
              scrl_redraw_lines(window_nr);
            }
          }
        }
      }
    }
  }

  return ( status ); 
}  /* scrl_delete_line */


/*******************************************************************************
Function    : scrl_page_up
Description : Scroll up one page or as much as possible of one page
INPUT       : - unsigned short window_nr    window number
OUTPUT      : - short   Indication scroll window not known
      SUCCEED, otherwise
*******************************************************************************/

short scrl_page_up(unsigned short window_nr)
{
  short status = FAIL;
  
  if ( scroll_init_ok ) {
    status = find_scroll_window( window_nr );   
    if ( status == SUCCEED ) {
      if ( scroll_wnd[index].top > 1 ) {          /* now we can go scrolling */
        /* adjust top and bottom indications */
        scroll_wnd[index].top -= (screen_wnd[window_nr].num_of_rows -
          scroll_wnd[index].header->nr_element);
        scroll_wnd[index].bottom -= (screen_wnd[window_nr].num_of_rows -
          scroll_wnd[index].header->nr_element);
        if ( scroll_wnd[index].top < 1 ) {
          /* there was less than one page to scroll, so adjust top and bottom */
          scroll_wnd[index].bottom += (0 - scroll_wnd[index].top + 1);
          scroll_wnd[index].top += (0 - scroll_wnd[index].top + 1);
        }
        /* redraw the lines in the window */
        scrl_redraw_lines( window_nr );
      }
    }
  }

  return ( status );
}  /* scrl_page_up */


/*******************************************************************************
Function    : scrl_page_down
Description : Scroll down one page or as much as possible of one page
INPUT       : - unsigned short window_nr    window number
OUTPUT      : - short   Indication scroll window not known
      SUCCEED, otherwise
*******************************************************************************/

short scrl_page_down(unsigned short window_nr)
{
  short status = FAIL; 

  if ( scroll_init_ok ) {
    status = find_scroll_window( window_nr );   
    if ( status == SUCCEED ) {
      if ( scroll_wnd[index].bottom <
        scroll_wnd[index].line->nr_element) {  /* now we can go scrolling */
        /* adjust top and bottom indications */
        scroll_wnd[index].top += (screen_wnd[window_nr].num_of_rows -
          scroll_wnd[index].header->nr_element);
        scroll_wnd[index].bottom += (screen_wnd[window_nr].num_of_rows -
          scroll_wnd[index].header->nr_element);
        if ( scroll_wnd[index].bottom >
          scroll_wnd[index].line->nr_element ) {
          /* there was less than one page to scroll, so adjust top and bottom */
          scroll_wnd[index].top -= (scroll_wnd[index].bottom -
            scroll_wnd[index].line->nr_element);
          scroll_wnd[index].bottom -= (scroll_wnd[index].bottom -
            scroll_wnd[index].line->nr_element);
        }
        
        /* redraw the lines in the window */
        scrl_redraw_lines(window_nr);
      }
    }
  }

  return ( status );
} /* scrl_page_down */


/*******************************************************************************
Function    : scrl_line_down
Description : Scroll down one line (at bottom )
INPUT       : - unsigned short window_nr    window number
OUTPUT      : - short   Indication scroll window not known
      SUCCEED, otherwise
*******************************************************************************/

short scrl_line_down(unsigned short window_nr)
{
  short status = FAIL;
  unsigned short current_scroll_wnd;
  ELEM *scroll_line;

  if ( scroll_init_ok ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      /* bottom should point to a line before the end of the list */
      if ( scroll_wnd[index].bottom < scroll_wnd[index].line->nr_element ) {
        current_scroll_wnd = scrn_get_current_window();
        if ( window_nr != current_scroll_wnd ) {
          scrn_select_window( window_nr );
        }
        scroll_wnd[index].top += 1;       /* adjust top and bottom indication */
        scroll_wnd[index].bottom += 1;
        /* locate the position of the bottom-line */
        scroll_line = find_nth_scroll_line(scroll_wnd[index].bottom);
        
        scrn_line_down(window_nr, scroll_wnd[index].header->nr_element);
        
        /* display new bottom line on the screen */
        scrl_update_line(window_nr,scroll_line->function_nr,scroll_line->elemid);
        scrn_select_window( current_scroll_wnd ); /* reset to original window */
      }
    }
  }

  return ( status );
} /* scrl_line_down */


/*******************************************************************************
Function    : scrl_line_up
Description : Scroll up one line (at top)
INPUT       : - unsigned short window_nr    window number
OUTPUT      : - short   Indication scroll window not known
      SUCCEED, otherwise
*******************************************************************************/

short scrl_line_up(unsigned short window_nr)
{
  short status = FAIL;
  unsigned short current_scroll_wnd;
  ELEM *scroll_line;

  if ( scroll_init_ok ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      /* top should point to a line after the start of the list */
      if (scroll_wnd[index].top > 1 ) {
        current_scroll_wnd = scrn_get_current_window();
        if ( window_nr != current_scroll_wnd ) {
          scrn_select_window( window_nr );
        }
        scroll_wnd[index].top -= 1;       /* adjust top and bottom indication */
        scroll_wnd[index].bottom -= 1;
        /* locate the position of the top-line */
        scroll_line = find_nth_scroll_line(scroll_wnd[index].top);
        
        scrn_line_up(window_nr, scroll_wnd[index].header->nr_element);
        
        /* display new top line on the screen */
        scrl_update_line(window_nr,scroll_line->function_nr,scroll_line->elemid);
        scrn_select_window( current_scroll_wnd ); /* reset window to original */
      } /* if */
    } /* if */
  } /* if */

  return ( status );
} /* scrl_line_up */


/*******************************************************************************
Function    : scrl_redraw_lines
Description : Redraw the scroll lines
INPUT       : - unsigned short window_nr    window number
OUTPUT      : - short   Indication scroll window not known
      SUCCEED, otherwise
*******************************************************************************/

short scrl_redraw_lines(unsigned short window_nr)
{
  short status = FAIL;
  ELEM *scroll_line;

  if ( scroll_init_ok ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      if ( scroll_wnd[index].top > 0 ) {
        /* locate top-line from which the redisplay starts */
        scroll_line = find_nth_scroll_line(scroll_wnd[index].top);
        if ( scroll_line != NULL ) {      /* start the redisplay of the lines */
          disp_scroll_lines(window_nr, scroll_wnd[index].header->nr_element,
          (short)(scroll_wnd[index].bottom - scroll_wnd[index].top + 1), scroll_line);
        }
      }
    }
  }

  return ( status );
} /* scrl_redraw_lines */


/*******************************************************************************
Function    : scrl_redraw_header
Description : Redraw the header of the scroll window
INPUT       : - unsigned short window_nr    window number
OUTPUT      : - short   Indication scroll window not known
      SUCCEED, otherwise
*******************************************************************************/

short scrl_redraw_header(unsigned short window_nr)
{
  short status;

  status = find_scroll_window( window_nr );   
  if ( status == SUCCEED ) {
    /* start redisplay of header lines */
    disp_scroll_lines(window_nr, 0, scroll_wnd[index].header->nr_element,
      scroll_wnd[index].header->first);
  }

  return ( status );
}  /* scrl_redraw_header */


/*******************************************************************************
Function    : scrl_update_line
Description : Update a line from the scroll window
INPUT       : - unsigned short window_nr    window number
        - short function_nr         display function number
        - short elemid              identification of a scroll line
OUTPUT      : - short   Indication if window is illegal
      SCRL_UNKNOWN_LINE line not found
      SUCCEED, otherwise
*******************************************************************************/

short scrl_update_line(unsigned short window_nr,short function_nr,short elemid)
{
  short status, i;
  ELEM *scroll_line;

  status = SUCCEED;
  if ( BNDRY_CHK_OK ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      /* locate the line to update */
      scroll_line = scroll_wnd[index].line->first;
      for ( i = 1 ; i <= scroll_wnd[index].line->nr_element &&
        scroll_line != NULL && !(scroll_line->elemid == elemid &&
        scroll_line->function_nr == function_nr); i++ ) {
        scroll_line = scroll_line->next;
      }
      if ( scroll_line != NULL ) {
        if (i >= scroll_wnd[index].top && i <= scroll_wnd[index].bottom) {
          /* if the updated line is displayed in the window, redraw it */
          disp_scroll_lines(window_nr, (short)(i - scroll_wnd[index].top +
            scroll_wnd[index].header->nr_element) , 1, scroll_line);
        }
      }
      else {
        status = SCRL_UNKNOWN_LINE;
      }
    }
  }

  return ( status ); 
} /* scrl_update_line */


/*******************************************************************************
Function    : scrl_add_header_line
Description : Add a header line from the scroll window
INPUT       : - unsigned short window_nr    window number
        - short function_nr         display function number
        - short elemid              identification of a scroll line
OUTPUT      : - short   Indication scroll window not known 
           or too many header lines
      MEM_UNAVAILABLE, could not be enough memory allocated
      SUCCEED, otherwise
*******************************************************************************/

short scrl_add_header_line(unsigned short window_nr,short function_nr,
         short elemid)
{
  short status;
  ELEM *new;

  status = FAIL;
  if ( BNDRY_CHK_OK && scroll_init_ok ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      /* do not to much add header lines */
      if ( screen_wnd[window_nr].num_of_rows <=
        scroll_wnd[index].header->nr_element) {
        status = -1;
      }
      else {
        /* create a new header line */
        new = (ELEM *) mem_allocate( sizeof(ELEM) );
        if ( new != NULL ) {
          new->elemid = elemid;
          new->function_nr = function_nr;
          new->next = NULL;
          if ( scroll_wnd[index].header->first == NULL ) {
            scroll_wnd[index].header->first = new;   /* header line is first */
          }
          else {
            scroll_wnd[index].header->last->next = new;
          }
          scroll_wnd[index].header->last = new; /* add new header line at end */
          scroll_wnd[index].header->nr_element++;
          
          /* if the new header line has to be added to a full window the top 
            indication has to be adjusted */
          if ( ( scroll_wnd[index].header->nr_element +
            scroll_wnd[index].line->nr_element ) >=
            screen_wnd[window_nr].num_of_rows ) {
            scroll_wnd[index].top++;
          }

          /* redraw the header and scroll lines */
          scrl_redraw_header(window_nr);
          scrl_redraw_lines(window_nr);
        }
        else {
          status = MEM_UNAVAILABLE;
        }
      }
    }
  }
  return ( status );

} /* scrl_add_header_line */


/*******************************************************************************
Function    : scrl_update_header_line
Description : Update a header line from the scroll window
INPUT       : - unsigned short window_nr    window number
        - short function_nr         display function number
        - short elemid              identification of a scroll line
OUTPUT      : - short   Indication scroll window not known
      SCRL_UNKNOWN_LINE header not found
      MEM_UNAVAILABLE, could not be enough memory allocated
      SUCCEED, otherwise
*******************************************************************************/

short scrl_update_header_line(unsigned short window_nr, short function_nr, 
            short elemid)
{
  short status, i;
  ELEM *scroll_header;
   
  status = SUCCEED;
  if ( BNDRY_CHK_OK && scroll_init_ok ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      /* locate the header line to be updated */
      if (scroll_wnd[index].header->first == NULL) {
        status = SCRL_UNKNOWN_LINE;
      }
      else {
        scroll_header = scroll_wnd[index].header->first;
        for ( i = 0 ; i <= scroll_wnd[index].header->nr_element &&
          !(scroll_header->elemid == elemid &&
          scroll_header->function_nr == function_nr); i++ ) {
          scroll_header = scroll_header->next;
        }
        if ( scroll_header == NULL ) {
          status = SCRL_UNKNOWN_LINE;
        }
        else {                             /* display the updated header line */
          disp_scroll_lines(window_nr, i, 1, scroll_header);
        }
      }
    }
  }
  
  return ( status );
} /* scrl_update_header_line */


/*******************************************************************************
Function    : scrl_delete
Description : Delete scroll window
INPUT       : - unsigned short window_nr    window number
OUTPUT      : - short   Indication scroll window not known
      SUCCEED, otherwise
*******************************************************************************/

short scrl_delete(unsigned short window_nr)
{
  short status = SUCCEED;
  ELEM *scroll_elem, *delete_elem;
     
  if ( scroll_init_ok ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      /* go through the list of scroll lines and delete them */
      scroll_elem = scroll_wnd[index].line->first;
      while ( scroll_elem != NULL ) {
        delete_elem = scroll_elem;
        scroll_elem = scroll_elem->next;
        mem_free(delete_elem);
      }
      scroll_wnd[index].line->first = NULL;
      scroll_wnd[index].line->last = NULL;
      scroll_wnd[index].line->nr_element = 0;
      
      /* go through the list of header lines and delete them */
      scroll_elem = scroll_wnd[index].header->first;
      while ( scroll_elem != NULL ) {
        delete_elem = scroll_elem;
        scroll_elem = scroll_elem->next;
        mem_free(delete_elem);
      }
      scroll_wnd[index].header->first = NULL;
      scroll_wnd[index].header->last = NULL;
      scroll_wnd[index].header->nr_element = 0;
      
      scroll_wnd[index].top = 0;          /* adjust top and bottom indication */
      scroll_wnd[index].bottom = 0;
    }
  }

  return ( status );
} /* scrl_delete */


/*******************************************************************************
Function    : scrl_modify_id
*******************************************************************************/

short scrl_modify_id(short fn, short old_id, short new_id)
{
  ELEM *scrl_line;

  scrl_line = scroll_wnd[index].line->last;     /* try the last first */
  if (scrl_line == NULL) {                      /* cause this one is  */
    return(FAIL);                               /* most likely to be  */
  }
  /* changed            */
  if (scrl_line->function_nr == fn && scrl_line->elemid == old_id) {
    scrl_line->elemid = new_id;
    return(SUCCEED);
  }
  scrl_line = scroll_wnd[index].line->first;
  while (scrl_line != NULL) {
    if (scrl_line->function_nr == fn && scrl_line->elemid == old_id) {
      scrl_line->elemid = new_id;
      return(SUCCEED);
    }
    scrl_line = scrl_line->next;
  }

  return(FAIL);
} /* scrl_modify_id */


/*******************************************************************************
Function    : disp_scroll_lines
Description : Display lines of scroll window
INPUT       : - unsigned short window_nr     window number
        - short row                    row 
        - short display_rows           number of rows to display
        - ELEM scroll_element          scroll-line or -header element
OUTPUT      : None
*******************************************************************************/

static void disp_scroll_lines(unsigned short window_nr, short row,
            short display_rows, ELEM *scroll_element)
{
  short countrows,nrow,ncol;
  unsigned short current_scroll_wnd;

  current_scroll_wnd = scrn_get_current_window();
  if ( window_nr != current_scroll_wnd ) {
    scrn_select_window(window_nr);
  }

  countrows = 1;
  while ( countrows <= display_rows ) {
    scrn_set_csr(row,0);                   /* set cursor to row in the window */
    if ( scrl_functions[scroll_element->function_nr] == NULL ) {
      scrn_clear_rest_of_line(window_nr, row, 0);
    }
    else {
      /* execute the user defined function to display this line */
      (*scrl_functions[scroll_element->function_nr])(scroll_element->elemid); 
      scrn_get_csr(&nrow, &ncol);
      scrn_clear_rest_of_line(window_nr, nrow, ncol);  
    }
    countrows++;               
    row++;                                  
    scroll_element = scroll_element->next;      /* next line to be disdplayed */
  }

  if ( window_nr != current_scroll_wnd ) {
    scrn_select_window(current_scroll_wnd);
  }

} /* disp_scroll_lines */


/******************************************************************************
Function    : scrl_goto_line
Description : Goto the specified line in the list but do not display in window
INPUT       : - unsigned short window_nr     window number
        - short function_nr            function number
        - short elemid                 element identification
OUTPUT      : -short   SCRL_UNKNOWN_LINE  line could not be found
           SUCCEED            otherwise
******************************************************************************/

short scrl_goto_line(unsigned short window_nr, short function_nr, 
         short elemid)
{
  short status, i;
  ELEM *scroll_line;

  status = SUCCEED;
  if ( BNDRY_CHK_OK && scroll_init_ok ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      /* locate the line to go to */
      scroll_line = scroll_wnd[index].line->first;
      for ( i = 1 ; i <= scroll_wnd[index].line->nr_element &&
        !(scroll_line->elemid == elemid &&
        scroll_line->function_nr == function_nr); i++ ) {
        scroll_line = scroll_line->next;
      }
      if ( scroll_line == NULL ) {
        status = SCRL_UNKNOWN_LINE;
      }
      else {
        if ( i < scroll_wnd[index].top ) {
          /* if line is placed in list before top-line */
          scroll_wnd[index].bottom -= (scroll_wnd[index].top - i);
          scroll_wnd[index].top = i;
        }
        else if ( i > scroll_wnd[index].bottom ) {
          /* if line is placed in list after bottom-line */
          scroll_wnd[index].top += (i - scroll_wnd[index].bottom);
          scroll_wnd[index].bottom = i;
        }
      }
    }
  }

  return ( status );
} /* scrl_goto_line() */


/*******************************************************************************
Function    : scrl_num_rows
Description : Determine number of scrollable lines
INPUT       : - unsigned short window_nr      window number
OUTPUT      : - short   indication scroll window not found
      SUCCEED, otherwise
*******************************************************************************/

short scrl_get_numrows(unsigned short window_nr)
{
  short status = SUCCEED;

  if ( scroll_init_ok ) {
    status = find_scroll_window( window_nr );
    if ( status == SUCCEED ) {
      /* total number of rows in the window minus the header rows is returned */
      status = screen_wnd[window_nr].num_of_rows - 
      scroll_wnd[index].header->nr_element;
    }
  }

  return ( status );

} /* scrl_get_numrows() */


/*******************************************************************************
Function    : find_scroll_window
Description : Search for scroll window identified by window_nr
INPUT       : - unsigned short window_nr      window number
OUTPUT      : - short   Indication window could not be found
      SUCCEED, otherwise
*******************************************************************************/

short find_scroll_window(unsigned short window_nr)
{
  short status, i;

  status = FAIL; 
  index = -1;
  if ( window_nr != NO_WINDOW ) {
    for (i = 0; scroll_wnd[i].window_nr != window_nr &&
      i < NR_OF_SCROLL_WINDOWS ; i++ ) {
      ;
    }
    if ( scroll_wnd[i].window_nr == window_nr ) {
      status = SUCCEED;
      index = i;
    }
  }

  return ( status );
} /* find_scroll_window */


/*******************************************************************************
Function    : find_nth_scroll_line
Description : Search for nth scroll line in current scroll window 
INPUT       : unsigned short window_nr    window number
        short nth_element           element n to be searched
OUTPUT      : - ELEM * pointer to nth element or NULL
*******************************************************************************/

ELEM *find_nth_scroll_line(short nth_element)
{
  short i;
  ELEM *scroll_line;

  scroll_line = scroll_wnd[index].line->first;

  for ( i = 1 ; ( i < nth_element ) && ( scroll_line != NULL ) ; i++ ) {
    scroll_line = scroll_line->next;
  }
   
  return ( scroll_line );
} /* find_nth_scroll_line */
