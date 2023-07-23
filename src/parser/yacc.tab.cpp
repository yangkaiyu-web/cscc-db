/* A Bison parser, made by GNU Bison 3.7.6.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30706

/* Bison version string.  */
#define YYBISON_VERSION "3.7.6"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "/home/one/db2023/src/parser/yacc.y"

#include "ast.h"
#include "yacc.tab.h"
#include <iostream>
#include <memory>
#include "errors.h"

int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);

void yyerror(YYLTYPE *locp, const char* s) {
    std::cerr << "Parser Error at line " << locp->first_line << " column " << locp->first_column << ": " << s << std::endl;
}

using namespace ast;

#line 87 "/home/one/db2023/src/parser/yacc.tab.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "yacc.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SHOW = 3,                       /* SHOW  */
  YYSYMBOL_TABLES = 4,                     /* TABLES  */
  YYSYMBOL_CREATE = 5,                     /* CREATE  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_DROP = 7,                       /* DROP  */
  YYSYMBOL_DESC = 8,                       /* DESC  */
  YYSYMBOL_INSERT = 9,                     /* INSERT  */
  YYSYMBOL_INTO = 10,                      /* INTO  */
  YYSYMBOL_VALUES = 11,                    /* VALUES  */
  YYSYMBOL_DELETE = 12,                    /* DELETE  */
  YYSYMBOL_FROM = 13,                      /* FROM  */
  YYSYMBOL_ASC = 14,                       /* ASC  */
  YYSYMBOL_ORDER = 15,                     /* ORDER  */
  YYSYMBOL_BY = 16,                        /* BY  */
  YYSYMBOL_WHERE = 17,                     /* WHERE  */
  YYSYMBOL_UPDATE = 18,                    /* UPDATE  */
  YYSYMBOL_SET = 19,                       /* SET  */
  YYSYMBOL_SELECT = 20,                    /* SELECT  */
  YYSYMBOL_INT = 21,                       /* INT  */
  YYSYMBOL_CHAR = 22,                      /* CHAR  */
  YYSYMBOL_FLOAT = 23,                     /* FLOAT  */
  YYSYMBOL_DATETIME = 24,                  /* DATETIME  */
  YYSYMBOL_INDEX = 25,                     /* INDEX  */
  YYSYMBOL_AND = 26,                       /* AND  */
  YYSYMBOL_JOIN = 27,                      /* JOIN  */
  YYSYMBOL_EXIT = 28,                      /* EXIT  */
  YYSYMBOL_HELP = 29,                      /* HELP  */
  YYSYMBOL_TXN_BEGIN = 30,                 /* TXN_BEGIN  */
  YYSYMBOL_TXN_COMMIT = 31,                /* TXN_COMMIT  */
  YYSYMBOL_TXN_ABORT = 32,                 /* TXN_ABORT  */
  YYSYMBOL_TXN_ROLLBACK = 33,              /* TXN_ROLLBACK  */
  YYSYMBOL_ORDER_BY = 34,                  /* ORDER_BY  */
  YYSYMBOL_BIGINT = 35,                    /* BIGINT  */
  YYSYMBOL_LEQ = 36,                       /* LEQ  */
  YYSYMBOL_NEQ = 37,                       /* NEQ  */
  YYSYMBOL_GEQ = 38,                       /* GEQ  */
  YYSYMBOL_T_EOF = 39,                     /* T_EOF  */
  YYSYMBOL_IDENTIFIER = 40,                /* IDENTIFIER  */
<<<<<<< HEAD
=======
<<<<<<< HEAD
  YYSYMBOL_VALUE_DATETIME = 41,            /* VALUE_DATETIME  */
  YYSYMBOL_VALUE_STRING = 42,              /* VALUE_STRING  */
  YYSYMBOL_VALUE_INT_BIGINT = 43,          /* VALUE_INT_BIGINT  */
  YYSYMBOL_VALUE_FLOAT = 44,               /* VALUE_FLOAT  */
  YYSYMBOL_45_ = 45,                       /* ';'  */
  YYSYMBOL_46_ = 46,                       /* '('  */
  YYSYMBOL_47_ = 47,                       /* ')'  */
  YYSYMBOL_48_ = 48,                       /* ','  */
  YYSYMBOL_49_ = 49,                       /* '.'  */
  YYSYMBOL_50_ = 50,                       /* '='  */
  YYSYMBOL_51_ = 51,                       /* '<'  */
  YYSYMBOL_52_ = 52,                       /* '>'  */
  YYSYMBOL_53_ = 53,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 54,                  /* $accept  */
  YYSYMBOL_start = 55,                     /* start  */
  YYSYMBOL_stmt = 56,                      /* stmt  */
  YYSYMBOL_txnStmt = 57,                   /* txnStmt  */
  YYSYMBOL_dbStmt = 58,                    /* dbStmt  */
  YYSYMBOL_ddl = 59,                       /* ddl  */
  YYSYMBOL_dml = 60,                       /* dml  */
  YYSYMBOL_fieldList = 61,                 /* fieldList  */
  YYSYMBOL_colNameList = 62,               /* colNameList  */
  YYSYMBOL_field = 63,                     /* field  */
  YYSYMBOL_type = 64,                      /* type  */
  YYSYMBOL_valueList = 65,                 /* valueList  */
  YYSYMBOL_value = 66,                     /* value  */
  YYSYMBOL_condition = 67,                 /* condition  */
  YYSYMBOL_optWhereClause = 68,            /* optWhereClause  */
  YYSYMBOL_whereClause = 69,               /* whereClause  */
  YYSYMBOL_col = 70,                       /* col  */
  YYSYMBOL_colList = 71,                   /* colList  */
  YYSYMBOL_op = 72,                        /* op  */
  YYSYMBOL_expr = 73,                      /* expr  */
  YYSYMBOL_setClauses = 74,                /* setClauses  */
  YYSYMBOL_setClause = 75,                 /* setClause  */
  YYSYMBOL_selector = 76,                  /* selector  */
  YYSYMBOL_tableList = 77,                 /* tableList  */
  YYSYMBOL_opt_order_clause = 78,          /* opt_order_clause  */
  YYSYMBOL_order_clause = 79,              /* order_clause  */
  YYSYMBOL_opt_asc_desc = 80,              /* opt_asc_desc  */
  YYSYMBOL_tbName = 81,                    /* tbName  */
  YYSYMBOL_colName = 82                    /* colName  */
