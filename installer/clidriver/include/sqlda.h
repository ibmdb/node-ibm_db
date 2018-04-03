/*****************************************************************************
 *
 * Source File Name = SQLDA.H
 *
 * (C) COPYRIGHT International Business Machines Corp. 1987, 2005
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Function = Include File defining:
 *              SQL Descriptor Area
 *
 * Operating System = Common C Include File
 *
 ****************************************************************************/

#ifndef  SQLDASIZE             /* Permit duplicate Includes                 */

#include "sqlsystm.h"          /* System-Specific Include                   */
/* _SQLOLDCHAR defaults to 'char'.  See sqlsystm.h for details. */

#if defined(DB2NT)
#if defined _WIN64
#pragma pack(8)
#else
#pragma pack(4)
#endif
#elif (defined(DB2AIX) && defined(__64BIT__))
#pragma options align=natural
#elif (defined(DB2AIX))
#pragma options align=power
#endif

/* SQLDA: SQL Descriptor Area - Variable descriptor                         */

SQL_STRUCTURE  sqlname                  /* Variable Name                    */
{
    short          length;              /* Name length [1..30]              */
    _SQLOLDCHAR    data[30];            /* Variable or Column name          */
};

SQL_STRUCTURE  sqldistinct_type         /* Name of user-defined type        */
{
    short          length;              /* Name length [1..27]              */
    char           data[27];            /* Name of user-defined type        */
    char           reserved1[3];        /* reserved                         */
};

/* Structure for any user-defined types:                                    */
/*   Distinct types, structured types, and reference types                  */
typedef SQL_STRUCTURE sqldistinct_type sqluser_def_type;

SQL_STRUCTURE  sqlvar                   /* Variable Description             */
{
    short          sqltype;             /* Variable data type               */
    short          sqllen;              /* Variable data length             */
    _SQLOLDCHAR   *SQL_POINTER sqldata; /* Pointer to variable data value   */
    short         *SQL_POINTER sqlind;  /* Pointer to Null indicator        */
    struct sqlname sqlname;             /* Variable name                    */
};

SQL_STRUCTURE  sqlda
{
    _SQLOLDCHAR    sqldaid[8];          /* Eye catcher = 'SQLDA   '         */
    /************************************************************************/
    /* The 7th byte has special meaning.  If it is a '2', this means there  */
    /* are twice as many sqlvars as there are host variables or columns.    */
    /* This is typically used when Large Objects (LOBs) or User Defined     */
    /* Types (UDFs) are present in the SQLDA.  The first N entries use the  */
    /* sqlvar structure and the second N entries use the sqlvar2 structure. */
    /************************************************************************/
    sqlint32       sqldabc;             /* SQLDA size in bytes=16+44*SQLN   */
    short          sqln;                /* Number of SQLVAR elements        */
    short          sqld;                /* # of columns or host vars.       */
    struct sqlvar  sqlvar[1];           /* first SQLVAR element             */
};

/* macro for allocating SQLDA */
#define SQLDASIZE(n) (offsetof(struct sqlda, sqlvar) + \
                     (n) * sizeof(struct sqlvar))

/****************************************************************************/
/* Because we may need to support 8 byte lengths in the future,             */
/* sql8bytelen is defined using 8 bytes.  Currently, however, we have only  */
/* four byte integers.                                                      */
/****************************************************************************/
union sql8bytelen
{
    sqlint32    reserve1[2];    /* reserved for future 8 byte lengths.      */
    sqlint32    sqllonglen;     /* this is what is currently used           */
};

union sql4bytelen
{
    unsigned char reserve1[4];    /* reserved                               */
    sqlint32      sqllonglen;     /* this is what is currently used         */
};

/****************************************************************************/
/* The sqlvar2 structure maps the second "N" sqlvar entries.  The           */
/* second "N" entries are used to hold the length of LOB columns            */
/* and host variables.  The second "N" entries may also be used to          */
/* hold the SQLDATALEN field for LOB columns on a FETCH USING               */
/* DESCRIPTOR request.                                                      */
/*                                                                          */
/* To set or retrieve these fields, use the macros provided below.          */
/****************************************************************************/

#ifdef db2Is64bit
#define SQLVAR2_PAD 8
#else
#define SQLVAR2_PAD 0
#endif

