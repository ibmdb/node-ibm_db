/******************************************************************************
** 
** Source File Name: SQLENV
** 
** (C) COPYRIGHT International Business Machines Corp. 1987, 1997
** All Rights Reserved
** Licensed Materials - Property of IBM
** 
** US Government Users Restricted Rights - Use, duplication or
** disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
** 
** Function = Include File defining:
**              Environment Commands - Constants
**              Environment Commands - Structures
**              Environment Commands - Function Prototypes
**              Environment Commands - Labels for SQLCODES
** 
** Operating System:LINUX
** 
*******************************************************************************/
#ifndef SQL_H_SQLENV
#define SQL_H_SQLENV

#ifdef __cplusplus
extern "C" {
#endif

/* Note: _SQLOLDCHAR defaults to 'char'.  See sqlsystm.h for details.         */

#include "sqlsystm.h"                  /* System dependent defines  */

/* Required Include Files */

#include "sql.h"                       /* Includes sqlsystm & sqlca */
#include "sqlda.h"
#include <stddef.h>                    /* needed for offsetof */


/* Interface Parameters/Structures/Return Codes                               */

/* Parameters for START USING DATABASE                                        */
#define SQL_USE_SHR            'S'     /* USE = Share                         */
#define SQL_USE_EXC            'X'     /* USE = Exclusive                     */
#define SQL_USE_EXC_SN         'N'     /* USE = Exclusive on Single Node      */

/* Sizes for entries in sqledinfo, sqleninfo, sqledbstat, sql_dir_entry,      */
/* sqle_reg_nwbindery                                                         */
#define SQL_ALIAS_SZ           8       /* Alias name                          */
/* SQL_USERID_SZ and SQL_MAX_USERID_SZ have been deprecated but are kept to   */
/* preserve compatibility. They may be removed in future release. Customers   */
/* should define their own constants according to section <User, user ID and  */
/* group naming rules> in the DB2 Information Center.                         */
#define SQL_USERID_SZ          30      /* User Id                             */
#define SQL_MAX_USERID_SZ      1024    /* Max User Id Size                    */
#define SQL_AUTHID_SZ          30      /* Auth Id                             */
#define SQL_DBNAME_SZ          8       /* Database name                       */
#define SQL_PLUGIN_NAME_SZ     32      /* plugin name                         */
#define SQL_SRVCON_GSSPLUGIN_LIST_SZ 255 /* GSS API plugin list               */
#define SQL_COMM_EXIT_NAME_SZ  32      /* communication buffer exit library   */
                                       /* name                                */
#define SQL_COMM_EXIT_LIST_SZ  128     /* communication buffer exit library   */
                                       /* list                                */

/* SQL_DRIVE_SZ has been deprecated but is kept to preserve compatibility.It  */
/* will be removed in future release. For new applications, use SQL_DB_PATH   */
/* SZ.                                                                        */
#define SQL_DRIVE_SZ_DB2OS2    2       /* Drive (Qualified) - OS/2            */
#define SQL_DRIVE_SZ_DB2DOS    2       /* Drive (Qualified) - Windows         */
#define SQL_DRIVE_SZ_DB2WINT   12      /* Drive (Qualified) - NT              */
#define SQL_DRIVE_SZ_DB2AIX    215     /* Path (Qualified) - AIX/UNIX         */
#define SQL_DRIVE_SZ           SQL_DRIVE_SZ_DB2AIX

#define SQL_DB_PATH_SZ         215     /* Database path (Qualified)           */

#define SQL_STORAGEPATH_SZ     175     /* Storage path                        */

#define SQL_INAME_SZ           8       /* Internal db name                    */
#define SQL_NNAME_SZ           8       /* Node name                           */
#define SQL_INSTNAME_SZ        8       /* Instance Name                       */
#define SQL_DBTYP_SZ           20      /* Type of database                    */
#define SQL_CMT_SZ             30      /* Comment                             */
#define SQL_LOCLU_SZ           8       /* Local_lu                            */
#define SQL_RMTLU_SZ           8       /* Partner_lu                          */
#define SQL_MODE_SZ            8       /* Mode                                */
#define SQL_TPNAME_SZ          64      /* length of tpname                    */
#define SQL_NRESERVE_SZ        0       /* Reserved                            */
#define SQL_DBSTAT_ALIAS_SZ    16      /* Alias name                          */
#define SQL_DBSTAT_DBNAME_SZ   16      /* Database name                       */
#define SQL_LONG_NAME_SZ       18      /* Host database name                  */
#define SQL_CS_SZ              256     /* collating sequence                  */
#define SQL_PARAMETER_SZ       512     /* Parameter string                    */
#define SQL_NETID_SZ           8       /* Network ID                          */
#define SQL_PATH_SZ            1024    /* Maximum Path size                   */
#define SQL_HOSTNAME_SZ        255     /* Host Name                           */
#define SQL_COMPUTERNAME_SZ    15      /* Computer Name                       */
#define SQL_PROFILE_SZ         235     /* Profile Name                        */
#define SQL_OPTION_SZ          10      /* Option Name                         */
#define SQL_DCEPRIN_SZ         1024    /* DCE principal name size             */
#define SQL_SERVICE_NAME_SZ    14      /* Service Name                        */
#define SQL_TPMON_NAME_SZ      19      /* TP Monitor Name                     */
#define SQL_SYM_DEST_NAME_SZ   8       /* Symbolic Destination Name           */
#define SQL_TMDATABASE_NAME_SZ 8       /* TM Database Name                    */
#define SQL_AR_SZ              32      /* AR Library Name                     */
#define SQL_SYSTEM_NAME_SZ     21      /* System Name                         */
#define SQL_REMOTE_INSTNAME_SZ 8       /* Remote Instance Name                */
#define SQL_IPXADDRESS_SZ      27      /* IPX address size                    */
#define SQL_LANADDRESS_SZ      12      /* Lan adapter address                 */
#define SQL_LDAP_DN_SZ         1000    /* LDAP Distinguished Name             */
#define SQL_LDAP_PSWD_SZ       256     /* LDAP Password                       */

#define SQL_DFTDRDAACT_SZ      25      /* Default DRDA accounting string.     */
#define SQL_DIR_NAME_SZ        255     /* Dir/Route Path/Obj Name             */
#define SQL_CLIENT_COMM_SZ     31      /* Client Comm. Protocol               */
#define SQL_SYSADM_SZ          8       /* Sysadm Group name                   */
/* Group name limits have been increased to support 30 characters in v8.2.    */
/* This will be further increased to 128 characters in a future release.      */
#define SQL_SYSADM_GROUP_SZ    128     /* Future Group name length            */
#define SQL_FILESERVER_SZ      48      /* Netware fileserver name             */
#define SQL_OBJECTNAME_SZ      48      /* Netware bindery name                */
#define SQL_IPXSOCKET_SZ       4       /* Netware IPX socket                  */
#define SQL_NW_UID_SZ          48      /* Netware userid                      */
#define SQL_NW_PSWD_SZ         128     /* Netware password                    */
#define SQL_COLLATE_INFO_SZ    260     /* Collate info size (256 + 4)         */
#define SQL_ACCOUNT_STR_SZ     199     /* Max accounting string               */
#define SQL_DSCVRCOMM_SZ       35      /* Discover protocols string size      */
#define SQL_CA_XPORT_DEV_SZ    15      /* CA Transport Device Name            */
#define SQL_LICVERSION_SZ      8       /* Connect licence version             */

/* After an SQL CONNECT, the 5th token in the SQLCA identifies the DBMS an    */
/* application has connected to.  The following are values returned from IBM  */
/* DBMS.                                                                      */
#define SQL_DBMS_ES10_OS2      "QOS/2 DBM"
#define SQL_DBMS_DB2_OS2       "QDB2/2"
#define SQL_DBMS_DB2_NT        "QDB2/NT"
#define SQL_DBMS_DB2_NT64      "QDB2/NT64"
#define SQL_DBMS_DB2_95        "QDB2/Windows 95"
#define SQL_DBMS_DB2_AIX       "QDB2/6000"
#define SQL_DBMS_DB2_AIX64     "QDB2/AIX64"
#define SQL_DBMS_DB2_AIX_PE    "QDB2/6000 PE"
#define SQL_DBMS_DB2_HPUX      "QDB2/HPUX"
#define SQL_DBMS_DB2_HPUX64    "QDB2/HP64"
#define SQL_DBMS_DB2_HPUXIA    "QDB2/HPUX-IA"
#define SQL_DBMS_DB2_HPUXIA64  "QDB2/HPUX-IA64"
#define SQL_DBMS_DB2_SUN       "QDB2/SUN"
#define SQL_DBMS_DB2_SUN64     "QDB2/SUN64"
#define SQL_DBMS_DB2_SUNX86    "QDB2/SUNX86"
#define SQL_DBMS_DB2_SUNX8664  "QDB2/SUNX8664"
#define SQL_DBMS_DB2_SNI       "QDB2/SNI"
#define SQL_DBMS_DB2_SCO       "QDB2/SCO"
#define SQL_DBMS_DB2_SGI       "QDB2/SGI"
#define SQL_DBMS_DB2_LINUXX8664 "QDB2/LINUXX8664"
#define SQL_DBMS_DB2_LINUXPPC64 "QDB2/LINUXPPC64"
#define SQL_DBMS_DB2_LINUXPPC64LE "QDB2/LINUXPPC64LE"
#define SQL_DBMS_DB2_LINUXPPC  "QDB2/LINUXPPC"
#define SQL_DBMS_DB2_LINUXIA64 "QDB2/LINUXIA64"
#define SQL_DBMS_DB2_INSPURKUXI64 "QDB2/INSPURKUXI64"
#define SQL_DBMS_DB2_LINUXZ64  "QDB2/LINUXZ64"
#define SQL_DBMS_DB2_LINUX390  "QDB2/LINUX390"
#define SQL_DBMS_DB2_LINUX     "QDB2/LINUX"
#define SQL_DBMS_DB2_DYNIX     "QDB2/PTX"
#define SQL_DBMS_DB2_MVS       "QDB2"
#define SQL_DBMS_DB2_DARWIN    "QDB2/DARWIN"
#define SQL_DBMS_DB2_DARWIN64  "QDB2/DARWIN64"
#define SQL_DBMS_OS400         "QAS"
#define SQL_DBMS_SQLDS_VM      "QSQLDS/VM"
#define SQL_DBMS_SQLDS_VSE     "QSQLDS/VSE"
#define SQL_DBMS_LU62_SPM      "QLU62SPM"
#define SQL_DBMS_IDS_UNIX32    "IDS/UNIX32"
#define SQL_DBMS_IDS_UNIX64    "IDS/UNIX64"
#define SQL_DBMS_IDS_NT32      "IDS/NT32"
#define SQL_DBMS_IDS_NT64      "IDS/NT64"

/* Parameters for Entry Type in sqledinfo                                     */
#define SQL_LDAP               '4'     /* Database is LDAP                    */
#define SQL_DCE                '3'     /* Database is DCE                     */
#define SQL_HOME               '2'     /* Database is Home                    */
#define SQL_REMOTE             '1'     /* Database is Remote                  */
#define SQL_INDIRECT           '0'     /* Database is Indirect                */

/* Parameters for adapter number in sqlectnd                                  */
#define SQL_ADAPTER_0          0       /* Adapter number 0                    */
#define SQL_ADAPTER_1          1       /* Adapter number 1                    */
#define SQL_ADAPTER_MIN        0       /* Minimum adapter number              */
#define SQL_ADAPTER_MAX        255     /* Maximum adapter number              */

/* Definitions of constants used for Structure IDs                            */
#define SQL_DCS_STR_ID         0x100   /* DCS directory entry id              */
#define SQL_NODE_STR_ID        0x200   /* Catalog node struct id              */

/* Parameters for protocol types in sqlectnd                                  */
#define SQL_PROTOCOL_APPC      0x0     /* APPC                                */
#define SQL_PROTOCOL_NETB      0x1     /* NETBIOS                             */
#define SQL_PROTOCOL_APPN      0x2     /* APPN                                */
#define SQL_PROTOCOL_TCPIP     0x3     /* TCP/IP v4 or v6                     */
#define SQL_PROTOCOL_CPIC      0x4     /* APPC using CPIC                     */
#define SQL_PROTOCOL_IPXSPX    0x5     /* IPX/SPX                             */
#define SQL_PROTOCOL_LOCAL     0x6     /* Local IPC                           */
#define SQL_PROTOCOL_NPIPE     0x7     /* Named Pipe                          */
#define SQL_PROTOCOL_SOCKS     0x8     /* TCP/IP using SOCKS                  */
#define SQL_PROTOCOL_TCPIP4    0x9     /* TCP/IPv4                            */
#define SQL_PROTOCOL_TCPIP6    0xa     /* TCP/IPv6                            */
#define SQL_PROTOCOL_SOCKS4    0xb     /* TCP/IPv4 using SOCKS                */
#define SQL_PROTOCOL_SSL       0xc     /* TCP/IP using SSL                    */
#define SQL_PROTOCOL_SSL4      0xd     /* IPv4 using SSL                      */
#define SQL_PROTOCOL_SSL6      0xe     /* IPv6 using SSL                      */
#define SQL_PROTOCOL_MAX       0xe     /* Max supported protocol              */

/* Security Type for APPC using CPIC                                          */
#define SQL_CPIC_SECURITY_NONE         0    /* None                           */
#define SQL_CPIC_SECURITY_SAME         1    /* Same                           */
#define SQL_CPIC_SECURITY_PROGRAM      2    /* Program                        */

/* Security Type for TCP/IP                                                   */
#define SQL_TCPIP_SECURITY_NONE        0    /* None                           */
#define SQL_TCPIP_SECURITY_SOCKS       1    /* TCP/IP SOCKS Support           */
#define SQL_TCPIP_SECURITY_SSL         2    /* TCP/IP SSL Support             */

/* Authentication Types                                                       */
#define SQL_AUTHENTICATION_SERVER      0    /* Authenticate on Server         */
#define SQL_AUTHENTICATION_CLIENT      1    /* Authenticate on Client         */
#define SQL_AUTHENTICATION_DCS         2    /* Authenticate via DDCS          */
#define SQL_AUTHENTICATION_DCE         3    /* Authenticate via DCE           */
#define SQL_AUTHENTICATION_SVR_ENCRYPT 4    /* Auth at Server with encrypt    */
#define SQL_AUTHENTICATION_DCS_ENCRYPT 5    /* Auth via DDCS with encrypt     */
#define SQL_AUTHENTICATION_DCE_SVR_ENC 6    /* Auth via DCE or Server with    */
                                            /* Encrypt (Valid only at         */
                                            /* server)                        */
#define SQL_AUTHENTICATION_KERBEROS    7    /* Auth via Kerberos              */
#define SQL_AUTHENTICATION_KRB_SVR_ENC 8    /* Auth via Kerberos or Server    */
                                            /* with Envrypt (Valid only at    */
                                            /* the server                     */
#define SQL_AUTHENTICATION_GSSPLUGIN   9    /* Auth via GSS API plugin        */
#define SQL_AUTHENTICATION_GSS_SVR_ENC 10   /* Auth via GSS API plugin or     */
                                            /* Server with Encrypt (Valid     */
                                            /* only at server)                */
#define SQL_AUTHENTICATION_DATAENC     11   /* Auth at server with encrypted  */
                                            /* data                           */
#define SQL_AUTHENTICATION_DATAENC_CMP 12   /* Auth at server with or         */
                                            /* without encrypted data (Valid  */
                                            /* only at server)                */
#define SQL_AUTHENTICATION_SVRENC_AESO 13   /* Same as SQL_AUTHENTICATION     */
                                            /* SVR_ENCRYPT but will only      */
                                            /* encrypt with AES encryption.   */
#define SQL_AUTHENTICATION_CERTIFICATE 14   /* Auth using Transport Layer     */
                                            /* Security Client Certificate    */
                                            /* Authentication Security        */
                                            /* mechanism                      */
#define SQL_AUTHENTICATION_NOT_SPEC    255  /* Authentication Not Specified   */

/* Alternate Authentication Encryption Algorithm                              */
#define SQL_ALTERNATE_AUTH_ENC_AES 0   /* Alternate auth via Server with      */
                                       /* Encrypt using AES algorithm only    */
                                       /* (Valid only at server)              */
#define SQL_ALTERNATE_AUTH_ENC_AES_CMP 1 /* Alternate auth via Server with    */
                                       /* Encrypt using AES or DES algorithm  */
                                       /* (Valid only at server)              */
#define SQL_ALTERNATE_AUTH_ENC_NOTSPEC 255 /* Alternate Authentication Not    */
                                       /* Specified                           */

/* Data Flow Encryption Types                                                 */
#define SQL_NO_ENCRYPT         1       /* No encryption for connection        */
#define SQL_DH_DES_56          2       /* DH_DES_56                           */
#define SQL_DH_3DES_128        3       /* DH_3DES_128                         */
#define SQL_DH_3DES_192        4       /* DH_3DES_192                         */

/* Defines for CF transport method                                            */
#define SQL_CF_TRANSPORT_RDMA  0       /* Using RDMA                          */
#define SQL_CF_TRANSPORT_TCP   1       /* Using TCP                           */

/* Parameters for Create Database API Collating Sequences                     */
#define SQL_CS_SYSTEM          0       /* Coll. Seq. from System              */
#define SQL_CS_USER            -1      /* Coll. Seq. from User                */
#define SQL_CS_NONE            -2      /* Coll. Seq. (None)                   */
#define SQL_CS_COMPATIBILITY   -4      /* Coll. Seq. from pre-v5              */
#define SQL_CS_SYSTEM_NLSCHAR  -8      /* SYSTEM table + NLS function         */
#define SQL_CS_USER_NLSCHAR    -9      /* USER table + NLS function           */
#define SQL_CS_IDENTITY_16BIT  -10     /* UTF-8S collation                    */
#define SQL_CS_UCA400_NO       -11     /* Unicode Collation Algorithm V400    */
                                       /* Normalization ON collation          */
#define SQL_CS_UCA400_LTH      -12     /* Unicode Collation Algorithm V400    */
                                       /* for Thai collation                  */
#define SQL_CS_UCA400_LSK      -13     /* Unicode Collation Algorithm V400    */
                                       /* for Slovakian collation             */
#define SQL_CS_UNICODE         -14     /* Codeset aware Unicode Collation     */

/* Defines for iQOptions field of db2InsQuiesceStruct and iQOptions field of  */
/* db2QuiesceStartStruct                                                      */
#define DB2INSQUIESCE_RESTRICTEDACCESS 0x1 /* Instance quiesced in            */
                                       /* restricted access mode              */

/* Defines for Start Database Manager OPTION parameter and Stop Database      */
/* Manager OPTION and CALLERAC parameters                                     */
#define SQLE_NONE              0x0
#define SQLE_FORCE             0x1
#define SQLE_DROP              0x2
#define SQLE_CONTINUE          0x4
#define SQLE_TERMINATE         0x8
#define SQLE_ADDNODE           0x10
#define SQLE_RESTART           0x20
#define SQLE_STANDALONE        0x40
#define SQLE_RESTART_LIGHT     0x80
#define SQLE_HOST              0x100
#define SQLE_QUIESCE           0x10000
/* From here on, internal options only                                        */
#define SQLE_SKELETON          0x200
#define SQLE_CM_INVOKE         0x400
#define SQLE_MEMBER            0x800
#define SQLE_CF                0x1000
#define SQLE_NODENUM           0x2000
#define SQLE_ADDCA             0x4000
#define SQLE_DROPCA            0x8000
#define SQLE_UPDATE            0x20000

/* Defines for Start Database Manager & Add Node Tablespace type              */
#define SQLE_TABLESPACES_NONE  0       /* No Temp Tablespaces                 */
#define SQLE_TABLESPACES_LIKE_NODE 1   /* Temp Tablespaces like Specified     */
                                       /* Node                                */
#define SQLE_TABLESPACES_LIKE_CATALOG 2 /* Temp Tablespaces like Catalog      */
                                       /* Node                                */

/* Parameters for Drop Node Action                                            */
#define SQL_DROPNODE_VERIFY    1       /* Verify                              */

/* Parameters for indicating the stored procedure invocation was via the      */
/* CALL statement                                                             */
#define SQL_CALLPROC           "$SQL$CAL"

/* Default values for Segmented Tables parameters                             */
#define SQL_DEF_NUMSEGS        1       /* Default number of segments-nonOS2   */
#define SQL_DEF_SEGPAGES       76800   /* OBSOLETE: Default max pages per     */
                                       /* seg                                 */
#define SQL_DEF_EXTSIZE        32      /* default extent size                 */
#define SQL_DEF_EXTSIZE_ANALYTICS 4    /* default analytics extent size       */

/* DUOW Connection Setting types used by sqleqryc() and                       */
/* sqlesetc().Associated values are in sql.h, used in common with the         */
/* precompiler.                                                               */
#define SQL_CONNECT_TYPE       1       /* Connect type                        */
#define SQL_RULES              2       /* Rules for changing connections      */
#define SQL_DISCONNECT         3       /* Disconnect type at syncpoint        */
#define SQL_SYNCPOINT          4       /* Syncpoint type                      */
#define SQL_MAX_NETBIOS_CONNECTIONS 5  /* Maximum concurrent connections      */
#define SQL_DEFERRED_PREPARE   6       /* Deferred Prepare                    */
#define SQL_CONNECT_NODE       7       /* Node to connect to                  */
#define SQL_ATTACH_NODE        8       /* Node to attach to                   */

/* SET CLIENT INFORMATION types used by sqleseti() and sqleqryi().            */
#define SQLE_CLIENT_INFO_USERID 1      /* Client user name                    */
#define SQLE_CLIENT_INFO_WRKSTNNAME 2  /* Client workstation name             */
#define SQLE_CLIENT_INFO_APPLNAME 3    /* Client application name             */
#define SQLE_CLIENT_INFO_ACCTSTR 4     /* Client accounting string            */
#define SQLE_CLIENT_INFO_PROGRAMID 5   /* Client programid identifier         */
#define SQLE_CLIENT_INFO_AUTOCOMMIT 6  /* Client autocommit                   */
#define SQLE_CLIENT_INFO_CORR_TOKEN 7  /* Client correlation token            */

/* Constants to be used to set AUTOCOMMIT.                                    */
#define SQLE_CLIENT_AUTOCOMMIT_ON 'Y'
#define SQLE_CLIENT_AUTOCOMMIT_OFF 'N'

/* SET CLIENT INFORMATION types maximum information lengths                   */
#define SQLE_CLIENT_USERID_MAX_LEN 255 /* Maximum client user information     */
                                       /* length                              */
#define SQLE_CLIENT_WRKSTNNAME_MAX_LEN 255 /* Maximum client workstation      */
                                       /* information length                  */
#define SQLE_CLIENT_APPLNAME_MAX_LEN 255 /* Maximum client application        */
                                       /* information length                  */
#define SQLE_CLIENT_ACCTSTR_MAX_LEN 255 /* Maximum client accounting          */
                                       /* information length                  */
#define SQLE_CLIENT_PROGRAMID_MAX_LEN 80 /* Maximum client program            */
                                       /* identifier length                   */
#define SQLE_CLIENT_AUTOCOMMIT_MAX_LEN 1 /* Maximum length for autocommit     */
#define SQLE_CLIENT_CORR_TOKEN_MAX_LEN 255 /* Maximum length for client       */
                                       /* correlation token                   */

#define SQL_PARM_UNCHANGE      'U'

/* A structure to identify an SQL statement.  The eyecatcher $SQL$CALL        */
/* identifies this as an EXEC SQL CALL statement.                             */
/******************************************************************************
** SQLSTMTID data structure
*******************************************************************************/
SQL_STRUCTURE SQLSTMTID
{
        char                   sqlcstid[8]; /* eye catcher                    */
        char                   sqluser[128]; /* creator id                    */
        char                   pkgname[8]; /* package name                    */
        char                   contoken[8]; /* consistency token              */
        short                  sectnum; /* reserved                           */
        char                   wchar_info; /* reserved                        */
        char                   fCacheRows; /* reserved                        */
        char                   buffer[38]; /* reserved                        */
};


/* Database Description Block structure                                       */

/* TableSpace types                                                           */
#define SQL_TBS_TYP_SMS        'S'
#define SQL_TBS_TYP_DMS        'D'
#define SQL_TBS_TYP_AUTO       'A'

/* TableSpace container types                                                 */
#define SQL_TBSC_TYP_PATH      'P'     /* path (directory)  SMS only          */
#define SQL_TBSC_TYP_DEV       'D'     /* device (raw disk) DMS only          */
#define SQL_TBSC_TYP_FILE      'F'     /* file (cooked file) DMS only         */

#define SQLE_DBDESC_2          "SQLDBD02" /* version 2 database descriptor    */
#define SQLE_DBTSDESC_1        "SQLTS001" /* version 2 tableSpace descriptor  */

/* Initial TableSpace names                                                   */
#define SQLCATTS_NAME          "SYSCATSPACE" /* system catalogs               */
#define SQLUSRTS_NAME          "USERSPACE1" /* user tables                    */
#define SQLTMPTS_NAME          "TEMPSPACE1" /* temp tables                    */

/* tableSpace container descriptor                                            */
/******************************************************************************
** SQLETSCDESC data structure
** Table: Fields in the SQLETSCDESC Structure
** |--------------------------------------------------------------------|
** |Field Name|Data Type | Description                                  |
** |----------|----------|----------------------------------------------|
** |SQLCTYPE  |CHAR(1)   | Identifies the type of this container. See   |
** |          |          | below for values.                            |
** |----------|----------|----------------------------------------------|
** |SQLCSIZE  |INTEGER   | Size of the container identified in SQLCONTR,|
** |          |          | specified in 4KB pages. Valid only when      |
** |          |          | SQLTSTYP is set to SQL_TBS_TYP_DMS.          |
** |----------|----------|----------------------------------------------|
** |SQLCLEN   |SMALLINT  | Length of following SQLCONTR value.          |
** |----------|----------|----------------------------------------------|
** |SQLCONTR  |CHAR(256) | Container string.                            |
** |----------|----------|----------------------------------------------|
** 
** pad1
** Reserved. Used for structure alignment and should not to be populated
** by user data.
** 
** pad2
** Reserved. Used for structure alignment and should not to be populated
** by user data.
** 
** Valid values for SQLCTYPE (defined in sqlenv) are:
** - SQL_TBSC_TYP_DEV 
** Device. Valid only when SQLTSTYP = SQL_TBS_TYP_DMS.
** - SQL_TBSC_TYP_FILE 
** File. Valid only when SQLTSTYP = SQL_TBS_TYP_DMS. 
** - SQL_TBSC_TYP_PATH 
** Path (directory). Valid only when SQLTSTYP = SQL_TBS_TYP_SMS. 
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQLETSCDESC.
**     05 SQLCTYPE               PIC X.
**     05 SQL-PAD1               PIC X(3).
**     05 SQLCSIZE               PIC S9(9) COMP-5.
**     05 SQLCLEN                PIC S9(4) COMP-5.
**     05 SQLCONTR               PIC X(256).
**     05 SQL-PAD2               PIC X(2).
** * 
*******************************************************************************/
SQL_STRUCTURE SQLETSCDESC
{
        char           sqlctype;                     /* container type        */
        char           pad1[3];                      /* reserved              */
        sqlint32       sqlcsize;                     /* container size (in    */
                                                     /* pages)                */
        short          sqlclen;                      /* length of container   */
                                                     /* name                  */
        char           sqlcontr[SQLB_MAX_CONTAIN_NAME_SZ]; /* container name  */
                                                     /* (includes 1 byte for  */
                                                     /* C NULL terminator)    */
        char           pad2[2];                      /* 2 bytes padding       */
                                                     /* before next           */
                                                     /* container             */
};


/* tableSpace descriptor                                                      */
/******************************************************************************
** SQLETSDESC data structure
** The tablespace description block structure (SQLETSDESC) is used to
** specify the attributes of any of the three initial table spaces.
** 
** Table: Fields in the SQLETSDESC Structure
** ---------------------------------------------------------------------
** |Field Name  | Data Type|Description                                 |
** |------------|----------|--------------------------------------------|
** |SQLTSDID    | CHAR(8)  |A structure identifier and "eye-catcher" for|
** |            |          |storage dumps. It is a string of eight bytes|
** |            |          |that must be initialized with the value of  |
** |            |          |SQLE_DBTSDESC_1 (defined in sqlenv). The    |
** |            |          |contents of this field are validated for    |
** |            |          |version control.                            |
** |------------|----------|--------------------------------------------|
** |SQLEXTNT    | INTEGER  |Table space extent size, in 4 KB pages. If a|
** |            |          |value of -1 is supplied, this field will    |
** |            |          |default to the current value of the         |
** |            |          |dft_extent_sz configuration parameter.      |
** |------------|----------|--------------------------------------------|
** |SQLPRFTC    | INTEGER  |Table space prefetch size, in 4 KB pages. If|
** |            |          |a value of -1 is supplied, this field will  |
** |            |          |default to the current value of the         |
** |            |          |dft_prefetch_sz configuration parameter.    |
** |------------|----------|--------------------------------------------|
** |SQLFSCACHING| UNSIGNED |CHAR  File system caching. If a value of 1  |
** |            |          |is supplied, file system caching will be    |
** |            |          |OFF for the current tablespace. If a value  |
** |            |          |of 0 is supplied, file system caching will  |
** |            |          |be ON for the current tablespace. Any other |
** |            |          |value will be treated as file system        |
** |            |          |caching ON.                                 |
** |------------|----------|--------------------------------------------|
** |SQLPOVHD    | DOUBLE   |Table space I/O overhead, in milliseconds.  |
** |            |          |If a value of -1 is supplied, this field    |
** |            |          |will default to an internal database manager|
** |            |          |value (currently 24.1 ms) that could change |
** |            |          |with future releases.                       |
** |------------|----------|--------------------------------------------|
** |SQLTRFRT    | DOUBLE   |Table space I/O transfer rate, in           |
** |            |          |milliseconds. If a value of -1 is supplied, |
** |            |          |this field will default to an internal      |
** |            |          |database manager value (currently 0.9 ms)   |
** |            |          |that could change with future releases.     |
** |------------|----------|--------------------------------------------|
** |SQLTSTYP    | CHAR(1)  |Indicates whether the table space is        |
** |            |          |system-managed or database-managed. See     |
** |            |          |below for values.                           |
** |------------|----------|--------------------------------------------|
** |SQLCCNT     | SMALLINT |Number of containers being assigned to the  |
** |            |          |table space. Indicates how many             |
** |            |          |SQLCTYPE/SQLCSIZE/SQLCLEN/SQLCONTR values   |
** |            |          |follow.                                     |
** |------------|----------|--------------------------------------------|
** |CONTAINR    | Array    |An array of sqlccnt SQLETSCDESC structures. |
** |------------|----------|--------------------------------------------|
** 
** Valid values for SQLTSTYP (defined in sqlenv) are:
** - SQL_TBS_TYP_SMS 
** System managed 
** - SQL_TBS_TYP_DMS 
** Database managed 
** 
** COBOL Structure
** 
** * File: sqletsd.cbl
** 01 SQLETSDESC.
**     05 SQLTSDID               PIC X(8).
**     05 SQLEXTNT               PIC S9(9) COMP-5.
**     05 SQLPRFTC               PIC S9(9) COMP-5.
**     05 SQLPOVHD               USAGE COMP-2.
**     05 SQLTRFRT               USAGE COMP-2.
**     05 SQLTSTYP               PIC X.
**     05 SQL-PAD1               PIC X.
**     05 SQLCCNT                PIC S9(4) COMP-5.
**     05 SQL-CONTAINR OCCURS 001 TIMES.
**         10 SQLCTYPE           PIC X.
**         10 SQL-PAD1           PIC X(3).
**         10 SQLCSIZE           PIC S9(9) COMP-5.
**         10 SQLCLEN            PIC S9(4) COMP-5.
**         10 SQLCONTR           PIC X(256).
**         10 SQL-PAD2           PIC X(2).
** * 
** 
*******************************************************************************/
SQL_STRUCTURE SQLETSDESC
{
        char           sqltsdid[8];                  /* eyecatcher            */
                                                     /* descriptor version    */
        sqlint32       sqlextnt;                     /* extent size (in       */
                                                     /* pages)                */
        sqlint32       sqlprftc;                     /* prefetch size (in     */
                                                     /* pages)                */
        double         sqlpovhd;                     /* i/o overhead          */
        double         sqltrfrt;                     /* i/o transfer rate     */
        char           sqltstyp;                     /* tableSpace type       */
        unsigned char  sqlfscaching;                 /* file system caching   */
                                                     /* policy                */
        short          sqlccnt;                      /* container count       */
        struct SQLETSCDESC containr[1];              /* array of container    */
                                                     /* specs                 */
};

/******************************************************************************
*******************************************************************************/
#define SQLETSDESC_SIZE(containerCount) \
(offsetof(struct SQLETSDESC, containr)+ \
(containerCount)*sizeof(struct SQLETSCDESC) )

/******************************************************************************
** sqledbdesc data structure
** The Database Description Block (SQLEDBDESC) structure can be used
** during a call to the sqlecrea API to specify permanent values for
** database attributes. These attributes include database comment,
** collating sequences, and table space definitions.
** 
** Table: Fields in the SQLEDBDESC Structure
** --------------------------------------------------------------------------
** |Field Name  | Data Type    | Description                                 |
** |------------|--------------|---------------------------------------------|
** |SQLDBDID    | CHAR(8)      | A structure identifier and "eye-catcher"    |
** |            |              | for storage dumps. It is a string of eight  |
** |            |              | bytes that must be initialized with the     |
** |            |              | value of SQLE_DBDESC_2 (defined in sqlenv). |
** |            |              | The contents of this field are validated for|
** |            |              | version control.                            |
** |------------|--------------|---------------------------------------------|
** |SQLDBCCP    | INTEGER      | The code page of the database comment. This |
** |            |              | value is no longer used by the database     |
** |            |              | manager.                                    |
** |------------|--------------|---------------------------------------------|
** |SQLDBCSS    | INTEGER      | A value indicating the source of the        |
** |            |              | database collating sequence. See below for  |
** |            |              | values.                                     |
** |            |              | Note:                                       |
** |            |              | Specify SQL_CS_NONE to specify that the     |
** |            |              | collating sequence for the database is      |
** |            |              | IDENTITY (which implements a binary         |
** |            |              | collating sequence). SQL_CS_NONE is the     |
** |            |              | default.                                    |
** |------------|--------------|---------------------------------------------|
** |SQLDBUDC    | CHAR(256)    | The nth byte of this field contains the sort|
** |            |              | weight of the code point whose underlying   |
** |            |              | decimal representation is n in the code     |
** |            |              | page of the database. If SQLDBCSS is not    |
** |            |              | equal to SQL_CS_USER, this field is ignored.|
** |------------|--------------|---------------------------------------------|
** |SQLDBCMT    | CHAR(30)     | The comment for the database.               |
** |------------|--------------|---------------------------------------------|
** |SQLDBSGP    | INTEGER      | Reserved field. No longer used.             |
** |------------|--------------|---------------------------------------------|
** |SQLDBNSG    | SHORT        | A value that indicates the number of file   |
** |            |              | segments to be created in the database. The |
** |            |              | minimum value for this field is 1 and the   |
** |            |              | maximum value for this field is 256. If a   |
** |            |              | value of -1 is supplied, this field will    |
** |            |              | default to 1.                               |
** |            |              | Note:                                       |
** |            |              | SQLDBNSG set to zero produces a default for |
** |            |              | Version 1 compatibility.                    |
** |------------|--------------|---------------------------------------------|
** |SQLTSEXT    | INTEGER      | A value, in 4KB pages, which indicates the  |
** |            |              | default extent size for each table space in |
** |            |              | the database. The minimum value for this    |
** |            |              | field is 2 and the maximum value for this   |
** |            |              | field is 256. If a value of -1 is supplied, |
** |            |              | this field will default to 32.              |
** |------------|--------------|---------------------------------------------|
** |SQLCATTS    | Pointer      | A pointer to a table space description      |
** |            |              | control block, SQLETSDESC, which defines the|
** |            |              | catalog table space. If null, a default     |
** |            |              | catalog table space based on the values of  |
** |            |              | SQLTSEXT and SQLDBNSG will be created.      |
** |------------|--------------|---------------------------------------------|
** |SQLUSRTS    | Pointer      | A pointer to a table space description      |
** |            |              | control block, SQLETSDESC, which defines the|
** |            |              | user table space. If null, a default user   |
** |            |              | table space based on the values of SQLTSEXT |
** |            |              | and SQLDBNSG will be created.               |
** |------------|--------------|---------------------------------------------|
** |SQLTMPTS    | Pointer      | A pointer to a table space description      |
** |            |              | control block, SQLETSDESC, which defines the|
** |            |              | system temporary table space. If null, a    |
** |            |              | default system temporary table space based  |
** |            |              | on the values of SQLTSEXT and SQLDBNSG will |
** |            |              | be created.                                 |
** |------------|--------------|---------------------------------------------|
** 
** pad1
** Reserved. Used for structure alignment and should not be populated by
** user data.
** 
** pad2
** Reserved. Used for structure alignment and should not be populated by
** user data.
** 
** Valid values for SQLDBCSS (defined in sqlenv) are:
** 
** - SQL_CS_SYSTEM
** Collating sequence based on the database territory.
** 
** - SQL_CS_USER 
** Collation sequence is specified by the 256-byte weight table
** supplied by the user. Each weight in the table is one byte in
** length.
** 
** - SQL_CS_NONE
** Collation sequence is IDENTITY, that is, binary code point order.
** 
** - SQLE_CS_COMPATABILITY
** Use pre-Version collating sequence.
** 
** - SQL_CS_SYSTEM_NLSCHAR 
** Collating sequence from system using the NLS version of compare
** routines for character types. This value can only be specified when
** creating a Thai TIS620-1 database.
** 
** - SQL_CS_USER_NLSCHAR
** Collation sequence is specified by the 256-byte weight table
** supplied by the user. Each weight in the table is one byte in
** length. This value can only be specified when creating a Thai
** TIS620-1 database.
** 
** - SQL_CS_IDENTITY_16BIT
** CESU-8 (Compatibility Encoding Scheme for UTF-16: 8-Bit) collation
** sequence as specified by the Unicode Technical Report #26, available
** at the Unicode Consortium web site (www.unicode.org). This value can
** only be specified when creating a Unicode database.
** 
** - SQL_CS_UCA400_NO
** UCA (Unicode Collation Algorithm) collation sequence based on the
** Unicode Standard version 4.00 with normalization implicitly set to
** 'on'. Details of the UCA can be found in the Unicode Technical
** Standard #10 available at the Unicode Consortium web site
** (www.unicode.org). This value can only be specified when creating a
** Unicode database.
** 
** - SQL_CS_UCA400_LTH
** UCA (Unicode Collation Algorithm) collation sequence based on the
** Unicode Standard version 4.00, with sorting of all Thai characters
** according to the Royal Thai Dictionary order. Details of the UCA can
** be found in the Unicode Technical Standard #10 available at the
** Unicode Consortium web site (www.unicode.org). This value can only
** be specified when creating a Unicode database.
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQLEDBDESC.
**     05 SQLDBDID               PIC X(8).
**     05 SQLDBCCP               PIC S9(9) COMP-5.
**     05 SQLDBCSS               PIC S9(9) COMP-5.
**     05 SQLDBUDC               PIC X(256).
**     05 SQLDBCMT               PIC X(30).
**     05 FILLER                 PIC X.
**     05 SQL-PAD                PIC X(1).
**     05 SQLDBSGP               PIC 9(9) COMP-5.
**     05 SQLDBNSG               PIC S9(4) COMP-5.
**     05 SQL-PAD2               PIC X(2).
**     05 SQLTSEXT               PIC S9(9) COMP-5.
**     05 SQLCATTS               USAGE IS POINTER.
**     05 SQLUSRTS               USAGE IS POINTER.
**     05 SQLTMPTS               USAGE IS POINTER.
** * 
** 
*******************************************************************************/
SQL_STRUCTURE sqledbdesc
{
        _SQLOLDCHAR    sqldbdid[8];                  /* eye catcher           */
        sqlint32       sqldbccp;                     /* comment code page     */
        sqlint32       sqldbcss;                     /* Source of collating   */
                                                     /* seq                   */
        unsigned char  sqldbudc[SQL_CS_SZ];          /* user-defined          */
                                                     /* collating seq         */
        _SQLOLDCHAR    sqldbcmt[SQL_CMT_SZ+1];       /* comment               */
        _SQLOLDCHAR    pad[1];                       /* reserved              */
        sqluint32      sqldbsgp;                     /* obsolete: use         */
                                                     /* sqltsext              */
        short          sqldbnsg;                     /* number of segments    */
        char           pad2[2];                      /* reserved              */
        sqlint32       sqltsext;                     /* default extent size   */
        struct SQLETSDESC *sqlcatts;
        struct SQLETSDESC *sqlusrts;
        struct SQLETSDESC *sqltmpts;
};


/* Tablespace descriptor extension                                            */
/******************************************************************************
** SQLETSDESCEXT data structure
** Extended table space description block (SQLETSDESCEXT) structure
** 
** The extended table space description block (SQLETSDESCEXT) structure
** is used to specify the attributes for the three initial table spaces.
** This structure is used in addition to, not instead of, the Table
** Space Description Block (SQLETSDESC) structure.
** 
** Table: Fields in the SQLETSDESCEXT Structure
** |----------------------------------------------------------------------|
** |Field name         |Data type|Description                             |
** |-------------------|---------|----------------------------------------|
** |SQLINITSIZE        |sqlint64 |Defines the initial size of each table  |
** |                   |         |space that uses automatic storage. This |
** |                   |         |field is only relevant for regular or   |
** |                   |         |large automatic storage table spaces.   |
** |                   |         |Use a value of                          |
** |                   |         |SQL_TBS_AUTOMATIC_INITSIZE for other    |
** |                   |         |table space types or if the intent is   |
** |                   |         |to have DB2 automatically determine an  |
** |                   |         |initial size.                           |
** |                   |         |Note:                                   |
** |                   |         |The actual value used by the database   |
** |                   |         |manager may be slightly smaller or      |
** |                   |         |larger than what was specified. This    |
** |                   |         |action is taken to keep sizes consistent|
** |                   |         |across containers in the table space and|
** |                   |         |the value provided may not allow for    |
** |                   |         |that consistency.                       |
** |-------------------|---------|----------------------------------------|
** |SQLINCREASESIZE    |sqlint64 |Defines the size that the database      |
** |                   |         |manager automatically increases the     |
** |                   |         |table space by when the table space     |
** |                   |         |becomes full. This field is only        |
** |                   |         |relevant for table spaces that have     |
** |                   |         |auto-resize enabled. Use a value of     |
** |                   |         |SQL_TBS_AUTOMATIC_INCSIZE if auto-resize|
** |                   |         |is disabled or if the intent is to have |
** |                   |         |the database manager determine the size |
** |                   |         |increase automatically.                 |
** |                   |         |Note:                                   |
** |                   |         |The actual value used by the database   |
** |                   |         |manager may be slightly smaller or      |
** |                   |         |larger than what was specified. This    |
** |                   |         |action is taken to keep sizes consistent|
** |                   |         |across containers in the table space and|
** |                   |         |the value provided may not allow for    |
** |                   |         |that consistency.                       |
** |-------------------|---------|----------------------------------------|
** |SQLMAXIMUMSIZE     |sqlint64 |Defines the maximum size to which the   |
** |                   |         |database manager automatically increases|
** |                   |         |the table space. Alternately, a value of|
** |                   |         |SQL_TBS_NO_MAXSIZE can be used to       |
** |                   |         |specify that the maximum size is        |
** |                   |         |"unlimited", in which case the table    |
** |                   |         |space can grow to the architectural     |
** |                   |         |limit for the table space or until a    |
** |                   |         |"filesystem full" condition is          |
** |                   |         |encountered. This field is only relevant|
** |                   |         |for table spaces that have auto-resize  |
** |                   |         |enabled. Use a value of                 |
** |                   |         |SQL_TBS_AUTOMATIC_MAXSIZE if auto-resize|
** |                   |         |is disabled or if the intent is to have |
** |                   |         |the database manager determine the      |
** |                   |         |maximum size automatically.             |
** |                   |         |Note:                                   |
** |                   |         |The actual value used by the database   |
** |                   |         |manager may be slightly smaller or      |
** |                   |         |larger than what was specified. This    |
** |                   |         |action is taken to keep sizes consistent|
** |                   |         |across containers in the table space and|
** |                   |         |the value provided may not allow for    |
** |                   |         |that consistency.                       |
** |-------------------|---------|----------------------------------------|
** |SQLAUTORESIZE      |CHAR(1)  |Specifies whether auto-resize is enabled|
** |                   |         |for the table space or not. See the     |
** |                   |         |information that follows this table for |
** |                   |         |values.                                 |
** |-------------------|---------|----------------------------------------|
** |SQLINITSIZEUNIT    |CHAR(1)  |If relevant, indicates whether          |
** |                   |         |SQLINITSIZE is being provided in bytes, |
** |                   |         |kilobytes, megabytes, or gigabytes.     |
** |                   |         |See the information that follows this   |
** |                   |         |table for values.                       |
** |-------------------|---------|----------------------------------------|
** |SQLINCREASESIZEUNIT|CHAR(1)  |If relevant, indicates whether          |
** |                   |         |SQLINCREASESIZE is being provided in    |
** |                   |         |bytes, kilobytes, megabytes, gigabytes, |
** |                   |         |or as a percentage. See the information |
** |                   |         |that follows this table for values.     |
** |-------------------|---------|----------------------------------------|
** |SQLMAXIMUMSIZEUNIT |CHAR(1)  |If relevant, indicates whether          |
** |                   |         |SQLMAXIMUMSIZE is being provided in     |
** |                   |         |bytes, kilobytes, megabytes, or         |
** |                   |         |gigabytes. See the information that     |
** |                   |         |follows this table for values.          |
** |-------------------|---------|----------------------------------------|
** 
** Valid values for SQLAUTORESIZE (defined in sqlenv) are:
** 
** - SQL_TBS_AUTORESIZE_NO
** Auto-resize is disabled for the table space. This value can only be
** specified for database-managed space (DMS) table spaces or automatic
** storage table spaces.
** 
** - SQL_TBS_AUTORESIZE_YES
** Auto-resize is enabled for the table space. This value can only be
** specified for database-managed space (DMS) table spaces or automatic
** storage table spaces.
** 
** - SQL_TBS_AUTORESIZE_DFT
** The database manager determines whether or not auto-resize is
** enabled based on the table space type: auto-resize is turned off for
** database-managed space (DMS) table spaces and on for automatic
** storage table spaces. Use this value for system-managed space (SMS)
** table spaces since auto-resize is not applicable for that type of
** table space.
** 
** Valid values for SQLINITSIZEUNIT, SQLINCREASESIZEUNIT and
** SQLMAXIMUMSIZEUNIT (defined in sqlenv) are:
** 
** - SQL_TBS_STORAGE_UNIT_BYTES
** The value specified in the corresponding size field is in bytes.
** 
** SQL_TBS_STORAGE_UNIT_KILOBYTES
** The value specified in the corresponding size field is in kilobytes
** (1 kilobyte = 1 024 bytes).
** 
** - SQL_TBS_STORAGE_UNIT_MEGABYTES
** The value specified in the corresponding size field is in megabytes
** (1 megabyte = 1 048 576 bytes)
** 
** - SQL_TBS_STORAGE_UNIT_GIGABYTES
** The value specified in the corresponding size field is in gigabytes
** (1 gigabyte = 1 073 741 824 bytes)
** 
** - SQL_TBS_STORAGE_UNIT_PERCENT
** The value specified in the corresponding size field is a percentage
** (valid range is 1 to 100). This value is only valid for
** SQLINCREASESIZEUNIT.
** 
*******************************************************************************/
SQL_STRUCTURE SQLETSDESCEXT
{
        sqlint64       sqlInitSize;                  /* Initial tablespace    */
                                                     /* size                  */
        sqlint64       sqlIncreaseSize;              /* Increase size         */
        sqlint64       sqlMaximumSize;               /* Maximum tablespace    */
                                                     /* size                  */
        char           sqlAutoResize;                /* Auto resize is no,    */
                                                     /* yes or dft            */
        char           sqlInitSizeUnit;              /* How initial size is   */
                                                     /* specified             */
        char           sqlIncreaseSizeUnit;          /* How increase size is  */
                                                     /* specified             */
        char           sqlMaximumSizeUnit;           /* How maximum size is   */
                                                     /* specified             */
};


#define SQL_TBS_STORAGE_UNIT_PERCENT 'P'             /* Increase size is at   */
                                                     /* percentage            */
#define SQL_TBS_STORAGE_UNIT_BYTES 'B'               /* Increase size is in   */
                                                     /* bytes                 */
#define SQL_TBS_STORAGE_UNIT_KILOBYTES 'K'           /* Size is in kilobytes  */
#define SQL_TBS_STORAGE_UNIT_MEGABYTES 'M'           /* Size is in megabytes  */
#define SQL_TBS_STORAGE_UNIT_GIGABYTES 'G'           /* Size is in gigabytes  */

#define SQL_TBS_AUTORESIZE_NO 0                      /* auto resize is        */
                                                     /* disable for           */
                                                     /* tablespace            */
#define SQL_TBS_AUTORESIZE_YES 1                     /* auto resize is        */
                                                     /* enabled for           */
                                                     /* tablespace            */
#define SQL_TBS_AUTORESIZE_DFT 2                     /* use default based on  */
                                                     /* tablespace type       */

#define SQL_TBS_AUTOMATIC_INITSIZE -1                /* Initial size          */
                                                     /* automatically         */
                                                     /* determined by DB2     */

#define SQL_TBS_AUTOMATIC_INCSIZE -1                 /* Increase size         */
                                                     /* automatically         */
                                                     /* determined by DB2     */

#define SQL_TBS_AUTOMATIC_MAXSIZE -1                 /* Maximum size          */
                                                     /* automatically         */
                                                     /* determined by DB2     */
#define SQL_TBS_NO_MAXSIZE -2                        /* No maximum size       */
                                                     /* (limited only by      */
                                                     /* DB2's architectural   */
                                                     /* limits                */

#define SQL_PAGESIZE_4K 4096                         /* Database page size:   */
                                                     /* 4 kilobytes           */
#define SQL_PAGESIZE_8K 8192                         /* Database page size:   */
                                                     /* 8 kilobytes           */
#define SQL_PAGESIZE_16K 16384                       /* Database page size:   */
                                                     /* 16 kilobytes          */
#define SQL_PAGESIZE_32K 32768                       /* Database page size:   */
                                                     /* 32 kilobytes          */

#define SQL_AUTOMATIC_STORAGE_NO 0                   /* automatic storage is  */
                                                     /* disabled for          */
                                                     /* database              */
#define SQL_AUTOMATIC_STORAGE_YES 1                  /* automatic storage is  */
                                                     /* enabled for database  */
#define SQL_AUTOMATIC_STORAGE_DFT 2                  /* DB2 makes choice to   */
                                                     /* enable automatic      */
                                                     /* storage or not        */

/* automatic storage management control block                                 */
/******************************************************************************
** sqleAutoStorageCfg data structure
** Automatic storage configuration (sqleAutoStorageCfg) structure
** 
** The automatic storage configuration (sqleAutoStorageCfg) structure
** can be used during a call to the sqlecrea API. It is an element of
** the sqledbdescext structure, and it specifies whether or not
** automatic storage is enabled for the database.
** 
** Table: Fields in the sqleAutoStorageCfg Structure
** |---------------------------------------------------------------------|
** |Field name          |Data type|Description                           |
** |--------------------|---------|--------------------------------------|
** |SQLENABLEAUTOSTORAGE|CHAR(1)  |Specifies whether or not automatic    |
** |                    |         |storage is enabled for the database.  |
** |                    |         |See the information that follows this |
** |                    |         |table for values.                     |
** |--------------------|---------|--------------------------------------|
** |SQLNUMSTORAGEPATHS  |sqluint32|A value indicating the number of      |
** |                    |         |storage paths being pointed to by the |
** |                    |         |SQLSTORAGEPATHS array. If the value   |
** |                    |         |is 0, the SQLSTORAGEPATHS pointer must|
** |                    |         |be NULL. The maximum number of storage|
** |                    |         |paths is 128 (SQL_MAX_STORAGE_PATHS). |
** |--------------------|---------|--------------------------------------|
** |SQLSTORAGEPATHS     |Pointer  |An array of string pointers that point|
** |                    |         |to storage paths. The number of       |
** |                    |         |pointers in the array is reflected by |
** |                    |         |SQLNUMSTORAGEPATHS. Set               |
** |                    |         |SQLSTORAGEPATHS to NULL if there are  |
** |                    |         |no storage paths being provided (in   |
** |                    |         |which case, SQLNUMSTORAGEPATHS must be|
** |                    |         |set to 0). The maximum length of each |
** |                    |         |path is 175 characters.               |
** |--------------------|---------|--------------------------------------|
** 
** Valid values for SQLENABLEAUTOSTORAGE (defined in sqlenv) are:
** 
** - SQL_AUTOMATIC_STORAGE_NO
** Automatic storage is disabled for the database. When this value is
** used, SQLNUMSTORAGEPATHS must be set to 0 and SQLSTORAGEPATHS must
** be set to NULL.
** 
** - SQL_AUTOMATIC_STORAGE_YES
** Automatic storage is enabled for the database. The storage paths
** used for automatic storage are specified using the SQLSTORAGEPATHS
** pointer. If this pointer is NULL, then a single storage path is
** assumed with a value determined by database manager configuration
** parameter dftdbpath.
** 
** - SQL_AUTOMATIC_STORAGE_DFT
** The database manager determines whether or not automatic storage is
** enabled. Currently, the choice is made based on the SQLSTORAGEPATHS
** pointer. If this pointer is NULL, automatic storage is not enabled,
** otherwise it is enabled.
** 
*******************************************************************************/
SQL_STRUCTURE sqleAutoStorageCfg
{
        char           sqlEnableAutoStorage;         /* Enable automatic      */
                                                     /* storage               */
        char           pad[3];                       /* reserved              */
        sqluint32      sqlNumStoragePaths;           /* Number of storage     */
                                                     /* paths                 */
        char           **sqlStoragePaths;            /* Automatic storage     */
                                                     /* paths                 */
};

#define SQL_MAX_STORAGE_PATHS 128                    /* Maximum number of     */
                                                     /* storage paths         */

/* Database descriptor extension control block                                */
/******************************************************************************
** sqledbdescext data structure
** The extended database description block (sqledbdescext) structure
** is used during a call to the sqlecrea API to specify permanent
** values for database attributes. The extended database description
** block enables automatic storage for a database, chooses a default
** page size for the database, or specifies values for new table space
** attributes that have been introduced. This structure is used in
** addition to, not instead of, the database description block
** (sqledbdesc) structure.
** 
** If this structure is not passed to the sqlecrea API, the following
** behavior is used:
** - Automatic storage is not enabled for the database
** - The default page size for the database is 4096 bytes (4 KB)
** - If relevant, DB2 UDB determines the value of the extended table
** space attributes automatically 
** 
** Table: Fields in the sqledbdescext structure
** |--------------------------------------------------------------------|
** |Field name    | Data type|Description                               |
** |--------------|----------|------------------------------------------|
** |SQLPAGESIZE   | sqluint32|Specifies the page size of the default    |
** |              |          |buffer pool as well as the initial table  |
** |              |          |spaces (SYSCATSPACE, TEMPSPACE1,          |
** |              |          |USERSPACE1) when the database is created. |
** |              |          |The value given also represents the       |
** |              |          |default page size for all future CREATE   |
** |              |          |BUFFERPOOL and CREATE TABLESPACE          |
** |              |          |statements. See the information that      |
** |              |          |follows this table for values.            |
** |--------------|----------|------------------------------------------|
** |SQLAUTOSTORAGE| Pointer  |A pointer to an automatic storage         |
** |              |          |configuration structure. This pointer     |
** |              |          |enables or disables automatic storage for |
** |              |          |the database. If a pointer is given,      |
** |              |          |automatic storage is enabled. If NULL,    |
** |              |          |automatic storage is not enabled.         |
** |--------------|----------|------------------------------------------|
** |SQLCATTSEXT   | Pointer  |A pointer to an extended table space      |
** |              |          |description control block (SQLETSDESCEXT) |
** |              |          |for the system catalog table space, which |
** |              |          |defines additional attributes to those    |
** |              |          |found in SQLETSDESC. If NULL, the database|
** |              |          |manager determines the value of these     |
** |              |          |attributes automatically (if relevant).   |
** |--------------|----------|------------------------------------------|
** |SQLUSRTSEXT   | Pointer  |A pointer to an extended table space      |
** |              |          |description control block (SQLETSDESCEXT) |
** |              |          |for the user table space, which defines   |
** |              |          |additional attributes to those found in   |
** |              |          |SQLETSDESC. If NULL, the database manager |
** |              |          |determines the value of these attributes  |
** |              |          |automatically (if relevant).              |
** |--------------|----------|------------------------------------------|
** |SQLTMPTSEXT   | Pointer  |A pointer to an extended table space      |
** |              |          |description control block (SQLETSDESCEXT) |
** |              |          |for the system temporary table space,     |
** |              |          |which defines additional attributes to    |
** |              |          |those found in SQLETSDESC. If NULL, the   |
** |              |          |database manager determines the value of  |
** |              |          |these attributes automatically (if        |
** |              |          |relevant).                                |
** |--------------|----------|------------------------------------------|
** |RESERVED      | Pointer  |Reserved field, set to NULL.              |
** |--------------|----------|------------------------------------------|
** 
** Valid values for SQLPAGESIZE (defined in sqlenv) are:
** - SQL_PAGESIZE_4K
** Default page size for the database is 4 096 bytes.
** - SQL_PAGESIZE_8K
** Default page size for the database is 8 192 bytes.
** - SQL_PAGESIZE_16K
** Default page size for the database is 16 384 bytes.
** - SQL_PAGESIZE_32K
** Default page size for the database is 32 768 bytes.
** 
*******************************************************************************/
SQL_STRUCTURE sqledbdescext
{
        sqluint32      sqlPageSize;                  /* Page size (4K, 8K,    */
                                                     /* 16K, 32K)             */
        struct sqleAutoStorageCfg *sqlAutoStorage;   /* Automatic storage     */
                                                     /* control block         */
        struct SQLETSDESCEXT *sqlcattsext;           /* Catalog tablespace    */
                                                     /* extension             */
        struct SQLETSDESCEXT *sqlusrtsext;           /* User tablespace       */
                                                     /* extension             */
        struct SQLETSDESCEXT *sqltmptsext;           /* Temporary tablespace  */
                                                     /* extension             */
        void           *reserved;                    /* Database creation     */
                                                     /* options               */
};


/* Database restrictive access option values                                  */
#define SQL_DB_RESTRICT_ACCESS_NO 0                  /* restrictive access    */
                                                     /* is set to NO for      */
                                                     /* database              */
#define SQL_DB_RESTRICT_ACCESS_YES 1                 /* restrictive access    */
                                                     /* is set to YES for     */
                                                     /* database              */
#define SQL_DB_RESTRICT_ACCESS_DFT 0                 /* restrictive access    */
                                                     /* is set to NO by       */
                                                     /* default for database  */

/* Database creation options placeholder                                      */
SQL_STRUCTURE sqledboptions
{
        void           *piAutoConfigInterface;
        sqlint32       restrictive;                  /* Database restrictive  */
                                                     /* access option         */
        void           *reserved;                    /* pointer to            */
                                                     /* sqledbdescextext      */
};


/* Database encryption option values                                          */
#define SQL_ENCRYPT_DB_NO 0
#define SQL_ENCRYPT_DB_YES 1
#define SQL_ENCRYPT_DB_DEFAULT 2

/* Encryption algorithms                                                      */
#define SQL_CIPHER_DEFAULT 0
#define SQL_CIPHER_3DES 1
#define SQL_CIPHER_AES 2

/* Encryption modes                                                           */
#define SQL_CIPHER_MODE_DEFAULT 0
#define SQL_CIPHER_MODE_CBC 1

/* Encryption keylengths in bytes                                             */
#define SQL_CIPHER_KEYLEN_DEFAULT 0                  /* Use documented        */
                                                     /* default key length    */
#define SQL_CIPHER_KEYLEN_3DES_168 24                /* 3DES key strength is  */
                                                     /* 168 bits, but it      */
                                                     /* occupies 192 bits of  */
                                                     /* memory                */
#define SQL_CIPHER_KEYLEN_AES_128 16
#define SQL_CIPHER_KEYLEN_AES_192 24
#define SQL_CIPHER_KEYLEN_AES_256 32

/* Database creation encryption options                                       */
SQL_STRUCTURE sqleDbEncryptionOptions
{
        unsigned short encryptDb;
        unsigned short cipherName;
        unsigned short cipherMode;
        sqluint32      cipherKeyLen;
        char           *masterKeyLabel;              /* Label of the master   */
                                                     /* key                   */
        unsigned short masterKeyLabelLen;
        void           *reserved;                    /* reserved              */
};


/* Extended database creation options                                         */
SQL_STRUCTURE sqledbdescextext
{
        struct sqleDbEncryptionOptions *pDbEncryptionOptions;
        void           *reserved1;                   /* reserved              */
        void           *reserved2;                   /* reserved              */
        void           *reserved;                    /* reserved              */
};


/* Old Database Directory Scan data structure                                 */
/******************************************************************************
** sqledinfo data structure
*******************************************************************************/
SQL_STRUCTURE sqledinfo
{
        _SQLOLDCHAR    alias[SQL_ALIAS_SZ];          /* Alias name            */
        _SQLOLDCHAR    dbname[SQL_DBNAME_SZ];        /* Database name         */
        _SQLOLDCHAR    drive[SQL_DRIVE_SZ];          /* Drive/Path            */
        _SQLOLDCHAR    intname[SQL_INAME_SZ];        /* Database              */
                                                     /* subdirectory          */
        _SQLOLDCHAR    nodename[SQL_NNAME_SZ];       /* Node name             */
        _SQLOLDCHAR    dbtype[SQL_DBTYP_SZ];         /* Release information   */
        _SQLOLDCHAR    comment[SQL_CMT_SZ];          /* Comment               */
        short          com_codepage;                 /* Code page of comment  */
        _SQLOLDCHAR    type;                         /* Entry type - defines  */
                                                     /* above                 */
        unsigned short authentication;               /* Authentication type   */
        char           glbdbname[SQL_DIR_NAME_SZ];   /* Global database name  */
        _SQLOLDCHAR    dceprincipal[SQL_DCEPRIN_SZ]; /* dce principal bin     */
                                                     /* string                */
        short          cat_nodenum;                  /* Catalog node number   */
        short          nodenum;                      /* Node number           */
};

/* Node Directory Scan data structure                                         */
/******************************************************************************
** sqleninfo data structure
** This structure returns information after a call to the sqlengne API.
** 
** Table: Fields in the SQLENINFO Structure
** --------------------------------------------------------------------------
** |Field Name       |  Data Type | Description                             |
** |-----------------|------------|-----------------------------------------|
** |NODENAME         |  CHAR(8)   | Used for the NetBIOS protocol; the nname|
** |                 |            | of the node where the database is       |
** |                 |            | located (valid in system directory only)|
** |-----------------|------------|-----------------------------------------|
** |LOCAL_LU         |  CHAR(8)   | Used for the APPN protocol; local       |
** |                 |            | logical unit.                           |
** |-----------------|------------|-----------------------------------------|
** |PARTNER_LU       |  CHAR(8)   | Used for the APPN protocol; partner     |
** |                 |            | logical unit.                           |
** |-----------------|------------|-----------------------------------------|
** |MODE             |  CHAR(8)   | Used for the APPN protocol; transmission|
** |                 |            | service mode.                           |
** |-----------------|------------|-----------------------------------------|
** |COMMENT          |  CHAR(30)  | The comment associated with the node.   |
** |-----------------|------------|-----------------------------------------|
** |COM_CODEPAGE     |  SMALLINT  | The code page of the comment. This field|
** |                 |            | is no longer used by the database       |
** |                 |            | manager.                                |
** |-----------------|------------|-----------------------------------------|
** |ADAPTER          |  SMALLINT  | Used for the NetBIOS protocol; the      |
** |                 |            | local network adapter.                  |
** |-----------------|------------|-----------------------------------------|
** |NETWORKID        |  CHAR(8)   | Used for the APPN protocol; network ID. |
** |-----------------|------------|-----------------------------------------|
** |PROTOCOL         |  CHAR(1)   | Communications protocol.                |
** |-----------------|------------|-----------------------------------------|
** |SYM_DEST_NAME    |  CHAR(8)   | Used for the APPC protocol; the symbolic|
** |                 |            | destination name.                       |
** |-----------------|------------|-----------------------------------------|
** |SECURITY_TYPE    |  SMALLINT  | Used for the APPC protocol; the         |
** |                 |            | security type. See below for values.    |
** |-----------------|------------|-----------------------------------------|
** |HOSTNAME         |  CHAR(255) | Used for the TCP/IP protocol; the name  |
** |                 |            | of the TCP/IP host on which the DB2     |
** |                 |            | server instance resides.                |
** |-----------------|------------|-----------------------------------------|
** |SERVICE_NAME     |  CHAR(14)  | Used for the TCP/IP protocol; the TCP/IP|
** |                 |            | service name or associated port number  |
** |                 |            | of the DB2 server instance.             |
** |-----------------|------------|-----------------------------------------|
** |FILESERVER       |  CHAR(48)  | Used for the IPX/SPX protocol; the name |
** |                 |            | of the NetWare file server where the DB2|
** |                 |            | server instance is registered.          |
** |-----------------|------------|-----------------------------------------|
** |OBJECTNAME       |  CHAR(48)  | The database manager server instance is |
** |                 |            | represented as the object, objectname,  |
** |                 |            | on the NetWare file server. The server's|
** |                 |            | IPX/SPX internetwork address is stored  |
** |                 |            | and retrieved from this object.         |
** |-----------------|------------|-----------------------------------------|
** |INSTANCE_NAME    |  CHAR(8)   | Used for the local and NPIPE protocols; |
** |                 |            | the name of the server instance.        |
** |-----------------|------------|-----------------------------------------|
** |COMPUTERNAME     |  CHAR(15)  | Used by the NPIPE protocol; the server  |
** |                 |            | node's computer name.                   |
** |-----------------|------------|-----------------------------------------|
** |SYSTEM_NAME      |  CHAR(21)  | The DB2 system name of the remote       |
** |                 |            | server.                                 |
** |-----------------|------------|-----------------------------------------|
** |REMOTE_INSTNAME  |  CHAR(8)   | The name of the DB2 server instance.    |
** |-----------------|------------|-----------------------------------------|
** |CATALOG_NODE_TYPE|  CHAR      | Catalog node type.                      |
** |-----------------|------------|-----------------------------------------|
** |OS_TYPE          |  UNSIGNED  | Identifies the operating system of the  |
** |                 |  SHORT     | server.                                 |
** |-----------------|------------|-----------------------------------------|
** |Note:                                                                   |
** |Each character field returned is blank filled up to the length of the   |
** |field.                                                                  |
** |------------------------------------------------------------------------|
** 
** Valid values for SECURITY_TYPE (defined in sqlenv) are:
** - SQL_CPIC_SECURITY_NONE 
** - SQL_CPIC_SECURITY_SAME 
** - SQL_CPIC_SECURITY_PROGRAM 
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQLENINFO.
**     05 SQL-NODE-NAME          PIC X(8).
**     05 SQL-LOCAL-LU           PIC X(8).
**     05 SQL-PARTNER-LU         PIC X(8).
**     05 SQL-MODE               PIC X(8).
**     05 SQL-COMMENT            PIC X(30).
**     05 SQL-COM-CODEPAGE       PIC 9(4) COMP-5.
**     05 SQL-ADAPTER            PIC 9(4) COMP-5.
**     05 SQL-NETWORKID          PIC X(8).
**     05 SQL-PROTOCOL           PIC X.
**     05 SQL-SYM-DEST-NAME      PIC X(8).
**     05 FILLER                 PIC X(1).
**     05 SQL-SECURITY-TYPE      PIC 9(4) COMP-5.
**     05 SQL-HOSTNAME           PIC X(255).
**     05 SQL-SERVICE-NAME       PIC X(14).
**     05 SQL-FILESERVER         PIC X(48).
**     05 SQL-OBJECTNAME         PIC X(48).
**     05 SQL-INSTANCE-NAME      PIC X(8).
**     05 SQL-COMPUTERNAME       PIC X(15).
**     05 SQL-SYSTEM-NAME        PIC X(21).
**     05 SQL-REMOTE-INSTNAME    PIC X(8).
**     05 SQL-CATALOG-NODE-TYPE  PIC X.
**     05 SQL-OS-TYPE            PIC 9(4) COMP-5.
** * 
*******************************************************************************/
SQL_STRUCTURE sqleninfo
{
        _SQLOLDCHAR    nodename[SQL_NNAME_SZ];       /* Node name             */
        _SQLOLDCHAR    local_lu[SQL_LOCLU_SZ];       /* Local_lu_name         */
        _SQLOLDCHAR    partner_lu[SQL_RMTLU_SZ];     /* Partner_lu_name       */
        _SQLOLDCHAR    mode[SQL_MODE_SZ];            /* Mode                  */
        _SQLOLDCHAR    comment[SQL_CMT_SZ];          /* Comment               */
        unsigned short com_codepage;                 /* Comment code page     */
        unsigned short adapter;                      /* NetBIOS Adapter #     */
        _SQLOLDCHAR    networkid[SQL_NETID_SZ];      /* APPN Network ID       */
        _SQLOLDCHAR    protocol;                     /* Protocol type         */
        _SQLOLDCHAR    sym_dest_name[SQL_SYM_DEST_NAME_SZ]; /* CPIC Sym.      */
                                                     /* Dest. Name            */
        unsigned short security_type;                /* CPIC Security Type    */
        _SQLOLDCHAR    hostname[SQL_HOSTNAME_SZ];    /* TCP/IP Host name      */
        _SQLOLDCHAR    service_name[SQL_SERVICE_NAME_SZ]; /* TCP/IP Service   */
                                                     /* name                  */
        char           fileserver[SQL_FILESERVER_SZ]; /* IPX/SPX fileserver   */
                                                     /* name                  */
        char           objectname[SQL_OBJECTNAME_SZ]; /* IPX/SPX objectname   */
        char           instance_name[SQL_INSTNAME_SZ]; /* LOCAL Instance      */
                                                     /* name                  */
        char           computername[SQL_COMPUTERNAME_SZ]; /* Computer name    */
        char           system_name[SQL_SYSTEM_NAME_SZ]; /* System name        */
        char           remote_instname[SQL_REMOTE_INSTNAME_SZ]; /* Remote     */
                                                     /* instance name         */
        _SQLOLDCHAR    catalog_node_type;            /* Catalog node type     */
        unsigned short os_type;                      /* OS type               */
        _SQLOLDCHAR    chgpwd_lu[SQL_RMTLU_SZ];      /* Change_password_lu    */
                                                     /* name                  */
        _SQLOLDCHAR    transpn[SQL_TPNAME_SZ];       /* TRANSaction_Program   */
                                                     /* Name                  */
        _SQLOLDCHAR    lanaddr[SQL_LANADDRESS_SZ];   /* LAN_Adapter_Address   */
};

/* General Catalog Node structures and defines                                */

/******************************************************************************
** sqle_node_struct data structure
** This structure is used to catalog nodes for the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-STRUCT Structure
** -----------------------------------------------------------------------
** |Field Name | Data Type | Description                                  |
** |----------------------------------------------------------------------|
** |STRUCT_ID  | SMALLINT  | Structure identifier.                        |
** |----------------------------------------------------------------------|
** |CODEPAGE   | SMALLINT  | Code page for comment.                       |
** |----------------------------------------------------------------------|
** |COMMENT    | CHAR(30)  | Optional description of the node.            |
** |----------------------------------------------------------------------|
** |NODENAME   | CHAR(8)   | Local name for the node where the database is|
** |           |           | located.                                     |
** |----------------------------------------------------------------------|
** |PROTOCOL   | CHAR(1)   | Communications protocol type.                |
** |----------------------------------------------------------------------|
** |Note:                                                                 |
** |The character fields passed in this structure must be null terminated |
** |or blank filled up to the length of the field.                        |
** |----------------------------------------------------------------------|
** 
** Valid values for PROTOCOL (defined in sqlenv) are:
** - SQL_PROTOCOL_APPC
** - SQL_PROTOCOL_APPN
** - SQL_PROTOCOL_CPIC
** - SQL_PROTOCOL_IPXSPX
** - SQL_PROTOCOL_LOCAL 
** - SQL_PROTOCOL_NETB
** - SQL_PROTOCOL_NPIPE
** - SQL_PROTOCOL_SOCKS
** - SQL_PROTOCOL_TCPIP
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-STRUCT.
**     05 STRUCT-ID              PIC 9(4) COMP-5.
**     05 CODEPAGE               PIC 9(4) COMP-5.
**     05 COMMENT                PIC X(30).
**     05 FILLER                 PIC X.
**     05 NODENAME               PIC X(8).
**     05 FILLER                 PIC X.
**     05 PROTOCOL               PIC X.
**     05 FILLER                 PIC X(1).
** * 
*******************************************************************************/
SQL_STRUCTURE sqle_node_struct
{
        unsigned short struct_id;                    /* Structure Identifier  */
        unsigned short codepage;                     /* Codepage for comment  */
        _SQLOLDCHAR    comment[SQL_CMT_SZ + 1];      /* Comment               */
        _SQLOLDCHAR    nodename[SQL_NNAME_SZ + 1];   /* Node name             */
        unsigned char  protocol;                     /* Protocol Type         */
};

/******************************************************************************
** sqle_node_appc data structure
** This structure is used to catalog APPC nodes for the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-APPC Structure
** ----------------------------------------------------------------------
** |Field Name  |Data Type | Description                                 |
** |---------------------------------------------------------------------|
** |LOCAL_LU    |CHAR(8)   | Local_lu name.                              |
** |---------------------------------------------------------------------|
** |PARTNER_LU  |CHAR(8)   | Alias Partner_lu name.                      |
** |---------------------------------------------------------------------|
** |MODE        |CHAR(8)   | Mode.                                       |
** |---------------------------------------------------------------------|
** |Note:                                                                |
** |The character fields passed in this structure must be null terminated|
** |or blank filled up to the length of the field.                       |
** |---------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-APPC.
**     05 LOCAL-LU               PIC X(8).
**     05 FILLER                 PIC X.
**     05 PARTNER-LU             PIC X(8).
**     05 FILLER                 PIC X.
**     05 TRANS-MODE             PIC X(8).
**     05 FILLER                 PIC X.
** * 
*******************************************************************************/
SQL_STRUCTURE sqle_node_appc                         /* For APPC protocol     */
{
        _SQLOLDCHAR    local_lu[SQL_LOCLU_SZ + 1];   /* Local_lu name         */
        _SQLOLDCHAR    partner_lu[SQL_RMTLU_SZ + 1]; /* Alias Partner_lu      */
                                                     /* name                  */
        _SQLOLDCHAR    mode[SQL_MODE_SZ + 1];        /* Mode                  */
};

/******************************************************************************
** sqle_node_netb data structure
** This structure is used to catalog NetBIOS nodes for the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-NETB Structure
** ------------------------------------------------------------------------
** |Field Name   |Data Type |Description                                   |
** |-----------------------------------------------------------------------|
** |ADAPTER      |SMALLINT  |Local LAN adapter.                            |
** |-----------------------------------------------------------------------|
** |REMOTE_NNAME |CHAR(8)   |Nname of the remote workstation that is stored|
** |             |          |in the database manager configuration file on |
** |             |          |the server instance.                          |
** |-----------------------------------------------------------------------|
** |Note:                                                                  |
** |The character fields passed in this structure must be null terminated  |
** |or blank filled up to the length of the field.                         |
** |-----------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-NETB.
**     05 ADAPTER                PIC 9(4) COMP-5.
**     05 REMOTE-NNAME           PIC X(8).
**     05 FILLER                 PIC X.
**     05 FILLER                 PIC X(1).
** * 
*******************************************************************************/
SQL_STRUCTURE sqle_node_netb                         /* For NetBIOS Protocol  */
{
        unsigned short adapter;                      /* Adapter Number        */
        _SQLOLDCHAR    remote_nname[SQL_RMTLU_SZ + 1]; /* Remote Workstation  */
                                                     /* name                  */
};

/******************************************************************************
** sqle_node_appn data structure
** This structure is used to catalog APPN nodes for the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-APPN Structure
** ----------------------------------------------------------------------
** |Field Name | Data Type | Description                                 |
** |---------------------------------------------------------------------|
** |NETWORKID  | CHAR(8)   | Network ID.                                 |
** |---------------------------------------------------------------------|
** |REMOTE_LU  | CHAR(8)   | Alias Remote_lu name.                       |
** |---------------------------------------------------------------------|
** |LOCAL_LU   | CHAR(8)   | Alias Local_lu name.                        |
** |---------------------------------------------------------------------|
** |MODE       | CHAR(8)   | Mode.                                       |
** |---------------------------------------------------------------------|
** |Note:                                                                |
** |The character fields passed in this structure must be null terminated|
** |or blank filled up to the length of the field.                       |
** |---------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-APPN.
**     05 NETWORKID              PIC X(8).
**     05 FILLER                 PIC X.
**     05 REMOTE-LU              PIC X(8).
**     05 FILLER                 PIC X.
**     05 LOCAL-LU               PIC X(8).
**     05 FILLER                 PIC X.
**     05 TRANS-MODE             PIC X(8).
**     05 FILLER                 PIC X.
** *
*******************************************************************************/
SQL_STRUCTURE sqle_node_appn                         /* For APPN protocol     */
{
        _SQLOLDCHAR    networkid[SQL_NETID_SZ + 1];  /* Network ID            */
        _SQLOLDCHAR    remote_lu[SQL_RMTLU_SZ + 1];  /* Remoter lu name       */
        _SQLOLDCHAR    local_lu[SQL_LOCLU_SZ + 1];   /* Local_lu name         */
        _SQLOLDCHAR    mode[SQL_MODE_SZ + 1];        /* Mode                  */
        _SQLOLDCHAR    chgpwd_lu[SQL_RMTLU_SZ+1];    /* Change password lu    */
                                                     /* name                  */
        _SQLOLDCHAR    transpn[SQL_TPNAME_SZ + 1];   /* Transaction Program   */
                                                     /* Name                  */
        _SQLOLDCHAR    lanaddr[SQL_LANADDRESS_SZ + 1]; /* LAN Adapter         */
                                                     /* Address               */
        unsigned short security_type;                /* Security Type         */
};

/******************************************************************************
** sqle_node_tcpip data structure
** This structure is used to catalog TCP/IP nodes for the sqlectnd API.
** 
** Note:
** To catalog a TCP/IP SOCKS node, set the PROTOCOL type in the node
** directory structure to SQL_PROTOCOL_SOCKS in the SQLE-NODE-STRUCT
** structure before calling the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-TCPIP Structure
** -----------------------------------------------------------------------
** |Field Name   | Data Type|Description                                  |
** |----------------------------------------------------------------------|
** |HOSTNAME     | CHAR(255)|The name of the TCP/IP host on which the DB2 |
** |             |          |server instance resides.                     |
** |----------------------------------------------------------------------|
** |SERVICE_NAME | CHAR(14) |TCP/IP service name or associated port number|
** |             |          |of the DB2 server instance.                  |
** |----------------------------------------------------------------------|
** |Note:                                                                 |
** |The character fields passed in this structure must be null terminated |
** |or blank filled up to the length of the field.                        |
** |----------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-TCPIP.
**     05 HOSTNAME               PIC X(255).
**     05 FILLER                 PIC X.
**     05 SERVICE-NAME           PIC X(14).
**     05 FILLER                 PIC X.
** *
*******************************************************************************/
SQL_STRUCTURE sqle_node_tcpip                        /* For TCP/IP protocol   */
{
        _SQLOLDCHAR    hostname[SQL_HOSTNAME_SZ+1];  /* Host name             */
        _SQLOLDCHAR    service_name[SQL_SERVICE_NAME_SZ+1]; /* Service Name   */
};

/******************************************************************************
** sqle_node_npipe data structure
** This structure is used to catalog named pipe nodes for the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-NPIPE Structure
** -----------------------------------------------------------------------
** |Field Name   |Data Type | Description                                |
** |---------------------------------------------------------------------|
** |COMPUTERNAME |CHAR(15)  | Computer name.                             |
** |---------------------------------------------------------------------|
** |INSTANCE_NAME|CHAR(8)   | Name of an instance.                       |
** |---------------------------------------------------------------------|
** |Note:                                                                |
** |The character fields passed in this structure must be null terminated|
** |or blank filled up to the length of the field.                       |
** |---------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-NPIPE.
**     05 COMPUTERNAME           PIC X(15).
**     05 FILLER                 PIC X.
**     05 INSTANCE-NAME          PIC X(8).
**     05 FILLER                 PIC X.
** * 
*******************************************************************************/
SQL_STRUCTURE sqle_node_npipe                        /* For NPIPE protocol    */
{
        char           computername[SQL_COMPUTERNAME_SZ+1]; /* Computer name  */
        char           instance_name[SQL_INSTNAME_SZ+1]; /* Instance Name     */
};

/******************************************************************************
** sqle_node_local data structure
** This structure is used to catalog local nodes for the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-LOCAL Structure
** -----------------------------------------------------------------------
** |Field Name    | Data Type | Description                               |
** |----------------------------------------------------------------------|
** |INSTANCE_NAME | CHAR(8)   | Name of an instance.                      |
** |----------------------------------------------------------------------|
** |Note:                                                                 |
** |The character fields passed in this structure must be null terminated |
** |or blank filled up to the length of the field.                        |
** |----------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-LOCAL.
**     05 SQL-INSTANCE-NAME      PIC X(8).
**     05 FILLER                 PIC X.
** * 
*******************************************************************************/
SQL_STRUCTURE sqle_node_local                        /* For LOCAL protocol    */
{
        char           instance_name[SQL_INSTNAME_SZ+1]; /* Instance Name     */
};

/******************************************************************************
** sqle_node_cpic data structure
** This structure is used to catalog CPIC nodes for the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-CPIC Structure
** ----------------------------------------------------------------------
** |Field Name    |Data Type |Description                                |
** |---------------------------------------------------------------------|
** |SYM_DEST_NAME |CHAR(8)   |Symbolic destination name of remote        |
** |              |          |partner.                                   |
** |---------------------------------------------------------------------|
** |SECURITY_TYPE |SMALLINT  |Security type.                             |
** |---------------------------------------------------------------------|
** |Note:                                                                |
** |The character fields passed in this structure must be null terminated|
** |or blank filled up to the length of the field.                       |
** |---------------------------------------------------------------------|
** 
** Valid values for SECURITY_TYPE (defined in sqlenv) are:
** - SQL_CPIC_SECURITY_NONE 
** - SQL_CPIC_SECURITY_SAME 
** - SQL_CPIC_SECURITY_PROGRAM 
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-CPIC.
**     05 SYM-DEST-NAME          PIC X(8).
**     05 FILLER                 PIC X.
**     05 FILLER                 PIC X(1).
**     05 SECURITY-TYPE          PIC 9(4) COMP-5.
** * 
*******************************************************************************/
SQL_STRUCTURE sqle_node_cpic                         /* For APPC using CPIC   */
                                                     /* protocol              */
{
        _SQLOLDCHAR    sym_dest_name[SQL_SYM_DEST_NAME_SZ+1]; /* Sym Dest     */
                                                     /* Name                  */
        unsigned short security_type;                /* Security Type         */
};

/******************************************************************************
** sqle_node_ipxspx data structure
** This structure is used to catalog IPX/SPX nodes for the sqlectnd API.
** 
** Table: Fields in the SQLE-NODE-IPXSPX Structure
** -----------------------------------------------------------------------
** |Field Name | Data Type | Description                                  |
** |----------------------------------------------------------------------|
** |FILESERVER | CHAR(48)  | Name of the NetWare file server where the DB2|
** |           |           | server instance is registered.               |
** |----------------------------------------------------------------------|
** |OBJECTNAME | CHAR(48)  | The database manager server instance is      |
** |           |           | represented as the object, objectname, on the|
** |           |           | NetWare file server. The server's IPX/SPX    |
** |           |           | internetwork address is stored and retrieved |
** |           |           | from this object.                            |
** |----------------------------------------------------------------------|
** |Note:                                                                 |
** |The character fields passed in this structure must be null terminated |
** |or blank filled up to the length of the field.                        |
** |----------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-NODE-IPXSPX.
**     05 SQL-FILESERVER         PIC X(48).
**     05 FILLER                 PIC X.
**     05 SQL-OBJECTNAME         PIC X(48).
**     05 FILLER                 PIC X.
** * 
*******************************************************************************/
SQL_STRUCTURE sqle_node_ipxspx                       /* For IPX/SPX protocol  */
{
        char           fileserver[SQL_FILESERVER_SZ+1]; /* Fileserver name    */
        char           objectname[SQL_OBJECTNAME_SZ+1]; /* Object name        */
};

/* Values for sqlefrce                                                        */
#define SQL_ASYNCH             0       /* Force Mode (Asynchronous)           */
#define SQL_ALL_USERS          -1      /* Force Count (All Users)             */

/* Values for priority type in sqlesapr                                       */
#define SQL_ABSOLUTE_PRIORITY  1       /* Absolute (i.e. fixed) Priority      */
#define SQL_DELTA_PRIORITY     0       /* Delta (i.e. nice) Priority          */

#define SQL_PRODNAME_SZ        30      /* Product name                        */
/******************************************************************************
** sql_dir_entry data structure
** This structure is used by the DCS directory APIs.
** 
** Table: Fields in the SQL-DIR-ENTRY Structure
** ----------------------------------------------------------------------
** |Field Name|Data Type  |Description                                   |
** |----------|-----------|----------------------------------------------|
** |STRUCT_ID |SMALLINT   |Structure identifier. Set to SQL_DCS_STR_ID   |
** |          |           |(defined in sqlenv).                          |
** |RELEASE   |SMALLINT   |Release version (assigned by the API).        |
** |CODEPAGE  |SMALLINT   |Code page for comment.                        |
** |COMMENT   |CHAR(30)   |Optional description of the database.         |
** |LDB       |CHAR(8)    |Local name of the database; must match        |
** |          |           |database alias in system database directory.  |
** |TDB       |CHAR(18)   |Actual name of the database.                  |
** |AR        |CHAR(32)   |Name of the application client.               |
** |PARM      |CHAR(512)  |Contains transaction program prefix,          |
** |          |           |transaction program name, SQLCODE mapping file|
** |          |           |name, and disconnect and security option.     |
** |----------|-----------|----------------------------------------------|
** |Note:                                                                |
** |The character fields passed in this structure must be null terminated|
** |or blank filled up to the length of the field.                       |
** |---------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQL-DIR-ENTRY.
**     05 STRUCT-ID              PIC 9(4) COMP-5.
**     05 RELEASE-LVL            PIC 9(4) COMP-5.
**     05 CODEPAGE               PIC 9(4) COMP-5.
**     05 COMMENT                PIC X(30).
**     05 FILLER                 PIC X.
**     05 LDB                    PIC X(8).
**     05 FILLER                 PIC X.
**     05 TDB                    PIC X(18).
**     05 FILLER                 PIC X.
**     05 AR                     PIC X(32).
**     05 FILLER                 PIC X.
**     05 PARM                   PIC X(512).
**     05 FILLER                 PIC X.
**     05 FILLER                 PIC X(1).
** * 
** 
*******************************************************************************/
SQL_STRUCTURE sql_dir_entry            /* DDCS Directory Entry Data           */
                                       /* Structure                           */
{
        unsigned short         struct_id; /* Structure Identifier             */
        unsigned short         release; /* Release level of entry             */
        unsigned short         codepage; /* Codepage of comment               */
        _SQLOLDCHAR            comment[SQL_CMT_SZ + 1]; /* Directory entry    */
                                       /* comment                             */
        _SQLOLDCHAR            ldb[SQL_DBNAME_SZ + 1]; /* Local DB name       */
        _SQLOLDCHAR            tdb[SQL_LONG_NAME_SZ + 1]; /* Target (host)    */
                                       /* DB name                             */
        _SQLOLDCHAR            ar[SQL_AR_SZ + 1]; /* AR library name          */
        _SQLOLDCHAR            parm[SQL_PARAMETER_SZ + 1]; /* Parameter       */
                                       /* string                              */
};

/* Database Codepage and Territory code info structures                       */

/******************************************************************************
** sqledbcinfo data structure
*******************************************************************************/
SQL_STRUCTURE sqledbcinfo
{
        _SQLOLDCHAR            sqldbcp[SQL_CODESET_LEN + 1]; /* database      */
                                       /* codeset                             */
        _SQLOLDCHAR            sqldbcc[SQL_LOCALE_LEN + 1]; /* database       */
                                       /* territory                           */
};

/* V2 Database Codepage and Territory code info structure                     */

/******************************************************************************
** sqledbcountryinfo data structure
** This structure is used to provide code set and territory options
** to the sqlecrea API.
** 
** Table: Fields in the SQLEDBTERRITORYINFO Structure
** -----------------------------------------------------------------------
** |Field Name   | Data Type | Description                               |
** ----------------------------------------------------------------------|
** |SQLDBCODESET | CHAR(9)   | Database code set.                        |
** ----------------------------------------------------------------------|
** |SQLDBLOCALE  | CHAR(5)   | Database territory.                       |
** |---------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQLEDBTERRITORYINFO.
**     05 SQLDBCODESET           PIC X(9).
**     05 FILLER                 PIC X.
**     05 SQLDBLOCALE            PIC X(5).
**     05 FILLER                 PIC X.
** * 
*******************************************************************************/
SQL_STRUCTURE sqledbcountryinfo
{
        char                   sqldbcodeset[SQL_CODESET_LEN + 1]; 
        char                   sqldblocale[SQL_LOCALE_LEN + 1]; /* database   */
                                       /* territory                           */
};

typedef SQL_STRUCTURE sqledbcountryinfo SQLEDBTERRITORYINFO;

/******************************************************************************
** sqle_conn_setting data structure
** This structure is used to specify connection setting types and values
** for the sqleqryc and sqlesetc APIs.
** 
** Table: Fields in the SQLE-CONN-SETTING Structure
** -------------------------------------------
** |Field Name|  Data Type  | Description    |
** |----------|-------------|----------------|
** |TYPE      |  SMALLINT   | Setting type.  |
** |VALUE     |  SMALLINT   | Setting value. |
** |----------|-------------|----------------|
** 
** The valid entries for the SQLE-CONN-SETTING TYPE element and the
** associated descriptions for each entry are listed below (defined in
** sqlenv and sql):
** 
** Table: Connection Settings
** ------------------------------------------------------------------------
** |Type                |Value                 |Description                |
** |--------------------|----------------------|---------------------------|
** |SQL_CONNECT_TYPE    |SQL_CONNECT_1         |Type 1 CONNECTs enforce the|
** |                    |                      |single database per unit of|
** |                    |                      |work semantics of older    |
** |                    |                      |releases, also known as the|
** |                    |                      |rules for remote unit of   |
** |                    |                      |work (RUOW).               |
** |                    |SQL_CONNECT_2         |Type 2 CONNECTs support the|
** |                    |                      |multiple databases per unit|
** |                    |                      |of work semantics of DUOW. |
** |--------------------|----------------------|---------------------------|
** |SQL_RULES           |SQL_RULES_DB2         |Enable the SQL CONNECT     |
** |                    |                      |statement to switch the    |
** |                    |                      |current connection to an   |
** |                    |                      |established (dormant)      |
** |                    |                      |connection.                |
** |                    |SQL_RULES_STD         |Permit only the            |
** |                    |                      |establishment of a new     |
** |                    |                      |connection through the SQL |
** |                    |                      |CONNECT statement. The SQL |
** |                    |                      |SET CONNECTION statement   |
** |                    |                      |must be used to switch the |
** |                    |                      |current connection to a    |
** |                    |                      |dormant connection.        |
** |--------------------|----------------------|---------------------------|
** |SQL_DISCONNECT      |SQL_DISCONNECT_EXPL   |Removes those connections  |
** |                    |                      |that have been explicitly  |
** |                    |                      |marked for release by the  |
** |                    |                      |SQL RELEASE statement at   |
** |                    |                      |commit.                    |
** |                    |SQL_DISCONNECT_COND   |Breaks those connections   |
** |                    |                      |that have no open WITH     |
** |                    |                      |HOLD cursors at commit,    |
** |                    |                      |and those that have been   |
** |                    |                      |marked for release by the  |
** |                    |                      |SQL RELEASE statement.     |
** |                    |SQL_DISCONNECT_AUTO   |Breaks all connections at  |
** |                    |                      |commit.                    |
** |--------------------|----------------------|---------------------------|
** |SQL_SYNCPOINT       |SQL_SYNC_TWOPHASE     |Requires a Transaction     |
** |                    |                      |Manager (TM) to coordinate |
** |                    |                      |two-phase commits among    |
** |                    |                      |databases that support     |
** |                    |                      |this protocol.             |
** |                    |SQL_SYNC_ONEPHASE     |Uses one-phase commits to  |
** |                    |                      |commit the work done by    |
** |                    |                      |each database in multiple  |
** |                    |                      |database transactions.     |
** |                    |                      |Enforces single updater,   |
** |                    |                      |multiple read behavior.    |
** |                    |SQL_SYNC_NONE         |Uses one-phase commits to  |
** |                    |                      |commit work done, but      |
** |                    |                      |does not enforce single    |
** |                    |                      |updater, multiple read     |
** |                    |                      |behavior.                  |
** |--------------------|----------------------|---------------------------|
** |SQL_MAX_NETBIOS_    |Between 1 and 254     |This specifies the maximum |
** |CONNECTIONS         |                      |number of concurrent       |
** |                    |                      |connections that can be    |
** |                    |                      |made using a NETBIOS       |
** |                    |                      |adapter in an application. |
** |--------------------|----------------------|---------------------------|
** |SQL_DEFERRED_PREPARE|SQL_DEFERRED_PREPARE_ |The PREPARE statement will |
** |                    |NO                    |be executed at the time it |
** |                    |                      |is issued.                 |
** |                    |SQL_DEFERRED_PREPARE_ |Execution of the PREPARE   |
** |                    |YES                   |statement will be deferred |
** |                    |                      |until the corresponding    |
** |                    |                      |OPEN, DESCRIBE, or EXECUTE |
** |                    |                      |statement is issued. The   |
** |                    |                      |PREPARE statement will not |
** |                    |                      |be deferred if it uses the |
** |                    |                      |INTO clause, which         |
** |                    |                      |requires an SQLDA to be    |
** |                    |                      |returned immediately.      |
** |                    |                      |However, if the PREPARE    |
** |                    |                      |INTO statement is issued   |
** |                    |                      |for a cursor that does not |
** |                    |                      |use any parameter markers, |
** |                    |                      |the processing will be     |
** |                    |                      |optimized by pre-OPENing   |
** |                    |                      |the cursor when the        |
** |                    |                      |PREPARE is executed.       |
** |                    |SQL_DEFERRED_PREPARE_ |Same as YES, except that a |
** |                    |ALL                   |PREPARE INTO statement     |
** |                    |                      |which contains parameter   |
** |                    |                      |markers is deferred. If a  |
** |                    |                      |PREPARE INTO statement does|
** |                    |                      |not contain parameter      |
** |                    |                      |markers, pre-OPENing of the|
** |                    |                      |cursor will still be       |
** |                    |                      |performed. If the PREPARE  |
** |                    |                      |statement uses the INTO    |
** |                    |                      |clause to return an SQLDA, |
** |                    |                      |the application must not   |
** |                    |                      |reference the content of   |
** |                    |                      |this SQLDA until the OPEN, |
** |                    |                      |DESCRIBE, or EXECUTE       |
** |                    |                      |statement is issued and    |
** |                    |                      |returned.                  |
** |--------------------|----------------------|---------------------------|
** |SQL_CONNECT_NODE    |Between 0 and 999, or |Specifies the node to which|
** |                    |the keyword           |a connect is to be made.   |
** |                    |SQL_CONN_CATALOG_NODE.|Overrides the value of the |
** |                    |                      |environment variable       |
** |                    |                      |DB2NODE.                   |
** |                    |                      |                           |
** |                    |                      |For example, if nodes 1, 2,|
** |                    |                      |and 3 are defined, the     |
** |                    |                      |client only needs to be    |
** |                    |                      |able to access one of these|
** |                    |                      |nodes. If only node 1      |
** |                    |                      |containing databases has   |
** |                    |                      |been cataloged, and this   |
** |                    |                      |parameter is set to 3, the |
** |                    |                      |next connect attempt will  |
** |                    |                      |result in a connection at  |
** |                    |                      |node 3, after an initial   |
** |                    |                      |connection at node 1.      |
** |--------------------|----------------------|---------------------------|
** |SQL_ATTACH_NODE     |Between 0 and 999.    |Specifies the node to which|
** |                    |                      |an attach is to be made.   |
** |                    |                      |Overrides the value of the |
** |                    |                      |environment variable       |
** |                    |                      |DB2NODE.                   |
** |                    |                      |                           |
** |                    |                      |For example, if nodes 1, 2,|
** |                    |                      |and 3 are defined, the     |
** |                    |                      |client only needs to be    |
** |                    |                      |able to access one of these|
** |                    |                      |nodes. If only node 1      |
** |                    |                      |containing databases has   |
** |                    |                      |been cataloged, and this   |
** |                    |                      |parameter is set to 3, then|
** |                    |                      |the next attach attempt    |
** |                    |                      |will result in an          |
** |                    |                      |attachment at node 3, after|
** |                    |                      |an initial attachment at   |
** |                    |                      |node 1.                    |
** |--------------------|----------------------|---------------------------|
** |Note:                                                                  |
** |These field names are defined for the C programming language. There are|
** |similar names for FORTRAN and COBOL, which have the same semantics.    |
** |-----------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQLE-CONN-SETTING.
**     05 SQLE-CONN-SETTING-ITEM OCCURS 7 TIMES.
**         10 SQLE-CONN-TYPE  PIC S9(4) COMP-5.
**         10 SQLE-CONN-VALUE PIC S9(4) COMP-5.
** * 
** 
*******************************************************************************/
SQL_STRUCTURE sqle_conn_setting        /* Connection setting structure for    */
                                       /* use with distributed unit of work   */
{
        unsigned short         type;   /* Connection setting type             */
        unsigned short         value;  /* Connection setting value            */
};

/******************************************************************************
** sqle_client_info data structure
** This structure is used to pass information to the sqleseti and
** sqleqryi APIs.
** 
** This structure specifies: 
** 
** - The type of information being set or queried 
** - The length of the data being set or queried 
** - A pointer to either: 
** -- An area that will contain the data being set
** -- An area of sufficient length to contain the data being queried
** 
** Applications can specify the following types of information:
** 
** - Client user ID being set or queried. A maximum of 255 characters can
** be set, although servers can truncate this to some platform-specific
** value.
** Note: 
** This user ID is for identification purposes only, and is not used for
** any authorization.
** 
** - Client workstation name being set or queried. A maximum of 255
** characters can be set, although servers can truncate this to some
** platform-specific value.
** 
** - Client application name being set or queried. A maximum of 255
** characters can be set, although servers can truncate this to some
** platform-specific value.
** 
** - Client current package path being set or queried. A maximum of 255
** characters can be set, although servers can truncate this to some
** platform-specific value.
** 
** - Client program ID being set or queried. A maximum of 80 characters
** can be set, although servers can truncate this to some
** platform-specific value.
** 
** - Client accounting string being set or queried. A maximum of 255
** characters can be set, although servers can truncate this to some
** platform-specific value.
** Note:
** The information can be set using the sqlesact API. However, sqlesact
** does not permit the accounting string to be changed once a connection
** exists, whereas sqleseti allows the accounting information to be
** changed for future, as well as already established, connections.
** 
** - Client correlation token being set or queried. A maximum of 255
** characters can be set, although servers can truncate this to some
** platform-specific value.
** 
** Table: Fields in the SQLE-CLIENT-INFO Structure
** ---------------------------------------------------------------------
** |Field Name|Data Type|Description                                   |
** |----------|---------|----------------------------------------------|
** |TYPE      |sqlint32 |Setting type.                                 |
** |----------|---------|----------------------------------------------|
** |LENGTH    |sqlint32 |Length of the value. On sqleseti calls, the   |
** |          |         |length can be between zero and the maximum    |
** |          |         |length defined for the type. A length of zero |
** |          |         |indicates a null value. On sqleqryi calls,    |
** |          |         |the length is returned, but the area pointed  |
** |          |         |to by pValue must be large enough to contain  |
** |          |         |the maximum length for the type. A length of  |
** |          |         |zero indicates a null value.                  |
** |----------|---------|----------------------------------------------|
** |PVALUE    |Pointer  |Pointer to an application-allocated buffer    |
** |          |         |that contains the specified value. The data   |
** |          |         |type of this value is dependent on the type   |
** |          |         |field.                                        |
** |----------|---------|----------------------------------------------|
** 
** The valid entries for the SQLE-CLIENT-INFO TYPE element and the
** associated descriptions for each entry are listed below:
** 
** Table: Connection Settings
** ------------------------------------------------------------------------
** |Type                         |Data Type |Description                   |
** |-----------------------------|----------|------------------------------|
** |SQLE_CLIENT_INFO_USERID      |CHAR(255) |The user ID for the client.   |
** |                             |          |Some servers may truncate the |
** |                             |          |value. This user ID is for    |
** |                             |          |identification purposes only, |
** |                             |          |and is not used for any       |
** |                             |          |authorization.                |
** |-----------------------------|----------|------------------------------|
** |SQLE_CLIENT_INFO_WRKSTNNAME  |CHAR(255) |The workstation name for the  |
** |                             |          |client. Some servers may      |
** |                             |          |truncate the value.           |
** |-----------------------------|----------|------------------------------|
** |SQLE_CLIENT_INFO_APPLNAME    |CHAR(255) |The application name for the  |
** |                             |          |client. Some servers may      |
** |                             |          |truncate the value.           |
** |-----------------------------|----------|------------------------------|
** |SQLE_CLIENT_INFO_PROGRAMID   |CHAR(80)  |The program identifier for    |
** |                             |          |the client. Once this element |
** |                             |          |is set, DB2 for z/OS          |
** |                             |          |associates this identifier    |
** |                             |          |with any statements inserted  |
** |                             |          |into the dynamic SQL statement|
** |                             |          |cache. This element is only   |
** |                             |          |supported for applications    |
** |                             |          |accessing DB2 for z/OS.       |
** |-----------------------------|----------|------------------------------|
** |SQLE_CLIENT_INFO_ACCTSTR     |CHAR(255) |The accounting string for the |
** |                             |          |client. Some servers may      |
** |                             |          |truncate the value.           |
** |-----------------------------|----------|------------------------------|
** |SQLE_CLIENT_INFO_AUTOCOMMIT  |CHAR(1)   |The autocommit setting of the |
** |                             |          |client. It can be set to      |
** |                             |          |SQLE_CLIENT_AUTOCOMMIT_ON or  |
** |                             |          |SQLE_CLIENT_AUTOCOMMIT_OFF.   |
** |-----------------------------|----------|------------------------------|
** |SQLE_CLIENT_INFO_CORR_TOKEN  |CHAR(255) |The correlation token for the |
** |                             |          |client. This option is        |
** |                             |          |supported only by DB2 for z/OS|
** |                             |          |servers,                      |
** |-----------------------------------------------------------------------|
** |Note:                                                                  |
** |These field names are defined for the C programming language. There    |
** |are similar names for FORTRAN and COBOL, which have the same           |
** |semantics.                                                             |
** |-----------------------------------------------------------------------|
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQLE-CLIENT-INFO.
**     05 SQLE-CLIENT-INFO-ITEM OCCURS 4 TIMES.
**         10 SQLE-CLIENT-INFO-TYPE   PIC S9(4) COMP-5.
**         10 SQLE-CLIENT-INFO-LENGTH PIC S9(4) COMP-5.
**         10 SQLE-CLIENT-INFO-VALUE  USAGE IS POINTER.
** * 
*******************************************************************************/
SQL_STRUCTURE sqle_client_info         /* SET CLIENT Information structure    */
                                       /* for use with sqleseti and sqleqryi  */
{
        unsigned short         type;   /* information setting type            */
        unsigned short         length; /* information setting length          */
        char                   *pValue; /* information setting value          */
};

/* Database Codepage and Territory code info structure using SQL_CODESET      */
/* SIZE and SQL_LOCALE_SIZE                                                   */

/******************************************************************************
** sqle_db_territory_info data structure
*******************************************************************************/
SQL_STRUCTURE sqle_db_territory_info
{
        char                   sqldbcodeset[SQL_CODESET_SIZE + 1]; 
        char                   sqldblocale[SQL_LOCALE_SIZE + 1]; /* database  */
                                       /* territory                           */
};

typedef SQL_STRUCTURE sqle_db_territory_info SQLE_DB_TERRITORY_INFO;

/* Register/Deregister constants and data structures                          */

/* Defines for LOCATION TO REGISTER parameter                                 */
#define SQL_NWBINDERY          0

/* Register/deregister info. for NW bindery data structure                    */
/******************************************************************************
** sqle_reg_nwbindery data structure
*******************************************************************************/
SQL_STRUCTURE sqle_reg_nwbindery
{
        char                   uid[SQL_NW_UID_SZ+1];
        unsigned short         reserved_len_1;
        char                   pswd[SQL_NW_PSWD_SZ+1];
        unsigned short         reserved_len_2;
};

/******************************************************************************
** Start/Stop Database Manager constants and data structures
*******************************************************************************/

/* V5 Start Options structure Eye Catcher value                               */
#define SQLE_STARTOPTID_V51    "SQLOPT01"

/* V5 Start Database Manager structure                                        */
/******************************************************************************
** sqle_start_options data structure
*******************************************************************************/
SQL_STRUCTURE sqle_start_options
{
        char                   sqloptid[8]; /* Structure Eye Catcher          */
        sqluint32              isprofile; /* Profile specified?               */
        char                   profile[SQL_PROFILE_SZ+1]; /* Profile          */
        sqluint32              isnodenum; /* Node number specified?           */
        SQL_PDB_NODE_TYPE      nodenum; /* Node number                        */
        sqluint32              option; /* Start option                        */
        sqluint32              ishostname; /* Hostname specified?             */
        char                   hostname[SQL_HOSTNAME_SZ+1]; /* Hostname       */
        sqluint32              isport; /* Port specified?                     */
        SQL_PDB_PORT_TYPE      port;   /* Port                                */
        sqluint32              isnetname; /* Netname specified?               */
        char                   netname[SQL_HOSTNAME_SZ+1]; /* Netname         */
        sqluint32              tblspace_type; /* Addnode tablespace type      */
        SQL_PDB_NODE_TYPE      tblspace_node; /* Addnode tablespace node      */
        sqluint32              iscomputer; /* Computername specified?         */
        char                   computer[SQL_COMPUTERNAME_SZ+1]; 
        char                   *pUserName; /* Logon Account user name         */
        char                   *pPassword; /* Logon Account password          */
};

/* V5 Stop Database Manager structure                                         */
/******************************************************************************
** sqledbstopopt data structure
*******************************************************************************/
SQL_STRUCTURE sqledbstopopt
{
        sqluint32              isprofile; /* Profile specified?               */
        char                   profile[SQL_PROFILE_SZ+1]; /* Profile          */
        sqluint32              isnodenum; /* Node number specified?           */
        SQL_PDB_NODE_TYPE      nodenum; /* Node number                        */
        sqluint32              ishostname; /* Hostname specified?             */
        char                   hostname[SQL_HOSTNAME_SZ+1]; /* Hostname       */
        sqluint32              option; /* Option                              */
        sqluint32              callerac; /* Caller action                     */
        sqlint32               iquiescedeferminutes; /* QUIESCE timeout       */
};

/* Add Node data structures                                                   */

/* V5 Add Node Option structure Eye Catcher value                             */
#define SQLE_ADDOPTID_V51      "SQLADD01"

/* Add Node structure                                                         */
/******************************************************************************
** sqle_addn_options data structure
** This structure is used to pass information to the sqleaddn API.
** 
** Table: Fields in the SQLE-ADDN-OPTIONS Structure
** -------------------------------------------------------------------------
** |Field Name   |Data Type        |Description                            |
** |-------------|-----------------|---------------------------------------|
** |SQLADDID     |CHAR             |An "eyecatcher" value which must be    |
** |             |                 |set to SQLE_ADDOPTID_V51.              |
** |-------------|-----------------|---------------------------------------|
** |TBLSPACE_TYPE|sqluint32        |Specifies the type of system temporary |
** |             |                 |table space definitions to be used for |
** |             |                 |the node being added. See below for    |
** |             |                 |values.                                |
** |-------------|-----------------|---------------------------------------|
** |TBLSPACE_NODE|SQL_PDB_NODE_TYPE|Specifies the node number from which   |
** |             |                 |the system temporary table space       |
** |             |                 |definitions should be obtained. The    |
** |             |                 |node number must exist in the          |
** |             |                 |db2nodes.cfg file, and is only used    |
** |             |                 |if the tblspace_type field is set to   |
** |             |                 |SQLE_TABLESPACES_LIKE_NODE.            |
** |-------------|-----------------|---------------------------------------|
** 
** Valid values for TBLSPACE_TYPE (defined in sqlenv) are:
** 
** - SQLE_TABLESPACES_NONE
** Do not create any system temporary table spaces. 
** 
** - SQLE_TABLESPACES_LIKE_NODE
** The containers for the system temporary table spaces should be the
** same as those for the specified node. 
** 
** - SQLE_TABLESPACES_LIKE_CATALOG
** The containers for the system temporary table spaces should be the
** same as those for the catalog node of each database. 
** 
** COBOL Structure
** 
** * File: sqlenv.cbl
** 01 SQLE-ADDN-OPTIONS.
**     05 SQLADDID               PIC X(8).
**     05 SQL-TBLSPACE-TYPE      PIC 9(9) COMP-5.
**     05 SQL-TBLSPACE-NODE      PIC S9(4) COMP-5.
**     05 FILLER                 PIC X(2).
** * 
** 
*******************************************************************************/
SQL_STRUCTURE sqle_addn_options
{
        char                   sqladdid[8]; /* Structure Eye Catcher          */
        sqluint32              tblspace_type; /* Temporary Tablespace Type    */
        SQL_PDB_NODE_TYPE      tblspace_node; /* Temporary Tablespace Node    */
};

#if defined(SQLZ_STACK)
#define SQLENOP 0
#else

/******************************************************************************
** Database Environment Commands -- Function Prototypes
*******************************************************************************/


#if defined(SQL_REL_10)
#define sqledrpd(dbn,pw,ca) sqledrpd_api(dbn,pw,ca)
#else
#define sqledrpd(dbn,ca) sqledrpd_api(dbn, (_SQLOLDCHAR *) "",ca)
#endif
#define sqlectdd(dbn,als,ty,nm,dr,com,au,ca) sqlectdd_api(dbn,\
  als,ty,nm,dr,com,au,ca)