=======
>>>>>>> origin/p4
  YYSYMBOL_VALUE_STRING = 41,              /* VALUE_STRING  */
  YYSYMBOL_VALUE_INT_BIGINT = 42,          /* VALUE_INT_BIGINT  */
  YYSYMBOL_VALUE_FLOAT = 43,               /* VALUE_FLOAT  */
  YYSYMBOL_44_ = 44,                       /* ';'  */
  YYSYMBOL_45_ = 45,                       /* '('  */
  YYSYMBOL_46_ = 46,                       /* ')'  */
  YYSYMBOL_47_ = 47,                       /* ','  */
  YYSYMBOL_48_ = 48,                       /* '.'  */
  YYSYMBOL_49_ = 49,                       /* '='  */
  YYSYMBOL_50_ = 50,                       /* '<'  */
  YYSYMBOL_51_ = 51,                       /* '>'  */
  YYSYMBOL_52_ = 52,                       /* '*'  */
  YYSYMBOL_YYACCEPT = 53,                  /* $accept  */
  YYSYMBOL_start = 54,                     /* start  */
  YYSYMBOL_stmt = 55,                      /* stmt  */
  YYSYMBOL_txnStmt = 56,                   /* txnStmt  */
  YYSYMBOL_dbStmt = 57,                    /* dbStmt  */
  YYSYMBOL_ddl = 58,                       /* ddl  */
  YYSYMBOL_dml = 59,                       /* dml  */
  YYSYMBOL_fieldList = 60,                 /* fieldList  */
  YYSYMBOL_colNameList = 61,               /* colNameList  */
  YYSYMBOL_field = 62,                     /* field  */
  YYSYMBOL_type = 63,                      /* type  */
  YYSYMBOL_valueList = 64,                 /* valueList  */
  YYSYMBOL_value = 65,                     /* value  */
  YYSYMBOL_condition = 66,                 /* condition  */
  YYSYMBOL_optWhereClause = 67,            /* optWhereClause  */
  YYSYMBOL_whereClause = 68,               /* whereClause  */
  YYSYMBOL_col = 69,                       /* col  */
  YYSYMBOL_colList = 70,                   /* colList  */
  YYSYMBOL_op = 71,                        /* op  */
  YYSYMBOL_expr = 72,                      /* expr  */
  YYSYMBOL_setClauses = 73,                /* setClauses  */
  YYSYMBOL_setClause = 74,                 /* setClause  */
  YYSYMBOL_selector = 75,                  /* selector  */
  YYSYMBOL_tableList = 76,                 /* tableList  */
  YYSYMBOL_opt_order_clause = 77,          /* opt_order_clause  */
  YYSYMBOL_order_clause = 78,              /* order_clause  */
  YYSYMBOL_opt_asc_desc = 79,              /* opt_asc_desc  */
  YYSYMBOL_tbName = 80,                    /* tbName  */
  YYSYMBOL_colName = 81                    /* colName  */
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  40
/* YYLAST -- Last index in YYTABLE.  */
<<<<<<< HEAD
#define YYLAST   115

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  72
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  132

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   298
=======
<<<<<<< HEAD
#define YYLAST   116
=======
#define YYLAST   115
>>>>>>> f4d1731 (tmp)

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  73
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  133

/* YYMAXUTOK -- Last valid token kind.  */
<<<<<<< HEAD
#define YYMAXUTOK   299
=======
#define YYMAXUTOK   298
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
<<<<<<< HEAD
      45,    46,    52,     2,    47,     2,    48,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
      50,    49,    51,     2,     2,     2,     2,     2,     2,     2,
=======
<<<<<<< HEAD
      46,    47,    53,     2,    48,     2,    49,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    45,
      51,    50,    52,     2,     2,     2,     2,     2,     2,     2,
=======
      45,    46,    52,     2,    47,     2,    48,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
      50,    49,    51,     2,     2,     2,     2,     2,     2,     2,
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
<<<<<<< HEAD
      35,    36,    37,    38,    39,    40,    41,    42,    43
=======
<<<<<<< HEAD
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44
=======
      35,    36,    37,    38,    39,    40,    41,    42,    43
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    57,    57,    62,    67,    72,    80,    81,    82,    83,
      87,    91,    95,    99,   106,   110,   117,   121,   125,   129,
     133,   140,   144,   148,   152,   159,   163,   170,   174,   181,
     188,   192,   197,   201,   205,   212,   216,   223,   227,   231,
<<<<<<< HEAD
=======
<<<<<<< HEAD
     235,   242,   249,   250,   257,   261,   268,   272,   279,   283,
     290,   294,   298,   302,   306,   310,   317,   321,   328,   332,
     339,   346,   350,   354,   358,   362,   369,   373,   377,   384,
     385,   386,   389,   391
