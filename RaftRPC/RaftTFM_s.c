

/* this ALWAYS GENERATED file contains the RPC server stubs */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 04:14:07 2038
 */
/* Compiler settings for RaftTFM.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0622 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#if !defined(_M_IA64) && !defined(_M_AMD64) && !defined(_ARM_)


#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off ) 

#include <string.h>
#include "RaftTFM_h.h"

#define TYPE_FORMAT_STRING_SIZE   13                                
#define PROC_FORMAT_STRING_SIZE   141                               
#define EXPR_FORMAT_STRING_SIZE   1                                 
#define TRANSMIT_AS_TABLE_SIZE    0            
#define WIRE_MARSHAL_TABLE_SIZE   0            

typedef struct _RaftTFM_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } RaftTFM_MIDL_TYPE_FORMAT_STRING;

typedef struct _RaftTFM_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } RaftTFM_MIDL_PROC_FORMAT_STRING;

typedef struct _RaftTFM_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } RaftTFM_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax = 
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};

extern const RaftTFM_MIDL_TYPE_FORMAT_STRING RaftTFM__MIDL_TypeFormatString;
extern const RaftTFM_MIDL_PROC_FORMAT_STRING RaftTFM__MIDL_ProcFormatString;
extern const RaftTFM_MIDL_EXPR_FORMAT_STRING RaftTFM__MIDL_ExprFormatString;

/* Standard interface: RaftTFM, ver. 1.0,
   GUID={0x5991308D,0x87CE,0x46D1,{0x85,0x85,0x76,0xAF,0x9D,0xEC,0x13,0xE6}} */


extern const MIDL_SERVER_INFO RaftTFM_ServerInfo;

extern const RPC_DISPATCH_TABLE RaftTFM_v1_0_DispatchTable;

static const RPC_SERVER_INTERFACE RaftTFM___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0x5991308D,0x87CE,0x46D1,{0x85,0x85,0x76,0xAF,0x9D,0xEC,0x13,0xE6}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    (RPC_DISPATCH_TABLE*)&RaftTFM_v1_0_DispatchTable,
    0,
    0,
    0,
    &RaftTFM_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE RaftTFM_v1_0_s_ifspec = (RPC_IF_HANDLE)& RaftTFM___RpcServerInterface;

extern const MIDL_STUB_DESC RaftTFM_StubDesc;