#define sqlecadb(dbn,als,ty,nm,dr,com,au,dp,ca) sqlecadb_api(dbn,\
  als,ty,nm,dr,com,au,dp,ca)
#define sqlectnd(node,prot,ca) sqlectnd_api(node,prot,ca)
#define sqledbcr(dbn,drv,dbdesc,au,dbcinfo,ca) \
  sqledbcr_api(dbn,drv,dbdesc,au,dbcinfo,ca)
#define sqledcgd(dbn,drv,com,ca) sqledcgd_api(dbn,drv,com,ca)
#define sqledcls(ha,ca) sqledcls_api(ha,ca)
#define sqledgne(ha,bu,ca) sqledgne_api(ha,bu,ca)
#define sqledosd(dr,ha,co,ca) sqledosd_api(dr,ha,co,ca)
#define sqlefrce(cn,ap,mo,ca) sqlefrce_api(cn,ap,mo,ca)
#define sqlegdad(entry,ca) sqlegdad_api(entry,ca)
#define sqlegdcl(ca) sqlegdcl_api(ca)
#define sqlegdel(entry,ca) sqlegdel_api(entry,ca)
#define sqlegdge(entry,ca) sqlegdge_api(entry,ca)
#define sqlegdgt(ct,entry,ca) sqlegdgt_api(ct,entry,ca)
#define sqlegdsc(ct,ca) sqlegdsc_api(ct,ca)
#define sqlegins(iname,ca) sqlegins_api(iname,ca)
#define sqlesdeg(na,pa,dg,ca) sqlesdeg_api(na,pa,dg,ca)