=======
>>>>>>> origin/p4
     238,   245,   246,   253,   257,   264,   268,   275,   279,   286,
     290,   294,   298,   302,   306,   313,   317,   324,   328,   335,
     342,   346,   350,   354,   358,   365,   369,   373,   380,   381,
     382,   385,   387
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SHOW", "TABLES",
  "CREATE", "TABLE", "DROP", "DESC", "INSERT", "INTO", "VALUES", "DELETE",
  "FROM", "ASC", "ORDER", "BY", "WHERE", "UPDATE", "SET", "SELECT", "INT",
  "CHAR", "FLOAT", "DATETIME", "INDEX", "AND", "JOIN", "EXIT", "HELP",
  "TXN_BEGIN", "TXN_COMMIT", "TXN_ABORT", "TXN_ROLLBACK", "ORDER_BY",
<<<<<<< HEAD
=======
<<<<<<< HEAD
  "BIGINT", "LEQ", "NEQ", "GEQ", "T_EOF", "IDENTIFIER", "VALUE_DATETIME",
  "VALUE_STRING", "VALUE_INT_BIGINT", "VALUE_FLOAT", "';'", "'('", "')'",
  "','", "'.'", "'='", "'<'", "'>'", "'*'", "$accept", "start", "stmt",
  "txnStmt", "dbStmt", "ddl", "dml", "fieldList", "colNameList", "field",
  "type", "valueList", "value", "condition", "optWhereClause",
  "whereClause", "col", "colList", "op", "expr", "setClauses", "setClause",
  "selector", "tableList", "opt_order_clause", "order_clause",
  "opt_asc_desc", "tbName", "colName", YY_NULLPTR
=======
>>>>>>> origin/p4
  "BIGINT", "LEQ", "NEQ", "GEQ", "T_EOF", "IDENTIFIER", "VALUE_STRING",
  "VALUE_INT_BIGINT", "VALUE_FLOAT", "';'", "'('", "')'", "','", "'.'",
  "'='", "'<'", "'>'", "'*'", "$accept", "start", "stmt", "txnStmt",
  "dbStmt", "ddl", "dml", "fieldList", "colNameList", "field", "type",
  "valueList", "value", "condition", "optWhereClause", "whereClause",
  "col", "colList", "op", "expr", "setClauses", "setClause", "selector",
  "tableList", "opt_order_clause", "order_clause", "opt_asc_desc",
  "tbName", "colName", YY_NULLPTR
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
<<<<<<< HEAD
=======
<<<<<<< HEAD
     295,   296,   297,   298,   299,    59,    40,    41,    44,    46,
      61,    60,    62,    42
=======
>>>>>>> origin/p4
     295,   296,   297,   298,    59,    40,    41,    44,    46,    61,
      60,    62,    42
>>>>>>> f4d1731 (tmp)
};
#endif

#define YYPACT_NINF (-75)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

<<<<<<< HEAD
#define YYTABLE_NINF (-72)
=======
<<<<<<< HEAD
#define YYTABLE_NINF (-73)
=======
#define YYTABLE_NINF (-72)
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
<<<<<<< HEAD
=======
<<<<<<< HEAD
      44,     7,     8,    10,   -37,     5,    24,   -37,   -27,   -75,
     -75,   -75,   -75,   -75,   -75,   -75,    50,    -1,   -75,   -75,
     -75,   -75,   -75,    41,   -37,   -37,   -37,   -37,   -75,   -75,
     -37,   -37,    42,     6,   -75,   -75,    15,    52,    35,   -75,
     -75,   -75,   -37,    51,    53,   -75,    55,    85,    81,    62,
      63,   -37,    62,   -75,    62,    62,    62,    58,    63,   -75,
     -75,    -5,   -75,    56,   -75,    -7,   -75,   -75,   -30,   -75,
      36,   -18,   -75,    22,    49,   -75,    79,    30,    62,   -75,
      49,   -37,   -37,    92,   -75,    62,   -75,    64,   -75,   -75,
     -75,   -75,   -75,    62,   -75,   -75,   -75,   -75,   -75,    47,
     -75,    63,   -75,   -75,   -75,   -75,   -75,   -75,    45,   -75,
     -75,   -75,   -75,    93,   -75,   -75,    65,   -75,   -75,    49,
     -75,   -75,   -75,   -75,    63,    66,   -75,    11,   -75,   -75,
     -75,   -75,   -75
=======
>>>>>>> origin/p4
      42,     7,     8,     9,   -23,    10,    13,   -23,   -27,   -78,
     -78,   -78,   -78,   -78,   -78,   -78,    19,   -15,   -78,   -78,
     -78,   -78,   -78,    22,   -23,   -23,   -23,   -23,   -78,   -78,
     -23,   -23,    18,    -5,   -78,   -78,    14,    31,    20,   -78,
     -78,   -78,   -23,    35,    48,   -78,    49,    77,    81,    60,
      61,   -23,    60,   -78,    60,    60,    60,    57,    61,   -78,
     -78,    -7,   -78,    54,   -78,   -11,   -78,   -78,     6,   -78,
      34,    30,   -78,    43,    44,   -78,    78,    46,    60,   -78,
      44,   -23,   -23,    90,   -78,    60,   -78,    62,   -78,   -78,
     -78,   -78,   -78,    60,   -78,   -78,   -78,   -78,    45,   -78,
      61,   -78,   -78,   -78,   -78,   -78,   -78,    24,   -78,   -78,
     -78,   -78,    92,   -78,   -78,    64,   -78,   -78,    44,   -78,
     -78,   -78,   -78,    61,    63,   -78,     4,   -78,   -78,   -78,
     -78,   -78
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     4,
       3,    10,    11,    12,    13,     5,     0,     0,     9,     6,
