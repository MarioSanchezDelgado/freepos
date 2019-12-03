/*
 *     Module Name       : BACK_TLS.C
 *
 *     Type              : Functions to maintain the invoice backlog file
 *                         on a check-out
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
 * 23-Jan-2001 Check backlog added. If record found in backlog which 
 *             contains wrong till_no, backlog is renamed in
 *             <backlog_name>.ALIEN.<date>.<time>. Start with new one.   M.W.
 * --------------------------------------------------------------------------
 * 20-Jan-2003 Added flush (commit) in writ_backlog()                    M.W.
 * --------------------------------------------------------------------------
 * 03-Feb-2003 If backlog is corrupt, till must be closed. File will be 
 *             renamed in <backlog_name>.BAD.<date>.<time>.              M.W.
 * --------------------------------------------------------------------------
 * 01-May-2000 Changed function switch_backlog() to make it possible   JHEE
 *             to store more than one backlog.sav file. Same functionality
 *             as Tsjechie
 * --------------------------------------------------------------------------
 * 12-Oct-2001 If BACKLOG_SAVE_DAYS is 0 the backlog in switch_backlog() 
 *             is copied to BACKLOG.SAV (Standard functionality).      J.D.M.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#include "Template.h" /* Every source should use this one! */

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
                                     /* POS.lib include files */
#include "pos_recs.h"
#include "back_tls.h"
#include "erlg_tls.h"
#include "date_tls.h"
#include "registry.h"

#define  IBLH_SIZE   sizeof(struct IBL_HEADER)
#define  IBLR_SIZE   sizeof(struct IBL_RECORD)
#define  IBLR_PRFX   (2 * sizeof(int))  /* offset of 'data' member in IBL_RECORD */
#define  IBLT_SIZE   sizeof(struct IBL_TAG)

static struct IBL_HEADER ibl_header;
static struct IBL_PREFIX ibl_prefix;
static struct IBL_TAG    ibl_tag;
static struct IBL_RECORD ibl_record;

static int  ibl_handle;
static _TCHAR ibl_name[66];

static short read_backlog(int, long, void *, int);
static short writ_backlog(short, long, void *, int);
static short addtag(int, long, long, long);
static short updtag(long);
static short deltag(long, long, long);
static void  check_backlog(TCHAR *);
static int   check_till_number(int);

extern STATUS_DEF   sts_data;

/*--------------------------------------------------------------------*/
/*                    BACKLOG routines                                */
/*--------------------------------------------------------------------*/
/*
   The backlog routines on the MS-DOS machines are something special.
   Well isn't everything something special. In fact it is, but anyway
   this is how they work.
   The aim of the backlog is to hold all the invoices on disk. Whenever
   there is a possibility to send any of the data, the program will do
   so. When the connection drops the program starts where it knows for
   certain that the data has arrived at the back-office computer.
   The structure of the file is:
    - a header record
    - a file body
      - an iteration of shifts
        - a shift_on TAG record
        - a shift_on record
        - a shift body
          - an iteration of invoice/shift_lift
            - a shift-lift
              - an invoice TAG record
              - a shift-lift record;
          or
            - an invoice
              - an invoice TAG record
              - an invoice_header record
                - an invoice_line group
                  - an iteration of invoice_line record
                - an invoice_payment group
                  - an iteration of invoice_payment record
                - an invoice_vat group
                  - an iteration of invoice_vat record
              - an invoice_end record
        - a shift_off record
   The header record in cooperation with the TAG records controls the
   current status of the file.
   Every new record is appended to the file. The position (header.wpos)
   is updated.
   A shift_on record is preceded by a TAG record, so is an complete
   invoice. Every TAG record contains a starting position (bpos). When
   the shift of the invoice is closed the ending position (epos) is
   updated in the corresponding TAG record. To find the TAG-records
   the header record maintains the position of the TAG-records for
   the current shift (header.spos) and invoice (header.ipos).
   When a confirm is received from back-office the delete-field is
   updated in the TAG-record. A shift consists of invoices, the
   shift-on precedes the invoices but the invoice-commits precede the
   shift-commit. So two distinctive delete-positions are needed to
   make a fast deleted possible.
   The shift_lift is a strange thing. Somehow no answer is required
   from the back-office. However it must be recognizable, so an
   invoice TAG is put in front, but the delete flag will never be set.
   This gives a nice advantage; when the program starts resending it
   always start with the shift_on, followed by the invoices but skipping
   the 'deleted' invoices. So all the shift_lift will always be re-sent.
 */