SQL_STRUCTURE  sqlvar2          /* Variable Description                     */
{
   union sql4bytelen len;       /* Four byte length.                        */
   unsigned char reserve2[3+SQLVAR2_PAD];   /* Reserved                     */
   unsigned char sqlflag4;      /* Indicates type of Var (see flag values   */
                                /* below)                                   */
   char *SQL_POINTER sqldatalen;/* Pointer to four (4) byte                 */
                                /* length buffer. This may be               */
                                /* used to hold the length for              */
                                /* lob data types.                          */
   sqluser_def_type sqldatatype_name; /* User-defined type name             */
};

/* 
 * Use of reinterpret_cast<> here is so C++ compilers won't object to the 
 * conversion of struct sqlvar * to struct sqlvar2 *.
 */
#if defined __cplusplus && __cplusplus >= 199707L
#define db2Sqlvar2Convert(e) (reinterpret_cast<struct sqlvar2 *>((e)))
#else
#define db2Sqlvar2Convert(e) ((struct sqlvar2 *)(e))
#endif

/****************************************************************************/
/* Flag values for sqlflag4 field of sqlvar2.                               */
/****************************************************************************/

#define SQLFLAG4_BASE    0x00  /* Type is base type or distinct type        */
#define SQLFLAG4_REF     0x01  /* Reference type                            */
#define SQLFLAG4_STRUCT  0x12  /* Structured type                           */

/****************************************************************************/
/* Macros for using the sqlvar2 fields.                                     */
/****************************************************************************/

/****************************************************************************/
/* A '2' in the 7th byte of sqldaid indicates a doubled amount of sqlvars.  */
/****************************************************************************/
#define   SQLDOUBLED  '2'
#define   SQLSINGLED  ' '

/****************************************************************************/
/* GETSQLDOUBLED(daptr) will return 1 if the SQLDA pointed to by daptr      */
/* has been doubled, or 0 if it has not been doubled.                       */
/****************************************************************************/
#define GETSQLDOUBLED(daptr) (((daptr)->sqldaid[6] == ( char) SQLDOUBLED) ? \
    (1)         : \
    (0)           )

/****************************************************************************/
/* SETSQLDOUBLED(daptr, SQLDOUBLED) will make the 7th byte of sqldaid to    */
/* be a '2'.                                                                */
/* SETSQLDOUBLED(daptr, SQLSINGLED) will make the 7th byte of sqldaid to    */
/* be a ' '.                                                                */
/****************************************************************************/
#define SETSQLDOUBLED(daptr, newvalue) (((daptr)->sqldaid[6] = (newvalue)))

/****************************************************************************/
/* GETSQLDALONGLEN(daptr,n) will return the data length of the nth entry    */
/* in the sqlda pointed to by daptr. Use this only if the sqlda was         */
/* doubled and the nth SQLVAR entry has a LOB datatype.                     */
/****************************************************************************/
#define GETSQLDALONGLEN(daptr,n) \
   ((db2Sqlvar2Convert(&((daptr)->sqlvar[(n) + ((daptr)->sqld)])))->len.sqllonglen)

/****************************************************************************/
/* SETSQLDALONGLEN(daptr,n,len) will set the sqllonglen field of the        */
/* sqlda pointed to by daptr to len for the nth entry. Use this only if     */
/* the sqlda was doubled and the nth SQLVAR entry has a LOB datatype.       */
/****************************************************************************/
#define SETSQLDALONGLEN(daptr,n,length)  { \
   struct sqlvar2     *var2ptr; \
   var2ptr = db2Sqlvar2Convert(&((daptr)->sqlvar[(n) + ((daptr)->sqld)])); \
   var2ptr->len.sqllonglen  =  (sqlint32) (length); \
   }

/****************************************************************************/
/* GETSQLDALENPTR(daptr,n) will return a pointer to the data length for     */
/* the nth entry in the sqlda pointed to by daptr.  Unlike the inline       */
/* value (union sql8bytelen len), which is 8 bytes, the sqldatalen pointer  */
/* field returns a pointer to a sqlint32 (4 byte) integer.                      */
/* If the SQLDATALEN pointer is zero, a NULL pointer will be returned.      */
/*                                                                          */
/* NOTE: Use this only if the sqlda has been doubled.                       */
/****************************************************************************/
#define GETSQLDALENPTR(daptr,n) \
   ((sqlint32 *) (db2Sqlvar2Convert(&(daptr)->sqlvar[(n) + (daptr)->sqld]))->sqldatalen)