<<<<<<< HEAD
=======
<<<<<<< HEAD
       7,     8,    14,     0,     0,     0,     0,     0,    72,    18,
       0,     0,     0,    73,    61,    48,    62,     0,     0,    47,
       1,     2,     0,     0,     0,    17,     0,     0,    42,     0,
       0,     0,     0,    15,     0,     0,     0,     0,     0,    22,
      73,    42,    58,     0,    49,    42,    63,    46,     0,    25,
       0,     0,    27,     0,     0,    44,    43,     0,     0,    23,
       0,     0,     0,    67,    16,     0,    30,     0,    32,    34,
      33,    29,    19,     0,    20,    39,    40,    37,    38,     0,
      35,     0,    54,    53,    55,    50,    51,    52,     0,    59,
      60,    65,    64,     0,    24,    26,     0,    28,    21,     0,
      45,    56,    57,    41,     0,     0,    36,    71,    66,    31,
      70,    69,    68
=======
>>>>>>> origin/p4
       7,     8,    14,     0,     0,     0,     0,     0,    71,    18,
       0,     0,     0,    72,    60,    47,    61,     0,     0,    46,
       1,     2,     0,     0,     0,    17,     0,     0,    41,     0,
       0,     0,     0,    15,     0,     0,     0,     0,     0,    22,
      72,    41,    57,     0,    48,    41,    62,    45,     0,    25,
       0,     0,    27,     0,     0,    43,    42,     0,     0,    23,
       0,     0,     0,    66,    16,     0,    30,     0,    32,    34,
      33,    29,    19,     0,    20,    39,    37,    38,     0,    35,
       0,    53,    52,    54,    49,    50,    51,     0,    58,    59,
      64,    63,     0,    24,    26,     0,    28,    21,     0,    44,
      55,    56,    40,     0,     0,    36,    70,    65,    31,    69,
      68,    67
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
<<<<<<< HEAD
     -78,   -78,   -78,   -78,   -78,   -78,   -78,   -78,    55,    25,
     -78,   -78,   -77,    12,    -2,   -78,    -8,   -78,   -78,   -78,
     -78,    36,   -78,   -78,   -78,   -78,   -78,    -3,   -47
=======
<<<<<<< HEAD
     -75,   -75,   -75,   -75,   -75,   -75,   -75,   -75,    59,    26,
     -75,   -75,   -74,    13,   -25,   -75,    -8,   -75,   -75,   -75,
     -75,    34,   -75,   -75,   -75,   -75,   -75,    -3,   -47
=======
     -78,   -78,   -78,   -78,   -78,   -78,   -78,   -78,    55,    25,
     -78,   -78,   -77,    12,    -2,   -78,    -8,   -78,   -78,   -78,
     -78,    36,   -78,   -78,   -78,   -78,   -78,    -3,   -47
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    16,    17,    18,    19,    20,    21,    68,    71,    69,
<<<<<<< HEAD
      91,    98,    99,    75,    59,    76,    77,    36,   107,   122,
      61,    62,    37,    65,   113,   127,   131,    38,    39
=======
<<<<<<< HEAD
      91,    99,   100,    75,    59,    76,    77,    36,   108,   123,
      61,    62,    37,    65,   114,   128,   132,    38,    39
=======
      91,    98,    99,    75,    59,    76,    77,    36,   107,   122,
      61,    62,    37,    65,   113,   127,   131,    38,    39
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
<<<<<<< HEAD
=======
<<<<<<< HEAD
      35,    29,    63,    28,    32,    67,   110,    70,    72,    72,
      58,    22,    58,    33,    24,    30,    26,    84,    85,   130,
      81,    43,    44,    45,    46,   131,    34,    47,    48,    92,
      93,    63,    23,    25,   121,    27,    79,    31,    70,    53,
      83,    82,    64,    78,    41,   126,   117,     1,    66,     2,
      40,     3,     4,     5,    42,   -72,     6,    86,    87,    88,
      89,    49,     7,    50,     8,    51,   102,   103,   104,    94,
      93,    90,     9,    10,    11,    12,    13,    14,   111,   112,
     105,   106,   107,    15,    52,    33,    95,    96,    97,    98,
      95,    96,    97,    98,   118,   119,    57,    54,    58,    55,
     122,    56,    60,    33,    74,   101,    80,   113,   125,   124,
     116,   115,   109,   129,   120,    73,   127
=======
>>>>>>> origin/p4
      35,    29,    63,   109,    32,    67,    58,    70,    72,    72,
      58,    22,   129,    33,    24,    26,    81,    28,   130,    40,
      30,    43,    44,    45,    46,    34,    31,    47,    48,    41,
     120,    63,    23,    25,    27,    42,    82,    49,    70,    53,
      78,   125,    64,   -71,    51,     1,   116,     2,    66,     3,
       4,     5,    84,    85,     6,    86,    87,    88,    89,    79,
       7,    50,     8,    83,    33,    95,    96,    97,    52,    90,
       9,    10,    11,    12,    13,    14,    92,    93,   110,   111,
      54,    15,   101,   102,   103,    95,    96,    97,    57,    94,
      93,   117,   118,    55,    56,   104,   105,   106,    58,   121,
      60,    33,    74,    80,   100,   112,   124,   115,   123,   128,
     114,    73,   119,     0,   108,   126
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

