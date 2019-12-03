/*
 *     Module Name       : Qry_mgr.h
 *
 *     Type              : Include file input manager
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

#ifndef QUERY_MGR_H
#define QUERY_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#define NO_IDX        -1
#define INVALID_QUERY -2   /* invalid query handle */

#define MAX_QUERIES    1   /* max number of queries that qry_mgr handles */
#define MAX_CHECKS    20

#define QRY_NEW       -1   /* new query, must be parsed                  */
#define QRY_BUSY       0   /* query is active, records are being fetched */
#define QRY_EOT      101   /* end of table                               */

#define PARSE_ERROR  -10   /* query could not be parsed (syntax error)   */

enum fld_type {QRY_LONG, QRY_DOUBLE, QRY_STRING, QRY_CHAR};
enum chk_types {
  LG_GREATER,        /* long, greater than                 */
  DB_GREATER,        /* double, greater than               */
  LG_SMALLER,        /* long, smaller than                 */
  DB_SMALLER,        /* double, smaller than               */
  LG_EQUAL,          /* long, equal to                     */
  DB_EQUAL,          /* double, equal to                   */
  SUB_STR,           /* substring                          */
  SUB_STR_START,     /* substring at start of string field */
  SUB_STR_END,       /* substring at end of string field   */
  EQUAL_STR,         /* equal string                       */
  CHAR_EQUAL         /* equal character                    */
};

typedef struct QUERY_FIELD {
  void         *field_ptr;
  enum fld_type field_type;
  short         idx;
} QRY_FIELD;

typedef struct QUERY_OBJECT {
  short         fno;
  void         *record_buffer;
  short         buffer_size;
  short         num_fields;
  QRY_FIELD    *fields;
} QRY_OBJECT;

short         qry_register_query  (const QRY_OBJECT *);
void          qry_unregister_query(short);
void          qry_add_condition   (short, short, const _TCHAR *);
const _TCHAR *qry_get_condition   (short, short);
short         qry_parse_query     (short);
int           qry_fetch_record    (short);

#ifdef __cplusplus
}
#endif

#endif