/*--------------------------------------------------------------------*/
/*                         open_backlog                               */
/*--------------------------------------------------------------------*/
int open_backlog(_TCHAR *name)
{
  short cc;

  check_backlog(name);

  ibl_handle = -1;
  ibl_handle = _topen(name, O_CREAT | O_BINARY | O_RDWR, S_IWRITE);
  if (ibl_handle == -1) {
    err_shw(_T("BACK_TLS: open_backlog() Unable to create file %s\n"), name);
    return(-1);
  }
  cc = read_backlog((short)ibl_handle, 0L, (void*)&ibl_header, IBLH_SIZE);
  if (cc < 0) {                       /* header not present, new file */
    ibl_header.wpos = IBLH_SIZE;
    ibl_header.rpos = IBLH_SIZE;
    ibl_header.dps1 = IBLH_SIZE;
    ibl_header.dps2 = IBLH_SIZE;
    ibl_header.spos = IBLH_SIZE;
    ibl_header.ipos = IBLH_SIZE;
    cc = writ_backlog_header();
  }
  _tcscpy(ibl_name, name);         /* preserve name for switch_backlog */
  return(cc);
} /* open_backlog */

/*--------------------------------------------------------------------*/
/*                         check_backlog                              */
/*--------------------------------------------------------------------*/
void check_backlog(TCHAR *name)
{
  short  cc;
  long   read_position;
  struct IBL_HEADER head;
  _TCHAR alien_name[80];
  _TCHAR dummy[250];
  
  ibl_handle = -1;
  ibl_handle = _topen(name, O_BINARY | O_RDONLY);
  if (ibl_handle == -1) {
    return;                       /* File not found -> No problems           */
  }
  else {

    cc = read_backlog(ibl_handle, 0L, (void*)&head, IBLH_SIZE);
    read_position = IBLH_SIZE;

    if (read_position >= head.wpos) {
      close(ibl_handle);          /* No records     -> No problems           */
      return;
    }

    while (read_position < head.wpos) {
      cc = read_backlog((short)ibl_handle, read_position, (void*)&ibl_record, IBLR_SIZE);

      if (ibl_record.type > invnone && ibl_record.type < invhigh) {
        cc = check_till_number(ibl_record.type);

        if (cc != SUCCEED) {
          close(ibl_handle);
          _tcscpy(alien_name, name);
          _tcscpy(&alien_name[_tcslen(name) - 3], _T("ALIEN."));
          _tcscat(alien_name, date_time_file_extension());
          _trename(name, alien_name);
          err_shw(_T("Backlog renamed to %s"), alien_name);

          if (cc == -1) {
            _stprintf(dummy, _T("Wrong record type in backlog. Backlog renamed to %s. Till will start with empty backlog, Call your Supervisor!"),
                                 alien_name);
          }
          else {
            _stprintf(dummy, _T("Tillnumber in backlog is %d (must be %d). Backlog renamed to %s. Till will start with empty backlog, Call your Supervisor!"),
                                 cc, sts_data.syst.till_no,alien_name);
          }
          MessageBox(NULL, (LPTSTR) dummy, NULL, MB_OK|MB_ICONEXCLAMATION|MB_SETFOREGROUND);
          return;
        }
      }
      read_position += ibl_record.size + IBLR_PRFX;
    }
    close(ibl_handle);
  }
} /* check_backlog */