static const yytype_int8 yycheck[] =
{
<<<<<<< HEAD
=======
<<<<<<< HEAD
       8,     4,    49,    40,     7,    52,    80,    54,    55,    56,
      17,     4,    17,    40,     6,    10,     6,    47,    48,     8,
      27,    24,    25,    26,    27,    14,    53,    30,    31,    47,
      48,    78,    25,    25,   108,    25,    61,    13,    85,    42,
      65,    48,    50,    48,    45,   119,    93,     3,    51,     5,
       0,     7,     8,     9,    13,    49,    12,    21,    22,    23,
      24,    19,    18,    48,    20,    13,    36,    37,    38,    47,
      48,    35,    28,    29,    30,    31,    32,    33,    81,    82,
      50,    51,    52,    39,    49,    40,    41,    42,    43,    44,
      41,    42,    43,    44,    47,    48,    11,    46,    17,    46,
     108,    46,    40,    40,    46,    26,    50,    15,    43,    16,
      46,    85,    78,    47,   101,    56,   124
=======
>>>>>>> origin/p4
       8,     4,    49,    80,     7,    52,    17,    54,    55,    56,
      17,     4,     8,    40,     6,     6,    27,    40,    14,     0,
      10,    24,    25,    26,    27,    52,    13,    30,    31,    44,
     107,    78,    25,    25,    25,    13,    47,    19,    85,    42,
      47,   118,    50,    48,    13,     3,    93,     5,    51,     7,
       8,     9,    46,    47,    12,    21,    22,    23,    24,    61,
      18,    47,    20,    65,    40,    41,    42,    43,    48,    35,
      28,    29,    30,    31,    32,    33,    46,    47,    81,    82,
      45,    39,    36,    37,    38,    41,    42,    43,    11,    46,
      47,    46,    47,    45,    45,    49,    50,    51,    17,   107,
      40,    40,    45,    49,    26,    15,    42,    45,    16,    46,
      85,    56,   100,    -1,    78,   123
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     7,     8,     9,    12,    18,    20,    28,
<<<<<<< HEAD
=======
<<<<<<< HEAD
      29,    30,    31,    32,    33,    39,    55,    56,    57,    58,
      59,    60,     4,    25,     6,    25,     6,    25,    40,    81,
      10,    13,    81,    40,    53,    70,    71,    76,    81,    82,
       0,    45,    13,    81,    81,    81,    81,    81,    81,    19,
      48,    13,    49,    81,    46,    46,    46,    11,    17,    68,
      40,    74,    75,    82,    70,    77,    81,    82,    61,    63,
      82,    62,    82,    62,    46,    67,    69,    70,    48,    68,
      50,    27,    48,    68,    47,    48,    21,    22,    23,    24,
      35,    64,    47,    48,    47,    41,    42,    43,    44,    65,
      66,    26,    36,    37,    38,    50,    51,    52,    72,    75,
      66,    81,    81,    15,    78,    63,    46,    82,    47,    48,
      67,    66,    70,    73,    16,    43,    66,    70,    79,    47,
       8,    14,    80
=======
>>>>>>> origin/p4
      29,    30,    31,    32,    33,    39,    54,    55,    56,    57,
      58,    59,     4,    25,     6,    25,     6,    25,    40,    80,
      10,    13,    80,    40,    52,    69,    70,    75,    80,    81,
       0,    44,    13,    80,    80,    80,    80,    80,    80,    19,
      47,    13,    48,    80,    45,    45,    45,    11,    17,    67,
      40,    73,    74,    81,    69,    76,    80,    81,    60,    62,
      81,    61,    81,    61,    45,    66,    68,    69,    47,    67,
      49,    27,    47,    67,    46,    47,    21,    22,    23,    24,
      35,    63,    46,    47,    46,    41,    42,    43,    64,    65,
      26,    36,    37,    38,    49,    50,    51,    71,    74,    65,
      80,    80,    15,    77,    62,    45,    81,    46,    47,    66,
      65,    69,    72,    16,    42,    65,    69,    78,    46,     8,
      14,    79
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
<<<<<<< HEAD
=======
<<<<<<< HEAD
       0,    54,    55,    55,    55,    55,    56,    56,    56,    56,
      57,    57,    57,    57,    58,    58,    59,    59,    59,    59,
      59,    60,    60,    60,    60,    61,    61,    62,    62,    63,
      64,    64,    64,    64,    64,    65,    65,    66,    66,    66,
      66,    67,    68,    68,    69,    69,    70,    70,    71,    71,
      72,    72,    72,    72,    72,    72,    73,    73,    74,    74,
      75,    76,    76,    77,    77,    77,    78,    78,    79,    80,
      80,    80,    81,    82
=======
>>>>>>> origin/p4
       0,    53,    54,    54,    54,    54,    55,    55,    55,    55,
      56,    56,    56,    56,    57,    57,    58,    58,    58,    58,
      58,    59,    59,    59,    59,    60,    60,    61,    61,    62,
      63,    63,    63,    63,    63,    64,    64,    65,    65,    65,
      66,    67,    67,    68,    68,    69,    69,    70,    70,    71,
      71,    71,    71,    71,    71,    72,    72,    73,    73,    74,
      75,    75,    76,    76,    76,    77,    77,    78,    79,    79,
      79,    80,    81
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     4,     6,     3,     2,     6,
       6,     7,     4,     5,     6,     1,     3,     1,     3,     2,
       1,     4,     1,     1,     1,     1,     3,     1,     1,     1,
<<<<<<< HEAD
=======
<<<<<<< HEAD
       1,     3,     0,     2,     1,     3,     3,     1,     1,     3,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     1,     1,     1,     3,     3,     3,     0,     2,     1,
       1,     0,     1,     1
=======
>>>>>>> origin/p4
       3,     0,     2,     1,     3,     3,     1,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     3,
       1,     1,     1,     3,     3,     3,     0,     2,     1,     1,
       0,     1,     1
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (&yylloc, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YY_LOCATION_PRINT
#  if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#   define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

#  else
#   define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#  endif
# endif /* !defined YY_LOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YY_LOCATION_PRINT (yyo, *yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex (&yylval, &yylloc);
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      yyerror_range[1] = yylloc;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* start: stmt ';'  */
#line 58 "/home/one/db2023/src/parser/yacc.y"
    {
        parse_tree = (yyvsp[-1].sv_node);
        YYACCEPT;
    }
<<<<<<< HEAD
#line 1634 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1635 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1634 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 3: /* start: HELP  */
#line 63 "/home/one/db2023/src/parser/yacc.y"
    {
        parse_tree = std::make_shared<Help>();
        YYACCEPT;
    }
<<<<<<< HEAD
#line 1643 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1644 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1643 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 4: /* start: EXIT  */
#line 68 "/home/one/db2023/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
<<<<<<< HEAD
#line 1652 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1653 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1652 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 5: /* start: T_EOF  */
#line 73 "/home/one/db2023/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
<<<<<<< HEAD
#line 1661 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1662 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1661 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 10: /* txnStmt: TXN_BEGIN  */
#line 88 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnBegin>();
    }
<<<<<<< HEAD
#line 1669 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1670 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1669 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 11: /* txnStmt: TXN_COMMIT  */
#line 92 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnCommit>();
    }