#define sqleregs(loc,reg,ca) sqleregs_api(loc,reg,ca)
#define sqledreg(loc,reg,ca) sqledreg_api(loc,reg,ca)
#define sqleintr() sqleintr_api()
#define sqleisig(ca) sqleisig_api(ca)
#define sqlefmem(ca,mptr) sqlefmem_api(ca,mptr)
#define sqlemgdb(db,id,pw,ca) sqlemgdb_api(db,id,pw,ca)
#define sqle_activate_db(db,id,pw,rr,ca) sqle_activate_db_api(db,id,pw,rr,ca)
#define sqle_deactivate_db(db,id,pw,rr,ca) sqle_deactivate_db_api(db,id,pw,rr,ca)
#define sqlencls(ha,ca) sqlencls_api(ha,ca)
#define sqlengne(ha,bu,ca) sqlengne_api(ha,bu,ca)
#define sqlenops(ha,bu,ca) sqlenops_api(ha,bu,ca)
#define sqlepstr(so,ca) sqlepstr_api(so,ca)
#define sqlepstp(so,ca) sqlepstp_api(so,ca)
#define sqlepstart(so,ca) sqlepstart_api(so,ca)
#define sqlerstd(dbn,id,pw,ca) sqlerstd_api(dbn,id,pw,ca)
#define sqlesapr(ag,pl,pt,ca) sqlesapr_api(ag,pl,pt,ca)
#define sqlestar() sqlestar_api()
#define sqlestop(ca) sqlestop_api(ca)
#define sqleuncd(dbn,ca) sqleuncd_api(dbn,ca)
#define sqleuncn(nn,ca) sqleuncn_api(nn,ca)
#define sqlesetc(set,num,ca) sqlesetc_api(set,num,ca)
#define sqleqryc(set,num,ca) sqleqryc_api(set,num,ca)
#define sqleseti(aliasl,alias,num,sci,ca) sqleseti_api(aliasl,alias,num,sci,ca)
#define sqleqryi(aliasl,alias,num,sci,ca) sqleqryi_api(aliasl,alias,num,sci,ca)
#define sqleatcp(iname,id,pw,npw,ca) sqleatcp_api(iname,id,pw,npw,ca)
#define sqleatin(iname,id,pw,ca) sqleatin_api(iname,id,pw,ca)
#define sqledtin(ca) sqledtin_api(ca)
#define sqlecrea(dbn,alias,drv,dbdesc,dbcinfo,rsvd2,dbdescext,ca) \
        sqlecrea_api(dbn,alias,drv,dbdesc,dbcinfo,rsvd2,dbdescext,ca)