/*--------------------------------------------------------------------*/
/*                         check_till_number                          */
/*--------------------------------------------------------------------*/
int check_till_number(int type)
{
  short  till_no;

  switch(type) {
    case invhead:
      till_no = (short)ntohl(((struct INVOICE_HEAD *) ibl_record.data)->till_no);
      break;
    case invline:
      till_no = (short)ntohl(((struct INVOICE_LINE *) ibl_record.data)->till_no);
      break;
    case invpaym:
      till_no = (short)ntohl(((struct INVOICE_PAYM *) ibl_record.data)->till_no);
      break;
    case  invpitm:
      till_no = (short)ntohl(((struct INVOICE_PAYM_ITEM *) ibl_record.data)->till_no);
      break;
    case invvatt:
      till_no = (short)ntohl(((struct INVOICE_VAT *) ibl_record.data)->till_no);
      break;
    case invmsam:
      till_no = (short)ntohl(((struct INVOICE_MSAM *) ibl_record.data)->till_no);
      break;
    case inveoej:
      till_no = (short)ntohl(((struct INVOICE_EOJ *) ibl_record.data)->till_no);
      break;
    case invshtn:
      till_no = (short)ntohl(((struct SHIFT_ON *) ibl_record.data)->till_no);
      break;
    case invshtl:
      till_no = (short)ntohl(((struct SHIFT_LIFT *) ibl_record.data)->till_no);
      break;
    case invshtf:
      till_no = (short)ntohl(((struct SHIFT_OFF *) ibl_record.data)->till_no);
      break;
    case invxred:         /* Xread and Zread use same structure SHIFT_XREAD  */
    case invzred:
      till_no = (short)ntohl(((struct SHIFT_XREAD *) ibl_record.data)->till_no);
      break;
    default:
      err_shw(_T("BACK_TLS: Check_till_number(): unexpected type %d"), type);
      return (-1);
      break;
  }

  if (till_no != sts_data.syst.till_no) {
    err_shw(_T("BACK_TLS: Wrong till_no %d (should be %d) in backlog."),
                                         till_no, sts_data.syst.till_no);
    return(till_no);
  }
  else {
    return (SUCCEED);
  }
} /* check_till_number */

/*--------------------------------------------------------------------*/
/*                         close_backlog                              */
/*--------------------------------------------------------------------*/
void close_backlog(void)
{
  close(ibl_handle);
} /* close_backlog */

/*--------------------------------------------------------------------*/
/*                         writ_backlog_header                        */
/*--------------------------------------------------------------------*/
short writ_backlog_header(void)
{
  short cc;
  int   stat;

  cc = writ_backlog((short)ibl_handle, 0L, (void*)&ibl_header, IBLH_SIZE);
  if (cc < 0) {
    return(cc);
  }
  stat = dup(ibl_handle);
  if (stat == -1) {
    return(-5);
  }
  close(stat);
  return(0);
} /* writ_backlog_header */

/*--------------------------------------------------------------------*/
/*                         resetbacklog                               */
/*--------------------------------------------------------------------*/
void resetbacklog(void)
{
  ibl_header.rpos = ibl_header.dps1;             /* shift on is first */
  ibl_header.dps2 = ibl_header.dps1;             /* adjust invoice    */
  writ_backlog_header();
} /* resetbacklog */

/*--------------------------------------------------------------------*/
/*                         add2backlog                                */
/* Currently only records of type 'enum INVTYPE' are written to back- */
/* log. The MAX_IBL_RECORD_DATA_SIZE is calculated as the maximum size*/
/* of all INVTYPE's.                                                  */
/*--------------------------------------------------------------------*/
int add2backlog(enum INVTYPE type, int size, void *data)
{
  short cc;

  ibl_record.type = type;
  if (size > MAX_IBL_RECORD_DATA_SIZE) {   /* Just to make finding errors a hell of a lot easier... */
    err_sys(_T("BACK_TLS: add2backlog Program error: size(%d) > MAX_IBL_RECORD_DATA_SIZE(%d) in %s line %d Program aborted."),
       size, MAX_IBL_RECORD_DATA_SIZE, __FILE__, __LINE__ );
  }
  ibl_record.size = size;
  memcpy(ibl_record.data, data, size);
  size += IBLR_PRFX;
  cc = writ_backlog((short)ibl_handle, ibl_header.wpos, (void*)&ibl_record, size);
  if (cc < 0) {
    return(cc);
  }
  ibl_header.wpos += size;
  return(cc);
} /* add2backlog */

