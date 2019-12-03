/*
 *     Module Name       : QRY_MGR.C
 *
 *     Type              : 
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
 * 15-May-2000 Initial Release WinPOS                                  R.N.B.
 * --------------------------------------------------------------------------
 */

#include <windows.h>
#include <windowsx.h>
#pragma hdrstop

#define  IS_VALID_ID(x)   (x!=INVALID_QUERY && query[x].used==1)

#include "Template.h" /* Every source should use this one! */

#include "assert.h"
#include "qry_mgr.h"
#include "inp_mgr.h"
#include "mem_mgr.h"
#include "comm_tls.h"

struct REGISTERED_QUERY {
  short       used;                  /* 0=not_used, 1=used                    */
  short       status;                /* NEW, BUSY or EOT                      */
  const QRY_OBJECT *user_query;            /* user defined query structure          */
  _TCHAR    (*fields_buffer)[BUFFER_LENGTH]; /* to store the conditions       */
                                             /* before they are parsed        */
  short       str_count;
};

struct FIELD_CHECK {
  enum chk_types type;               /* LG_GREATER, SUBSTR, etc....           */
  void          *field;              /* field adres in the record_buffer (see QRY_OBJECT) */
  union VALUES {                     /* value that should be matched:         */
    long      lg;                    /*   integer value                       */
    double    db;                    /*   double value                        */
    _TCHAR    ch;                    /*   single character                    */
    struct {                         /*   string value:                       */
      _TCHAR *p;                     /*     pointer to string                 */
      int     l;                     /*     length of string                  */
      short   s;                     /*     indicator: 0=nieuw condition      */    
    }            str;                /*     1=follows other string condition  */
  } value;
  short          eot_on_fail;        /* 0=continue search on fail, 1=stop     */
                                     /* search on fail (for index checks)     */
};

struct PARSED_QUERY {
  short       fno;                   /* file to search in                     */
  short       idx_search;            /* 0=fno is not an index, 1=fno is index */
  short       query_id;              /* reference to a registered query       */
  int         num_checks;            /* count of elements in field_chk[]      */
  struct FIELD_CHECK field_chk[MAX_CHECKS]; /* array of checks that need to be*/
                                            /* done on each fetched record    */
  _TCHAR    (*string_buffer)[BUFFER_LENGTH];
};

static short qry_parse_condition(QRY_FIELD *, _TCHAR *);

struct REGISTERED_QUERY  query[MAX_QUERIES];
struct PARSED_QUERY      parsed_qry = {-1, 0, INVALID_QUERY, 0, {0}, NULL};
short                    num_chk_idx;
short                    num_chk_no_idx;
struct FIELD_CHECK       chk_idx[MAX_CHECKS];     /* index checks      */
struct FIELD_CHECK       chk_no_idx[MAX_CHECKS];  /* long, char checks */
short                    str_parse_count;

/*-------------------------------------------------------------------------*/
/*               qry_register_query                                        */
/*-------------------------------------------------------------------------*/
short qry_register_query(const QRY_OBJECT *usr_query)
{
  int qry_id, i;

  for (qry_id=0; qry_id<MAX_QUERIES && query[qry_id].used==1; qry_id++) {;}

  assert(qry_id < MAX_QUERIES);             /* increase MAX_QUERIES on fail */

  if (qry_id < MAX_QUERIES) {
    query[qry_id].used = 1;
    query[qry_id].status = QRY_NEW;
    query[qry_id].user_query = usr_query;
    query[qry_id].str_count = 0;

    query[qry_id].fields_buffer = (_TCHAR (*)[BUFFER_LENGTH]) mem_allocate(usr_query->num_fields*BUFFER_LENGTH*sizeof(_TCHAR));
    for (i=0; i<usr_query->num_fields; i++) {
      *query[qry_id].fields_buffer[i] = _T('\0');
      if (usr_query->fields[i].field_type == QRY_STRING) {
        query[qry_id].str_count++;
      }
    }
  }
  else {
    qry_id = INVALID_QUERY;
  }

  return (qry_id);
}; /* qry_register_query */

