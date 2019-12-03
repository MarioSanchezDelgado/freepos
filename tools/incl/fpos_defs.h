#define OS2DEF_INCLUDED


#if     !defined(__CDSTYPES_H_INCLUDED)
#define __CDSTYPES_H_INCLUDED
#endif
/******************************************************************************
   Type macros for hiding of OS-specific pointer and function declarations
******************************************************************************/

#define CDS_PTR                 _far *

#define INTERNAL_ENTRY          _near _fastcall
#define EXTERNAL_ENTRY          _far _cdecl
#define CDS_ENTRY               EXTERNAL_ENTRY _loadds
#define CALLBACK_ENTRY          CDS_ENTRY
#define INTERRUPT_ENTRY         _far _cdecl _interrupt

/******************************************************************************
 Type definitions for hiding of OS-specific pointer and function declarations
******************************************************************************/

//#ifndef OS2DEF_INCLUDED         /* Avoid conflicts with PHAPI.H or OS2DEF.H */

/* STANDARD types */
//typedef void                    VOID;
//typedef char                    CHAR;
//typedef short                   SHORT;
//typedef long                    LONG;
//typedef int                     INT;
//typedef double                  DOUBLE;


/* UNSIGNED types */

//typedef unsigned char           UCHAR;
//typedef unsigned short          USHORT;
//typedef unsigned long           ULONG;
//typedef unsigned int            UINT;
//
///* POINTER types */
//
//typedef VOID CDS_PTR            PVOID;
//typedef CHAR CDS_PTR            PCHAR;
//typedef SHORT CDS_PTR           PSHORT;
//typedef LONG CDS_PTR            PLONG;
//typedef INT CDS_PTR             PINT;
//
//typedef UCHAR CDS_PTR           PUCHAR;
//typedef USHORT CDS_PTR          PUSHORT;
//typedef ULONG CDS_PTR           PULONG;
//typedef UINT CDS_PTR            PUINT;
//#endif /* !OS2DEF_INCLUDED */
//
///* POINTER to FUNCTION types */
//
//typedef VOID                    (*PFVOID)();
//typedef CHAR                    (*PFCHAR)();
//typedef SHORT                   (*PFSHORT)();
//typedef LONG                    (*PFLONG)();
//typedef INT                     (*PFINT)();
//
//typedef UCHAR                   (*PFUCHAR)();
//typedef USHORT                  (*PFUSHORT)();
//typedef ULONG                   (*PFULONG)();
//typedef UINT                    (*PFUINT)();
//
///* Miscellaneous types */
//
//typedef UCHAR                   BOOLEAN;

/******************************************************************************
                           Miscellaneous Constants
******************************************************************************/
                                /* NULL CDS pointer */
#define CDS_NULL                (VOID CDS_PTR)0
#ifndef NULL                    
                                /* NULL internal pointer */ 
#define NULL                    (VOID *)0
#endif

                                /* BOOLEAN - affirmative */
#ifndef TRUE
#define TRUE                    1
#endif
#ifndef YES
#define YES                     1
#endif

                                /* BOOLEAN - negative */
#ifndef FALSE
#define FALSE                   0
#endif
#ifndef NO
#define NO                      0
#endif

#ifndef SUCCESSFUL
#define SUCCESSFUL              0
#endif

#define OK           0
#define SUCCEED      0
#define FAIL         1
#define YES          1
#define NO           0
#define TRUE         1
#define FALSE        0
#define FOREVER      for (;;)



/******************************************************************************
                                    Macros
******************************************************************************/

//#ifndef OS2DEF_INCLUDED         /* Avoid conflicts with PHAPI.H or OS2DEF.H */

/* Convert to any type */

//#define MAKETYPE(v,type)   (*((type *)&v))
//
///* Compute offset of a field inside any structure */
//
//#define FIELDOFFSET(type,field)   ((USHORT)(&(((type *)0)->field)))
//
///* Convert to 16 bit integers into a 32 bit integer */
//
//#define MAKEULONG(l,h)  ((ULONG)(((USHORT)(l)) | ((ULONG)((USHORT)(h))) << 16))
//#define MAKELONG(l,h)   ((LONG)MAKEULONG(l,h))
//
///* Convert two bytes into a 16 bit integer */
//
//#define MAKEUSHORT(l,h) (((USHORT)(l)) | ((USHORT)(h)) << 8)
//#define MAKESHORT(l,h)  ((SHORT)MAKEUSHORT(l,h))
//
///* Isolate upper or lower 8 bits of a 16 bit integer */
//
//#define LOUCHAR(w)      ((UCHAR)(w))
//#define HIUCHAR(w)      ((UCHAR)(((USHORT)(w) >> 8) & 0xFF))
//
///* Isolate upper or lower 16 bits of a 32 bit integer */
//
//#define LOUSHORT(l)     ((USHORT)(l))
//#define HIUSHORT(l)     ((USHORT)(((ULONG)(l) >> 16) & 0xFFFF))
//
//#endif /* !OS2DEF_INCLUDED */
//
///* Isolate upper or lower 4 bits of an 8 bit integer */
//
//#define LONIBBLE(b)     ((UCHAR)((b) & (UCHAR)0x0F))
//#define HINIBBLE(b)     ((UCHAR)(((UCHAR)(b) >> 4) & (UCHAR)0x0F))

//#endif                                 /* !__CDSTYPES_H_INCLUDED */
