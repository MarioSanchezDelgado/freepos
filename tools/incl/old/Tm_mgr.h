/*
 *     Module Name       : TM_MGR.H
 *
 *     Type              : Include file transaction manager
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
 * 14-Dec-1999 Initial Release WinPOS                                    P.M.
 * --------------------------------------------------------------------------
 * 10-Jan-1999 Added function tm_remv                                    P.M.
 * --------------------------------------------------------------------------
 */

#ifndef TM_MGR_H
#define TM_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#define TM_MAX_GROUPS          10
                                               /* Returned errors         */
#define TM_2MANY_GROUPS     -1301              /* Maximum # of groups     */
#define TM_IO_ERROR         -1302              /* Nothing more to say     */
#define TM_OUTER_SPACE      -1303              /* Ramdrive to small       */
#define TM_UNDEFINED        -1304              /* Access undefined struct */
#define TM_2MANY_RECORDS    -1305              /* Maximum # of records    */
#define TM_BOF_EOF          -1306              /* Begin/End of structure  */

typedef short TM_INDX;                         /* Typedefs                */
typedef unsigned short TM_NAME;

extern short   tm_init(void);                  /* Prototypes              */
extern void    tm_exit(void);
extern short   tm_define_struct(TM_NAME name, int max, int size);
extern short   tm_reset_struct (TM_NAME name);
extern TM_INDX tm_upda_nth(TM_NAME name, void *data, TM_INDX indx);
extern TM_INDX tm_read_nth(TM_NAME name, void *data, TM_INDX indx);
extern TM_INDX tm_appe(TM_NAME name, void *data);
extern TM_INDX tm_upda(TM_NAME name, void *data);
extern TM_INDX tm_frst(TM_NAME name, void *data);
extern TM_INDX tm_last(TM_NAME name, void *data);
extern TM_INDX tm_next(TM_NAME name, void *data);
extern TM_INDX tm_prev(TM_NAME name, void *data);
extern TM_INDX tm_remv(TM_NAME name);
extern int     tm_coun(TM_NAME name);


#ifdef __cplusplus
}
#endif

#endif