/*-------------------------------------------------------------------------*/
/*               qry_unregister_query                                      */
/*-------------------------------------------------------------------------*/
void qry_unregister_query(short qry_id)
{
  if (IS_VALID_ID(qry_id)) {
                        /* free the parsed query */
    if (parsed_qry.query_id==qry_id) {
      parsed_qry.query_id = INVALID_QUERY;
      if (parsed_qry.string_buffer != NULL) {
        mem_free(parsed_qry.string_buffer);
        parsed_qry.string_buffer = NULL;
      }
    }
                        /* reset registered query */
    query[qry_id].used = 0;
    query[qry_id].status = QRY_NEW;
    query[qry_id].user_query = NULL;
    mem_free(query[qry_id].fields_buffer);
  }

  return ;
}; /* qry_unregister_query */


/*-------------------------------------------------------------------------*/
/*               qry_add_condition                                         */
/*-------------------------------------------------------------------------*/
void qry_add_condition(short qry_id, short field_id, const _TCHAR *condition)
{
  if (IS_VALID_ID(qry_id)) {
    query[qry_id].status = QRY_NEW;            /* query will be (re)parsed */
    _tcscpy(query[qry_id].fields_buffer[field_id], condition);
  }

  return;
}; /* qry_add_condition */

/*-------------------------------------------------------------------------*/
/*               qry_get_condition                                         */
/*-------------------------------------------------------------------------*/
const _TCHAR *qry_get_condition(short qry_id, short field_id)
{
  if (IS_VALID_ID(qry_id)) {
    return (query[qry_id].fields_buffer[field_id]);
  }

  return (NULL);
}; /* qry_get_condition */


/*-------------------------------------------------------------------------*/
/*               qry_parse_query                                           */
/*-------------------------------------------------------------------------*/
short qry_parse_query(short qry_id)
{
  const QRY_OBJECT *usr_qry = query[qry_id].user_query;
  int i,
      cond_id = 0,
      status = SUCCEED;

  
  if (! IS_VALID_ID(qry_id)) {
    return (PARSE_ERROR);
  }
                                                           /* intialisations */
  parsed_qry.query_id   = qry_id;
  parsed_qry.fno        = query[qry_id].user_query->fno;
  parsed_qry.idx_search = 0;
  parsed_qry.num_checks = 0;
  query[qry_id].status  = QRY_NEW;

  if (parsed_qry.string_buffer != NULL) {
    mem_free(parsed_qry.string_buffer);
    parsed_qry.string_buffer = NULL;
  }
  if (query[qry_id].str_count > 0) {
    parsed_qry.string_buffer = (_TCHAR (*)[BUFFER_LENGTH]) mem_allocate(query[qry_id].str_count*BUFFER_LENGTH*sizeof(_TCHAR));
  }

  num_chk_idx = num_chk_no_idx = str_parse_count = 0;

  for (i=0; i<usr_qry->num_fields && status!=PARSE_ERROR; i++) {
    if (query[qry_id].fields_buffer[i][0] != '\0') {
      status = qry_parse_condition(&usr_qry->fields[i], query[qry_id].fields_buffer[i]);
    }
  }
         /* first copy the array with index checks and after that the rest. This */
         /* ensures that index fields are checked first.                         */
  if (status != PARSE_ERROR) {
    parsed_qry.num_checks = num_chk_idx + num_chk_no_idx;
    if (parsed_qry.num_checks < MAX_CHECKS) {
      memcpy(parsed_qry.field_chk, chk_idx, num_chk_idx*sizeof(struct FIELD_CHECK));
      memcpy(&parsed_qry.field_chk[num_chk_idx], chk_no_idx, num_chk_no_idx*sizeof(struct FIELD_CHECK));
    }
    else {
      status = PARSE_ERROR;
    }
  }

  return (status);
} /* qry_parse_query */