/****************************************************************************/
/* SETSQLDALENPTR(daptr,n,ptr) will set a pointer to the data length for    */
/* the nth entry in the sqlda pointed to by daptr.                          */
/* Use this only if the sqlda has been doubled.                             */
/****************************************************************************/
#define SETSQLDALENPTR(daptr,n,ptr)  {  \
   struct sqlvar2 *var2ptr;    \
   var2ptr = db2Sqlvar2Convert(&((daptr)->sqlvar[(n) + ((daptr)->sqld)])); \
   var2ptr->sqldatalen  = (char *) ptr; \
   }

/****************************************************************************/
/* GETSQLDAFLAG4(daptr,n) will return the type character flag (sqlflag4)    */
/* from the nth entry in the sqlda pointed to by daptr.                     */
/* Use this only if the sqlda was doubled.                                  */
/****************************************************************************/

#define GETSQLDAFLAG4(daptr,n) ( \
    (char) ((db2Sqlvar2Convert( &((daptr)->sqlvar[(n) + \
                               ((daptr)->sqld)])))->sqlflag4) )

/****************************************************************************/
/* SETSQLDAFLAG4(daptr,n,flag) will set the type character flag (sqlflag4)  */
/* from the nth entry in the sqlda pointed to by daptr.                     */
/* Use this only if the sqlda was doubled.                                  */
/****************************************************************************/

#define SETSQLDAFLAG4(daptr,n,flag) { \
   struct sqlvar2 *var2ptr; \
   var2ptr = db2Sqlvar2Convert(&((daptr)->sqlvar[(n) + ((daptr)->sqld)])); \
   var2ptr->sqlflag4 = (char) (flag); \
   }

/****************************************************************************/
/* Flag values for SQL subtype field of sqlvar (sqlname.data[4])            */
/* This field is only valid is sqlname.length is 8 and the first two bytes  */
/* of sqlname.data are 0x0000.                                              */
/****************************************************************************/
#define SQLSUBTYPE_NONE 0x00
#define SQLSUBTYPE_XML  0x01

/****************************************************************************/
/* Macros for accessing SQL subtype information from sqlvar.sqlname fields  */
/****************************************************************************/

/****************************************************************************/
/* GETSQLSUBTYPE(daptr,n) will return the value of the SQL Subtype          */
/* Indicator of the nth entry in the sqlda pointed to by daptr.             */
/****************************************************************************/
#define GETSQLSUBTYPE(daptr,n)                                       \
    ( ( ( ((daptr)->sqlvar[(n)]).sqlname.length == (short) 8 )           \
      &&( ((daptr)->sqlvar[(n)]).sqlname.data[0] == (char) 0x00 )        \
      &&( ((daptr)->sqlvar[(n)]).sqlname.data[1] == (char) 0x00 ) ) ?    \
    ( (char) ( ((daptr)->sqlvar[(n)]).sqlname.data[4] ) ) :              \
    ( (char) SQLSUBTYPE_NONE ) )

/****************************************************************************/
/* SETSQLSUBTYPE(daptr,n, val) will set the SQL subtype value in the        */
/* sqlname field of the sqlda pointed to by daptr to val for the nth entry. */
/****************************************************************************/
#define SETSQLSUBTYPE(daptr,n,val)  {                    \
    ((daptr)->sqlvar[(n)]).sqlname.length = (short) 8;       \
    ((daptr)->sqlvar[(n)]).sqlname.data[0] = (char) 0x00;    \
    ((daptr)->sqlvar[(n)]).sqlname.data[1] = (char) 0x00;    \
    ((daptr)->sqlvar[(n)]).sqlname.data[4] = (char) (val);   \
    }

/****************************************************************************/
/* DB2 INTERNAL USE ONLY.                                                   */
/****************************************************************************/
union sqldaU
{
   _SQLOLDCHAR    sqldaid[8];
   struct sqlda   orig;
};

/****************************************************************************/
/* DB2 INTERNAL USE ONLY.                                                   */
/****************************************************************************/
union sqlvarPU
{
   struct sqlvar * pOrigVar;
};

/****************************************************************************/
/* DB2 INTERNAL USE ONLY.                                                   */
/****************************************************************************/
union sqlvar2PU
{
   struct sqlvar2  * pOrigVar2;
};

#if defined(DB2NT)
#pragma pack()
#elif defined(DB2AIX)
#pragma options align=reset
#endif

#endif /* SQLDASIZE */