#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif
#if !(TARGET_IS_NT60_OR_LATER)
#error You need Windows Vista or later to run this stub because it uses these features:
#error   compiled for Windows Vista.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const RaftTFM_MIDL_PROC_FORMAT_STRING RaftTFM__MIDL_ProcFormatString =
    {
        0,
        {

	/* Procedure append_entry_rpc */

			0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/*  2 */	NdrFcLong( 0x0 ),	/* 0 */
/*  6 */	NdrFcShort( 0x0 ),	/* 0 */
/*  8 */	NdrFcShort( 0x24 ),	/* x86 Stack size/offset = 36 */
/* 10 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 12 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 14 */	NdrFcShort( 0xfd8 ),	/* 4056 */
/* 16 */	NdrFcShort( 0x38 ),	/* 56 */
/* 18 */	0x40,		/* Oi2 Flags:  has ext, */
			0x8,		/* 8 */
/* 20 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 22 */	NdrFcShort( 0x0 ),	/* 0 */
/* 24 */	NdrFcShort( 0x0 ),	/* 0 */
/* 26 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter argument_term_ */

/* 28 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 30 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 32 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter argument_leader_id_ */

/* 34 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 36 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 38 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter argument_prev_log_index_ */

/* 40 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 42 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 44 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter argument_prev_log_term_ */

/* 46 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 48 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 50 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter argument_entries_ */

/* 52 */	NdrFcShort( 0xa ),	/* Flags:  must free, in, */
/* 54 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 56 */	NdrFcShort( 0x2 ),	/* Type Offset=2 */

	/* Parameter argument_leader_commit_ */

/* 58 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 60 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 62 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter result_term_ */

/* 64 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 66 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 68 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter result_success_ */

/* 70 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 72 */	NdrFcShort( 0x20 ),	/* x86 Stack size/offset = 32 */
/* 74 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Procedure request_vote_rpc */

/* 76 */	0x0,		/* 0 */
			0x48,		/* Old Flags:  */
/* 78 */	NdrFcLong( 0x0 ),	/* 0 */
/* 82 */	NdrFcShort( 0x1 ),	/* 1 */
/* 84 */	NdrFcShort( 0x1c ),	/* x86 Stack size/offset = 28 */
/* 86 */	0x32,		/* FC_BIND_PRIMITIVE */
			0x0,		/* 0 */
/* 88 */	NdrFcShort( 0x0 ),	/* x86 Stack size/offset = 0 */
/* 90 */	NdrFcShort( 0x20 ),	/* 32 */
/* 92 */	NdrFcShort( 0x38 ),	/* 56 */
/* 94 */	0x40,		/* Oi2 Flags:  has ext, */
			0x6,		/* 6 */
/* 96 */	0x8,		/* 8 */
			0x1,		/* Ext Flags:  new corr desc, */
/* 98 */	NdrFcShort( 0x0 ),	/* 0 */
/* 100 */	NdrFcShort( 0x0 ),	/* 0 */
/* 102 */	NdrFcShort( 0x0 ),	/* 0 */

	/* Parameter argument_term_ */

/* 104 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 106 */	NdrFcShort( 0x4 ),	/* x86 Stack size/offset = 4 */
/* 108 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter argument_candidate_id_ */

/* 110 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 112 */	NdrFcShort( 0x8 ),	/* x86 Stack size/offset = 8 */
/* 114 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter argument_last_log_index_ */

/* 116 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 118 */	NdrFcShort( 0xc ),	/* x86 Stack size/offset = 12 */
/* 120 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter argument_last_log_term_ */

/* 122 */	NdrFcShort( 0x48 ),	/* Flags:  in, base type, */
/* 124 */	NdrFcShort( 0x10 ),	/* x86 Stack size/offset = 16 */
/* 126 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter result_term_ */

/* 128 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 130 */	NdrFcShort( 0x14 ),	/* x86 Stack size/offset = 20 */
/* 132 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

	/* Parameter result_vote_granted_ */

/* 134 */	NdrFcShort( 0x2150 ),	/* Flags:  out, base type, simple ref, srv alloc size=8 */
/* 136 */	NdrFcShort( 0x18 ),	/* x86 Stack size/offset = 24 */
/* 138 */	0x8,		/* FC_LONG */
			0x0,		/* 0 */

			0x0
        }
    };

static const RaftTFM_MIDL_TYPE_FORMAT_STRING RaftTFM__MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */
/*  2 */	
			0x1d,		/* FC_SMFARRAY */
			0x3,		/* 3 */
/*  4 */	NdrFcShort( 0xfa0 ),	/* 4000 */
/*  6 */	0x8,		/* FC_LONG */
			0x5b,		/* FC_END */
/*  8 */	
			0x11, 0xc,	/* FC_RP [alloced_on_stack] [simple_pointer] */
/* 10 */	0x8,		/* FC_LONG */
			0x5c,		/* FC_PAD */

			0x0
        }
    };

static const unsigned short RaftTFM_FormatStringOffsetTable[] =
    {
    0,
    76
    };


static const MIDL_STUB_DESC RaftTFM_StubDesc = 
    {
    (void *)& RaftTFM___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    0,
    0,
    RaftTFM__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x60001, /* Ndr library version */
    0,
    0x801026e, /* MIDL Version 8.1.622 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

static const RPC_DISPATCH_FUNCTION RaftTFM_table[] =
    {
    NdrServerCall2,
    NdrServerCall2,
    0
    };
static const RPC_DISPATCH_TABLE RaftTFM_v1_0_DispatchTable = 
    {
    2,
    (RPC_DISPATCH_FUNCTION*)RaftTFM_table
    };

static const SERVER_ROUTINE RaftTFM_ServerRoutineTable[] = 
    {
    (SERVER_ROUTINE)append_entry_rpc,
    (SERVER_ROUTINE)request_vote_rpc
    };

static const MIDL_SERVER_INFO RaftTFM_ServerInfo = 
    {
    &RaftTFM_StubDesc,
    RaftTFM_ServerRoutineTable,
    RaftTFM__MIDL_ProcFormatString.Format,
    RaftTFM_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#pragma optimize("", on )
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* !defined(_M_IA64) && !defined(_M_AMD64) && !defined(_ARM_) */