#define sqlesact(str,ca) sqlesact_api(str,ca)
#define sqleaddn(rsvd,ca)  sqleaddn_api(rsvd,ca)
#define sqlecran(dbn,rsvd,ca)  sqlecran_api(dbn,rsvd,ca)
#define sqledpan(dbn,rsvd,ca)  sqledpan_api(dbn,rsvd,ca)
#define sqledrpn(ac,rsvd,ca)  sqledrpn_api(ac,rsvd,ca)
#endif

/******************************************************************************
** sqlectdd API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Catalog Database              */
  sqlectdd_api (
        _SQLOLDCHAR * pDbName,               /* database                      */
        _SQLOLDCHAR * pDbAlias,              /* alias                         */
        unsigned char Type,                  /* type                          */
        _SQLOLDCHAR * pNodeName,             /* node name                     */
        _SQLOLDCHAR * pPath,                 /* drive/path                    */
        _SQLOLDCHAR * pComment,              /* comment                       */
        unsigned short Authentication,       /* authentication                */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlecadb API
** Stores database location information in the system database directory. The
** database can be located either on the local workstation or on a remote node.
** 
** Scope
** 
** This API affects the system database directory. In a partitioned database
** environment, when cataloging a local database into the system database
** directory, this API must be called from a database partition server where the
** database resides.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlecadb API parameters
** 
** pDbName
** Input. A string containing the database name.
** 
** pDbAlias
** Input. A string containing an alias for the database.
** 
** Type
** Input. A single character that designates whether the database is indirect,
** remote, or is cataloged via DCE. Valid values (defined in sqlenv) are:
** - SQL_INDIRECT
** Specifies that the database resides at this instance.
** - SQL_REMOTE
** Specifies that the database resides at another instance.
** - SQL_DCE
** Specifies that the database is cataloged via DCE.
** 
** pNodeName
** Input. A string containing the name of the node where the database is
** located. May be NULL.
** Note:
** If neither pPath nor pNodeName is specified, the database is assumed to
** be local, and the location of the database is assumed to be that specified in
** the database manager configuration parameter dftdbpath.
** 
** pPath
** Input. A string which, on UNIX based systems, specifies the name of the path
** on which the database being cataloged resides. Maximum length is 215
** characters.
** 
** On the Windows operating system, this string specifies the letter of
** the drive on which the database being cataloged resides.
** 
** If a NULL pointer is provided, the default database path is assumed to
** be that specified by the database manager configuration parameter
** dftdbpath.
** 
** pComment
** Input. A string containing an optional description of the database. A null
** string indicates no comment. The maximum length of a comment string is 30
** characters.
** 
** Authentication
** Input. Contains the authentication type specified for the database.
** Authentication is a process that verifies that the user is who he/she
** claims to be. Access to database objects depends on the user's
** authentication. Valid values (from sqlenv) are:
** 
** - SQL_AUTHENTICATION_SERVER
** Specifies that authentication takes place on the node containing the target
** database.
** 
** - SQL_AUTHENTICATION_CLIENT
** Specifies that authentication takes place on the node where the
** application is invoked.
** 
** - SQL_AUTHENTICATION_KERBEROS
** Specifies that authentication takes place using Kerberos Security
** Mechanism.
** 
** - SQL_AUTHENTICATION_NOT_SPECIFIED
** Authentication not specified.
** 
** - SQL_AUTHENTICATION_SVR_ENCRYPT
** Specifies that authentication takes place on the node containing the target
** database, and that the authentication password is to be encrypted.
** 
** - SQL_AUTHENTICATION_DATAENC
** Specifies that authentication takes place on the node containing the target
** database, and that connections must use data encryption.
** 
** - SQL_AUTHENTICATION_GSSPLUGIN
** Specifies that authentication takes place using an external GSS API-based
** plug-in security mechanism.
** 
** This parameter can be set to SQL_AUTHENTICATION_NOT_SPECIFIED, except when
** cataloging a database that resides on a DB2 Version 1 server.
** 
** Specifying the authentication type in the database catalog results in a
** performance improvement during a connect.
** 
** pPrincipal
** Input. A string containing the principal name of the DB2
** server on which the database resides. This value should only be specified
** when authentication is SQL_AUTHENTICATION_KERBEROS.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** Use CATALOG DATABASE to catalog databases located on local or remote
** nodes, recatalog databases that were uncataloged previously, or maintain
** multiple aliases for one database (regardless of database location).
** 
** DB2 automatically catalogs databases when they are created. It catalogs an
** entry for the database in the local database directory, and another entry in
** the system database directory. If the database is created from a
** remote client (or a client which is executing from a different instance
** on the same machine), an entry is also made in the system database
** directory at the client instance.
** 
** Databases created at the current instance (as defined by the value of the
** DB2INSTANCE environment variable) are cataloged as indirect. Databases
** created at other instances are cataloged as remote (even if they
** physically reside on the same machine).
** 
** CATALOG DATABASE automatically creates a system database directory if one
** does not exist. The system database directory is stored on the path that
** contains the database manager instance that is being used. The system
** database directory is maintained outside of the database. Each entry
** in the directory contains:
** 
** - Alias
** - Authentication type
** - Comment
** - Database
** - Entry type
** - Local database directory (when cataloging a local database)
** - Node name (when cataloging a remote database)
** - Release information.
** 
** If a database is cataloged with the type parameter set to SQL_INDIRECT, the
** value of the authentication parameter provided will be ignored, and the
** authentication in the directory will be set to
** SQL_AUTHENTICATION_NOT_SPECIFIED.
** 
** If directory caching is enabled, database, node, and DCS directory files are
** cached in memory. An application's directory cache is created during
** its first directory lookup. Since the cache is only refreshed when
** the application modifies any of the directory files, directory changes
** made by other applications may not be effective until the application
** has restarted. To refresh DB2's shared cache (server only), stop
** (db2stop) and then restart (db2start) the database manager. To
** refresh the directory cache for another application, stop and then
** restart that application.
** 
** REXX API syntax
** 
** CATALOG DATABASE dbname [AS alias] [ON path|AT NODE nodename]
** [AUTHENTICATION authentication] [WITH "comment"]
** CATALOG GLOBAL DATABASE db_global_name AS alias
** USING DIRECTORY {DCE} [WITH "comment"]
** 
** REXX API parameters
** 
** dbname
** Name of the database to be cataloged.
** 
** alias
** Alternate name for the database. If an alias is not specified, the database
** name is used as the alias.
** 
** path
** Path on which the database being cataloged resides.
** 
** nodename
** Name of the remote workstation where the database being cataloged resides.
** Note:
** If neither path nor nodename is specified, the database is assumed to be
** local, and the location of the database is assumed to be that specified in
** the database manager configuration parameter dftdbpath.
** 
** authentication
** Place where authentication is to be done. Valid values are:
** 
** - SERVER
** Authentication occurs at the node containing the target database. This is the
** default.
** 
** - CLIENT
** Authentication occurs at the node where the application is invoked.
** 
** - KERBEROS
** Specifies that authentication takes place using Kerberos Security
** Mechanism.
** 
** - NOT_SPECIFIED
** Authentication not specified.
** 
** - SVR_ENCRYPT
** Specifies that authentication takes place on the node containing the target
** database, and that the authentication password is to be encrypted.
** 
** - DATAENC
** Specifies that authentication takes place on the node containing the target
** database, and that connections must use data encryption.
** 
** - GSSPLUGIN
** Specifies that authentication takes place using an external GSS API-based
** plug-in security mechanism.
** 
** comment
** Describes the database or the database entry in the system database
** directory. The maximum length of a comment string is 30 characters.
** A carriage return or a line feed character is not permitted. The
** comment text must be enclosed by double quotation marks.
** 
** db_global_name
** The fully qualified name that uniquely identifies the database in
** the DCE name space.
** 
** DCE
** The global directory service being used.
** 
** REXX examples
** 
** call SQLDBS 'CATALOG GLOBAL DATABASE /.../cell1/subsys/database/DB3
** AS dbtest USING DIRECTORY DCE WITH "Sample Database"'
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Catalog Database              */
  sqlecadb_api (
        _SQLOLDCHAR * pDbName,               /* database                      */
        _SQLOLDCHAR * pDbAlias,              /* alias                         */
        unsigned char Type,                  /* type                          */
        _SQLOLDCHAR * pNodeName,             /* node name                     */
        _SQLOLDCHAR * pPath,                 /* drive/path                    */
        _SQLOLDCHAR * pComment,              /* comment                       */
        unsigned short Authentication,       /* authentication                */
        _SQLOLDCHAR * pPrincipal,            /* principal name if auth=DCE    */
                                             /* or KERBEROS                   */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlectnd API
** Stores information in the node directory about the location of a DB2 server
** instance based on the communications protocol used to access that instance.
** The information is needed to establish a database connection or attachment
** between an application and a server instance.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlectnd API parameters
** 
** pNodeInfo
** Input. A pointer to a node directory structure.
** 
** pProtocolInfo
** Input. A pointer to the protocol structure:
** - SQLE-NODE-CPIC
** - SQLE-NODE-IPXSPX
** - SQLE-NODE-LOCAL
** - SQLE-NODE-NETB
** - SQLE-NODE-NPIPE
** - SQLE-NODE-TCPIP.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** DB2 creates the node directory on the first call to this API if the node
** directory does not exist. On the Windows operating system, the node directory
** is stored in the directory of the instance being used. On UNIX based systems,
** it is stored in the DB2 install directory (sqllib, for example).
** 
** If directory caching is enabled, database, node, and DCS directory files are
** cached in memory. An application's directory cache is created during
** its first directory lookup. Since the cache is only refreshed when
** the application modifies any of the directory files, directory changes
** made by other applications may not be effective until the application
** has restarted. To refresh DB2's shared cache (server only), stop
** (db2stop command) and then restart (db2start command) the database manager.
** To refresh the directory cache for another application, stop and then
** restart that application.
** 
** REXX API syntax
** 
** CATALOG APPC NODE nodename DESTINATION symbolic_destination_name
** [SECURITY {NONE|SAME|PROGRAM}]
** [WITH comment]
** 
** REXX API parameters
** 
** nodename
** Alias for the node to be cataloged.
** 
** symbolic_destination_name
** Symbolic destination name of the remote partner node.
** 
** comment
** An optional description associated with this node directory entry. Do not
** include a CR/LF character in a comment. Maximum length is 30 characters. The
** comment text must be enclosed by double quotation marks.
** 
** REXX API syntax
** 
** CATALOG IPXSPX NODE nodename REMOTE file_server SERVER objectname
** [WITH comment]
** 
** REXX API parameters
** 
** nodename
** Alias for the node to be cataloged.
** 
** file_server
** Name of the NetWare file server where the internetwork address of the
** database manager instance is registered. The internetwork address is
** stored in the bindery at the NetWare file server, and is accessed
** using objectname.
** 
** objectname
** The database manager server instance is represented as the object,
** objectname, on the NetWare file server. The server's IPX/SPX internetwork
** address is stored and retrieved from this object.
** 
** comment
** An optional description associated with this node directory entry. Do not
** include a CR/LF character in a comment. Maximum length is 30 characters. The
** comment text must be enclosed by double quotation marks.
** 
** REXX API syntax
** 
** CATALOG LOCAL NODE nodename INSTANCE instance_name [WITH comment]
** 
** REXX API parameters
** 
** nodename
** Alias for the node to be cataloged.
** 
** instance_name
** Name of the instance to be cataloged.
** 
** comment
** An optional description associated with this node directory entry. Do not
** include a CR/LF character in a comment. Maximum length is 30 characters. The
** comment text must be enclosed by double quotation marks.
** 
** REXX API syntax
** 
** CATALOG NETBIOS NODE nodename REMOTE server_nname ADAPTER adapternum
** [WITH comment]
** 
** REXX API parameters
** 
** nodename
** Alias for the node to be cataloged.
** 
** server_nname
** Name of the remote workstation. This is the workstation name (nname) found in
** the database manager configuration file of the server instance.
** 
** adapternum
** Local LAN adapter number.
** 
** comment
** An optional description associated with this node directory entry. Do not
** include a CR/LF character in a comment. Maximum length is 30 characters. The
** comment text must be enclosed by double quotation marks.
** 
** REXX API syntax
** 
** CATALOG NPIPE NODE nodename REMOTE computer_name INSTANCE instance_name
** 
** REXX API parameters
** 
** nodename
** Alias for the node to be cataloged.
** 
** computer_name
** The computer name of the node on which the target database resides.
** 
** instance_name
** Name of the instance to be cataloged.
** 
** REXX API syntax
** 
** CATALOG TCPIP NODE nodename REMOTE hostname SERVER servicename
** [WITH comment]
** 
** REXX API parameters
** 
** nodename
** Alias for the node to be cataloged.
** 
** hostname
** Host name of the node where the target database resides.
** 
** servicename
** Either the service name of the database manager instance on the remote node,
** or the port number associated with that service name.
** 
** comment
** An optional description associated with this node directory entry. Do not
** include a CR/LF character in a comment. Maximum length is 30 characters. The
** comment text must be enclosed by double quotation marks.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Catalog Node                  */
  sqlectnd_api (
        struct sqle_node_struct * pNodeInfo, /* node structure ptr            */
        void * pProtocolInfo,                /* protocol structure ptr        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledcgd API
** Changes a database comment in the system database directory or the local
** database directory. New comment text can be substituted for text currently
** associated with a comment.
** 
** Scope
** 
** This API only affects the database partition server on which it is issued.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqledcgd API parameters
** 
** pDbAlias
** Input. A string containing the database alias. This is the name that is
** cataloged in the system database directory, or the name cataloged in
** the local database directory if the path is specified.
** 
** pPath
** Input. A string containing the path on which the local database directory
** resides. If the specified path is a null pointer, the system database
** directory is used.
** 
** The comment is only changed in the local database directory or the
** system database directory on the database partition server on which
** the API is executed. To change the database comment on all database
** partition servers, run the API on every database partition server.
** 
** pComment
** Input. A string containing an optional description of the database. A null
** string indicates no comment. It can also indicate no change to an existing
** database comment.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** New comment text replaces existing text. To append information, enter the old
** comment text, followed by the new text.
** 
** Only the comment for an entry associated with the database alias is modified.
** Other entries with the same database name, but with different aliases,
** are not affected.
** 
** If the path is specified, the database alias must be cataloged in the local
** database directory. If the path is not specified, the database alias must be
** cataloged in the system database directory.
** 
** REXX API syntax
** 
** CHANGE DATABASE database_alias COMMENT [ON path] WITH comment
** 
** REXX API parameters
** 
** database_alias
** Alias of the database whose comment is to be changed.
** 
** To change the comment in the system database directory, it is necessary to
** specify the database alias.
** 
** If the path where the database resides is specified (with the path
** parameter), enter the name (not the alias) of the database. Use this
** method to change the comment in the local database directory.
** 
** path
** Path on which the database resides.
** 
** comment
** Describes the entry in the system database directory or the local database
** directory. Any comment that helps to describe the cataloged database can be
** entered. The maximum length of a comment string is 30 characters. A carriage
** return or a line feed character is not permitted. The comment text must be
** enclosed by double quotation marks.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Change Database Comment       */
  sqledcgd_api (
        _SQLOLDCHAR * pDbAlias,              /* database alias                */
        _SQLOLDCHAR * pPath,                 /* drive/path                    */
        _SQLOLDCHAR * pComment,              /* comment                       */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledcls API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Close Directory Scan          */
  sqledcls_api (
        unsigned short Handle,               /* handle                        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledgne API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get Next Directory Entry      */
  sqledgne_api (
        unsigned short Handle,               /* handle                        */
        struct sqledinfo ** ppDbDirEntry,    /* buffer                        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledosd API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Open Directory Scan           */
  sqledosd_api (
        _SQLOLDCHAR * pPath,                 /* drive/path                    */
        unsigned short * pHandle,            /* handle                        */
        unsigned short * pNumEntries,        /* count                         */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledrpd API
** Deletes the database contents and all log files for the database, uncatalogs
** the database, and deletes the database subdirectory.
** 
** Scope
** 
** By default, this API affects all database partition servers that are
** listed in the db2nodes.cfg file.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** Instance. It is not necessary to call ATTACH before dropping a remote
** database. If the database is cataloged as remote, an instance
** attachment to the remote node is established for the duration of
** the call.
** 
** API include file
** 
** sqlenv.h
** 
** sqledrpd API parameters
** 
** pDbAlias
** Input. A string containing the alias of the database to be dropped. This name
** is used to reference the actual database name in the system database
** directory.
** 
** pReserved2
** A spare pointer that is set to null or points to zero. Reserved for
** future use.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The sqledrpd API deletes all user data and log files. If the log files are
** needed for a roll-forward recovery after a restore operation, the
** files should be saved prior to calling this API.
** 
** The database must not be in use; all users must be disconnected from the
** database before the database can be dropped.
** 
** To be dropped, a database must be cataloged in the system database directory.
** Only the specified database alias is removed from the system database
** directory. If other aliases with the same database name exist, their entries
** remain. If the database being dropped is the last entry in the local database
** directory, the local database directory is deleted automatically.
** 
** If this API is called from a remote client (or from a different
** instance on the same machine), the specified alias is removed from
** the client's system database directory. The corresponding database
** name is removed from the server's system database directory.
** 
** This API unlinks all files that are linked through any DATALINK
** columns. Since the unlink operation is performed asynchronously
** on the DB2 Data Links Manager, its effects may not be seen immediately
** on the DB2 Data Links Manager, and the unlinked files may not be
** immediately available for other operations. When the API is called,
** all the DB2 Data Links Managers configured to that database must
** be available; otherwise, the drop database operation will fail.
** 
** REXX API syntax
** 
** DROP DATABASE dbalias
** 
** REXX API parameters
** 
** dbalias
** The alias of the database to be dropped.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Drop Database                 */
  sqledrpd_api (
        _SQLOLDCHAR * pDbAlias,              /* database alias                */
        _SQLOLDCHAR * pReserved2,            /* reserved                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlesdeg API
** Sets the maximum run time degree of intra-partition parallelism for SQL
** statement execution for specified active applications. It has no effect
** on CREATE INDEX statement execution parallelism.
** 
** Scope
** 
** This API affects all database partition servers that are listed in the
** db2nodes.cfg file.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** Instance. To change the maximum run time degree of parallelism on a remote
** server, it is first necessary to attach to that server. If no attachment
** exists, the SET RUNTIME DEGREE statement fails.
** 
** API include file
** 
** sqlenv.h
** 
** sqlesdeg API parameters
** 
** NumAgentIds
** Input. An integer representing the total number of active applications
** to which the new degree value will apply. This number should be the
** same as the number of elements in the array of agent IDs.
** 
** If this parameter is set to SQL_ALL_USERS (defined in sqlenv), the new degree
** will apply to all active applications. If it is set to zero, an error is
** returned.
** 
** pAgentIds
** Input. Pointer to an array of unsigned long integers. Each entry
** describes the agent ID of the corresponding application. To list the
** agent IDs of the active applications, use the db2GetSnapshot API.
** 
** Degree
** Input. The new value for the maximum run time degree of parallelism.
** The value must be in the range 1 to 32767.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The database system monitor functions are used to gather the agent IDs and
** degrees of active applications.
** 
** Minimal validation is performed on the array of agent IDs. The user
** must ensure that the pointer points to an array containing the total
** number of elements specified. If NumAgentIds is set to SQL_ALL_USERS,
** the array is ignored.
** 
** If one or more specified agent IDs cannot be found, the unknown agent IDs are
** ignored, and the function continues. No error is returned. An agent
** ID may not be found, for instance, if the user signs off between the
** time an agent ID is collected and the API is called.
** 
** Agent IDs are recycled, and are used to change the degree of parallelism for
** applications some time after being gathered by the database system monitor.
** When a user signs off, therefore, another user may sign on and acquire
** the same agent ID through this recycling process, with the result that
** the new degree of parallelism will be modified for the wrong user.
** 
** REXX API syntax
** 
** This API can be called from REXX through the SQLDB2 interface.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Set intra query parallelism   */
                                             /* degree                        */
  sqlesdeg_api (
        sqlint32 NumAgentIds,                /* number of agents to update    */
        sqluint32 * pAgentIds,               /* array of agent ids            */
        sqlint32 Degree,                     /* degree of parallelism         */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlefrce API
** Forces local or remote users or applications off the system to allow for
** maintenance on a server.
** 
** Attention: If an operation that cannot be interrupted (a database
** restore, for example) is forced, the operation must be successfully
** re-executed before the database becomes available.
** 
** Scope
** 
** This API affects all database partition servers that are listed in the
** db2nodes.cfg file.
** 
** In a partitioned database environment, this API does not have to be
** issued from the coordinator partition of the application being forced.
** This API can be issued from any database partition server in the
** partitioned database environment.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** - sysmaint
** 
** Required connection
** 
** Instance. To force users off a remote server, it is necessary to first attach
** to that server. If no attachment exists, this API is executed locally.
** 
** API include file
** 
** sqlenv.h
** 
** sqlefrce API parameters
** 
** NumAgentIds
** Input. An integer representing the total number of users to be
** terminated. This number should be the same as the number of elements
** in the array of agent IDs.
** 
** If this parameter is set to SQL_ALL_USERS (defined in sqlenv), all
** applications with either database connections or instance attachments
** are forced. If it is set to zero, an error is returned.
** 
** pAgentIds
** Input. Pointer to an array of unsigned long integers. Each entry
** describes the agent ID of the corresponding database user.
** 
** ForceMode
** Input. An integer specifying the operating mode of the sqlefrce API. Only the
** asynchronous mode is supported. This means that the API does not wait until
** all specified users are terminated before returning. It returns as
** soon as the API has been issued successfully, or an error occurs. As
** a result, there may be a short interval between the time the force
** application call completes and the specified users have been
** terminated.
** 
** This parameter must be set to SQL_ASYNCH (defined in sqlenv).
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The database manager remains active so that subsequent database manager
** operations can be handled without the need for db2start.
** 
** To preserve database integrity, only users who are idling or executing
** interruptible database operations can be terminated.
** 
** After a force has been issued, the database will still accept requests to
** connect. Additional forces may be required to completely force all users off.
** The database system monitor functions are used to gather the agent IDs of the
** users to be forced.
** 
** When the force mode is set to SQL_ASYNCH (the only value permitted), the API
** immediately returns to the calling application.
** 
** Minimal validation is performed on the array of agent IDs to be forced. The
** user must ensure that the pointer points to an array containing the total
** number of elements specified. If NumAgentIds is set to SQL_ALL_USERS,
** the array is ignored.
** 
** When a user is terminated, a unit of work rollback is performed to ensure
** database consistency.
** 
** All users that can be forced will be forced. If one or more specified
** agent IDs cannot be found, sqlcode in the sqlca structure is set to
** 1230. An agent ID may not be found, for instance, if the user signs
** off between the time an agent ID is collected and sqlefrce is called.
** The user that calls this API is never forced off.
** 
** Agent IDs are recycled, and are used to force applications some time after
** being gathered by the database system monitor. When a user signs off,
** therefore, another user may sign on and acquire the same agent ID
** through this recycling process, with the result that the wrong user
** may be forced.
** 
** REXX API syntax
** 
** FORCE APPLICATION {ALL | :agentidarray} [MODE ASYNC]
** 
** REXX API parameters
** 
** ALL
** All applications will be disconnected. This includes applications that have
** database connections and applications that have instance attachments.
** 
** agentidarray
** A compound REXX host variable containing the list of agent IDs to be
** terminated. In the following, XXX is the name of the host variable:
** - XXX.0
** Number of agents to be terminated
** - XXX.1
** First agent ID
** - XXX.2
** Second agent ID
** - XXX.3
** and so on.
** 
** ASYNC
** The only mode currently supported means that sqlefrce does not wait until all
** specified applications are terminated before returning.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Force Users                   */
  sqlefrce_api (
        sqlint32 NumAgentIds,                /* number of users to force      */
        sqluint32 * pAgentIds,               /* array of agent ids            */
        unsigned short ForceMode,            /* mode of operation             */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlegdad API
** Stores information about remote databases in the Database Connection Services
** (DCS) directory. These databases are accessed through an Application
** Requester (AR), such as DB2 Connect. Having a DCS directory entry with
** a database name matching a database name in the system database
** directory invokes the specified AR to forward SQL requests to the remote
** server where the database resides.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlegdad API parameters
** 
** pDCSDirEntry
** Input. A pointer to an sql_dir_entry (Database Connection Services directory)
** structure.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The DB2 Connect program provides connections to DRDA Application Servers
** such as:
** - DB2 for OS/390 databases on System/370 and System/390 architecture
** host computers
** - DB2 for VM and VSE databases on System/370 and System/390
** architecture host computers
** - OS/400 databases on Application System/400 (AS/400) host computers.
** 
** The database manager creates a Database Connection Services directory if one
** does not exist. This directory is stored on the path that contains
** the database manager instance that is being used. The DCS directory is
** maintained outside of the database.
** 
** The database must also be cataloged as a remote database in the system
** database directory.
** 
** Note:
** If directory caching is enabled, database, node, and DCS directory files are
** cached in memory. An application's directory cache is created during
** its first directory lookup. Since the cache is only refreshed when
** the application modifies any of the directory files, directory changes
** made by other applications may not be effective until the application
** has restarted. To refresh DB2's shared cache (server only), stop
** (db2stop) and then restart (db2start) the database manager. To refresh
** the directory cache for another application, stop and then restart
** that application.
** 
** REXX API syntax
** 
** CATALOG DCS DATABASE dbname [AS target_dbname]
** [AR arname] [PARMS parms] [WITH comment]
** 
** REXX API parameters
** 
** dbname
** The local database name of the directory entry to be added.
** 
** target_dbname
** The target database name.
** 
** arname
** The application client name.
** 
** parms
** Parameter string. If specified, the string must be enclosed by
** double quotation marks.
** 
** comment
** Description associated with the entry. Maximum length is 30 characters.
** Enclose the comment by double quotation marks.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Catalog DCS Database          */
  sqlegdad_api (
        struct sql_dir_entry * pDCSDirEntry, /* pointer to entry structure    */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlegdcl API
** Frees the resources that are allocated by the sqlegdsc API.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqledcl API parameters
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** REXX API syntax
** 
** CLOSE DCS DIRECTORY
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Close DCS Directory Scan      */
  sqlegdcl_api (
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlegdel API
** Deletes an entry from the Database Connection Services (DCS) directory.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqledgel API parameters
** 
** pDCSDirEntry
** Input/Output. A pointer to the Database Connection Services directory
** structure. Fill in the ldb field of this structure with the local name of the
** database to be deleted. The DCS directory entry with a matching
** local database name is copied to this structure before being deleted.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** DCS databases are also cataloged in the system database directory as remote
** databases that can be uncataloged using the sqleuncd API.
** 
** To recatalog a database in the DCS directory, use the sqlegdad API.
** 
** To list the DCS databases that are cataloged on a node, use the sqlegdsc,
** sqlegdgt, and sqlegdcl APIs.
** 
** If directory caching is enabled (using the dir_cache configuration parameter,
** database, node, and DCS directory files are cached in memory. An
** application's directory cache is created during its first directory
** lookup. Since the cache is only refreshed when the application
** modifies any of the directory files, directory changes made by other
** applications may not be effective until the application has restarted.
** To refresh DB2's shared cache (server only), stop (db2stop) and then
** restart (db2start) the database manager. To refresh the
** directory cache for another application, stop and then restart that
** application.
** 
** REXX API syntax
** 
** UNCATALOG DCS DATABASE dbname [USING :value]
** 
** REXX API parameters
** 
** dbname
** The local database name of the directory entry to be deleted.
** 
** value
** A compound REXX host variable into which the directory entry information is
** returned. In the following, XXX represents the host variable name. If no name
** is given, the name SQLGWINF is used.
** 
** - XXX.0
** Number of elements in the variable (always 7)
** - XXX.1
** RELEASE
** - XXX.2
** LDB
** - XXX.3
** TDB
** - XXX.4
** AR
** - XXX.5
** PARMS
** - XXX.6
** COMMENT
** - XXX.7
** RESERVED.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Uncatalog DCS Database        */
  sqlegdel_api (
        struct sql_dir_entry * pDCSDirEntry, /* pointer to entry structure    */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlegdge API
** Returns information for a specific entry in the Database Connection Services
** (DCS) directory.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlegdge API parameters
** 
** pDCSDirEntry
** 
** Input/Output. Pointer to the Database Connection Services directory
** structure. Fill in the ldb field of this structure with the local
** name of the database whose DCS directory entry is to be retrieved.
** The remaining fields in the structure are filled in upon return of
** this API.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** REXX API syntax
** 
** GET DCS DIRECTORY ENTRY FOR DATABASE dbname [USING :value]
** 
** REXX API parameters
** 
** dbname
** Specifies the local database name of the directory entry to be obtained.
** value
** 
** A compound REXX host variable into which the directory entry information is
** returned. In the following, XXX represents the host variable name. If no name
** is given, the name SQLGWINF is used.
** 
** - XXX.0
** Number of elements in the variable (always 7)
** - XXX.1
** RELEASE 
** - XXX.2
** LDB 
** - XXX.3
** TDB
** - XXX.4
** AR
** - XXX.5
** PARMS
** - XXX.6
** COMMENT
** - XXX.7
** RESERVED.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get DCS Directory Entry For   */
                                             /* Database                      */
  sqlegdge_api (
        struct sql_dir_entry * pDCSDirEntry, /* pointer to entry structure    */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlegdgt API
** Transfers a copy of Database Connection Services (DCS) directory entries to a
** buffer supplied by the application.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlegdgt API parameters
** 
** pNumEntries
** Input/Output. Pointer to a short integer representing the number of
** entries to be copied to the caller's buffer. The number of entries
** actually copied is returned.
** 
** pDCSDirEntries
** Output. Pointer to a buffer where the collected DCS directory entries will be
** held upon return of the API call. The buffer must be large enough to hold the
** number of entries specified in the pNumEntries parameter.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The sqlegdsc API, which returns the entry count, must be called prior to
** issuing GET DCS DIRECTORY ENTRIES.
** 
** If all entries are copied to the caller, the Database Connection Services
** directory scan is automatically closed, and all resources are released.
** 
** If entries remain, subsequent calls to this API should be made, or CLOSE DCS
** DIRECTORY SCAN should be called, to release system resources.
** 
** REXX API syntax
** 
** GET DCS DIRECTORY ENTRY [USING :value]
** 
** REXX API parameters
** 
** value
** A compound REXX host variable into which the directory entry information is
** returned. In the following, XXX represents the host variable name. If no name
** is given, the name SQLGWINF is used.
** 
** - XXX.0
** Number of elements in the variable (always 7)
** - XXX.1
** RELEASE
** - XXX.2
** LDB
** - XXX.3
** TDB
** - XXX.4
** AR
** - XXX.5
** PARMS
** - XXX.6
** COMMENT
** - XXX.7
** RESERVED.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get DCS Directory Entries     */
  sqlegdgt_api (
        short * pNumEntries,                 /* pointer to count variable     */
        struct sql_dir_entry * pDCSDirEntries, /* pointer to entry structure  */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlegdsc API
** Stores a copy in memory of the Database Connection Services directory
** entries, and returns the number of entries. This is a snapshot of the
** directory at the time the directory is opened.
** 
** The copy is not updated if the directory itself changes after a call to this
** API. Use sqlegdgt API and sqlegdcl API to release the resources associated
** with calling this API.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlegdsc API parameters
** 
** pNumEntries
** Output. Address of a 2-byte area to which the number of directory entries is
** returned.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The caller of the scan uses the returned value pNumEntries to allocate enough
** memory to receive the entries. If a scan call is received while a copy is
** already held, the previous copy is released, and a new copy is collected.
** 
** REXX API syntax
** 
** OPEN DCS DIRECTORY
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Open DCS Directory Scan       */
  sqlegdsc_api (
        short * pNumEntries,                 /* pointer to count variable     */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlegins API
** Returns the value of the DB2INSTANCE environment variable.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlegins API parameters
** 
** pInstance
** Output. Pointer to a string buffer where the database manager
** instance name is placed. This buffer must be at least 9 bytes in
** length, including 1 byte for the null terminating character.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The value in the DB2INSTANCE environment variable is not necessarily the
** instance to which the user is attached.
** 
** To identify the instance to which a user is currently attached, call
** sqleatin - Attach, with null arguments except for the sqlca structure.
** 
** REXX API syntax
** 
** GET INSTANCE INTO :instance
** 
** REXX API parameters
** 
** instance
** A REXX host variable into which the database manager instance name is to be
** placed.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get Instance                  */
  sqlegins_api (
        _SQLOLDCHAR * pInstance,             /* pointer to instance name      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleintr API
** Stops a request. This API is called from a control break signal handler in an
** application. The control break signal handler can be the default,
** installed by sqleisig - Install Signal Handler, or a routine supplied
** by the programmer and installed using an appropriate operating system
** call.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqleintr API parameters
** 
** None
** 
** Usage notes
** 
** No database manager APIs should be called from an interrupt handler except
** sqleintr. However, the system will not prevent it.
** 
** Any database transaction in a state of committing or rollback cannot be
** interrupted.
** 
** An interrupted database manager request returns a code indicating that it was
** interrupted.
** 
** The following table summarizes the effect of an interrupt operation on other
** APIs:
** 
** Table: INTERRUPT Actions
** -----------------------------------------------------------------------------
** |Database Activity                  |Action                                 |
** |-----------------------------------|---------------------------------------|
** |BACKUP                             |Utility cancelled. Data on media may be|
** |                                   |incomplete.                            |
** |-----------------------------------|---------------------------------------|
** |BIND                               |Binding cancelled. Package creation    |
** |                                   |rolled back.                           |
** |-----------------------------------|---------------------------------------|
** |COMMIT                             |None. COMMIT completes.                |
** |-----------------------------------|---------------------------------------|
** |CREATE DATABASE/CREATE DATABASE AT |After a certain point, these APIs are  |
** |NODE/ADD NODE/DROP NODE VERIFY     |not interruptible. If the interrupt    |
** |                                   |call is received before this point, the|
** |                                   |database is not created. If the        |
** |                                   |interrupt call is received after this  |
** |                                   |point, it is ignored.                  |
** |-----------------------------------|---------------------------------------|
** |DROP DATABASE/DROP DATABASE AT NODE|None. The APIs complete.               |
** |-----------------------------------|---------------------------------------|
** |EXPORT/IMPORT/RUNSTATS             |Utility cancelled. Database updates    |
** |                                   |rolled back.                           |
** |-----------------------------------|---------------------------------------|
** |FORCE APPLICATION                  |None. FORCE APPLICATION completes.     |
** |-----------------------------------|---------------------------------------|
** |LOAD                               |Utility cancelled. Data in table may be|
** |                                   |incomplete.                            |
** |-----------------------------------|---------------------------------------|
** |PREP                               |Precompile cancelled. Package creation |
** |                                   |rolled back.                           |
** |-----------------------------------|---------------------------------------|
** |REORGANIZE TABLE                   |The interrupt will be delayed until the|
** |                                   |copy is complete. The recreation of    |
** |                                   |the indexes will be resume on the next |
** |                                   |attempt to access the table.           |
** |-----------------------------------|---------------------------------------|
** |RESTORE                            |Utility cancelled. DROP DATABASE       |
** |                                   |performed. Not applicable to table     |
** |                                   |space level restore.                   |
** |-----------------------------------|---------------------------------------|
** |ROLLBACK                           |None. ROLLBACK completes.              |
** |-----------------------------------|---------------------------------------|
** |Directory Services                 |Directory left in consistent state.    |
** |                                   |Utility function may or may not be     |
** |                                   |performed.                             |
** |-----------------------------------|---------------------------------------|
** |SQL Data Definition statements     |Database transactions are set to the   |
** |                                   |state existing prior to invocation of  |
** |                                   |the SQL statement.                     |
** |-----------------------------------|---------------------------------------|
** |Other SQL statements               |Database transactions are set to the   |
** |                                   |state existing prior to invocation of  |
** |                                   |the SQL statement.                     |
** |-----------------------------------|---------------------------------------|
** 
** REXX API syntax
** 
** INTERRUPT
** 
** Examples
** 
** call SQLDBS 'INTERRUPT'
*******************************************************************************/
SQL_API_RC SQL_API_INTR                      /* Interrupt                     */
        sqleintr_api ( void );

/******************************************************************************
** sqleisig API
** Installs the default interrupt (usually Control-C and/or
** Control-Break) signal handler. When this default handler detects
** an interrupt signal, it resets the signal and calls sqleintr.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqleisig API parameters
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** If an application has no signal handler, and an interrupt is received, the
** application is terminated. This API provides simple signal handling, and can
** be used if an application does not have extensive interrupt handling
** requirements.
** 
** The API must be called for the interrupt signal handler to function properly.
** 
** If an application requires a more elaborate interrupt handling scheme,
** a signal handling routine that can also call the sqleintr API can be
** developed. Use either the operating system call or the
** language-specific library signal function. The sqleintr API should be
** the only database manager operation performed by a customized signal
** handler. Follow all operating system programming techniques and
** practices to ensure that the previously installed signal handlers
** work properly.
** 
** REXX API syntax
** 
** INSTALL SIGNAL HANDLER
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Install Signal Handler        */
  sqleisig_api (
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlefmem API
** Frees memory allocated by DB2 APIs on the caller's behalf. Intended for use
** with the sqlbtcq and sqlbmtsq APIs.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlefmem API parameters
** 
** pSqlca
** Output. A pointer to the sqlca structure
** 
** pBuffer
** Input. Pointer to the memory to be freed.
** 
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Free Memory                   */
  sqlefmem_api (
        struct sqlca * pSqlca,               /* SQLCA                         */
        void * pBuffer);                     /* buffer pointer                */

/******************************************************************************
** sqlemgdb API
** Converts previous versions of DB2 databases to current versions.
** 
** Authorization
** 
** sysadm
** 
** Required connection
** 
** This API establishes a database connection.
** 
** API include file
** 
** sqlenv.h
** 
** Usage notes
** 
** This API will only migrate a database to a newer version, and cannot
** be used to convert a migrated database to its previous version.
** 
** The database must be cataloged before migration.
** 
** sqlemgdb API parameters
** 
** pDbAlias
** Input. A string containing the alias of the database that is cataloged in the
** system database directory.
** 
** pUserName
** Input. A string containing the user name of the application. May be NULL.
** 
** pPassword
** Input. A string containing the password of the supplied user name
** (if any). May be NULL.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** REXX API syntax
** 
** MIGRATE DATABASE dbalias [USER username USING password]
** 
** REXX API parameters
** 
** dbalias
** Alias of the database to be migrated.
** 
** username
** User name under which the database is to be restarted.
** 
** password
** Password used to authenticate the user name.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Migrate Database              */
  sqlemgdb_api (
        _SQLOLDCHAR * pDbAlias,              /* database alias                */
        _SQLOLDCHAR * pUserName,             /* user name                     */
        _SQLOLDCHAR * pPassword,             /* password                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqle_activate_db API
** Activates the specified database and starts up all necessary database
** services, so that the database is available for connection and use by
** any application.
** 
** Scope
** 
** This API activates the specified database on all database partition
** servers. If one or more of these database partition servers encounters
** an error during activation of the database, a warning is returned. The
** database remains activated on all database partition servers on which
** the API has succeeded.
** Note:
** If it is the coordinator partition or the catalog partition that
** encounters the error, the API returns a negative sqlcode, and the
** database will not be activated on any database partition server.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** - sysmaint
** 
** Required connection
** 
** None. Applications invoking ACTIVATE DATABASE cannot have any existing
** database connections.
** 
** API include file
** 
** sqlenv.h
** 
** sqle_activate_db API parameters
** 
** pDbAlias
** Input. Pointer to the database alias name.
** 
** pUserName
** Input. Pointer to the user ID starting the database. Can be NULL.
** 
** pPassword
** Input. Pointer to the password for the user name. Can be NULL, but must be
** specified if a user name is specified.
** 
** pReserved
** Reserved for future use.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** If a database has not been started, and a DB2 CONNECT TO (or an implicit
** connect) is encountered in an application, the application must wait
** while the database manager starts up the required database. In such
** cases, this first application spends time on database initialization
** before it can do any work. However, once the first application has
** started a database, other applications can simply connect and use it.
** 
** Database administrators can use ACTIVATE DATABASE to start up selected
** databases. This eliminates any application time spent on database
** initialization.
** 
** Databases initialized by ACTIVATE DATABASE can only be shut down by
** sqle_deactivate_db, or by db2InstanceStop. To obtain a list of activated
** databases, call db2GetSnapshot.
** 
** If a database was started by a DB2 CONNECT TO (or an implicit connect) and
** subsequently an ACTIVATE DATABASE is issued for that same database, then
** DEACTIVATE DATABASE must be used to shut down that database.
** 
** ACTIVATE DATABASE behaves in a similar manner to a DB2 CONNECT TO (or
** an implicit connect) when working with a database requiring a restart (for
** example, database in an inconsistent state). The database will be restarted
** before it can be initialized by ACTIVATE DATABASE.
** 
** REXX API syntax
** 
** This API can be called from REXX through the SQLDB2 interface.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Activate Database             */
  sqle_activate_db_api (
        char * pDbAlias,                     /* database alias                */
        char * pUserName,                    /* user name                     */
        char * pPassword,                    /* password                      */
        void * pReserved,                    /* Reserved for future use       */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqle_deactivate_db API
** Stops the specified database.
** 
** Scope
** 
** In a partitioned database environment, this API deactivates the specified
** database on all database partition servers. If one or more of these database
** partition servers encounters an error, a warning is returned. The
** database will be successfully deactivated on some database partition
** servers, but may remain activated on the database partition servers
** encountering the error.
** 
** Note:
** If it is the coordinator partition or the catalog partition that
** encounters the error, the API returns a negative sqlcode, and the
** database will not be reactivated on any database partition server on
** which it was deactivated.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** - sysmaint
** 
** Required connection
** 
** None. Applications invoking DEACTIVATE DATABASE cannot have any existing
** database connections.
** 
** API include file
** 
** sqlenv.h
** 
** sqle_deactivate_db API parameters
** 
** pDbAlias
** Input. Pointer to the database alias name.
** 
** pUserName
** Input. Pointer to the user ID stopping the database. Can be NULL.
** 
** pPassword
** Input. Pointer to the password for the user name. Can be NULL, but must be
** specified if a user name is specified.
** 
** pReserved
** Reserved for future use.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** Databases initialized by ACTIVATE DATABASE can only be shut down by
** DEACTIVATE DATABASE. db2InstanceStop automatically stops all activated
** databases before stopping the database manager. If a database was initialized
** by ACTIVATE DATABASE, the last DB2 CONNECT RESET statement (counter equal
**  0) will not shut down the database; DEACTIVATE DATABASE must be used.
** 
** REXX API syntax
** 
** This API can be called from REXX through the SQLDB2 interface.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Deactivate Database           */
  sqle_deactivate_db_api (
        char * pDbAlias,                     /* database alias                */
        char * pUserName,                    /* user name                     */
        char * pPassword,                    /* password                      */
        void * pReserved,                    /* Reserved for future use       */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlencls API
** Frees the resources that are allocated by the sqlenops API.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlencls API parameters
** 
** Handle
** Input. Identifier returned from the associated OPEN NODE DIRECTORY SCAN API.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** REXX API syntax
** 
** CLOSE NODE DIRECTORY :scanid
** 
** REXX API parameters
** 
** scanid
** A host variable containing the scanid returned from the OPEN NODE
** DIRECTORY SCAN API.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Close Node Directory Scan     */
  sqlencls_api (
        unsigned short Handle,               /* handle                        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlengne API
** Returns the next entry in the node directory after sqlenops - Open Node
** Directory Scan is called. Subsequent calls to this API return additional
** entries.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlengne API parameters
** 
** Handle
** Input. Identifier returned from sqlenops - Open Node Directory Scan.
** 
** ppNodeDirEntry
** Output. Address of a pointer to an sqleninfo structure. The caller
** of this API does not have to provide memory for the structure, just
** the pointer. Upon return from the API, the pointer points to the next
** node directory entry in the copy of the node directory allocated by
** sqlenops - Open Node Directory Scan.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** All fields in the node directory entry information buffer are padded to the
** right with blanks.
** 
** The sqlcode value of sqlca is set to 1014 if there are no more entries
** to scan when this API is called.
** 
** The entire directory can be scanned by calling this API pNumEntries times.
** 
** REXX API syntax
** 
** GET NODE DIRECTORY ENTRY :scanid [USING :value]
** 
** REXX API parameters
** 
** scanid
** A REXX host variable containing the identifier returned from the OPEN NODE
** DIRECTORY SCAN API.
** 
** value
** A compound REXX host variable to which the node entry information is
** returned. If no name is given, the name SQLNINFO is used. In the
** following, XXX represents the host variable name (the corresponding
** field names are taken from the structure returned by the API):
** 
** - XXX.0
** Number of elements in the variable (always 16)
** - XXX.1
** NODENAME
** - XXX.2
** LOCALLU
** - XXX.3
** PARTNERLU
** - XXX.4
** MODE
** - XXX.5
** COMMENT
** - XXX.6
** RESERVED
** - XXX.7
** PROTOCOL (protocol type)
** - XXX.8
** ADAPTER (NetBIOS adapter #)
** - XXX.9
** RESERVED
** - XXX.10
** SYMDESTNAME (symbolic destination name)
** - XXX.11
** SECURITY (security type)
** - XXX.12
** HOSTNAME 
** - XXX.13
** SERVICENAME
** - XXX.14
** FILESERVER
** - XXX.15
** OBJECTNAME
** - XXX.16
** INSTANCE (local instance name).
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get Next Node Directory       */
                                             /* Entry                         */
  sqlengne_api (
        unsigned short Handle,               /* handle                        */
        struct sqleninfo ** ppNodeDirEntry,  /* buffer                        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlenops API
** Stores a copy in memory of the node directory, and returns the number of
** entries. This is a snapshot of the directory at the time the directory is
** opened. This copy is not updated, even if the directory itself is changed
** later.
** 
** Call the sqlengne API to advance through the node directory and examine
** information about the node entries. Close the scan by calling the
** sqlencls API. This removes the copy of the directory from memory.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlenops API parameters
** 
** pHandle
** Output. Identifier returned from this API. This identifier must be passed to
** the sqlengne API and sqlencls API.
** 
** pNumEntries
** Output. Address of a 2-byte area to which the number of directory entries is
** returned.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** Storage allocated by this API is freed by calling sqlencls - Close Node
** Directory Scan.
** 
** Multiple node directory scans can be issued against the node directory.
** However, the results may not be the same. The directory may change between
** openings.
** 
** There can be a maximum of eight node directory scans per process.
** 
** REXX API syntax
** 
** OPEN NODE DIRECTORY USING :value
** 
** REXX API parameters
** 
** value
** A compound REXX variable to which node directory information is returned. In
** the following, XXX represents the host variable name.
** 
** - XXX.0
** Number of elements in the variable (always 2)
** - XXX.1
** Specifies a REXX host variable containing a number for scanid
** - XXX.2
** The number of entries contained within the directory.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Open Node Directory Scan      */
  sqlenops_api (
        unsigned short * pHandle,            /* handle                        */
        unsigned short * pNumEntries,        /* count                         */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlerstd API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Restart Database              */
  sqlerstd_api (
        _SQLOLDCHAR * pDbAlias,              /* database alias                */
        _SQLOLDCHAR * pUserName,             /* user name                     */
        _SQLOLDCHAR * pPassword,             /* password                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlepstart API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* V5 Start Database Manager     */
  sqlepstart_api (
        struct sqle_start_options * pStartOptions, /* start options           */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlepstp API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* V5 Stop Database Manager      */
  sqlepstp_api (
        struct sqledbstopopt * pStopOptions, /* STOP OPTIONS                  */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleuncd API
** Deletes an entry from the system database directory.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqleuncd API parameters
** 
** pDbAlias
** Input. A string containing the database alias that is to be uncataloged.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** Only entries in the system database directory can be uncataloged. Entries in
** the local database directory can be deleted using the sqledrpd API.
** 
** To recatalog the database, use the sqlecadb API.
** 
** To list the databases that are cataloged on a node, use the db2DbDirOpenScan,
** db2DbDirGetNextEntry, and db2DbDirCloseScan APIs.
** 
** The authentication type of a database, used when communicating with a
** down-level server, can be changed by first uncataloging the database,
** and then cataloging it again with a different type.
** 
** If directory caching is enabled using the dir_cache configuration parameter,
** database, node, and DCS directory files are cached in memory. An
** application's directory cache is created during its first directory
** lookup. Since the cache is only refreshed when the application
** modifies any of the directory files, directory changes made by other
** applications may not be effective until the application has
** restarted. To refresh DB2's shared cache (server only), stop (db2stop)
** and then restart (db2start) the database manager. To refresh the
** directory cache for another application, stop and then restart that
** application.
** 
** REXX API syntax
** 
** UNCATALOG DATABASE dbname
** 
** REXX API parameters
** 
** dbname
** Alias of the database to be uncataloged.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Uncatalog Database            */
  sqleuncd_api (
        _SQLOLDCHAR * pDbAlias,              /* database alias                */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleuncn API
** Deletes an entry from the node directory.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqleuncn API parameters
** 
** pNodeName
** Input. A string containing the name of the node to be uncataloged.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** To recatalog the node, use the sqlectnd API.
** 
** To list the nodes that are cataloged, use the db2DbDirOpenScan,
** db2DbDirGetNextEntry, and db2DbDirCloseScan APIs.
** 
** If directory caching is enabled using the dir_cache configuration parameter,
** database, node, and DCS directory files are cached in memory. An
** application's directory cache is created during its first directory
** lookup. Since the cache is only refreshed when the application modifies
** any of the directory files, directory changes made by other
** applications may not be effective until the application has restarted.
** To refresh DB2's shared cache (server only), stop (db2stop) and then
** restart (db2start) the database manager. To refresh the
** directory cache for another application, stop and then restart that
** application.
** 
** REXX API syntax
** 
** UNCATALOG NODE nodename
** 
** REXX API parameters
** 
** nodename
** Name of the node to be uncataloged.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Uncatalog Node                */
  sqleuncn_api (
        _SQLOLDCHAR * pNodeName,             /* node name                     */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlesetc API
** Specifies connection settings for the application. Use the sqle_conn_setting
** data structure to specify the connection setting types and values.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlesetc API parameters
** 
** pConnectionSettings
** Input. A pointer to the sqle_conn_setting structure, which specifies
** connection setting types and values. Allocate an array of NumSettings
** sqle_conn_setting structures. Set the type field of each element in
** this array to indicate the connection option to set. Set the value
** field to the desired value for the option.
** 
** NumSettings
** Input. Any integer (from 0 to 7) representing the number of connection option
** values to set.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** If this API is successful, the connections in the subsequent units of
** work will use the connection settings specified. If this API is
** unsuccessful, the connection settings are unchanged.
** 
** The connection settings for the application can only be changed when
** there are no existing connections (for example, before any connection
** is established, or after RELEASE ALL and COMMIT).
** 
** Once the SET CLIENT API has executed successfully, the connection settings
** are fixed and can only be changed by again executing the SET CLIENT API. All
** corresponding precompiled options of the application modules will be
** overridden.
** 
** REXX API syntax
** 
** SET CLIENT USING :values
** 
** REXX API parameters
** 
** values
** A compound REXX host variable containing the connection settings for the
** application process. In the following, XXX represents the host variable name.
** 
** - XXX.0
** Number of connection settings to be established
** 
** - XXX.1
** Specifies how to set up the CONNECTION type. The valid values are:
** -- 1
** Type 1 CONNECT
** -- 2
** Type 2 CONNECT
** 
** - XXX.2
** Specifies how to set up the SQLRULES. The valid values are:
** -- DB2
** Process type 2 CONNECT according to the DB2 rules
** -- STD
** Process type 2 CONNECT according to the Standard rules
** 
** - XXX.3
** Specifies how to set up the scope of disconnection to databases at commit.
** The valid values are:
** -- EXPLICIT
** Disconnect only those marked by the SQL RELEASE statement
** -- CONDITIONAL
** Disconnect only those that have no open WITH HOLD cursors
** -- AUTOMATIC
** Disconnect all connections
** 
** - XXX.4
** Specifies how to set up the coordination among multiple database
** connections during commits or rollbacks. The valid values are:
** -- TWOPHASE
** Use Transaction Manager (TM) to coordinate two-phase commits.
** The SYNCPOINT option is ignored and is available only for backward
** compatibility.
** 
** - XXX.5
** Specifies the maximum number of concurrent connections for a NETBIOS
** adapter.
** 
** - XXX.6
** Specifies when to execute the PREPARE statement. The valid values are:
** -- NO
** The PREPARE statement will be executed at the time it is issued
** -- YES
** The PREPARE statement will not be executed until the corresponding
** OPEN, DESCRIBE, or EXECUTE statement is issued. However, the
** PREPARE INTO statement is not deferred
** -- ALL
** Same as YES, except that the PREPARE INTO statement is also deferred
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Set Client                    */
  sqlesetc_api (
        struct sqle_conn_setting * pConnectionSettings, /* conn setting       */
                                             /* array                         */
        unsigned short NumSettings,          /* number of settings            */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleqryc API
** Returns current connection settings for an application process. The
** sqle_conn_setting data structure is populated with the connection setting
** types and values.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqleqryc API parameters
** 
** pConnectionSettings
** Input/Output. A pointer to an sqle_conn_setting structure, which specifies
** connection setting types and values. The user defines an array of NumSettings
** connection settings structures, and sets the type field of each element
** in this array to indicate one of the five possible connection settings
** options. Upon return, the value field of each element contains the
** current setting of the option specified.
** 
** NumSettings
** Input. Any integer (from 0 to 7) representing the number of connection option
** values to be returned.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The connection settings for an application process can be queried at any time
** during execution.
** 
** If QUERY CLIENT is successful, the fields in the sqle_conn_setting structure
** will contain the current connection settings of the application process.
** If SET CLIENT has never been called, the settings will contain the
** values of the precompile options only if an SQL statement has already 
** been processed; otherwise, they will contain the default values for
** the precompile options.
** 
** REXX API syntax
** 
** QUERY CLIENT INTO :output
** 
** REXX API parameters
** 
** output
** A compound REXX host variable containing information about the current
** connection settings of the application process. In the following, XXX
** represents the host variable name.
** 
** - XXX.1
** Current connection setting for the CONNECTION type
** - XXX.2
** Current connection setting for the SQLRULES
** - XXX.3
** Current connection setting indicating which connections will be released when
** a COMMIT is issued.
** - XXX.4
** Current connection setting of the SYNCPOINT option. The SYNCPOINT option
** is ignored and is available only for backward compatibility. Indicates
** whether a transaction manager should be used to enforce two-phase
** commit semantics, whether the database manager should ensure that
** there is only one database being updated when multiple databases are
** accessed within a single transaction, or whether neither of these
** options is to be used.
** - XXX.5
** Current connection setting for the maximum number of concurrent
** connections for a NETBIOS adapter.
** - XXX.6
** Current connection setting for deferred PREPARE.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Query Client                  */
  sqleqryc_api (
        struct sqle_conn_setting * pConnectionSettings, /* conn setting       */
                                             /* array                         */
        unsigned short NumSettings,          /* number of settings            */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleseti API
** Permits an application to set client information (by setting the fields in
** the sqle_client_info data structure) associated with a specific connection,
** provided a connection already exists.
** 
** In a TP monitor or 3-tier client/server application environment, there is a
** need to obtain information about the client, and not just the application
** server that is working on behalf of the client. By using this API, the
** application server can pass the client's user ID, workstation information,
** program information, and other accounting information to the DB2 server;
** otherwise, only the application server's information is passed, and that
** information is likely to be the same for the many client invocations that go
** through the same application server.
** 
** The application can elect to not specify an alias, in which case the client
** information will be set for all existing, as well as future,
** connections. This API will only permit information to be changed outside
** of a unit of work, either before any SQL is executed, or after a commit
** or a rollback. If the call is successful, the values for the connection
** will be sent at the next opportunity, grouped with the next SQL request
** sent on that connection; a successful call means that the values have
** been accepted, and that they will be propagated to subsequent
** connections.
** 
** This API can be used to establish values prior to connecting to a
** database, or it can be used to set or modify the values once a
** connection has been established.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqleseti API parameters
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database alias. If a value greater than zero is provided, pDbAlias must point
** to the alias name, and the settings will affect only the specified
** connection. If zero is specified, the settings will affect all existing
** and future connections.
** 
** pDbAlias
** Input. A pointer to a string containing the database alias.
** 
** NumItems
** Input. Number of entries being modified. The minimum value is 1.
** 
** pClient_Info
** Input. A pointer to an array of NumItems sqle_client_info structures, each
** containing a type field indicating which value to set, the length of that
** value, and a pointer to the new value.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** If an alias name was provided, a connection to the alias must already exist,
** and all connections to that alias will inherit the changes. The information
** will be retained until the connection for that alias is broken. If an alias
** name was not provided, settings for all existing connections will be changed,
** and any future connections will inherit the changes. The information will be
** retained until the program terminates.
** 
** The field names represent guidelines for the type of information that can be
** provided. For example, a TP monitor application could choose to provide
** the TP monitor transaction ID along with the application name in the
** SQL_CLIENT_INFO_APPLNAM field. This would provide better monitoring and
** accounting on the DB2 server, where the DB2 transaction ID can be associated
** with the TP monitor transaction ID.
** 
** Currently this API will pass information to DB2 OS/390 Version 5 and
** higher and DB2 UDB Version 7 and higher. All information (except the
** accounting string) is displayed on the DISPLAY THREAD command, and
** will all be logged into the accounting records.
** 
** The data values provided with the API can also be accessed by SQL special
** register. The values in these registers are stored in the database code page.
** Data values provided with this API are converted to the database code page
** before being stored in the special registers. Any data value that exceeds the
** maximum supported size after conversion to the database code page will be
** truncated before being stored at the server. These truncated values will be
** returned by the special registers. The original data values will also
** be stored at the server and are not converted to the database code page.
** The unconverted values can be returned by calling the sqleqryi API.
** 
** Calling the sqleseti API in a CLI program before a connection will not
** work. Calling the sqleseti API in a CLI program after a connection has been
** established may result in unpredictable behavior. It is recommended that
** the corresponding CLI functions SQLSetConnectAttr() or SQLSetEnvAttr() be
** used instead.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Set Client Information        */
  sqleseti_api (
        unsigned short DbAliasLen,           /* database alias length         */
        char * pDbAlias,                     /* database alias                */
        unsigned short NumItems,             /* number of types to set        */
        struct sqle_client_info* pClient_Info, /* information to query        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleqryi API
** Returns existing client information by populating the fields in the
** sqle_client_info data structure. Since this API permits specification of
** a database alias, an application can query client information associated
** with a specific connection. Returns null if the sqleseti API has not
** previously established a value.
** 
** If a specific connection is requested, this API returns the latest values for
** that connection. If all connections are specified, the API returns the values
** that are to be associated with all connections; that is, the values passed in
** the last call to sqleseti (specifying all connections).
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqleqryi API parameters
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database alias. If a value greater than zero is provided, pDbAlias must point
** to the alias name. Returns the settings associated with the last call to
** sqleseti for this alias (or a call to sqleseti specifying a zero
** length alias). If zero is specified, returns the settings associated
** with the last call to sqleseti which specified a zero length alias.
** 
** pDbAlias
** Input. A pointer to a string containing the database alias.
** 
** NumItems
** Input. Number of entries being modified. The minimum value is 1.
** 
** pClient_Info
** Input. A pointer to an array of NumItems sqle_client_info structures, each
** containing a type field indicating which value to return, and a
** pointer to the returned value. The area pointed to must be large enough
** to accommodate the value being requested.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** The settings can be queried at any time during execution. If the API call is
** successful, the current settings are returned to the specified areas.
** Returns a length of zero and a null-terminated string (\0) for any
** fields that have not been set through a call to the sqleseti API.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Query Client Information      */
  sqleqryi_api (
        unsigned short DbAliasLen,           /* database alias length         */
        char * pDbAlias,                     /* database alias                */
        unsigned short NumItems,             /* number of types to set        */
        struct sqle_client_info* pClient_Info, /* information to query        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlecrea API
** Initializes a new database with an optional user-defined collating sequence,
** creates the three initial table spaces, creates the system tables, and
** allocates the recovery log.
** 
** Scope
** 
** In a partitioned database environment, this API affects all database
** partition servers that are listed in the db2nodes.cfg file.
** 
** The database partition server from which this API is called becomes
** the catalog partition for the new database.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** Instance. To create a database at another (remote) node, it is necessary to
** first attach to that node. A database connection is temporarily
** established by this API during processing.
** 
** API include file
** 
** sqlenv.h
** 
** sqlecrea API parameters
** 
** pDbName
** Input. A string containing the database name. This is the database name that
** will be cataloged in the system database directory. Once the database has
** been successfully created in the server's system database directory, it is
** automatically cataloged in the system database directory with a
** database alias identical to the database name. Must not be NULL.
** 
** pLocalDbAlias
** Input. A string containing the alias to be placed in the client's system
** database directory. Can be NULL. If no local alias is specified, the database
** name is the default.
** 
** pPath
** Input. On UNIX-based systems, specifies the path on which to create the
** database. If a path is not specified, the database is created on the default
** database path specified in the database manager configuration file (dftdbpath
** parameter). On the Windows operating system, specifies the letter of
** the drive on which to create the database. Can be NULL.
** 
** Note:
** For partitioned database environments, a database should not be created in
** an NFS-mounted directory. If a path is not specified, ensure that the
** dftdbpath database manager configuration parameter is not set to an
** NFS-mounted path (for example, on UNIX based systems, it should not
** specify the $HOME directory of the instance owner). The path specified
** for this API in a partitioned database environment cannot be a relative
** path.
** 
** pDbDescriptor
** Input. A pointer to the database description block that is used when creating
** the database. The database description block can be used by you to supply
** values that are permanently stored in the configuration file of the database.
** The supplied values are a collating sequence, a database comment, or a table
** space definition. The supplied value can be NULL if you do not want to
** supply any values. For information about the values that can be supplied
** through this parameter, see the SQLEDBDESC data structure topic.
** 
** pTerritoryInfo
** Input. A pointer to the sqledbterritoryinfo structure, containing the locale
** and the code set for the database. Can be NULL.
** 
** Reserved2
** Input. Reserved for future use.
** 
** pDbDescriptorExt
** Input. This parameter refers to an extended database description block
** (sqledbdescext) that is used when creating the database. The extended
** database description block enables automatic storage for a database, chooses
** a default page size for the database, or specifies values for new table space
** attributes that have been introduced. This parameter can be set to null or
** zero, if you do not want use the permanent values for these database attributes
** when creating the database, then you should not use the extended database
** description block. If set to null or zero, a default page size of 4 096 bytes
** is chosen for the database and automatic storage is disabled.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** CREATE DATABASE:
** 
** - Creates a database in the specified subdirectory. In a partitioned database
** environment, creates the database on all database partition servers listed in
** db2nodes.cfg, and creates a $DB2INSTANCE/NODExxxx directory under the
** specified subdirectory at each database partition server, where xxxx
** represents the local database partition server number. In a single-partition
** environment, creates a $DB2INSTANCE/NODE0000 directory under the specified
** subdirectory.
** 
** - Creates the system catalog tables and recovery log.
** 
** - Catalogs the database in the following database directories:
** -- server's local database directory on the path indicated by pPath or, if
** the path is not specified, the default database path defined in the
** database manager system configuration file. A local database
** directory resides on each file system that contains a database.
** -- server's system database directory for the attached instance. The
** resulting directory entry will contain the database name and a
** database alias.
** 
** If the API was called from a remote client, the client's system
** database directory is also updated with the database name and an
** alias.
** 
** Creates a system or a local database directory if neither exists. If
** specified, the comment and code set values are placed in both
** directories.
** 
** - Stores the specified code set, territory, and collating sequence. A flag is
** set in the database configuration file if the collating sequence consists of
** unique weights, or if it is the identity sequence.
** 
** - Creates the schemata called SYSCAT, SYSFUN, SYSIBM, and SYSSTAT
** with SYSIBM as the owner. The database partition server on which this API
** is called becomes the catalog partition for the new database. Two database
** partition groups are created automatically: IBMDEFAULTGROUP and
** IBMCATGROUP.
** 
** - Binds the previously defined database manager bind files to the database
** (these are listed in db2ubind.lst). If one or more of these files do not bind
** successfully, sqlecrea returns a warning in the SQLCA, and provides
** information about the binds that failed. If a bind fails, the user can take
** corrective action and manually bind the failing file. The database is created
** in any case. A schema called NULLID is implicitly created when performing
** the binds with CREATEIN privilege granted to PUBLIC.
** 
** - Creates SYSCATSPACE, TEMPSPACE1, and USERSPACE1 table
** spaces. The SYSCATSPACE table space is only created on the catalog
** partition. All database partitions have the same table space definitions.
** 
** - Grants the following:
** -- DBADM authority, and CONNECT, CREATETAB, BINDADD,
** CREATE_NOT_FENCED, IMPLICIT_SCHEMA, and LOAD privileges
** to the database creator
** -- CONNECT, CREATETAB, BINDADD, and IMPLICIT_SCHEMA
** privileges to PUBLIC
** -- USE privilege on the USERSPACE1 table space to PUBLIC
** -- SELECT privilege on each system catalog to PUBLIC
** -- BIND and EXECUTE privilege to PUBLIC for each successfully
** bound utility
** -- EXECUTE WITH GRANT privilege to PUBLIC on all functions in the
** SYSFUN schema.
** -- EXECUTE privilege to PUBLIC on all procedures in SYSIBM
** schema.
** 
** With dbadm authority, one can grant these privileges to (and revoke
** them from) other users or PUBLIC. If another administrator with
** sysadm or dbadm authority over the database revokes these privileges,
** the database creator nevertheless retains them.
** 
** In a partitioned database environment, the database manager creates a
** subdirectory, $DB2INSTANCE/NODExxxx, under the specified or default
** path on all database partition servers. The xxxx is the node number
** as defined in the db2nodes.cfg file (that is, node 0 becomes
** NODE0000). Subdirectories SQL00001 through SQLnnnnn will reside on
** this path. This ensures that the database objects associated with
** different database partition servers are stored in different
** directories (even if the subdirectory $DB2INSTANCE under the
** specified or default path is shared by all database partition servers).
** 
** On Windows and AIX operating systems, the length of the code set name is
** limited to a maximum of 9 characters. For example, specify a code set
** name such as ISO885915 instead of ISO8859-15.
** 
** Execution of the CREATE DATABASE command will fail if the application is
** already connected to a database.
** 
** If the database description block structure is not set correctly, an error
** message is returned.
** 
** The "eye-catcher" of the database description block must be set to the
** symbolic value SQLE_DBDESC_2 (defined in sqlenv). The following sample
** user-defined collating sequences are available in the host language include
** files:
** 
** sqle819a
** If the code page of the database is 819 (ISO Latin/1), this sequence
** will cause sorting to be performed according to the host CCSID 500 (EBCDIC
** International).
** 
** sqle819b
** If the code page of the database is 819 (ISO Latin/1),this sequence
** will cause sorting to be performed according to the host CCSID 037
** (EBCDIC US English).
** 
** sqle850a
** If the code page of the database is 850 (ASCII Latin/1), this sequence will
** cause sorting to be performed according to the host CCSID 500 (EBCDIC
** International).
** 
** sqle850b
** If the code page of the database is 850 (ASCII Latin/1), this sequence will
** cause sorting to be performed according to the host CCSID 037 (EBCDIC US
** English).
** 
** sqle932a
** If the code page of the database is 932 (ASCII Japanese), this sequence will
** cause sorting to be performed according to the host CCSID 5035 (EBCDIC
** Japanese).
** 
** sqle932b
** If the code page of the database is 932 (ASCII Japanese), this sequence will
** cause sorting to be performed according to the host CCSID 5026 (EBCDIC
** Japanese).
** 
** The collating sequence specified during CREATE DATABASE cannot be changed
** later, and all character comparisons in the database use the specified
** collating sequence. This affects the structure of indexes as well as the
** results of queries.
** 
** Use sqlecadb to define different alias names for the new database.
** 
** REXX API syntax
** 
** CREATE DATABASE dbname [ON path] [ALIAS dbalias]
** [USING CODESET codeset TERRITORY territory]
** [COLLATE USING {SYSTEM | IDENTITY | USER :udcs}]
** [NUMSEGS numsegs] [DFT_EXTENT_SZ dft_extentsize]
** [CATALOG TABLESPACE <tablespace_definition>]
** [USER TABLESPACE <tablespace_definition>]
** [TEMPORARY TABLESPACE <tablespace_definition>]
** [WITH comment]
** 
** Where <tablespace_definition> stands for:
** MANAGED BY {
** SYSTEM USING :SMS_string |
** DATABASE USING :DMS_string }
** [ EXTENTSIZE number_of_pages ]
** [ PREFETCHSIZE number_of_pages ]
** [ OVERHEAD number_of_milliseconds ]
** [ TRANSFERRATE number_of_milliseconds ]
** 
** REXX API parameters
** 
** dbname
** Name of the database.
** 
** dbalias
** Alias of the database.
** 
** path
** Path on which to create the database.
** 
** If a path is not specified, the database is created on the default database
** path specified in the database manager configuration file (dftdbpath
** configuration parameter).
** 
** Note:
** For partitioned database environments, a database should not be created in
** an NFS-mounted directory. If a path is not specified, ensure that the
** dftdbpath database manager configuration parameter is not set to an
** NFS-mounted path (for example, on UNIX based systems, it should not
** specify the $HOME directory of the instance owner). The path specified
** for this API in a partitioned database environment cannot be a relative
** path.
** 
** codeset
** Code set to be used for data entered into the database.
** 
** territory
** Territory code (locale) to be used for data entered into the database.
** 
** SYSTEM
** Collating sequence that is based on the database territory.
** 
** IDENTITY
** The collation sequence as it is determined by the binary order of
** each byte of the string, where strings are compared byte for byte,
** starting with the leftmost byte.
** 
** USER udcs
** The collating sequence is specified by the calling application in a host
** variable containing a 256-byte string defining the collating sequence.
** 
** numsegs
** Number of segment directories that will be created and used to store the DAT,
** IDX, and LF files.
** 
** dft_extentsize
** Specifies the default extent size for table spaces in the database.
** 
** SMS_string
** A compound REXX host variable identifying one or more containers that will
** belong to the table space, and where the table space data will be stored.
** In the following, XXX represents the host variable name. Note that each
** of the directory names cannot exceed 254 bytes in length.
** 
** - XXX.0
** Number of directories specified
** - XXX.1
** First directory name for SMS table space
** - XXX.2
** Second directory name for SMS table space
** - XXX.3
** and so on.
** 
** DMS_string
** A compound REXX host variable identifying one or more containers that will
** belong to the table space, where the table space data will be stored,
** container sizes (specified in a number of 4KB pages) and types (file or
** device). The specified devices (not files) must already exist. In the
** following, XXX represents the host variable name. Note that each of the
** container names cannot exceed 254 bytes in length.
** 
** - XXX.0
** Number of strings in the REXX host variable (number of first level elements)
** - XXX.1.1
** Type of the first container (file or device)
** - XXX.1.2
** First file name or device name
** - XXX.1.3
** Size (in pages) of the first container
** - XXX.2.1
** Type of the second container (file or device)
** - XXX.2.2
** Second file name or device name
** - XXX.2.3
** Size (in pages) of the second container
** - XXX.3.1
** and so on.
** 
** EXTENTSIZE number_of_pages
** Number of 4KB pages that will be written to a container before skipping
** to the next container.
** 
** PREFETCHSIZE number_of_pages
** Number of 4KB pages that will be read from the table space when data
** prefetching is being performed.
** 
** OVERHEAD number_of_milliseconds
** Number that specifies the I/O controller overhead, disk seek, and
** latency time in milliseconds.
** 
** TRANSFERRATE number_of_milliseconds
** Number that specifies the time in milliseconds to read one 4 KB page into
** memory.
** 
** comment
** Description of the database or the database entry in the system directory. Do
** not use a carriage return or line feed character in the comment. Be sure to
** enclose the comment text in double quotation marks. Maximum size is 30
** characters.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Create Database               */
  sqlecrea_api (
        char * pDbName,                      /* database                      */
        char * pLocalDbAlias,                /* local alias                   */
        char * pPath,                        /* drive/path                    */
        struct sqledbdesc * pDbDescriptor,   /* database descriptor block     */
        SQLEDBTERRITORYINFO * pTerritoryInfo, /* db locale and codeset        */
        char Reserved2,                      /* reserved                      */
        void * pDbDescriptorExt,             /* db descriptor extension       */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleatcp API
** Enables an application to specify the node at which instance-level functions
** (CREATE DATABASE and FORCE APPLICATION, for example) are to be
** executed. This node may be the current instance (as defined by the value
** of the DB2INSTANCE environment variable), another instance on the same
** workstation, or an instance on a remote workstation. Establishes a
** logical instance attachment to the node specified, and starts a
** physical communications connection to the node if one does not already
** exist.
** 
** Note:
** This API extends the function of the sqleatin API by permitting the optional
** change of the user password for the instance being attached.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** This API establishes an instance attachment.
** 
** API include file
** 
** sqlenv.h
** 
** sqleatcp API parameters
** 
** pNodeName
** Input. A string containing the alias of the instance to which the user
** wants to attach. This instance must have a matching entry in the local
** node directory. The only exception is the local instance (as specified
** by the DB2INSTANCE environment variable), which can be specified as
** the object of an attachment, but cannot be used as a node name in the
** node directory. May be NULL.
** 
** pUserName
** Input. A string containing the user name under which the attachment is to be
** authenticated. May be NULL.
** 
** pPassword
** Input. A string containing the password for the specified user name. May be
** NULL.
** 
** pNewPassword
** Input. A string containing the new password for the specified user name.
** Set to NULL if a password change is not required.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** Note:
** A node name in the node directory can be regarded as an alias for an
** instance.
** 
** If an attach request succeeds, the sqlerrmc field of the sqlca will contain 9
** tokens separated by hexadecimal FF (similar to the tokens returned when a
** CONNECT request is successful):
** 
** 1. Country/region code of the application server
** 2. Code page of the application server
** 3. Authorization ID
** 4. Node name (as specified on the API)
** 5. Identity and platform type of the server
** 6. Agent ID of the agent which has been started at the server
** 7. Agent index
** 8. Node number of the server
** 9. Number of database partitions if the server is a partitioned
** database server.
** 
** If the node name is a zero-length string or NULL, information about
** the current state of attachment is returned. If no attachment exists,
** sqlcode 1427 is returned. Otherwise, information about the attachment
** is returned in the sqlerrmc field of the sqlca (as outlined above).
** 
** If an attachment has not been made, instance-level APIs are executed against
** the current instance, specified by the DB2INSTANCE environment variable.
** 
** Certain functions (db2start, db2stop, and all directory services, for
** example) are never executed remotely. That is, they affect only the
** local instance environment, as defined by the value of the DB2INSTANCE
** environment variable.
** 
** If an attachment exists, and the API is issued with a node name, the current
** attachment is dropped, and an attachment to the new node is attempted.
** 
** Where the user name and password are authenticated, and where the password is
** changed, depend on the authentication type of the target instance.
** 
** The node to which an attachment is to be made can also be specified by a call
** to the sqlesetc API.
** 
** REXX API syntax
** 
** Calling this API directly from REXX is not supported. However, REXX
** programmers can utilize this function by calling the DB2 command line
** processor to execute the ATTACH command.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Attach                        */
  sqleatcp_api (
        char * pNodeName,                    /* node name                     */
        char * pUserName,                    /* user name                     */
        char * pPassword,                    /* password                      */
        char * pNewPassword,                 /* new password                  */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleatin API
** Enables an application to specify the node at which instance-level functions
** (CREATE DATABASE and FORCE APPLICATION, for example) are to be
** executed. This node may be the current instance (as defined by the
** value of the DB2INSTANCE environment variable), another instance on
** the same workstation, or an instance on a remote workstation.
** Establishes a logical instance attachment to the node specified,
** and starts a physical communications connection to the node if
** one does not already exist.
** 
** Note:
** If a password change is required, use the sqleatcp API instead of the
** sqleatin API.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** This API establishes an instance attachment.
** 
** API include file
** 
** sqlenv.h
** 
** sqleatin API parameters
** 
** pNodeName
** Input. A string containing the alias of the instance to which the user
** wants to attach. This instance must have a matching entry in the local
** node directory. The only exception is the local instance (as specified
** by the DB2INSTANCE environment variable), which can be specified as
** the object of an attachment, but cannot be used as a node name in the
** node directory. Can be NULL.
** 
** pUserName
** Input. A string containing the user name under which the attachment is to be
** authenticated. Can be NULL.
** 
** pPassword
** Input. A string containing the password for the specified user name. Can be
** NULL.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** Note:
** A node name in the node directory can be regarded as an alias for an
** instance.
** 
** If an attach request succeeds, the sqlerrmc field of the sqlca will contain
** 9 tokens separated by hexadecimal FF (similar to the tokens returned when a
** CONNECT request is successful):
** 
** 1. Country/region code of the application server
** 2. Code page of the application server
** 3. Authorization ID
** 4. Node name (as specified on the API)
** 5. Identity and platform type of the server
** 6. Agent ID of the agent which has been started at the server
** 7. Agent index
** 8. Node number of the server
** 9. Number of database partitions if the server is a partitioned database
** server.
** 
** If the node name is a zero-length string or NULL, information about
** the current state of attachment is returned. If no attachment exists,
** sqlcode 1427 is returned. Otherwise, information about the attachment
** is returned in the sqlerrmc field of the sqlca (as outlined above).
** 
** If an attachment has not been made, instance-level APIs are executed against
** the current instance, specified by the DB2INSTANCE environment variable.
** 
** Certain functions (db2start, db2stop, and all directory services, for
** example) are never executed remotely. That is, they affect only the
** local instance environment, as defined by the value of the DB2INSTANCE
** environment variable.
** 
** If an attachment exists, and the API is issued with a node name, the current
** attachment is dropped, and an attachment to the new node is attempted.
** 
** Where the user name and password are authenticated depends on the
** authentication type of the target instance.
** 
** The node to which an attachment is to be made can also be specified by a call
** to the sqlesetc API.
** 
** REXX API syntax
** 
** ATTACH [TO nodename [USER username USING password]]
** 
** REXX API parameters
** 
** nodename
** Alias of the instance to which the user wants to attach. This instance must
** have a matching entry in the local node directory. The only exception is the
** local instance (as specified by the DB2INSTANCE environment variable), which
** can be specified as the object of an attachment, but cannot be used as a node
** name in the node directory.
** 
** username
** Name under which the user attaches to the instance.
** 
** password
** Password used to authenticate the user name.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Attach                        */
  sqleatin_api (
        char * pNodeName,                    /* node name                     */
        char * pUserName,                    /* user name                     */
        char * pPassword,                    /* password                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledtin API
** Removes the logical instance attachment, and terminates the physical
** communication connection if there are no other logical connections using this
** layer.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None. Removes an existing instance attachment.
** 
** API include file
** 
** sqlenv.h
** 
** sqledtin API parameters
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** REXX API syntax
** 
** DETACH
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Detach                        */
  sqledtin_api (
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlesact API
** Provides accounting information that will be sent to a DRDA server with the
** application's next connect request.
** 
** Authorization
** 
** None
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqlesact API parameters
** 
** pAccountingString
** Input. A string containing the accounting data.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** To send accounting data with a connect request, an application should
** call this API before connecting to a database. The accounting string
** can be changed before connecting to another database by calling the
** API again; otherwise, the value remains in effect until the end of
** the application. The accounting string can be at most
** SQL_ACCOUNT_STR_SZ (defined in sqlenv) bytes long; longer strings
** will be truncated. To ensure that the accounting string is converted
** correctly when transmitted to the DRDA server, use only the characters
** A to Z, 0 to 9, and the underscore (_).
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Set Accounting String         */
  sqlesact_api (
        char * pAccountingString,            /* accounting string             */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleregs API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Register DB2 Server           */
  sqleregs_api (
        unsigned short Registry,             /* location at which to reg      */
        void * pRegisterInfo,                /* register info.                */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledreg API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Deregister DB2 Server         */
  sqledreg_api (
        unsigned short Registry,             /* location at which to dereg    */
        void * pRegisterInfo,                /* deregister info.              */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqleaddn API
** Adds a new database partition server to the partitioned database environment.
** This API creates database partitions for all databases currently
** defined in the instance on the new database partition server. The
** user can specify the source database partition server for any system
** temporary table spaces to be created with the databases, or specify
** that no system temporary table spaces are to be created. The API
** must be issued from the database partition server that is being
** added, and can only be issued on a database partition server.
** 
** Scope
** 
** This API only affects the database partition server on which it is executed.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None
** 
** API include file
** 
** sqlenv.h
** 
** sqleaddn API parameters
** 
** pAddNodeOptions
** Input. A pointer to the optional sqle_addn_options structure. This
** structure is used to specify the source database partition server,
** if any, of the system temporary table space definitions for all
** database partitions created during the add node operation. If not
** specified (that is, a NULL pointer is specified), the system
** temporary table space definitions will be the same as those for
** the catalog partition.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** Before adding a new database partition server, ensure that there
** is sufficient storage for the containers that must be created
** for all existing databases on the system.
** 
** The add node operation creates an empty database partition on
** the new database partition server for every database that exists
** in the instance. The configuration parameters for the new
** database partitions are set to the default value.
** 
** If an add node operation fails while creating a database partition
** locally, it enters a clean-up phase, in which it locally drops all
** databases that have been created. This means that the database
** partitions are removed only from the database partition server
** being added (that is, the local database partition server).
** Existing database partitions remain unaffected on all other
** database partition servers. If this fails, no further clean
** up is done, and an error is returned.
** 
** The database partitions on the new database partition server cannot
** be used to contain user data until after the ALTER DATABASE PARTITION
** GROUP statement has been used to add the database partition server
** to a database partition group.
** 
** This API will fail if a create database or a drop database operation is in
** progress. The API can be called again once the operation has completed.
** 
** If system temporary table spaces are to be created with the database
** partitions, sqleaddn may have to communicate with another database
** partition server in the partitioned database environment in order to retrieve
** the table space definitions. The start_stop_time database manager
** configuration parameter is used to specify the time, in minutes,
** by which the other database partition server must respond with the
** table space definitions. If this time is exceeded, the API fails.
** Increase the value of start_stop_time, and call the API again.
** 
** REXX API syntax
** 
** This API can be called from REXX through the SQLDB2 interface.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Add Node                      */
  sqleaddn_api (
        void * pAddNodeOptions,              /* add node options              */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlecran API
** Creates a database only on the database partition server that calls the API.
** This API is not intended for general use. For example, it should be used with
** db2Restore if the database partition at a database partition server was
** damaged and must be recreated. Improper use of this API can cause
** inconsistencies in the system, so it should only be used with caution.
** 
** Note:
** If this API is used to recreate a database partition that was dropped
** (because it was damaged), the database at this database partition server
** will be in the restore-pending state. After recreating the database
** partition, the database must immediately be restored on this database
** partition server.
** 
** Scope
** 
** This API only affects the database partition server on which it is called.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** Instance. To create a database at another database partition server, it is
** necessary to first attach to that database partition server. A database
** connection is temporarily established by this API during processing.
** 
** API include file
** 
** sqlenv.h
** 
** sqlecran API parameters
** 
** pDbName
** Input. A string containing the name of the database to be created. Must
** not be NULL.
** 
** pReserved
** Input. A spare pointer that is set to null or points to zero. Reserved for
** future use.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** When the database is successfully created, it is placed in restore-pending
** state. The database must be restored on this database partition server before
** it can be used.
** 
** REXX API syntax
** 
** This API can be called from REXX through the SQLDB2 interface.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Create Database at Node       */
  sqlecran_api (
        char * pDbName,                      /* database name                 */
        void * pReserved,                    /* reserved                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledpan API
** Drops a database at a specified database partition server. Can only be
** run in a partitioned database environment.
** 
** Scope
** 
** This API only affects the database partition server on which it is called.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** Required connection
** 
** None. An instance attachment is established for the duration of the call.
** 
** API include file
** 
** sqlenv.h
** 
** sqledpan API parameters
** 
** pDbAlias
** Input. A string containing the alias of the database to be dropped. This name
** is used to reference the actual database name in the system database
** directory.
** 
** pReserved
** Reserved. Should be NULL.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** Improper use of this API can cause inconsistencies in the system, so
** it should only be used with caution.
** 
** REXX API syntax
** 
** This API can be called from REXX through the SQLDB2 interface.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Drop Database at Node         */
  sqledpan_api (
        char * pDbAlias,                     /* database alias                */
        void * pReserved,                    /* reserved                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqledrpn API
** Verifies whether a database partition server is being used by a database. A
** message is returned, indicating whether the database partition server can be
** dropped.
** 
** Scope
** 
** This API only affects the database partition server on which it is issued.
** 
** Authorization
** 
** One of the following:
** - sysadm
** - sysctrl
** 
** API include file
** 
** sqlenv.h
** 
** sqledrpn API parameters
** 
** Action
** The action requested. The valid value is:
** SQL_DROPNODE_VERIFY
** 
** pReserved
** Reserved. Should be NULL.
** 
** pSqlca
** Output. A pointer to the sqlca structure.
** 
** Usage notes
** 
** If a message is returned, indicating that the database partition server
** is not in use, use the db2stop command with DROP NODENUM to remove the
** entry for the database partition server from the db2nodes.cfg file,
** which removes the database partition server from the partitioned
** database environment.
** 
** If a message is returned, indicating that the database partition server is in
** use, the following actions should be taken:
** 
** 1. The database partition server to be dropped will have a database partition
** on it for each database in the instance. If any of these database partitions
** contain data, redistribute the database partition groups that use these
** database partitions. Redistribute the database partition groups to move the
** data to database partitions that exist at database partition servers that are
** not being dropped.
** 
** After the database partition groups are redistributed, drop the database
** partition from every database partition group that uses it. To remove a
** database partition from a database partition group, you can use either the
** drop node option of the sqludrdt API or the ALTER DATABASE PARTITION
** GROUP statement.
** 
** 2. Drop any event monitors that are defined on the database partition server.
** 
** 3. Rerun sqledrpn to ensure that the database partition at the database
** partition server is no longer in use.
** 
** REXX API syntax
** 
** This API can be called from REXX through the SQLDB2 interface.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Drop Node Verify              */
  sqledrpn_api (
        unsigned short Action,               /* Action                        */
        void * pReserved,                    /* reserved                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlesapr API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Set Application Priority      */
  sqlesapr_api (
        sqluint32 agentId,                   /* agentid of application coord  */
        sqlint32 priorityLevel,              /* priority level                */
        unsigned short priorityType,         /* priority type                 */
        struct sqlca * pSqlca);              /* SQLCA                         */

/* GENERIC INTERFACES                                                         */

/******************************************************************************
** sqlgdcgd API
** sqlgdcgd API-specific parameters
** 
** CommentLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** comment. Set to zero if no comment is provided.
** 
** PathLen
** Input. A 2-byte unsigned integer representing the length in bytes of the path
** parameter. Set to zero if no path is provided.
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database alias.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Change Database Comment       */
  sqlgdcgd (
        unsigned short CommentLen,           /* comment length                */
        unsigned short PathLen,              /* drive/path length             */
        unsigned short DbAliasLen,           /* database length               */
        struct sqlca * pSqlca,               /* SQLCA                         */
        _SQLOLDCHAR * pComment,              /* comment                       */
        _SQLOLDCHAR * pPath,                 /* drive/path                    */
        _SQLOLDCHAR * pDbAlias);             /* database                      */

/******************************************************************************
** sqlgdcls API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Close Directory Scan          */
  sqlgdcls (
        unsigned short Handle,               /* handle                        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgdgne API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get Next Database Directory   */
                                             /* Entry                         */
  sqlgdgne (
        unsigned short Handle,               /* handle                        */
        struct sqledinfo ** ppDbDirEntry,    /* buffer                        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgdrpd API
** sqlgdrpd API-specific parameters
** 
** Reserved1
** Reserved for future use.
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database alias.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Drop Database                 */
  sqlgdrpd (
        unsigned short Reserved1,            /* reserved                      */
        unsigned short DbAliasLen,           /* database alias length         */
        struct sqlca * pSqlca,               /* SQLCA                         */
        _SQLOLDCHAR * pReserved2,            /* reserved                      */
        _SQLOLDCHAR * pDbAlias);             /* database alias                */

/******************************************************************************
** sqlgdosd API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Open Directory Scan           */
  sqlgdosd (
        unsigned short PathLen,              /* drive/path length             */
        struct sqlca * pSqlca,               /* SQLCA                         */
        unsigned short * pNumEntries,        /* count                         */
        unsigned short * pHandle,            /* handle                        */
        _SQLOLDCHAR * pPath);                /* drive/path                    */

/******************************************************************************
** sqlgsdeg API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Set parallelism degree        */
  sqlgsdeg (
        struct sqlca * pSqlca,               /* SQLCA                         */
        sqlint32 Degree,                     /* mode of operation             */
        sqluint32 * pAgentIds,               /* array of agent ids            */
        sqlint32 NumAgentIds);               /* num of users to force         */

/******************************************************************************
** sqlgfrce API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Force Users                   */
  sqlgfrce (
        struct sqlca * pSqlca,               /* SQLCA                         */
        unsigned short ForceMode,            /* mode of operation             */
        sqluint32 * pAgentIds,               /* array of agent ids            */
        sqlint32 NumAgentIds);               /* num of users to force         */

/******************************************************************************
** sqlggdad API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Catalog DCS Database          */
  sqlggdad (
        struct sqlca * pSqlca,               /* SQLCA                         */
        struct sql_dir_entry * pDCSDirEntry); /* directory entry              */

/******************************************************************************
** sqlggdcl API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Close DCS Directory Scan      */
  sqlggdcl (
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlggdel API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Uncatalog DCS Database        */
  sqlggdel (
        struct sqlca * pSqlca,               /* SQLCA                         */
        struct sql_dir_entry * pDCSDirEntry); /* directory entry              */

/******************************************************************************
** sqlggdge API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get DCS Directory Entry For   */
                                             /* Database                      */
  sqlggdge (
        struct sqlca * pSqlca,               /* SQLCA                         */
        struct sql_dir_entry * pDCSDirEntry); /* directory entry              */

/******************************************************************************
** sqlggdgt API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get DCS Directory Entries     */
  sqlggdgt (
        struct sqlca * pSqlca,               /* SQLCA                         */
        short * pNumEntries,                 /* count variable                */
        struct sql_dir_entry * pDCSDirEntries); /* entry structure            */

/******************************************************************************
** sqlggdsc API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Open DCS Directory Scan       */
  sqlggdsc (
        struct sqlca * pSqlca,               /* SQLCA                         */
        short * pNumEntries);                /* count variable                */

/******************************************************************************
** sqlggins API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get Instance                  */
  sqlggins (
        struct sqlca * pSqlca,               /* SQLCA                         */
        _SQLOLDCHAR * pInstance);            /* pointer to instance name      */

/******************************************************************************
** sqlgctdd API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Catalog Database              */
  sqlgctdd (
        unsigned short CommentLen,           /* comment length                */
        unsigned short PathLen,              /* drive/path length             */
        unsigned short NodeNameLen,          /* node name length              */
        unsigned short DbAliasLen,           /* alias length                  */
        unsigned short DbNameLen,            /* database length               */
        struct sqlca * pSqlca,               /* SQLCA                         */
        unsigned short Authentication,       /* authentication                */
        _SQLOLDCHAR * pComment,              /* comment                       */
        _SQLOLDCHAR * pPath,                 /* drive/path                    */
        _SQLOLDCHAR * pNodeName,             /* node name                     */
        unsigned char Type,                  /* type                          */
        _SQLOLDCHAR * pDbAlias,              /* alias                         */
        _SQLOLDCHAR * pDbName);              /* database                      */

/******************************************************************************
** sqlgcadb API
** sqlgcadb API-specific parameters
** 
** PrinLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** principal name. Set to zero if no principal is provided. This value should be
** nonzero only when authentication is specified as SQL_AUTHENTICATION_KERBEROS.
** 
** CommentLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** comment. Set to zero if no comment is provided.
** 
** PathLen
** Input. A 2-byte unsigned integer representing the length in bytes of the path
** of the local database directory. Set to zero if no path is provided.
** 
** NodeNameLen
** Input. A 2-byte unsigned integer representing the length in bytes of the node
** name. Set to zero if no node name is provided.
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database alias.
** 
** DbNameLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database name.
** 
** pPrinName
** Input. A string containing the principal name of the DB2 server on which the
** database resides. This value should only be specified when authentication is
** SQL_AUTHENTICATION_KERBEROS.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Catalog Database              */
  sqlgcadb (
        unsigned short PrinLen,              /* principal length              */
        unsigned short CommentLen,           /* comment length                */
        unsigned short PathLen,              /* drive/path length             */
        unsigned short NodeNameLen,          /* node name length              */
        unsigned short DbAliasLen,           /* alias length                  */
        unsigned short DbNameLen,            /* database length               */
        struct sqlca * pSqlca,               /* SQLCA                         */
        _SQLOLDCHAR * pPrinName,             /* Principal Name                */
        unsigned short Authentication,       /* authentication                */
        _SQLOLDCHAR * pComment,              /* comment                       */
        _SQLOLDCHAR * pPath,                 /* drive/path                    */
        _SQLOLDCHAR * pNodeName,             /* node name                     */
        unsigned char Type,                  /* type                          */
        _SQLOLDCHAR * pDbAlias,              /* alias                         */
        _SQLOLDCHAR * pDbName);              /* database                      */

/******************************************************************************
** sqlgctnd API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Catalog Node                  */
  sqlgctnd (
        struct sqlca * pSqlca,               /* SQLCA                         */
        struct sqle_node_struct * pNodeInfo, /* node structure                */
        void * pProtocolInfo);               /* Protocol Structure            */

/******************************************************************************
** sqlgintr API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Interrupt                     */
  sqlgintr (
        void);

/******************************************************************************
** sqlgisig API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Install Signal Handler        */
  sqlgisig (
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgmgdb API
** sqlgmgdb API-specific parameters
** 
** PasswordLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** password. Set to zero when no password is supplied.
** 
** UserNameLen
** Input. A 2-byte unsigned integer representing the length in bytes of the user
** name. Set to zero when no user name is supplied.
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database alias.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Migrate Database              */
  sqlgmgdb (
        unsigned short PasswordLen,          /* password length               */
        unsigned short UserNameLen,          /* user name length              */
        unsigned short DbAliasLen,           /* database alias length         */
        struct sqlca * pSqlca,               /* SQLCA                         */
        _SQLOLDCHAR * pPassword,             /* password                      */
        _SQLOLDCHAR * pUserName,             /* user name                     */
        _SQLOLDCHAR * pDbAlias);             /* database alias                */

/******************************************************************************
** sqlg_activate_db API
** sqlg_activate_db API-specific parameters
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length of the
** database alias name in bytes.
** 
** UserNameLen
** Input. A 2-byte unsigned integer representing the length of the user name in
** bytes. Set to zero if no user name is supplied.
** 
** PasswordLen
** Input. A 2-byte unsigned integer representing the length of the password in
** bytes. Set to zero if no password is supplied.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Activate Database             */
  sqlg_activate_db (
        unsigned short DbAliasLen,           /* database alias length         */
        unsigned short UserNameLen,          /* user name length              */
        unsigned short PasswordLen,          /* password length               */
        char * pDbAlias,                     /* database alias                */
        char * pUserName,                    /* user name                     */
        char * pPassword,                    /* password                      */
        void * pReserved,                    /* reserved                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlg_deactivate_db API
** sqlg_deactivate_db API-specific parameters
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length of the
** database alias name in bytes.
** 
** UserNameLen
** Input. A 2-byte unsigned integer representing the length of the user name in
** bytes. Set to zero if no user name is supplied.
** 
** PasswordLen
** Input. A 2-byte unsigned integer representing the length of the password in
** bytes. Set to zero if no password is supplied.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Deactivate Database           */
  sqlg_deactivate_db (
        unsigned short DbAliasLen,           /* database alias length         */
        unsigned short UserNameLen,          /* user name length              */
        unsigned short PasswordLen,          /* password length               */
        char * pDbAlias,                     /* database alias                */
        char * pUserName,                    /* user name                     */
        char * pPassword,                    /* password                      */
        void * pReserved,                    /* reserved                      */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgncls API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Close Node Directory Scan     */
  sqlgncls (
        unsigned short Handle,               /* handle                        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgngne API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Get Next Node Directory       */
                                             /* Entry                         */
  sqlgngne (
        unsigned short Handle,               /* handle                        */
        struct sqleninfo ** ppNodeDirEntry,  /* buffer                        */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgnops API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Open Node Directory Scan      */
  sqlgnops (
        unsigned short * pHandle,            /* handle                        */
        unsigned short * pNumEntries,        /* count                         */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgproc API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Database Application Remote   */
                                             /* Interface                     */
  sqlgproc (
        unsigned short ProgramNameLen,       /* program name length           */
        struct sqlca * pSqlca,               /* SQLCA                         */
        char * pProgramName,                 /* Path of program to run        */
        struct sqlda * pInputSqlda,          /* input SQLDA                   */
        struct sqlda * pOutputSqlda,         /* output SQLDA                  */
        struct sqlchar * pInput);            /* variable length area ptr      */

/******************************************************************************
** sqlgrstd API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Restart Database              */
  sqlgrstd (
        unsigned short PasswordLen,          /* password length               */
        unsigned short UserNameLen,          /* user name length              */
        unsigned short DbAliasLen,           /* database alias length         */
        struct sqlca * pSqlca,               /* SQLCA                         */
        _SQLOLDCHAR * pPassword,             /* password                      */
        _SQLOLDCHAR * pUserName,             /* user name                     */
        _SQLOLDCHAR * pDbAlias);             /* database alias                */

/******************************************************************************
** sqlgsetc API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Set Client                    */
  sqlgsetc (
        struct sqle_conn_setting * pConnectionSettings, /* conn setting       */
                                             /* array                         */
        unsigned short NumSettings,          /* number of settings            */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgqryc API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Query Client                  */
  sqlgqryc (
        struct sqle_conn_setting * pConnectionSettings, /* conn setting       */
                                             /* array                         */
        unsigned short NumSettings,          /* number of settings            */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgcrea API
** sqlgcrea API-specific parameters
** 
** PathLen
** Input. A 2-byte unsigned integer representing the length of the path
** in bytes. Set to zero if no path is provided.
** 
** LocalDbALiasLen
** Input. A 2-byte unsigned integer representing the length of the local
** database alias in bytes. Set to zero if no local alias is provided.
** 
** DbNameLen
** Input. A 2-byte unsigned integer representing the length of the
** database name in bytes.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Create Database               */
  sqlgcrea (
        unsigned short PathLen,              /* drive/path length             */
        unsigned short LocalDbAliasLen,      /* local alias length            */
        unsigned short DbNameLen,            /* database length               */
        struct sqlca * pSqlca,               /* SQLCA                         */
        void * pReserved1,                   /* reserved                      */
        unsigned short Reserved2,            /* reserved                      */
        SQLEDBTERRITORYINFO * pTerritoryInfo, /* db locale and codeset        */
        struct sqledbdesc * pDbDescriptor,   /* db description block          */
        char * pPath,                        /* drive/path                    */
        char * pLocalDbAlias,                /* local alias                   */
        char * pDbName);                     /* database                      */

/******************************************************************************
** sqlgatin API
** sqlgatin API-specific parameters
** 
** PasswordLen
** Input. A 2-byte unsigned integer representing the length of the password in
** bytes. Set to zero if no password is supplied.
** 
** UserNameLen
** Input. A 2-byte unsigned integer representing the length of the user name in
** bytes. Set to zero if no user name is supplied.
** 
** NodeNameLen
** Input. A 2-byte unsigned integer representing the length of the node name in
** bytes. Set to zero if no node name is supplied.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Attach                        */
  sqlgatin (
        unsigned short PasswordLen,          /* password length               */
        unsigned short UserNameLen,          /* user name length              */
        unsigned short NodeNameLen,          /* node name length              */
        struct sqlca * pSqlca,               /* SQLCA                         */
        char * pPassword,                    /* password                      */
        char * pUserName,                    /* user name                     */
        char * pNodeName);                   /* node name                     */

/******************************************************************************
** sqlgdtin API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Detach                        */
  sqlgdtin (
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgpstart API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* V5 Start Database Manager     */
  sqlgpstart (
        struct sqle_start_options * pStartOptions, /* start options           */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgpstp API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* V5 Stop Database Manager      */
  sqlgpstp (
        struct sqledbstopopt * pStopOptions, /* STOP OPTIONS                  */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlguncd API
** sqlguncd API-specific parameters
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database alias.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Uncatalog Database            */
  sqlguncd (
        unsigned short DbAliasLen,           /* database alias length         */
        struct sqlca * pSqlca,               /* SQLCA                         */
        _SQLOLDCHAR * pDbAlias);             /* database alias                */

/******************************************************************************
** sqlguncn API
** sqlguncn API-specific parameters
** 
** NodeNameLen
** Input. A 2-byte unsigned integer representing the length in bytes of the node
** name.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Uncatalog Node                */
  sqlguncn (
        unsigned short NodeNameLen,          /* node name length              */
        struct sqlca * pSqlca,               /* SQLCA                         */
        _SQLOLDCHAR * pNodeName);            /* node name                     */

/******************************************************************************
** sqlgsact API
** sqlgsact API-specific parameters
** 
** AccountingStringLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** accounting string.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Set Accounting String         */
  sqlgsact (
        unsigned short AccountingStringLen,  /* accounting string length      */
        char * pAccountingString,            /* accounting string             */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgregs API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Register DB2 Server           */
  sqlgregs (
        unsigned short Registry,             /* location at which to reg      */
        void * pRegisterInfo,                /* register info.                */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgdreg API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Deregister DB2 Server         */
  sqlgdreg (
        unsigned short Registry,             /* location at which to dereg    */
        void * pRegisterInfo,                /* deregister info.              */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgfmem API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Free Memory                   */
  sqlgfmem (
        struct sqlca * pSqlca,               /* SQLCA                         */
        void * pBuffer);                     /* buffer pointer                */

/******************************************************************************
** sqlgaddn API
** sqlgaddn API-specific parameters
** 
** addnOptionsLen
** Input. A 2-byte unsigned integer representing the length of the optional
** sqle_addn_options structure in bytes.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Add Node                      */
  sqlgaddn (
        unsigned short addnOptionsLen,       /* options length                */
        struct sqlca * pSqlca,               /* SQLCA                         */
        void * pAddNodeOptions);             /* add node options              */

/******************************************************************************
** sqlgcran API
** sqlgcran API-specific parameters
** 
** reservedLen
** Input. Reserved for the length of pReserved.
** 
** dbNameLen
** Input. A 2-byte unsigned integer representing the length of the database name
** in bytes.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Create Database at Node       */
  sqlgcran (
        unsigned short reservedLen,          /* reserved length               */
        unsigned short dbNameLen,            /* database name length          */
        struct sqlca * pSqlca,               /* SQLCA                         */
        void * pReserved,                    /* reserved                      */
        char * pDbName);                     /* database name                 */

/******************************************************************************
** sqlgdpan API
** sqlgdpan API-specific parameters
** 
** Reserved1
** Reserved for future use.
** 
** DbAliasLen
** Input. A 2-byte unsigned integer representing the length in bytes of the
** database alias.
** 
** pReserved2
** A spare pointer that is set to null or points to zero. Reserved for
** future use.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Drop Database at Node         */
  sqlgdpan (
        unsigned short Reserved1,            /* reserved                      */
        unsigned short DbAliasLen,           /* database alias length         */
        struct sqlca * pSqlca,               /* SQLCA                         */
        void * pReserved2,                   /* reserved                      */
        char * pDbAlias);                    /* database alias                */

/******************************************************************************
** sqlgdrpn API
** sqlgdrpn API-specific parameters
** 
** Reserved1
** Reserved for the length of pReserved2.
** 
** pReserved2
** A spare pointer that is set to NULL or points to 0. Reserved for future use.
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Drop Node Verify              */
  sqlgdrpn (
        unsigned short Reserved1,            /* reserved                      */
        struct sqlca * pSqlca,               /* SQLCA                         */
        void * pReserved2,                   /* reserved                      */
        unsigned short Action);              /* Action                        */

/* SQL Return Codes in SQLCODE for Environment Commands                       */

#ifndef SQL_RC_OK
#define SQL_RC_OK 0 /* everything is ok */
#endif /* SQL_RC_OK */

#ifndef SQL_RC_INVALID_SQLCA
#define SQL_RC_INVALID_SQLCA -1 /* invalid sqlca */
#endif /* SQL_RC_INVALID_SQLCA */

#define SQLE_RC_STILL_EXECUTING 16           /* Request is still executing    */
#define SQLE_RC_INLUW  -752                  /* Connect to other DB not       */
                                             /* allowed                       */
#define SQLE_RC_W863   863                   /* Only SBCS data allowed        */

#define SQLE_RC_E953   -953                  /* Agent heap too small          */
#define SQLE_RC_E957   -957                  /* Requestor comm heap too       */
                                             /* small                         */
#define SQLE_RC_E959   -959                  /* Server comm heap too small    */
#define SQLE_RC_E961   -961                  /* Rqstr remote svcs heap too    */
                                             /* small                         */
#define SQLE_RC_E962   -962                  /* Svr remote svcs heap too      */
                                             /* small                         */
#define SQLE_RC_NO_APP_CTRL_SEG -987         /* Application control shared    */
                                             /* memory set cannot be          */
                                             /* allocated                     */

#define SQLE_RC_INVNEWLOGP 993               /* New log path is invalid       */
#define SQLE_RC_INVLOGP 995                  /* Current log path is invalid   */

#define SQLE_RC_INVALIAS -1000               /* Invalid alias                 */
#define SQLE_RC_INVDBNAME -1001              /* Invalid database name         */
#define SQLE_RC_INVDRIVE -1002               /* Invalid drive                 */

#define SQLE_RC_INVPSW_DB2OS2 -1003          /* Invalid password - OS/2       */
#define SQLE_RC_INVPSW_DB2NT -1003           /* Invalid password - NT         */
#define SQLE_RC_INVPSW_DB2DOS -1003          /* Invalid password - DOS        */
#define SQLE_RC_INVPSW_DB2WIN -1003          /* Invalid password - Windows    */
#define SQLE_RC_INVPSW_DB2MAC -1003          /* Invalid password - Mac        */
#define SQLE_RC_INVPSW_DB2AIX -10002         /* Invalid password - AIX        */

#define SQLE_RC_INVPSW SQLE_RC_INVPSW_DB2AIX

#define SQLE_RC_INSSTOR -1004                /* Insuf storage on file system  */
#define SQLE_RC_DUPALIAS -1005               /* Duplicate alias               */
#define SQLE_RC_WRONGCODEPG -1006            /* Appl code page does not       */
                                             /* match db                      */
#define SQLE_RC_INV_NEWPSW -1008             /* Invalid new password          */
#define SQLE_RC_INVREMOTE -1009              /* Invalid remote command        */

#define SQLE_RC_INVTYPE -1010                /* Invalid type                  */
#define SQLE_RC_NODRIVE -1011                /* No drive for indirect entry   */
#define SQLE_RC_NONODE -1012                 /* No nodename for remote entry  */
#define SQLE_RC_NODB   -1013                 /* Alias or database name not    */
                                             /* found                         */
#define SQLE_RC_NOMORE 1014                  /* No more entries               */
#define SQLE_RC_DB_RESTART -1015             /* Database needs restart        */
#define SQLE_RC_INVLLU -1016                 /* Invalid local_lu alias        */
#define SQLE_RC_INVMODE -1017                /* Invalid mode                  */
#define SQLE_RC_DUPNODE -1018                /* Duplicate node name           */
#define SQLE_RC_INVNODE -1019                /* Invalid node name             */

#define SQLE_RC_MAXNODE -1020                /* Node could not be cataloged   */
#define SQLE_RC_NOTNODE -1021                /* Nodename not found            */

#define SQLE_RC_INSSYS_DB2OS2 -1022          /* Insuf system resources - OS   */
                                             /* 2                             */
#define SQLE_RC_INSSYS_DB2NT -1022           /* Insuf system resources - NT   */
#define SQLE_RC_INSSYS_DB2DOS -1022          /* Insuf system resources - DOS  */
#define SQLE_RC_INSSYS_DB2WIN -1022          /* Insuf system resources -      */
                                             /* Windows                       */
#define SQLE_RC_INSSYS_DB2MAC -1022          /* Insuf system resources -      */
                                             /* Macintosh                     */
#define SQLE_RC_INSSYS_DB2AIX -10003         /* Insuf system resources - AIX  */

#define SQLE_RC_INSSYS SQLE_RC_INSSYS_DB2AIX

#define SQLE_RC_NOCONV -1023                 /* Communication conversation    */
                                             /* failed                        */
#define SQLE_RC_NOSUDB -1024                 /* No database connection        */
                                             /* exists                        */
#define SQLE_RC_DBACT  -1025                 /* Databases are active          */
#define SQLE_RC_INVSTRT -1026                /* Database manager already      */
                                             /* started                       */
#define SQLE_RC_NONODEDIR -1027              /* Node directory not found      */
#define SQLE_RC_INVRLU -1029                 /* Partner lu not specified      */

#define SQLE_RC_MAXDB  -1030                 /* Database directory is full    */
#define SQLE_RC_NODBDIR -1031                /* Database directory not found  */
#define SQLE_RC_NOSTARTG -1032               /* Database manager not started  */
#define SQLE_RC_DIRBUSY -1033                /* Database directory being      */
                                             /* updated                       */
#define SQLE_RC_DBBAD  -1034                 /* Database is damaged           */
#define SQLE_RC_DB_INUSE -1035               /* Database already in use       */
#define SQLE_RC_FILEDB -1036                 /* Database file error           */
#define SQLE_RC_NODE_DIR_EMPTY 1037          /* No entry in Node directory    */
#define SQLE_RC_FILENODE -1038               /* Node directory file error     */

#define SQLE_RC_FILEDIR_DB2OS2 -1039         /* Directory file error - OS/2   */
#define SQLE_RC_FILEDIR_DB2NT -1039          /* Directory file error - NT     */
#define SQLE_RC_FILEDIR_DB2DOS -1039         /* Directory file error - DOS    */
#define SQLE_RC_FILEDIR_DB2WIN -1039         /* Directory file error -        */
                                             /* Windows                       */
#define SQLE_RC_FILEDIR_DB2MAC -1039         /* Directory file error - Mac    */
#define SQLE_RC_FILEDIR_DB2AIX -10004        /* Directory file error - AIX    */

#define SQLE_RC_FILEDIR SQLE_RC_FILEDIR_DB2AIX

#define SQLE_RC_MAXAPPLS -1040               /* Max number of applications    */
                                             /* reached                       */
#define SQLE_RC_MAXDBS -1041                 /* Max number of databases       */
                                             /* started                       */
#define SQLE_RC_SYSERR -1042                 /* System error                  */
#define SQLE_RC_CATBOOT -1043                /* Catalog bootstrap failure     */
#define SQLE_RC_INTRRPT -1044                /* Request interrupted by        */
                                             /* ctrl+break                    */
#define SQLE_RC_INVINDIR -1045               /* Invalid level of indirection  */
#define SQLE_RC_INVAUTHID -1046              /* Invalid userid                */
#define SQLE_RC_APCONN -1047                 /* Appl already connect to       */
                                             /* another db                    */

#define SQLE_RC_USEINVALID_DB2OS2 -1048      /* Invalid use specified - OS/2  */
#define SQLE_RC_USEINVALID_DB2NT -1048       /* Invalid use specified - NT    */
#define SQLE_RC_USEINVALID_DB2DOS -1048      /* Invalid use specified - DOS   */
#define SQLE_RC_USEINVALID_DB2WIN -1048      /* Invalid use specified -       */
                                             /* Windows                       */
#define SQLE_RC_USEINVALID_DB2MAC -1048      /* Invalid use specified -       */
                                             /* MacOS                         */
#define SQLE_RC_USEINVALID_DB2AIX -10005     /* Invalid use specified - AIX   */

#define SQLE_RC_USEINVALID SQLE_RC_USEINVALID_DB2AIX

#define SQLE_RC_APPSERR -900                 /* Appl state in error           */

#define SQLE_RC_UNCHOME -1050                /* Cannot uncatalog 'Home'       */
                                             /* database                      */
#define SQLE_RC_NODIRDRV -1051               /* Db direct. drive does not     */
                                             /* exist                         */
#define SQLE_RC_NODBDRV -1052                /* Database drive does not       */
                                             /* exist                         */
#define SQLE_RC_INTBUSY -1053                /* Interrupt already in          */
                                             /* progress                      */
#define SQLE_RC_COMMINP -1054                /* Commit in progress - no       */
                                             /* int's                         */
#define SQLE_RC_ROLLINP -1055                /* Rollback in progress - no     */
                                             /* int's                         */
#define SQLE_RC_NOINTER -1360                /* Cannot be interrupted - no    */
                                             /* int's                         */
#define SQLE_RC_TIMEOUT 1361                 /* Windows Client Execution      */
                                             /* Timeout                       */
#define SQLE_RC_MAXSCAN -1056                /* Maximum allowable scans       */
                                             /* exceeded                      */
#define SQLE_RC_NODENTRY 1057                /* No entries in directory       */
#define SQLE_RC_INVHAND -1058                /* Invalid input handle          */
#define SQLE_RC_NOSCAN -1059                 /* Open scan not issued          */

#define SQLE_RC_NOCONNECT -1060              /* User lacks connect privilege  */
#define SQLE_RC_QSNOCONNECT -20157           /* User lacks connect privilege  */
                                             /* for Quiesced Db               */
#define SQLE_RC_RESTART_WITH_INDOUBTS 1061   /* RESTART was successful, but   */
                                             /* indoubt transactions exist    */
#define SQLE_RC_BADPATH -1062                /* Database path not found       */
#define SQLE_RC_START_OK -1063               /* Database manager started OK   */
#define SQLE_RC_STOP_OK -1064                /* Database manager stopped OK   */
#define SQLE_RC_DB_BADBINDS 1065             /* N utilities not bound         */

#define SQLE_RC_NOMSG_DB2OS2 -1068           /* Message file not found - OS   */
                                             /* 2                             */
#define SQLE_RC_NOMSG_DB2NT -1068            /* Message file not found - NT   */
#define SQLE_RC_NOMSG_DB2DOS -1068           /* Message file not found - DOS  */
#define SQLE_RC_NOMSG_DB2WIN -1068           /* Message file not found -      */
                                             /* Windows                       */
#define SQLE_RC_NOMSG_DB2MAC -1068           /* Message file not found -      */
                                             /* MacOS                         */
#define SQLE_RC_NOMSG_DB2AIX -10007          /* Message file not found - AIX  */

#define SQLE_RC_NOMSG  SQLE_RC_NOMSG_DB2AIX

#define SQLE_RC_INVDROP -1069                /* DB invalid type for Drop      */

#define SQLE_RC_INVDBNAME_PTR -1070          /* Invalid Database Name         */
                                             /* pointer                       */
#define SQLE_RC_INVALIAS_PTR -1071           /* Invalid Alias pointer         */
#define SQLE_RC_RESOURCE_ERROR -1072         /* Resources in inconsistent     */
                                             /* state                         */
#define SQLE_RC_BAD_ND_REL -1073             /* Invalid Node Directory        */
                                             /* release                       */
#define SQLE_RC_INVPSW_PTR -1074             /* Invalid Password pointer      */
#define SQLE_RC_INVCOMM_PTR -1075            /* Invalid Comment pointer       */
#define SQLE_RC_INVCNT_PTR -1076             /* Invalid Count pointer         */
#define SQLE_RC_INVHAND_PTR -1077            /* Invalid Handle pointer        */
#define SQLE_RC_INVBUFF_PTR -1078            /* Invalid Buffer pointer        */
#define SQLE_RC_INVNODE_PTR -1079            /* Invalid Node pointer          */
#define SQLE_RC_INVUSERID_PTR -1150          /* Invalid Userid pointer        */
#define SQLE_RC_INVPARM_PTR -1151            /* Invalid Parms pointer         */

#define SQLE_RC_INVLLU_PTR -1080             /* Invalid Local Lu pointer      */
#define SQLE_RC_INVRLU_PTR -1081             /* Invalid Remote Lu pointer     */
#define SQLE_RC_INVMODE_PTR -1082            /* Invalid Mode pointer          */
#define SQLE_RC_BAD_DBDB -1083               /* Bad Database Description      */
                                             /* Block                         */
#define SQLE_RC_KSEGSFAIL -1084              /* Cannot Allocate Kernel        */
                                             /* Segments                      */
#define SQLE_RC_APPHEAPFAIL -1085            /* Cannot Allocate Application   */
                                             /* heap                          */
#define SQLE_RC_OS2ERROR -1086               /* Unexpected OS/2 error         */
#define SQLE_RC_BIND_LIST 1087               /* Bind list could not be        */
                                             /* opened                        */
#define SQLE_RC_BIND_ERROR 1088              /* Error occurred during bind    */
#define SQLE_RC_BIND_INTRRPT 1089            /* Binding was interrupted       */

#define SQLE_RC_BAD_APP_REL -1090            /* Release number of appl is     */
                                             /* invalid                       */
#define SQLE_RC_BAD_DB_REL -1091             /* Release number of database    */
                                             /* bad                           */

#define SQLE_RC_INSAUTH -1092                /* Authorization error           */
#define SQLE_RC_NOLOGON -1093                /* User not logged on            */
#define SQLE_RC_NDBUSY -1094                 /* Node directory being updated  */
#define SQLE_RC_MAX_NDSCAN -1095             /* Max node scans open           */
#define SQLE_RC_REQTYPE -1096                /* Invalid type for requester    */
                                             /* node                          */
#define SQLE_RC_NODERR -1097                 /* Node not found for remote db  */
#define SQLE_RC_APCONN_SAME -1098            /* Appl is already connected to  */
                                             /* db                            */

#define SQLE_RC_WRPROT_ERR_DB2OS2 -1099      /* Write protect error on        */
                                             /* diskette                      */
#define SQLE_RC_WRPROT_ERR_DB2NT -1099       /* Write protect error on        */
                                             /* diskette                      */
#define SQLE_RC_WRPROT_ERR_DB2DOS -1099      /* Write protect error on        */
                                             /* diskette                      */
#define SQLE_RC_WRPROT_ERR_DB2WIN -1099      /* Write protect error on        */
                                             /* diskette                      */
#define SQLE_RC_WRPROT_ERR_DB2MAC -1099      /* Write protect error on        */
                                             /* diskette                      */
#define SQLE_RC_WRPROT_ERR_DB2AIX -10021     /* Write protect error on        */
                                             /* filesystem                    */

#define SQLE_RC_WRPROT_ERR SQLE_RC_WRPROT_ERR_DB2AIX

#define SQLE_RC_NODE_WARN 1100               /* Node not cataloged for        */
                                             /* database                      */
#define SQLE_RC_REMCONN_ERR -1101            /* Remote communications error   */
#define SQLE_RC_MIG_NODB -1102               /* No database name provided in  */
                                             /* call                          */
#define SQLE_RC_MIG_OK 1103                  /* Migration was successful      */
#define SQLE_RC_INVPROG_PTR -1104            /* Invalid program name pointer  */

#define SQLE_RC_INV_SPDB_DB2OS2 -1105        /* Invalid disconnect from       */
                                             /* database                      */
#define SQLE_RC_INV_SPDB_DB2NT -1105         /* Invalid disconnect from       */
                                             /* database                      */
#define SQLE_RC_INV_SPDB_DB2DOS -1105        /* Invalid disconnect from       */
                                             /* database                      */
#define SQLE_RC_INV_SPDB_DB2WIN -1105        /* Invalid disconnect from       */
                                             /* database                      */
#define SQLE_RC_INV_SPDB_DB2MAC -1105        /* Invalid disconnect from       */
                                             /* database                      */
#define SQLE_RC_INV_SPDB_DB2AIX -10017       /* Invalid disconnect from       */
                                             /* database                      */

#define SQLE_RC_INV_SPDB SQLE_RC_INV_SPDB_DB2AIX

#define SQLE_RC_INVALID_PROC_DB2OS2 -1106    /* Function could not be         */
                                             /* executed                      */
#define SQLE_RC_INVALID_PROC_DB2NT -1106     /* Function could not be         */
                                             /* executed                      */
#define SQLE_RC_INVALID_PROC_DB2DOS -1106    /* Function could not be         */
                                             /* executed                      */
#define SQLE_RC_INVALID_PROC_DB2WIN -1106    /* Function could not be         */
                                             /* executed                      */
#define SQLE_RC_INVALID_PROC_DB2MAC -1106    /* Function could not be         */
                                             /* executed                      */
#define SQLE_RC_INVALID_PROC_DB2AIX -10010   /* Function could not be         */
                                             /* executed                      */

#define SQLE_RC_INVALID_PROC SQLE_RC_INVALID_PROC_DB2AIX

#define SQLE_RC_INTRP_PROC_DB2OS2 -1107      /* Program interrupted - OS/2    */
#define SQLE_RC_INTRP_PROC_DB2NT -1107       /* Program interrupted - NT      */
#define SQLE_RC_INTRP_PROC_DB2DOS -1107      /* Program interrupted - DOS     */
#define SQLE_RC_INTRP_PROC_DB2WIN -1107      /* Program interrupted -         */
                                             /* Windows                       */
#define SQLE_RC_INTRP_PROC_DB2MAC -1107      /* Program interrupted - MacOS   */
#define SQLE_RC_INTRP_PROC_DB2AIX -10011     /* Program interrupted - AIX     */

#define SQLE_RC_INTRP_PROC SQLE_RC_INVALID_PROC_DB2AIX

#define SQLE_RC_SYSERR_PROC_DB2OS2 -1108     /* System error on library load  */
#define SQLE_RC_SYSERR_PROC_DB2NT -1108      /* System error on library load  */
#define SQLE_RC_SYSERR_PROC_DB2DOS -1108     /* System error on library load  */
#define SQLE_RC_SYSERR_PROC_DB2WIN -1108     /* System error on library load  */
#define SQLE_RC_SYSERR_PROC_DB2MAC -1108     /* System error on library load  */
#define SQLE_RC_SYSERR_PROC_DB2AIX -10012    /* System error on library load  */

#define SQLE_RC_SYSERR_PROC SQLE_RC_SYSERR_PROC_DB2AIX

#define SQLE_RC_NOFILE_PROC_DB2OS2 -1109     /* Library could not be loaded   */
#define SQLE_RC_NOFILE_PROC_DB2NT -1109      /* Library could not be loaded   */
#define SQLE_RC_NOFILE_PROC_DB2DOS -1109     /* Library could not be loaded   */
#define SQLE_RC_NOFILE_PROC_DB2WIN -1109     /* Library could not be loaded   */
#define SQLE_RC_NOFILE_PROC_DB2MAC -1109     /* Library could not be loaded   */
#define SQLE_RC_NOFILE_PROC_DB2AIX -10013    /* Library could not be loaded   */

#define SQLE_RC_NOFILE_PROC SQLE_RC_NOFILE_PROC_DB2AIX

#define SQLE_RC_ERROR_PROC -1110             /* Program error                 */

#define SQLE_RC_BADPGN_PROC_DB2OS2 -1111     /* Invalid DARI prog name        */
                                             /* format                        */
#define SQLE_RC_BADPGN_PROC_DB2NT -1111      /* Invalid DARI prog name        */
                                             /* format                        */
#define SQLE_RC_BADPGN_PROC_DB2DOS -1111     /* Invalid DARI prog name        */
                                             /* format                        */
#define SQLE_RC_BADPGN_PROC_DB2WIN -1111     /* Invalid DARI prog name        */
                                             /* format                        */
#define SQLE_RC_BADPGN_PROC_DB2MAC -1111     /* Invalid DARI prog name        */
                                             /* format                        */
#define SQLE_RC_BADPGN_PROC_DB2AIX -10014    /* Invalid DARI prog name        */
                                             /* format                        */

#define SQLE_RC_BADPGN_PROC SQLE_RC_BADPGN_PROC_DB2AIX

#define SQLE_RC_INSMEM_PROC_DB2OS2 -1112     /* Insuf memory to load lib      */
#define SQLE_RC_INSMEM_PROC_DB2NT -1112      /* Insuf memory to load lib      */
#define SQLE_RC_INSMEM_PROC_DB2DOS -1112     /* Insuf memory to load lib      */
#define SQLE_RC_INSMEM_PROC_DB2WIN -1112     /* Insuf memory to load lib      */
#define SQLE_RC_INSMEM_PROC_DB2MAC -1112     /* Insuf memory to load lib      */
#define SQLE_RC_INSMEM_PROC_DB2AIX -10015    /* Insuf memory to load lib      */

#define SQLE_RC_INSMEM_PROC SQLE_RC_INSMEM_PROC_DB2AIX

#define SQLE_RC_SQLDA_DATATYPE -1113         /* Data type in output SQLDA     */
                                             /* changed                       */
#define SQLE_RC_SQLDA_LENGTH -1114           /* Data length in output SQLDA   */
                                             /* changed                       */
#define SQLE_RC_SQLDA_VARS -1115             /* Num of sqlvars changed in     */
                                             /* SQLDA                         */
#define SQLE_RC_BKP_PEND -1116               /* Backup pending                */
#define SQLE_RC_ROLLFWD_PEND -1117           /* Roll forward pending          */
#define SQLE_RC_BKP_INPROG -1118             /* Need to rerun the Backup      */
                                             /* process                       */
#define SQLE_RC_RST_INPROG -1119             /* Need to rerun the Restore     */
                                             /* process                       */
#define SQLE_RC_BR_INPROG -1120              /* Need to rerun either Backup   */
                                             /* or Restore process            */

#define SQLE_RC_INVNODESTR_PTR -1121         /* Invalid Node structure        */
                                             /* pointer                       */
#define SQLE_RC_INVPROTOCOL_PTR -1122        /* Invalid Protocol structure    */
                                             /* pointer                       */
#define SQLE_RC_INVPROTOCOL -1123            /* Invalid protocol type         */
#define SQLE_RC_INVRNNAME -1124              /* Invalid remote workstation    */
                                             /* name                          */
#define SQLE_RC_INVADAPTER -1125             /* Invalid adapter number        */
#define SQLE_RC_INVNETID -1126               /* Invalid network id            */
#define SQLE_RC_INVPLU -1127                 /* Invalid real partner LU name  */

#define SQLE_RC_DARI_INSSYS -1129            /* Insuf system resources for    */
                                             /* DARI                          */
#define SQLE_RC_DARI_MAXDARI -1130           /* Max DARI process limit        */
                                             /* reached                       */
#define SQLE_RC_DARI_ABEND -1131             /* DARI process abnormally       */
                                             /* terminated                    */
#define SQLE_RC_DARI_INV_RQST -1132          /* Invalid DB2 request in DARI   */
#define SQLE_RC_DARI_VAR_POINTER_CHG -1133   /* SQLVAR's sqldata or sqlind    */
                                             /* ptrs were altered             */
#define SQLE_RC_DARI_RQST_AUTH_ERR -1134     /* DB2 request is not allowed    */
                                             /* when DB auth is client        */
#define SQLE_RC_BAD_NUMSEGS -1135            /* Invalid numsegs on create db  */
#define SQLE_RC_BAD_EXTSIZE -1136            /* Invalid extSize on create db  */
#define SQLE_RC_MOUNTED_SEGS -1137           /* Mounted Segment Directories   */
                                             /* on a drop database request    */

#define SQLE_RC_WARN_DLMON 1187              /* Deadlocks event monitor not   */
                                             /* created during create db      */

#define SQLE_RC_WARN_DB2LK 1243              /* Failed to drop db2look        */
                                             /* operation table/view          */
                                             /* SYSTOOLS.DB2LOOK_INFO         */
                                             /* SYSTOOLS.DB2LOOK_INOF_V       */
                                             /* during database migration     */

#define SQLE_RC_INVALID_VALUE -1197          /* API or command option has an  */
                                             /* invalid value                 */

#define SQLE_RC_INVOS_OBJ -1200              /* Invalid object specified      */
#define SQLE_RC_INVOS_STAT -1201             /* Invalid status specified      */
#define SQLE_RC_INVOS_NOSTAT -1202           /* Status has not been           */
                                             /* collected                     */
#define SQLE_RC_INVOS_NOUSER -1203           /* No users connected to         */
                                             /* database                      */
#define SQLE_RC_UNSUPP_CODEPG -1204          /* Active codepage is not        */
                                             /* supported                     */

#define SQLE_RC_INV_CNTRYINFO_DB2OS2 -1205   /* Invalid territory             */
                                             /* information                   */
#define SQLE_RC_INV_CNTRYINFO_DB2NT -1205    /* Invalid territory             */
                                             /* information                   */
#define SQLE_RC_INV_CNTRYINFO_DB2DOS -1205   /* Invalid territory             */
                                             /* information                   */
#define SQLE_RC_INV_CNTRYINFO_DB2WIN -1205   /* Invalid territory             */
                                             /* information                   */
#define SQLE_RC_INV_CNTRYINFO_DB2MAC -1205   /* Invalid territory             */
                                             /* information                   */
#define SQLE_RC_INV_CNTRYINFO_DB2AIX -1205   /* Invalid territory             */
                                             /* information                   */

#define SQLE_RC_INV_CNTRYINFO SQLE_RC_INV_CNTRYINFO_DB2AIX

#define SQLE_RC_INV_COMPUTERNAME -1211       /* Invalid Computer Name         */
#define SQLE_RC_INV_INSTANCENAME -1212       /* Invalid Instance Name         */
#define SQLE_RC_INVCHGPWDLU -1213            /* Invalid Change Password LU    */
#define SQLE_RC_INVTPNAME -1214              /* Invalid Transaction Pgm Name  */
#define SQLE_RC_INVLANADDRESS -1215          /* Invalid LAN Adapter Addr      */
#define SQLE_RC_NO_SHRD_SEG -1220            /* DB2 Shared Memory Set alloc   */
                                             /* failed                        */
#define SQLE_RC_NO_ASL_HEAP -1221            /* ASL heap cannot be allocated  */
#define SQLE_RC_ASL_TOO_SMALL -1222          /* ASL heap is too small         */
#define SQLE_RC_NO_AGENT_AVAIL -1223         /* No more agents available      */
#define SQLE_RC_AGENT_GONE -1224             /* DB2 agent not active          */
#define SQLE_RC_PROC_LIMIT -1225             /* Op. Sys. couldn't spawn a     */
                                             /* process                       */
#define SQLE_RC_MAXCOORDS -1226              /* Max number of coords reached  */
#define SQLE_RC_DROPDB_WARN 1228             /* Drop database warning         */

#define SQLE_RC_AGENT_NOT_FORCED 1230        /* At least one agent not        */
                                             /* forced                        */
#define SQLE_RC_INVCOUNT -1231               /* Invalid Force Users count     */
#define SQLE_RC_INVFRCE_MODE -1232           /* Invalid Force Users mode      */

#define SQLE_RC_INV_TBS_DESC -1241           /* Invalid TableSpace            */
                                             /* descriptor                    */

#define SQLE_RC_NO_SETCONNOPT -1246          /* Cannot set connection         */
                                             /* options - existing            */
                                             /* connections                   */

#define SQLC_RC_NPIPE_BROKEN               -1281
#define SQLC_RC_NPIPE_BUSY                 -1282
#define SQLC_RC_NPIPE_PIPE_INUSE           -1283
#define SQLC_RC_NPIPE_PIPE_NOT_FOUND       -1284
#define SQLC_RC_NPIPE_INVALID_NAME         -1285
#define SQLC_RC_NPIPE_NO_RESOURCE          -1286
#define SQLC_RC_NPIPE_INST_NOT_FOUND       -1287

#define SQLE_RC_DS_FAILED -1291              /* Directory Services failed     */
#define SQLE_RC_DS_BAD_GLB_NAME -1292        /* Bad global name               */
#define SQLE_RC_DS_BAD_GLB_DIR_ENTRY -1293   /* Bad global directory entry    */
#define SQLE_RC_DS_BAD_DIR_PATH_NAME -1294   /* Bad DIR_PATH_NAME             */
#define SQLE_RC_DS_BAD_ROUTE_NAME -1295      /* Bad ROUTE_OBJ_NAME            */
#define SQLE_RC_DS_UNSUPPORTED_CMD -1297     /* Command not supported         */

#define SQLE_RC_DCE_INVPN -1300              /* Invalid DCE principal name    */
#define SQLE_RC_DCE_ERR_KEYTAB -1301         /* Error DCE keytab file         */
#define SQLE_RC_DCE_ERR_MAPPING -1302        /* DCE principal and DB2 authid  */
                                             /* mapping error                 */
#define SQLE_RC_SECD_ERR_RESTART -1303       /* Security daemon could not be  */
                                             /* restarted                     */
#define SQLE_RC_INVSTCP -1304                /* Invalid security type for     */
                                             /* TCP/IP protocol               */
#define SQLE_RC_DCE_ERR -1305                /* DCE internal error            */
#define SQLE_RC_AUD_INV_PARM -1306           /* Invalid parameter to audit    */
#define SQLE_RC_AUD_ERR -1307                /* Audit error                   */
#define SQLE_RC_DCE_INV_PRINC -1309          /* DCE invalid server principal  */
                                             /* name                          */

#define SQLE_RC_FILEDCS -1310                /* DCS Directory file access     */
                                             /* error                         */
#define SQLE_RC_DCSDIR_NF -1311              /* DCS Directory not found       */
#define SQLE_RC_NO_ENTRY 1312                /* DCS Directory is empty        */
#define SQLE_RC_MAX_ENTRY -1313              /* DCS Directory is full         */
#define SQLE_RC_INVENTRY_PTR -1314           /* Entry parameter pointer       */
                                             /* invalid                       */
#define SQLE_RC_INVLDB -1315                 /* Local DB name has invalid     */
                                             /* chars                         */
#define SQLE_RC_LDB_NF -1316                 /* DCS Directory entry not       */
                                             /* found                         */
#define SQLE_RC_DUPLDB -1317                 /* DCS Directory duplicate       */
                                             /* entry                         */
#define SQLE_RC_INVLENGTH -1318              /* Invalid element length        */
#define SQLE_RC_ENTRYNOT_COL -1319           /* Entries have not been         */
                                             /* collected                     */

#define SQLE_RC_GDBUSY -1320                 /* Cannot access DCS Dir at      */
                                             /* this time                     */
#define SQLE_RC_INVSTRUCT_ID -1321           /* Invalid structure ID          */
#define SQLE_RC_AUD_WRITE_ERR -1322          /* Error writing to audit log    */
#define SQLE_RC_AUD_CFG_FILE_ERR -1323       /* Error accessing db2audit.cfg  */
#define SQLE_RC_DRDANSP -1325                /* Remote function not           */
                                             /* supported                     */
#define SQLE_RC_ACCD   -1326                 /* File or directory access      */
                                             /* denied                        */
#define SQLE_RC_IMPLCONN_INVDB -1327         /* Implicit connect - invalid    */
                                             /* dbname                        */
#define SQLE_RC_IMPLCONN_NODB -1328          /* Implicit connect - alias not  */
                                             /* found                         */
#define SQLE_RC_PATH_TOO_LONG -1329          /* Input path too long           */

#define SQLE_RC_INVSDNAME -1330              /* Invalid symbolic destination  */
                                             /* name                          */
#define SQLE_RC_INVSTYPE -1331               /* Invalid CPIC security type    */
#define SQLE_RC_INV_HOSTNAME -1332           /* Invalid Host Name             */
#define SQLE_RC_INV_SERNAME -1333            /* Invalid Service Name          */

#define SQLE_RC_DOUBLE_REMOTE -1334          /* Double-hops not allowed       */
#define SQLE_RC_INVAR  -1335                 /* AR name has invalid chars     */

#define SQLE_RC_UNKNOWN_FILESERVER -1340     /* File server is unknown        */
#define SQLE_RC_INV_FSERVER -1342            /* Invalid File Server           */
#define SQLE_RC_INV_OBJNAME -1343            /* Invalid Object Name           */

#define SQLE_RC_BR_ACTIVE -1350              /* Backup or Restore is active   */

#define SQLE_RC_ALREADY_QUIESCED 1371        /* Quiesce already active        */
#define SQLE_RC_UNQUIESCE_FAILED 1373        /* Unquiesce failed              */
#define SQLE_RC_INV_INSTANCE -1390           /* Invalid Instance Name         */
#define SQLE_RC_INSTANCE_USING -1391         /* Another Instance is using     */
                                             /* the DB                        */
#define SQLE_RC_INV_DB2PATH -1393            /* Invalid DB2 Path Name         */

#define SQLE_RC_BAD_AUTH -1400               /* Unsupported authentication    */
                                             /* type                          */
#define SQLE_RC_DIFF_AUTH -1401              /* Authentication types do not   */
                                             /* match                         */
#define SQLE_RC_AUTH_ERR -1402               /* Authentication failed due to  */
                                             /* unexpected error              */
#define SQLE_RC_AUTH_FAILURE -1403           /* Invalid user name and/or      */
                                             /* password                      */
#define SQLE_RC_PASSWORD_EXPIRED -1404       /* Password has expired          */
#define SQLE_RC_PASSWORD_WITHOUT_USERID    -1425   /* Password without        */
                                                   /* userid                  */
#define SQLE_RC_DB2INSTDFT_ERROR -1426       /* Error getting DB2INSTDFT      */
#define SQLE_RC_NOT_INSTANCE_ATTACHED -1427  /* No current attachment         */
#define SQLE_RC_WRONG_ATTACH -1428           /* Attached to wrong instance    */
#define SQLE_RC_RELPATH_NOT_ALLOWED -1431    /* Relative path not allowed     */
#define SQLE_RC_WRONG_CONNECT -1433          /* Connected to wrong database   */

#define SQLE_RC_CTX_INV_PARM -1441           /* Ctx parm invalid              */
#define SQLE_RC_CTX_NOTINUSE -1442           /* App Ctx not in use            */
#define SQLE_RC_CTX_USING -1443              /* Already using Ctx             */
#define SQLE_RC_CTX_INUSE -1444              /* App Ctx in use                */
#define SQLE_RC_CTX_NO_CTX -1445             /* Thread does not have context  */

#define SQLE_RC_INVREGINFO_PTR -1450         /* Invalid registration info.    */
                                             /* ptr.                          */
#define SQLE_RC_REG_INVNODE -1451            /* Reg. issued from invalid      */
                                             /* node                          */
#define SQLE_RC_INVREGLOC -1452              /* Invalid registration          */
                                             /* location                      */
#define SQLE_RC_INVCFG_FSNAME -1453          /* Invalid file server in DBM    */
                                             /* cfg.                          */
#define SQLE_RC_INVCFG_OBJNAME -1454         /* Invalid object name in DBM    */
                                             /* cfg.                          */
#define SQLE_RC_INVCFG_IPXSOCKET -1455       /* Invalid IPX socket in DBM     */
                                             /* cfg.                          */
#define SQLE_RC_DUPLICATE_OBJNAME -1456      /* Object name already exists    */
#define SQLE_RC_NWDS_CONNEXISTS -1457        /* NWDS connection exists,       */
                                             /* cannot log into NW            */
                                             /* fileserver                    */
#define SQLE_RC_REG_NOT_NEEDED -1458         /* DB2 server reg./dereg. not    */
                                             /* needed                        */
#define SQLE_RC_INVSTYPE_TCP -1461           /* Invalid TCP/IP Security       */

#define SQLE_RC_ONE_BUFFERPOOL 1478          /* Database is started but only  */
                                             /* one bufferpool is started     */

#define SQLE_RC_SECD_FAILURE -1525           /* An error occured when         */
                                             /* starting the DB2 security     */
                                             /* daemon                        */

#define SQLE_RC_VI_ERROR -1526               /* FCM startup error when using  */
                                             /* VI                            */

#define SQLE_RC_UNSUPP_FUNCTION -1650        /* Function is no longer         */
                                             /* supported                     */

#define SQLE_RC_INV_SERVERLVL -1651          /* Invalid server level for      */
                                             /* request                       */
#define SQLE_RC_FILEIO_ERR -1652             /* File I/O error                */
#define SQLE_RC_INV_PROFILE_PATH -1653       /* Invalid profile path          */
#define SQLE_RC_INSTPATH_ERR -1654           /* Instance path error           */

#define SQLE_RC_GENERATOR_FAILED -1660       /* Generator failed              */

#define SQLE_RC_DSCVR_DISABLED -1670         /* Discover is disabled in DBM   */
                                             /* CFG                           */
#define SQLE_RC_SEARCH_DSCVR_FAILED -1671    /* Search discovery failed       */
#define SQLE_RC_INV_DSCVR_ADDRLST -1673      /* Invalid discover address      */
                                             /* list                          */
#define SQLE_RC_INV_DSCVR_ADDR -1674         /* Invalid discover address      */
#define SQLE_RC_INV_ADMINSERVER -1675        /* Invalid admin. server         */

#define SQLE_RC_INV_SCHEMA -1700             /* Invalid Schema name found     */
#define SQLE_RC_DB_NOT_MIGR -1701            /* DB cannot be migrated         */
#define SQLE_RC_CRT_EVENT_FAIL -1703         /* Fail to create db2event dir   */
#define SQLE_RC_DB_MIG_FAIL -1704            /* DB migration failed           */
#define SQLE_RC_UPDATE_DIR_FAIL 1705         /* Fail to update directory      */
                                             /* entry                         */

#define SQLE_RC_INV_REQINFO_PTR -1800        /* Invalid Request Info pointer  */
#define SQLE_RC_INV_REQUEST_TYPE -1801       /* Invalid Request Type          */
#define SQLE_RC_NO_NODE_ENTRY -1802          /* No entry belongs to Request   */
                                             /* Type                          */
#define SQLE_RC_NODE_EXIST -1803             /* Node already exists in node   */
                                             /* directory                     */

#define SQLE_RC_DB_ACTIVATED 1490            /* DB is already activated       */
#define SQLE_RC_DB_NOT_STOPPED -1491         /* DB is still active            */
#define SQLE_RC_DB_NOT_UP -1492              /* DB is not active              */
#define SQLE_RC_APP_IS_CONNECTED -1493       /* Application is connected to   */
                                             /* a database                    */
#define SQLE_RC_ACT_DB_CONNECTED 1494        /* There is already DB           */
                                             /* connection                    */
#define SQLE_RC_DEACT_DB_CONNECTED 1495      /* There is still DB connection  */
#define SQLE_RC_DEACT_DB_NOT_ACTIVATED 1496  /* DB is not activated           */
#define SQLE_RC_ACTDEACT_ERROR 1497          /* Error occurs on some nodes    */
#define SQLE_RC_DEACT_DB_IOSUSPENDED 1649    /* Db is in IO Suspend mode.     */
#define SQLE_RC_INVALID_PARM -2032           /* Invalid parameter             */

#define SQLE_RC_QUIESCE_PENDING -3807        /* Quiesce is pending            */
#define SQLE_RC_UNQUIESCE_PENDING -3808      /* Unquiesce is pending          */
#define SQLE_RC_E4411  -4411                 /* Error not Admin Server        */
#define SQLE_RC_NOADMSTART -4414             /* Admin Server not started      */

#define SQLE_RC_COMM_FAILED 5043             /* Communications support        */
                                             /* failed                        */

#define SQLE_RC_HCA_DOWN 1659                /* One or more CF ports failed   */

#define SQLE_RC_NO_VENDOR_CFG -5500          /* Vendor cfg file not found     */
#define SQLE_RC_BAD_VENDOR_CFG -5501         /* Vendor cfg file invalid       */

#define SQLE_RC_SDIRERR -6022                /* System database directory is  */
                                             /* not shared by all PDB nodes   */
#define SQLE_RC_PATH_NOT_QUAL -6027          /* The path specified for the    */
                                             /* database directory is not     */
                                             /* valid                         */
#define SQLE_RC_LOCALDB_NOT_FOUND -6028      /* Database is not found in      */
                                             /* local database directory      */
#define SQLE_RC_INV_PARM -6030               /* Invalid parameter for start   */
                                             /* stop                          */
#define SQLE_RC_ERR_DB2NODES_CFG -6031       /* Error in db2nodes.cfg file    */
#define SQLE_RC_STARTED_PARTIALLY 6032       /* Some nodes have not been      */
                                             /* started properly              */
#define SQLE_RC_STOPPED_PARTIALLY 6033       /* Some nodes have not been      */
                                             /* stopped properly              */
#define SQLE_RC_NODE_NOT_INUSE 6034          /* The node is not in used by    */
                                             /* any database                  */
#define SQLE_RC_NODE_INUSE 6035              /* The node is used by a         */
                                             /* database                      */
#define SQLE_RC_START_STOP_IN_PROG -6036     /* Start/stop command in         */
                                             /* progress                      */
#define SQLE_RC_NODE_TIMEOUT -6037           /* Timeout reached for start     */
                                             /* stop                          */
#define SQLE_RC_INVDROP_ACTION -6046         /* Invalid action for DROP NODE  */
#define SQLE_RC_COMM_ERR -6048               /* Communication error in start  */
                                             /* stop                          */
#define SQLE_RC_NODE_NEED_STOP -6072         /* Need to stop the node before  */
                                             /* starting the node with the    */
                                             /* restart option                */
#define SQLE_RC_ADD_NODE_FAIL -6073          /* Add Node failed               */
#define SQLE_RC_ADD_NODE_CRDP -6074          /* Add Node is not allowed       */
                                             /* because a conflicting         */
                                             /* command is  in progress       */
#define SQLE_RC_ADD_NODE_OK 6075             /* Add Node operation            */
                                             /* successful                    */
#define SQLE_RC_ONLINEADDNODE_OPERERR -1483  /* Online add node failed        */
#define SQLE_RC_ADDNODE_RESTRICTED -1484     /* A conflicting command is not  */
                                             /* allowed while add node is     */
                                             /* running                       */
#define SQLE_RC_ONLINEADDNODE_STATERR -1485  /* State of the instance does    */
                                             /* not allow online add node to  */
                                             /* proceed                       */
#define SQLE_RC_ADDNODE_DONEOFFLINE 1487     /* A node is added offline to a  */
                                             /* single node instance          */
#define SQLE_RC_ADDNODE_DONEINCLUSTER 1488   /* A node is added  to an        */
                                             /* instance managed by the DB2   */
                                             /* Cluster Manager               */
#define SQLE_RC_ONLINEADDNODE_DONE 1489      /* A node is added online        */
                                             /* successfully                  */
#define SQLE_RC_CM_INCONSISTENT -1517        /* Cluster resources are         */
                                             /* inconsistent                  */
#define SQLE_RC_OLDVIEWAPP_EXIST -1524       /* An application with oldview   */
                                             /* of the instance exists.       */
#define SQLE_RC_LAST_CF -1529                /* The last CA cat not be        */
                                             /* dropped.                      */
#define SQLE_RC_LAST_MEMBER -1541            /* The last member cat not be    */
                                             /* dropped.                      */
#define SQLE_RC_MAXNUM_CF -1542              /* The maximum number of CA's    */
                                             /* in the DB2 instance has been  */
                                             /* reached.                      */
#define SQLE_RC_HOST_HAS_CF -1543            /* The host specified for the    */
                                             /* CA is a duplicate             */
#define SQLE_RC_TOPOLOGY_BACKUP -1544        /* Backup of a database          */
                                             /* following a topology change   */
                                             /* cannot be performed from a    */
                                             /* newly added member.           */
#define SQLE_RC_TOPOLOGY_RESTORE -1545       /* Restore of a database image   */
                                             /* from a previous topology is   */
                                             /* not supported                 */
#define SQLE_RC_TOPOLOGY_ROLLFORWARD -1546   /* Rollforward through an add    */
                                             /* or drop member operation is   */
                                             /* not permitted                 */
#define SQLE_RC_ADDDROP_RECOVERY -1547       /* Recovery is needed for a      */
                                             /* previously failed add or      */
                                             /* drop operation.               */
#define SQLE_RC_RFWD_WRONG_MEMBER -1714      /* Rollforward cannot be         */
                                             /* initiated from a member that  */
                                             /* is not in the current         */
                                             /* database topology             */
#define SQLE_RC_TOP_NO_COMMON_MEMBER -1710   /* The operation cannot be       */
                                             /* performed because the source  */
                                             /* member topology and the       */
                                             /* target member topology have   */
                                             /* no common member              */
#define SQLE_RC_TOP_UPGRADE -1712            /* The command failed because    */
                                             /* the source and target member  */
                                             /* topologies are not at the     */
                                             /* same DB2 product level        */
#define SQLE_RC_CF_TRANSPORT_ERROR -1721     /* Error in starting up with     */
                                             /* the current CF_TRANSPORT      */
                                             /* METHOD database manager       */
                                             /* configuration parameter       */
#define SQLE_RC_10GE_CARD_RESTRICTION -1726  /* Ethernet card speed does not  */
                                             /* meet the 10GE minimum         */
#define SQLE_RC_DROP_PROMPT 6076             /* Prompt for db2stop DROP       */
                                             /* NODENUM                       */
#define SQLE_RC_DROP_OK 6077                 /* Drop node OK. Files still     */
                                             /* exist                         */
#define SQLE_RC_DROP_FAILED -6078            /* Drop node failed              */
#define SQLE_RC_DROP_CANCEL 6079             /* Drop node cancelled           */
                                             /* successfully                  */
#define SQLE_RC_ADD_NODE_NODB 6080           /* Add Node succeeded but no     */
                                             /* databases created on the      */
                                             /* node                          */
#define SQLE_RC_FORCE_TIMEOUT_ERR -6081      /* Timeout reached in stop       */
                                             /* FORCE                         */
#define SQLE_RC_CLUSTER_MGR_FAILED -1677     /* DB2 Cluster Services failed   */
                                             /* to update the states of       */
                                             /* resources                     */
#define SQLE_RC_CF_HOST_STOPPED 1678         /* The CF host was previously    */
                                             /* stopped by db2stop INSTANCE   */
                                             /* ON command                    */
#define SQLE_RC_CF_HOST_UNREACHED -1679      /* The CF host can't be reached  */
#define SQLE_RC_GUEST_STOP_FAILED -1687      /* The member is performing      */
                                             /* recovery or resolving in      */
                                             /* doubt transaction             */
#define SQLE_RC_MEMBER_HOST_UNREACHED 1680   /* The member host can't be      */
                                             /* reached, member will be       */
                                             /* started as guest member       */
#define SQLE_RC_MEMBER_HOST_STOPPED 1681     /* The member host was           */
                                             /* previously stopped, member    */
                                             /* will be started as guest      */
                                             /* member                        */
#define SQLE_RC_CF_NOT_STOPPABLE -1688       /* The CF contains dirty pages   */
                                             /* or holds locks                */
#define SQLE_RC_MEMBER_START_LIGHT 1682      /* The member is started as      */
                                             /* guest member                  */
#define SQLE_RC_HOME_HOST_NOT_STARTED 1689   /* The home host has not         */
                                             /* started yet when the member   */
                                             /* is stopped                    */
#define SQLE_RC_HOST_UNREACHED -1690         /* The host can't be reached     */
                                             /* during db2stop member or CF   */
#define SQLE_RC_MEMBER_OR_CF_RUNNING -1691   /* The active member or CF is    */
                                             /* running on the host           */
#define SQLE_RC_RESTART_LIGHT_FAILED -1683   /* Restart light animation       */
                                             /* failed                        */
#define SQLE_RC_IDLE_START_FAILED -1684      /* The idle process failed to    */
                                             /* start                         */
#define SQLE_RC_IDLE_STOP_FAILED -1692       /* The idle process failed to    */
                                             /* stop                          */
#define SQLE_RC_MEMBER_CF_NOT_STARTED -1685  /* The member failed to start    */
                                             /* because CF failed to start    */
#define SQLE_RC_HOST_START_FAILED -1686      /* The instance on the host      */
                                             /* failed to start               */
#define SQLE_RC_HOST_STOP_FAILED -1693       /* The isntance on the host      */
                                             /* failed to stop                */

#define SQLE_RC_DB2_SERVER_LICENSE -8000     /* No DB2 license                */
#define SQLE_RC_DB2_LICENSE -8001            /* Out of DB2 client licenses    */
#define SQLE_RC_DDCS_LICENSE -8002           /* Out of DDCS clients           */

#define SQLE_RC_NO_FED_CFG -5180             /* Federated cfg file not found  */
#define SQLE_RC_BAD_FED_CFG -5181            /* Federated cfg file invalid    */

#define SQLE_RC_BAD_PORT -1888               /* Invalid port number           */
#define SQLE_RC_IGNORE_UPD_ALT_SVR 1889      /* Update Alternate Server       */
                                             /* Ignored                       */
#define SQLE_RC_BAD_HOSTNAME -1890           /* Invalid host name             */
#define SQLE_RC_HOSTNAME_PTR -1891           /* Invalid host name pointer     */
#define SQLE_RC_PORT_PTR -1892               /* Invalid port number pointer   */

/* The following functions and symbols are obsolete and may not be            */
/* supportedin future releases. The obsolete functions are provided for       */
/* backward compatibilityand exported from DB2API.LIB. All applications       */
/* should be migrated to use new APIs.Note: Some of the function parameters   */
/* may be NO-OP.      Some structures are larger (eg. SQLEDINFO) in V2.       */
#define SQLE_RC_BAD_SEGPAGES -1136           /* Invalid numsegs on create db  */

#define SQLE_RC_CS_LICENSE -8003             /* No CS/6000 license            */
#define SQLE_RC_SNA_LICENSE -8004            /* No SNA/6000 license           */

#define SQL_AUTHENTICATION_UNDEF 255         /* Authentication Undefined      */

/******************************************************************************
** sqledbcr API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* CREATE DATABASE               */
  sqledbcr_api (
        _SQLOLDCHAR *,                       /* database                      */
        _SQLOLDCHAR *,                       /* drive/path                    */
        struct sqledbdesc *,                 /* db description block          */
        unsigned short,                      /* authentication                */
        struct sqledbcinfo *,                /* database territory info       */
        struct sqlca *);                     /* SQLCA                         */

/******************************************************************************
** sqlgdbcr API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* CREATE DATABASE               */
  sqlgdbcr (
        unsigned short,                      /* length drive/path             */
        unsigned short,                      /* length database               */
        struct sqlca *,                      /* SQLCA                         */
        struct sqledbcinfo *,                /* database territory info       */
        unsigned short,                      /* authentication                */
        struct sqledbdesc *,                 /* db descriptor block           */
        _SQLOLDCHAR *,                       /* drive/path                    */
        _SQLOLDCHAR *);                      /* database                      */

/******************************************************************************
** sqlestar API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Start Database Manager        */
  sqlestar_api (
        void);

/* PE V1.2 Start Database Manager structure                                   */
/******************************************************************************
** sqledbstrtopt data structure
*******************************************************************************/
SQL_STRUCTURE sqledbstrtopt
{
        sqluint32      isprofile;            /* Profile specified?            */
        char           profile[SQL_PROFILE_SZ+1]; /* Profile                  */
        sqluint32      isnodenum;            /* Node number specified?        */
        SQL_PDB_NODE_TYPE nodenum;           /* Node number                   */
        sqluint32      iscommand;            /* Command specified?            */
        char           scommand[SQL_OPTION_SZ+1]; /* Command                  */
        sqluint32      ishostname;           /* Hostname specified?           */
        char           hostname[SQL_HOSTNAME_SZ+1]; /* Hostname               */
        sqluint32      isport;               /* Port specified?               */
        SQL_PDB_PORT_TYPE port;              /* Port                          */
        sqluint32      isnetname;            /* Netname specified?            */
        char           netname[SQL_HOSTNAME_SZ+1]; /* Netname                 */
};

/******************************************************************************
** sqlepstr API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* PE V1.2 Start Database        */
                                             /* Manager                       */
  sqlepstr_api (
        struct sqledbstrtopt * pStartOptions, /* START OPTIONS                */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgstar API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Start Database Manager        */
  sqlgstar (
        void);

/******************************************************************************
** sqlgpstr API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* PE V1.2 Start Database        */
                                             /* Manager                       */
  sqlgpstr (
        struct sqledbstrtopt * pStartOptions, /* START OPTIONS                */
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlestop API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Stop Database Manager         */
  sqlestop_api (
        struct sqlca * pSqlca);              /* SQLCA                         */

/******************************************************************************
** sqlgstdm API
*******************************************************************************/
SQL_API_RC SQL_API_FN                        /* Stop Database Manager         */
  sqlgstdm (
        struct sqlca * pSqlca);              /* SQLCA                         */

/* End of obsolete functions and symbols                                      */

#ifdef __cplusplus 
}
#endif

#endif /* SQL_H_SQLENV */