/*--------------------------------------------------------------------*/
/*                         addshifttag                                */
/*--------------------------------------------------------------------*/
int addshifttag(long date_on, short time_on)
{
  ibl_header.spos = ibl_header.wpos;
  return(addtag(1, ibl_header.spos, date_on, (long)time_on));
} /* addshifttag */

/*--------------------------------------------------------------------*/
/*                         addinvoicetag                              */
/*--------------------------------------------------------------------*/
int addinvoicetag(long invo_no)
{
  ibl_header.ipos = ibl_header.wpos;
  return(addtag(2, ibl_header.ipos, invo_no, 0L));
} /* addinvoicetag */

/*--------------------------------------------------------------------*/
/*                         updshifttag                                */
/*--------------------------------------------------------------------*/
int updshifttag(void)
{
  return(updtag(ibl_header.spos));
} /* updshifttag */

/*--------------------------------------------------------------------*/
/*                         updinvoicetag                              */
/*--------------------------------------------------------------------*/
int updinvoicetag(void)
{
  return(updtag(ibl_header.ipos));
} /* updinvoicetag */

/*--------------------------------------------------------------------*/
/*                         delshifttag                                */
/*--------------------------------------------------------------------*/
int delshifttag(long date_on, long time_on)
{
  short cc;

  cc = deltag(ibl_header.dps1, date_on, time_on);
  if (cc < 0) {
    return(cc);
  }
  ibl_header.dps1 = ibl_tag.epos;          /* where next shift tag is */
  ibl_header.dps2 = ibl_tag.epos;          /* move invoice ptr also   */
  cc =writ_backlog_header();
  return(cc);
} /* delshifttag */

/*--------------------------------------------------------------------*/
/*                         delinvoicetag                              */
/*--------------------------------------------------------------------*/
int delinvoicetag(long invo_no)
{
  short cc;

  cc = deltag(ibl_header.dps2, invo_no, 0L);
  if (cc < 0) {
    return(cc);
  }
  ibl_header.dps2 = ibl_tag.epos;        /* where next invoice tag is */
  return(writ_backlog_header());
} /* delinvoicetag */

/*--------------------------------------------------------------------*/
/*                         getheaderinfo                              */
/*--------------------------------------------------------------------*/
void getheaderinfo(long *head_info)
  /*long *head_info;*/   /* is an array of 7 longs, defined in caller */
{
  *head_info = filelength(ibl_handle);
  head_info++;
  memcpy(head_info, &ibl_header, IBLH_SIZE);
} /* getheaderinfo */

/*--------------------------------------------------------------------*/
/*                         getshifton                                 */
/*--------------------------------------------------------------------*/
int getshifton(void *data, int size, int type)
  /*int    size;*/                        /* size of shift_on record  */
  /*int    type;*/                        /* invshtn, from storemsg.h */
{
  short  cc;
  long   pos;

  pos = ibl_header.spos + IBLR_PRFX + IBLT_SIZE;  /* latest SHIFT tag */
  cc = read_backlog((short)ibl_handle, pos, (void*)&ibl_record, IBLR_PRFX + size);
  if (cc < 0) {
    return(cc);
  }
  if (ibl_record.type != type) {
    return(-11);
  }
  memcpy(data, ibl_record.data, size);
  return(type);
} /* getshifton */