<<<<<<< HEAD
#line 1677 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1678 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1677 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 12: /* txnStmt: TXN_ABORT  */
#line 96 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnAbort>();
    }
<<<<<<< HEAD
#line 1685 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1686 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1685 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 13: /* txnStmt: TXN_ROLLBACK  */
#line 100 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnRollback>();
    }
<<<<<<< HEAD
#line 1693 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1694 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1693 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 14: /* dbStmt: SHOW TABLES  */
#line 107 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowTables>();
    }
<<<<<<< HEAD
#line 1701 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1702 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1701 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 15: /* dbStmt: SHOW INDEX FROM tbName  */
#line 111 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowIndexes>((yyvsp[0].sv_str));
    }
<<<<<<< HEAD
#line 1709 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1710 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1709 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 16: /* ddl: CREATE TABLE tbName '(' fieldList ')'  */
#line 118 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateTable>((yyvsp[-3].sv_str), (yyvsp[-1].sv_fields));
    }
<<<<<<< HEAD
#line 1717 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1718 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1717 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 17: /* ddl: DROP TABLE tbName  */
#line 122 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropTable>((yyvsp[0].sv_str));
    }
<<<<<<< HEAD
#line 1725 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1726 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1725 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 18: /* ddl: DESC tbName  */
#line 126 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DescTable>((yyvsp[0].sv_str));
    }
<<<<<<< HEAD
#line 1733 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1734 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1733 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 19: /* ddl: CREATE INDEX tbName '(' colNameList ')'  */
#line 130 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
<<<<<<< HEAD
#line 1741 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1742 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1741 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 20: /* ddl: DROP INDEX tbName '(' colNameList ')'  */
#line 134 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
<<<<<<< HEAD
#line 1749 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1750 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1749 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 21: /* dml: INSERT INTO tbName VALUES '(' valueList ')'  */
#line 141 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<InsertStmt>((yyvsp[-4].sv_str), (yyvsp[-1].sv_vals));
    }
<<<<<<< HEAD
#line 1757 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1758 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1757 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 22: /* dml: DELETE FROM tbName optWhereClause  */
#line 145 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DeleteStmt>((yyvsp[-1].sv_str), (yyvsp[0].sv_conds));
    }
<<<<<<< HEAD
#line 1765 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1766 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1765 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 23: /* dml: UPDATE tbName SET setClauses optWhereClause  */
#line 149 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<UpdateStmt>((yyvsp[-3].sv_str), (yyvsp[-1].sv_set_clauses), (yyvsp[0].sv_conds));
    }
<<<<<<< HEAD
#line 1773 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1774 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1773 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 24: /* dml: SELECT selector FROM tableList optWhereClause opt_order_clause  */
#line 153 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<SelectStmt>((yyvsp[-4].sv_cols), (yyvsp[-2].sv_strs), (yyvsp[-1].sv_conds), (yyvsp[0].sv_orderby));
    }
<<<<<<< HEAD
#line 1781 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1782 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1781 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 25: /* fieldList: field  */
#line 160 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_fields) = std::vector<std::shared_ptr<Field>>{(yyvsp[0].sv_field)};
    }
<<<<<<< HEAD
#line 1789 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1790 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1789 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 26: /* fieldList: fieldList ',' field  */
#line 164 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_fields).push_back((yyvsp[0].sv_field));
    }
<<<<<<< HEAD
#line 1797 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1798 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1797 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 27: /* colNameList: colName  */
#line 171 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
<<<<<<< HEAD
#line 1805 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1806 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1805 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 28: /* colNameList: colNameList ',' colName  */
#line 175 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
<<<<<<< HEAD
#line 1813 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1814 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1813 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 29: /* field: colName type  */
#line 182 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_field) = std::make_shared<ColDef>((yyvsp[-1].sv_str), (yyvsp[0].sv_type_len));
    }
<<<<<<< HEAD
#line 1821 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1822 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1821 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 30: /* type: INT  */
#line 189 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_INT, sizeof(int));
    }