/*-------------------------------------------------------------------------*/
/*               qry_parse_condition                                       */
/*  NB NOT YET FINISHED !!  (only for string fields is finsihed)           */
/*                                                                         */
/* - The following fields of FIELD_CHECK must be filled:                   */
/*   eot_on_fail                                                           */
/*   value.??                                                              */
/*   field                                                                 */
/*   type                                                                  */
/* - increase the check count cnt                                          */
/* - if an index field is used, the field should be initialised in the     */
/*   record buffer, otherwise pos_start_rec won't work well                */
/*   Thus: *(...) fld->field_ptr = ...                                     */
/*-------------------------------------------------------------------------*/
short qry_parse_condition(QRY_FIELD *fld, _TCHAR *cond)
{
  short  status = SUCCEED,
        *cnt,
         eot_on_fail,
         first = 0;  /* for string search: 0=new string condition on a field */
                     /* 1=next string condition following the previous one   */                     
  struct FIELD_CHECK *ptr;
  _TCHAR *p,
         *end,
         *cpy_cond;

                                             /* Check if index can be used */
  if (fld->idx == NO_IDX || num_chk_idx>0) {     /* not an index field or  */
    eot_on_fail=0;                             /* an index is already used */
  }
  else {                                             /* index field        */ 
    if (fld->field_type == QRY_STRING
        && *cond == _T('%')
        && *(cond+1) != _T('%')) {
      eot_on_fail=0;           /* string index cannot be used in this case */
    }
    else {
      eot_on_fail=1;
    }
  }

       /* intialise some vars depending on whether or not an index is used */
  if (eot_on_fail==0) {  /* no index */
    ptr = chk_no_idx;
    cnt = &num_chk_no_idx;
  }
  else {                 /* index */
    ptr = chk_idx;
    cnt = &num_chk_idx;
    parsed_qry.fno = fld->idx;
    parsed_qry.idx_search = 1;
  }

  if (*cnt >= MAX_CHECKS) {
    return PARSE_ERROR;                       /* too many field conditions */
  }
                                              /* parse the field condition */
  switch (fld->field_type) {
  case QRY_STRING:    /* look for one or more substrings in a field */
    cpy_cond = parsed_qry.string_buffer[str_parse_count++];
    _tcscpy(cpy_cond, cond);
    end = cpy_cond + _tcslen(cpy_cond);

    p = _tcstok(cpy_cond, _T("%"));
    while (p != NULL) {
      ptr[*cnt].value.str.p = p;
      ptr[*cnt].value.str.l = _tcslen(p);
      ptr[*cnt].value.str.s = first;

      if (first==0) {
        first=1;
      }
                                        /* determine type of condition */
      if (p==cpy_cond) {
        if (p + _tcslen(p) == end) {
          ptr[*cnt].type = EQUAL_STR;
        }
        else {
          ptr[*cnt].type = SUB_STR_START;
        }
      }
      else {
        if (p + _tcslen(p) == end) {
          ptr[*cnt].type = SUB_STR_END;
        }
        else {
          ptr[*cnt].type = SUB_STR;
        }
      }

      if (ptr[*cnt].type == SUB_STR_START && eot_on_fail==1) {  /* index check */
        _tcscpy((_TCHAR*)fld->field_ptr, p);  /* intialise field in record buffer */
        ptr[*cnt].eot_on_fail = 1;
      }
      else {
        ptr[*cnt].eot_on_fail = 0;
      }
      ptr[*cnt].field = fld->field_ptr;

      (*cnt)++;
      p = _tcstok(NULL, _T("%"));
    }
    break;

      /* !!!!! NOT YET FINISHED !!!! */
  case QRY_LONG: 
    break;
  case QRY_DOUBLE:
    break;
  case QRY_CHAR:
    break;
  default:
    status = PARSE_ERROR;
    break;
  }

  return (status);
} /* qry_parse_condition */