/*--------------------------------------------------------------------*/
/*                         getshiftoff                                */
/*--------------------------------------------------------------------*/
int getshiftoff(void *data, int size, int type)
{
  short cc;
  long  pos;

  pos = ibl_header.spos + IBLR_PRFX;              /* latest SHIFT tag */
  cc = read_backlog((short)ibl_handle, pos, (void*)&ibl_tag, IBLT_SIZE);
  if (cc < 0) {
    return(cc);
  }
  if (ibl_tag.epos == 0) {
    return(-12);
  }
  pos = ibl_tag.epos - IBLR_PRFX - size;
  cc = read_backlog((short)ibl_handle, pos, (void*)&ibl_record, IBLR_PRFX + size);
  if (cc < 0) {
    return(cc);
  }
  if (ibl_record.type != type) {
    return(-13);
  }
  memcpy(data, ibl_record.data, size);
  return(type);
} /* getshiftoff */

/*--------------------------------------------------------------------*/
/*                         getbacklog                                 */
/*--------------------------------------------------------------------*/
int getbacklog(int *type, void *data)
{
  short  cc;
  struct IBL_TAG * htag;
  short  retry;

  do {
    retry = 0;
    if (ibl_header.rpos >= ibl_header.wpos) {               /* all sent */
      return(0);
    }
    cc = read_backlog((short)ibl_handle, ibl_header.rpos, (void*)&ibl_record, IBLR_SIZE);
    *type = ibl_record.type;
    ibl_header.rpos += ibl_record.size;
    ibl_header.rpos += IBLR_PRFX;
    if (*type == TAG) {
      htag = (struct IBL_TAG *)&ibl_record.data[0];
      if (htag->deleted) {
        ibl_header.rpos = htag->epos;
      }
      retry = 1;
    }
  } while(retry == 1);
  memcpy(data, ibl_record.data, ibl_record.size);
  return(ibl_record.size);
} /* getbacklog */

/*--------------------------------------------------------------------*/
/*                         scanbacklog                                */
/*--------------------------------------------------------------------*/
int scanbacklog(int *type, void *data)
{
  short    cc;
  static struct IBL_HEADER head = {-1};

  if (head.wpos == -1) {
    memcpy(&head, &ibl_header, IBLH_SIZE);
    head.rpos = IBLH_SIZE;
  }
  if (head.rpos >= head.wpos) {                      /* all processed */
    head.wpos = -1;
    return(0);
  }
  cc = read_backlog((short)ibl_handle, head.rpos, (void*)&ibl_record, IBLR_SIZE);
  *type = ibl_record.type;
  head.rpos += ibl_record.size;
  head.rpos += IBLR_PRFX;
  memcpy(data, ibl_record.data, ibl_record.size);
  return(ibl_record.size);
} /* scanbacklog */