<<<<<<< HEAD
#line 1829 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1830 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1829 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 31: /* type: CHAR '(' VALUE_INT_BIGINT ')'  */
#line 193 "/home/one/db2023/src/parser/yacc.y"
    {
        // 这里应该检查精度
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_STRING, std::stoi((yyvsp[-1].sv_str).c_str()));
    }
<<<<<<< HEAD
#line 1838 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1839 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1838 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 32: /* type: FLOAT  */
#line 198 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_FLOAT, sizeof(float));
    }
<<<<<<< HEAD
#line 1846 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1847 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1846 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 33: /* type: BIGINT  */
#line 202 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_BIGINT, 8);
    }
<<<<<<< HEAD
#line 1854 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1855 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1854 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 34: /* type: DATETIME  */
#line 206 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_DATETIME, 8);
    }
<<<<<<< HEAD
#line 1862 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1863 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1862 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 35: /* valueList: value  */
#line 213 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_vals) = std::vector<std::shared_ptr<Value>>{(yyvsp[0].sv_val)};
    }
<<<<<<< HEAD
#line 1870 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1871 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1870 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 36: /* valueList: valueList ',' value  */
#line 217 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_vals).push_back((yyvsp[0].sv_val));
    }
<<<<<<< HEAD
#line 1878 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1879 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1878 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 37: /* value: VALUE_INT_BIGINT  */
#line 224 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<Int_Bint_Lit>((yyvsp[0].sv_str));
    }
<<<<<<< HEAD
#line 1886 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
<<<<<<< HEAD
#line 1887 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
#line 1886 "/home/one/db2023/src/parser/yacc.tab.cpp"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
    break;

  case 38: /* value: VALUE_FLOAT  */
#line 228 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<FloatLit>((yyvsp[0].sv_float));
    }
<<<<<<< HEAD
=======
<<<<<<< HEAD
#line 1895 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 39: /* value: VALUE_DATETIME  */
#line 232 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<DatetimeLit>((yyvsp[0].sv_str));
    }
#line 1903 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 40: /* value: VALUE_STRING  */
#line 236 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<StringLit>((yyvsp[0].sv_str));
    }
#line 1911 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 41: /* condition: col op expr  */
#line 243 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_cond) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_col), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr));
    }
#line 1919 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 42: /* optWhereClause: %empty  */
#line 249 "/home/one/db2023/src/parser/yacc.y"
                      { /* ignore*/ }
#line 1925 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 43: /* optWhereClause: WHERE whereClause  */
#line 251 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_conds) = (yyvsp[0].sv_conds);
    }
#line 1933 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 44: /* whereClause: condition  */
#line 258 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_conds) = std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)};
    }
#line 1941 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 45: /* whereClause: whereClause AND condition  */
#line 262 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_conds).push_back((yyvsp[0].sv_cond));
    }
#line 1949 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 46: /* col: tbName '.' colName  */
#line 269 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>((yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 1957 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 47: /* col: colName  */
#line 273 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[0].sv_str));
    }
#line 1965 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 48: /* colList: col  */
#line 280 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_cols) = std::vector<std::shared_ptr<Col>>{(yyvsp[0].sv_col)};
    }
#line 1973 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 49: /* colList: colList ',' col  */
#line 284 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_cols).push_back((yyvsp[0].sv_col));
    }
#line 1981 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 50: /* op: '='  */
#line 291 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_EQ;
    }
#line 1989 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 51: /* op: '<'  */
#line 295 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LT;
    }
#line 1997 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 52: /* op: '>'  */
#line 299 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GT;
    }
#line 2005 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 53: /* op: NEQ  */
#line 303 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_NE;
    }
#line 2013 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 54: /* op: LEQ  */
#line 307 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LE;
    }
#line 2021 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 55: /* op: GEQ  */
#line 311 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GE;
    }
#line 2029 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 56: /* expr: value  */
#line 318 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_val));
    }
#line 2037 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 57: /* expr: col  */
#line 322 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_col));
    }
#line 2045 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 58: /* setClauses: setClause  */
#line 329 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses) = std::vector<std::shared_ptr<SetClause>>{(yyvsp[0].sv_set_clause)};
    }
#line 2053 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 59: /* setClauses: setClauses ',' setClause  */
#line 333 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses).push_back((yyvsp[0].sv_set_clause));
    }
#line 2061 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 60: /* setClause: colName '=' value  */
#line 340 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-2].sv_str), (yyvsp[0].sv_val));
    }
#line 2069 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 61: /* selector: '*'  */
#line 347 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_cols) = {};
    }
#line 2077 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 63: /* tableList: tbName  */
#line 355 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
#line 2085 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 64: /* tableList: tableList ',' tbName  */
=======
>>>>>>> origin/p4
#line 1894 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 39: /* value: VALUE_STRING  */
#line 232 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<StringLit>((yyvsp[0].sv_str));
    }
#line 1902 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 40: /* condition: col op expr  */
#line 239 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_cond) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_col), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr));
    }
#line 1910 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 41: /* optWhereClause: %empty  */
#line 245 "/home/one/db2023/src/parser/yacc.y"
                      { /* ignore*/ }
#line 1916 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 42: /* optWhereClause: WHERE whereClause  */
#line 247 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_conds) = (yyvsp[0].sv_conds);
    }
#line 1924 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 43: /* whereClause: condition  */
#line 254 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_conds) = std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)};
    }
#line 1932 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 44: /* whereClause: whereClause AND condition  */
#line 258 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_conds).push_back((yyvsp[0].sv_cond));
    }