/*-------------------------------------------------------------------------*/
/*               qry_fetch_record                                          */
/*-------------------------------------------------------------------------*/
int qry_fetch_record(short qry_id)
{
  short   check_status = TRUE,
         *qry_stat = &(query[qry_id].status);
  int     i;
  _TCHAR *substr = NULL;


  if (! IS_VALID_ID(qry_id) || parsed_qry.query_id != qry_id) {
    return (INVALID_QUERY);
  }


  if (*qry_stat == QRY_NEW) {
    if (parsed_qry.idx_search == 0) {
      *qry_stat = pos_first_rec(-1, query[qry_id].user_query->buffer_size,
                              parsed_qry.fno, query[qry_id].user_query->record_buffer);
    }
    else {
      *qry_stat = pos_start_rec(-1, query[qry_id].user_query->buffer_size,
                              parsed_qry.fno, query[qry_id].user_query->record_buffer);
    }
  }
  else if (*qry_stat != QRY_EOT) {
    *qry_stat = pos_next_rec_no_key(-1, query[qry_id].user_query->buffer_size,
                              parsed_qry.fno, query[qry_id].user_query->record_buffer);
  }

  while (*qry_stat == QRY_BUSY) {
 
    check_status = TRUE;  /* reset */
    substr = NULL;
                                               /* perform all checks on record */
    for (i=0; i<parsed_qry.num_checks && check_status!=FALSE; i++) {
      switch (parsed_qry.field_chk[i].type) {
      case SUB_STR:
        if (parsed_qry.field_chk[i].value.str.s==0) {
          substr = (_TCHAR *)parsed_qry.field_chk[i].field;
        }
        substr = _tcsstr(substr, parsed_qry.field_chk[i].value.str.p);
        check_status = FALSE;
        if (substr) {
          substr += parsed_qry.field_chk[i].value.str.l;
          check_status = TRUE;
        }
        break;
      case SUB_STR_START:
        substr = (_TCHAR *)parsed_qry.field_chk[i].field;
        check_status = (_tcsncmp(substr, parsed_qry.field_chk[i].value.str.p, parsed_qry.field_chk[i].value.str.l) == 0);
        substr += parsed_qry.field_chk[i].value.str.l;
        break;
      case SUB_STR_END:
        if (parsed_qry.field_chk[i].value.str.s==0) {
          substr = (_TCHAR *)parsed_qry.field_chk[i].field;
        }
        check_status = (_tcscmp(substr + _tcslen(substr) - parsed_qry.field_chk[i].value.str.l, 
                                parsed_qry.field_chk[i].value.str.p) == 0);
        break;
      case EQUAL_STR:
        check_status = (_tcscmp((_TCHAR *)parsed_qry.field_chk[i].field, 
                                parsed_qry.field_chk[i].value.str.p) == 0);
        break;
      case LG_GREATER:
        check_status = (*(long*)parsed_qry.field_chk[i].field > parsed_qry.field_chk[i].value.lg);
        break;
      case DB_GREATER:
        check_status = (*(double*)parsed_qry.field_chk[i].field > parsed_qry.field_chk[i].value.db);
        break;
      case LG_SMALLER:
        check_status = (*(long*)parsed_qry.field_chk[i].field < parsed_qry.field_chk[i].value.lg);
        break;
      case DB_SMALLER:
        check_status = (*(double*)parsed_qry.field_chk[i].field < parsed_qry.field_chk[i].value.db);
        break;
      case LG_EQUAL:
        check_status = (*(long*)parsed_qry.field_chk[i].field == parsed_qry.field_chk[i].value.lg);
        break;
      case DB_EQUAL:
        check_status = (*(double*)parsed_qry.field_chk[i].field == parsed_qry.field_chk[i].value.db);
        break;
      case CHAR_EQUAL:
       check_status = (*(char*)parsed_qry.field_chk[i].field == parsed_qry.field_chk[i].value.ch);
        break;
      default:
        break;
      }
    } /* for */

    if (check_status==FALSE) {
      if (parsed_qry.field_chk[i-1].eot_on_fail == 0) { /* quit on fail ? */
        *qry_stat = pos_next_rec_no_key(-1, query[qry_id].user_query->buffer_size,
                              parsed_qry.fno, query[qry_id].user_query->record_buffer);
      }
      else {
        *qry_stat = QRY_EOT;  /* eot reached because index check failed */
      }
    }
    else {
      break;                  /* okay, record matched conditions */
    }
  } /* while */

  return(*qry_stat);
} /* qry_fetch_record */