/*--------------------------------------------------------------------*/
/*                         switch_backlog                             */
/*--------------------------------------------------------------------*/
/* This function first copies the current backlog.dat file to a save  */
/* file. Then it speeds thru the saved file and discovers the records */
/* which are NOT already flagged as being deleted. These records are  */
/* copied (back) to the newly empty created backlog.dat file.         */
/* There is however one disturbing catch (not 22). The structure of   */
/* the file (as described above) is later elaborated with data con-   */
/* cerning X-Read and Z-read totals. This data is stored as a complete*/
/* shift. This means however, that this type of shift only consists   */
/* of one record (X-Read or Z-Read).                                  */
/* Now here is the catch. This function is meant to be transparent. It*/
/* has no knowledge of the information which is kept in the backlog-  */
/* file. Therefore it needs the type given in the header of the       */
/* function. The caller tells with this type, which is the shift_off. */
/* So, X-Read and Z-Read should ALWAYS be greater than this type to   */
/* make it possible for this function to detect a 'shift' of only ONE */
/* record.                                                            */
/*--------------------------------------------------------------------*/
int switch_backlog(int type, long run_date, short till_no, _TCHAR *bl_dir)
  /*int    type;*/                        /* indicates shift-off type */
{
/* ENVIRONMENT VARIABLES REGISTRY */
#define  LEN_BACKLOG_SAVE_DAYS 3
  int    cc;
  int    sav_handle;
  _TCHAR sav_name[66];
  struct IBL_HEADER head;
  struct IBL_TAG    tag;
  int    cpy_shift;
  _TCHAR julian_date_char[10];
  short  julian_date_short;
  long   year;
  short  i;
  long   eod;
  short  f_till_no;
  short  f_julian_date;
  short  f_year;
  long   d_days;
  struct _tfinddata_t fileinfo;
  _TCHAR reg_backlog_save_days[LEN_BACKLOG_SAVE_DAYS+1];
  short  maxno_days;
  long   hfile;

  close_backlog();

  /* More backups will be made of the backlog.dat file. */
  /* The name of the backlog sav files will be:         */
  /* TTYYJJJQ.BLG, TT=till_no, YY=year, JJJ=julian_date.*/
  /*               Q=sequence number (0-9)              */

  /* Retrieve the maximum number of days to save the backlog from the registry*/
  
  SetRegistryEcho(FALSE);
  ReadEnvironmentValue(TREE_TILL_DEPENDENT, _T("BACKLOG_SAVE_DAYS"),  reg_backlog_save_days, 
                        LEN_BACKLOG_SAVE_DAYS);
  SetRegistryEcho(TRUE);

  maxno_days = (short)_tcstol(reg_backlog_save_days, NULL, 10);

  get_jul_date(run_date, julian_date_char);
  julian_date_short = (short)_tcstol(julian_date_char, NULL, 10);
  year = (((run_date - 19000000) - (run_date % 1000L)) / 10000L) % 100L;

  /* scan backlog directory for old backlog files and remove them */

  _stprintf(sav_name, _T("%s*.blg"), bl_dir);
  hfile = _tfindfirst(sav_name, &fileinfo);
  if (hfile != -1L) {
    eod = 0;
    while (eod == 0) {
      _stscanf(fileinfo.name, _T("%02d%02d%03d"), &f_till_no, &f_year, &f_julian_date);
      d_days = ((long)year*366+(long) julian_date_short) - ((long)f_year*366+(long)f_julian_date);
      if (d_days<0) {
        d_days += (long) 36600;
      }
      if (d_days>(long) (maxno_days+1) || maxno_days == 0) {
        _stprintf(sav_name, _T("%s%s"), bl_dir, fileinfo.name);
        _tremove(sav_name);
      }
      eod = _tfindnext(hfile, &fileinfo);
    }
    _findclose(hfile);
  }

  /* Delete the BACKLOG.SAV file if it exists */
  _tcscpy(sav_name, ibl_name);
  _tcscpy(&sav_name[_tcslen(ibl_name) - 3], _T("SAV"));
  cc = _tremove(sav_name);

  if(maxno_days) { /* Change the name of the backlog file. */
    /* Determine first available backlog name for todays saved backlog. If */
    /* more than ten end of days were performed during the same day, the   */
    /* backlog copied during the last end of day will get lost             */
    for (i=0 ; i<10 ; i++) {
      _stprintf(sav_name, _T("%s%02d%.2ld%.3d%.1d.BLG"), bl_dir, till_no, year, julian_date_short,i);
      if (_tfindfirst(sav_name, &fileinfo) == -1) {
        break;
      }
    }
    /* After more than ten "end of days" in one day, the backlog will be saved in */
    /* the file TTYYDD9.BLG                                                       */

    /* Delete the TTYYDDDQ.BLG file if it exists */
    cc = _tremove(sav_name);
  }

  /* Rename the backlog file to BACKLOG.SAV or a TTYYDDDQ.BLG file */
  /* depending on the setting of BACKLOG_SAVE_DAYS.                */
  cc = _trename(ibl_name, sav_name);
  cc = open_backlog(ibl_name);              /* initialize new backlog */
  if (cc < 0) {
    return(cc);
  }
  sav_handle = -1;
  sav_handle = _topen(sav_name, O_BINARY | O_RDWR, S_IWRITE);
  if (sav_handle == -1) {
    err_shw(_T("BACK_TLS: switch_backlog() Unable to create file %s\n"), sav_name);
    return(-1);
  }
  cpy_shift = FALSE;
  cc = read_backlog(sav_handle, 0L, (void*)&head, IBLH_SIZE);
  head.rpos = IBLH_SIZE;                     /* find missing invoices */
  while (TRUE) {
    if (head.rpos >= head.wpos) {
      close(sav_handle);
      return(0);
    }
    cc = read_backlog(sav_handle, head.rpos, (void*)&ibl_record, IBLR_SIZE);
    if (ibl_record.type == TAG) {
      memcpy(&tag, ibl_record.data, IBLT_SIZE);
      if (tag.type == 1) {                               /* shift TAG */
        if (!tag.deleted) {
          err_shw(_T("BACK_TLS: switch_backlog() Active shift %ld-%ld found"), tag.key1, tag.key2);
          addshifttag(tag.key1, (short)tag.key2);
          head.rpos += IBLT_SIZE + IBLR_PRFX; /* shift on or X/Z-Read */
          cc = read_backlog(sav_handle, head.rpos, (void*)&ibl_record, IBLR_SIZE);
          cc = add2backlog(ibl_record.type, ibl_record.size, ibl_record.data);
          head.rpos += ibl_record.size + IBLR_PRFX;
          if (ibl_record.type > type) {      /* see comment in header */
            updshifttag();
          }
          else {                         /* a real shift, NO X/Z-read */
            cpy_shift = TRUE;
          }
        }
        else {                                    /* shift is deleted */
          head.rpos += IBLT_SIZE + IBLR_PRFX;
          cc = read_backlog(sav_handle, head.rpos, (void*)&ibl_record, IBLR_SIZE);
          head.rpos += ibl_record.size + IBLR_PRFX;
        }
      }
      else {                                           /* invoice TAG */
        if (!tag.deleted &&                            /* not deleted */
            (tag.key1 >=0 ||                           /* invoice     */
             (tag.key1 == -1 && cpy_shift))) {         /* shift lift  */
          /* shift lift is only copied when the shift needs copying   */
          err_shw(_T("BACK_TLS: switch_backlog() Active invoice/shift_lift %ld found"), tag.key1);
          addinvoicetag(tag.key1);
          head.rpos += IBLT_SIZE + IBLR_PRFX;
          while (TRUE) {                        /* copy entire invoice */
            cc = (int)read_backlog(sav_handle, head.rpos, (void*)&ibl_record, IBLR_SIZE);
            if (head.rpos >= tag.epos || ibl_record.type == type) {
              break;
            }
            cc = add2backlog(ibl_record.type, ibl_record.size, ibl_record.data);
            head.rpos += ibl_record.size + IBLR_PRFX;
          }
          updinvoicetag();
        }
        else {                                  /* invoice is deleted */
          head.rpos = tag.epos;
        }
      }
    }
    else {                                           /* normal record */
      if (cpy_shift && ibl_record.type == type) {    /* need shift off*/
        cpy_shift = FALSE;
        cc = add2backlog(ibl_record.type, ibl_record.size, ibl_record.data);
        updshifttag();
      }
      head.rpos += ibl_record.size + IBLR_PRFX;
    }
  }
} /* switch_backlog */