#line 1940 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 45: /* col: tbName '.' colName  */
#line 265 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>((yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 1948 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 46: /* col: colName  */
#line 269 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[0].sv_str));
    }
#line 1956 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 47: /* colList: col  */
#line 276 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_cols) = std::vector<std::shared_ptr<Col>>{(yyvsp[0].sv_col)};
    }
#line 1964 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 48: /* colList: colList ',' col  */
#line 280 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_cols).push_back((yyvsp[0].sv_col));
    }
#line 1972 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 49: /* op: '='  */
#line 287 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_EQ;
    }
#line 1980 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 50: /* op: '<'  */
#line 291 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LT;
    }
#line 1988 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 51: /* op: '>'  */
#line 295 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GT;
    }
#line 1996 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 52: /* op: NEQ  */
#line 299 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_NE;
    }
#line 2004 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 53: /* op: LEQ  */
#line 303 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LE;
    }
#line 2012 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 54: /* op: GEQ  */
#line 307 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GE;
    }
#line 2020 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 55: /* expr: value  */
#line 314 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_val));
    }
#line 2028 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 56: /* expr: col  */
#line 318 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_col));
    }
#line 2036 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 57: /* setClauses: setClause  */
#line 325 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses) = std::vector<std::shared_ptr<SetClause>>{(yyvsp[0].sv_set_clause)};
    }
#line 2044 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 58: /* setClauses: setClauses ',' setClause  */
#line 329 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses).push_back((yyvsp[0].sv_set_clause));
    }
#line 2052 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 59: /* setClause: colName '=' value  */
#line 336 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-2].sv_str), (yyvsp[0].sv_val));
    }
#line 2060 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 60: /* selector: '*'  */
#line 343 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_cols) = {};
    }
#line 2068 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 62: /* tableList: tbName  */
#line 351 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
#line 2076 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 63: /* tableList: tableList ',' tbName  */
#line 355 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 2084 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 64: /* tableList: tableList JOIN tbName  */
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4
#line 359 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
<<<<<<< HEAD
=======
<<<<<<< HEAD
#line 2093 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 65: /* tableList: tableList JOIN tbName  */
#line 363 "/home/one/db2023/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 2101 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 66: /* opt_order_clause: ORDER BY order_clause  */
#line 370 "/home/one/db2023/src/parser/yacc.y"
    { 
        (yyval.sv_orderby) = (yyvsp[0].sv_orderby); 
    }
#line 2109 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 67: /* opt_order_clause: %empty  */
#line 373 "/home/one/db2023/src/parser/yacc.y"
                      { /* ignore*/ }
#line 2115 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 68: /* order_clause: col opt_asc_desc  */
#line 378 "/home/one/db2023/src/parser/yacc.y"
    { 
        (yyval.sv_orderby) = std::make_shared<OrderBy>((yyvsp[-1].sv_col), (yyvsp[0].sv_orderby_dir));
    }
#line 2123 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 69: /* opt_asc_desc: ASC  */
#line 384 "/home/one/db2023/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_ASC;     }
#line 2129 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 70: /* opt_asc_desc: DESC  */
#line 385 "/home/one/db2023/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_DESC;    }
#line 2135 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 71: /* opt_asc_desc: %empty  */
#line 386 "/home/one/db2023/src/parser/yacc.y"
            { (yyval.sv_orderby_dir) = OrderBy_DEFAULT; }
#line 2141 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;


#line 2145 "/home/one/db2023/src/parser/yacc.tab.cpp"
=======
>>>>>>> origin/p4
#line 2092 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 65: /* opt_order_clause: ORDER BY order_clause  */
#line 366 "/home/one/db2023/src/parser/yacc.y"
    { 
        (yyval.sv_orderby) = (yyvsp[0].sv_orderby); 
    }
#line 2100 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 66: /* opt_order_clause: %empty  */
#line 369 "/home/one/db2023/src/parser/yacc.y"
                      { /* ignore*/ }
#line 2106 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 67: /* order_clause: col opt_asc_desc  */
#line 374 "/home/one/db2023/src/parser/yacc.y"
    { 
        (yyval.sv_orderby) = std::make_shared<OrderBy>((yyvsp[-1].sv_col), (yyvsp[0].sv_orderby_dir));
    }
#line 2114 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 68: /* opt_asc_desc: ASC  */
#line 380 "/home/one/db2023/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_ASC;     }
#line 2120 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 69: /* opt_asc_desc: DESC  */
#line 381 "/home/one/db2023/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_DESC;    }
#line 2126 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;

  case 70: /* opt_asc_desc: %empty  */
#line 382 "/home/one/db2023/src/parser/yacc.y"
            { (yyval.sv_orderby_dir) = OrderBy_DEFAULT; }
#line 2132 "/home/one/db2023/src/parser/yacc.tab.cpp"
    break;


#line 2136 "/home/one/db2023/src/parser/yacc.tab.cpp"
<<<<<<< HEAD
=======
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          goto yyexhaustedlab;
      }
    }

  yyerror_range[1] = yylloc;
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, &yylloc);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if 1
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (&yylloc, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturn;
#endif


/*-------------------------------------------------------.
| yyreturn -- parsing is finished, clean up and return.  |
`-------------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

<<<<<<< HEAD
#line 388 "/home/one/db2023/src/parser/yacc.y"
=======
<<<<<<< HEAD
#line 392 "/home/one/db2023/src/parser/yacc.y"
=======
#line 388 "/home/one/db2023/src/parser/yacc.y"
>>>>>>> f4d1731 (tmp)
>>>>>>> origin/p4