/*--------------------------------------------------------------------*/
/*                         read_backlog                               */
/*--------------------------------------------------------------------*/
static short read_backlog(int handle, long pos, void *data, int len)
{
  if (pos != lseek(handle, pos, SEEK_SET)) {
    return(-1);
  }
  if (len != _read(handle, data, len)) {
    return(-2);
  }
  return(0);
} /* read_backlog */

/*--------------------------------------------------------------------*/
/*                         writ_backlog                               */
/*--------------------------------------------------------------------*/
static short writ_backlog(short handle, long pos, void *data, int len)
{
  short error;

  if (pos != lseek(handle, pos, SEEK_SET)) {
    return(-3);
  }
  if (len != _write(handle, data, len)) {
    return(-4);                                /* serious write error */
  }
  error = _commit(handle);
  if (error != 0) {
    return(-4);                                /* serious write error */
  }
  return(0);
} /* writ_backlog */

/*--------------------------------------------------------------------*/
/*                         addtag                                     */
/*--------------------------------------------------------------------*/
static short addtag(int type, long pos, long key1, long key2)
{
  ibl_tag.bpos = pos;
  ibl_tag.epos = 0;
  ibl_tag.type = type;
  ibl_tag.deleted = FALSE;
  ibl_tag.key1 = key1;
  ibl_tag.key2 = key2;
  return(add2backlog(TAG, IBLT_SIZE, (void*)&ibl_tag));
} /* addtag */

/*--------------------------------------------------------------------*/
/*                         updtag                                     */
/*--------------------------------------------------------------------*/
static short updtag(long pos)
{
  short cc;

  cc = read_backlog((short)ibl_handle, pos+IBLR_PRFX, (void*)&ibl_tag, IBLT_SIZE);
  if (cc < 0) {
    return(cc);
  }
  ibl_tag.epos = ibl_header.wpos;              /* the ending position */
  cc = writ_backlog((short)ibl_handle, pos+IBLR_PRFX, (void*)&ibl_tag, IBLT_SIZE);
  if (cc < 0) {
    return(cc);
  }
  cc = writ_backlog_header();
  return(cc);
} /* updtag */

/*--------------------------------------------------------------------*/
/*                         deltag                                     */
/*--------------------------------------------------------------------*/
static short deltag(long pos, long key1, long key2)
{
  short cc;
  int   first = 1;

re_scan:
  while (TRUE) {
    while (TRUE) {
      if (pos >= ibl_header.wpos) {              /* beyond everything */
        if (first) {                       /* timing with e.g. x-read */
          first = 0;
          pos = IBLH_SIZE;                    /* start from beginning */
          goto re_scan;
        }
        return(-6);
      }
      cc = read_backlog((short)ibl_handle, pos, (void*)&ibl_prefix, IBLR_PRFX);
      if (cc < 0) {
        return(cc);
      }
      if (ibl_prefix.type == TAG) {                 /* next tag found */
        break;
      }
      pos += IBLR_PRFX + ibl_prefix.size;           /* to next record */
    }
    cc = read_backlog((short)ibl_handle, pos+IBLR_PRFX, (void*)&ibl_tag, IBLT_SIZE);
    if (cc < 0) {
      return(cc);
    }
    if (ibl_tag.key1 == key1 && ibl_tag.key2 == key2) {
      break;
    }
    pos += IBLR_PRFX + IBLT_SIZE;
  }
  ibl_tag.deleted = TRUE;
  cc = writ_backlog((short)ibl_handle, pos+IBLR_PRFX, (void*)&ibl_tag, IBLT_SIZE);
  if (cc < 0) {
    return(cc);
  }
  cc = writ_backlog_header();
  return(cc);
} /* deltag */

/*--------------------------------------------------------------------*/
/*                      rename_corrupt_backlog                        */
/*--------------------------------------------------------------------*/
void rename_corrupt_backlog(void)
{
  _TCHAR bad_backlog_name[80];
  _TCHAR dummy[250];

  _tcscpy(bad_backlog_name, ibl_name);
  _tcscpy(&bad_backlog_name[_tcslen(ibl_name) - 3], _T("BAD."));
  _tcscat(bad_backlog_name, date_time_file_extension());
  _trename(ibl_name, bad_backlog_name);
  _stprintf(dummy, _T("Corrupt backlog renamed to %s. Call the system administrator to load the invoice data to BackOffice "), bad_backlog_name);
  MessageBox(NULL, (LPTSTR) dummy, (LPTSTR) _T("RENAME BACKLOG"), MB_OK|MB_ICONSTOP|MB_SETFOREGROUND);

} /* rename_corrupt_backlog */
