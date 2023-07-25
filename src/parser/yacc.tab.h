/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_HOME_YONCHICY_RMDB_SRC_PARSER_YACC_TAB_H_INCLUDED
# define YY_YY_HOME_YONCHICY_RMDB_SRC_PARSER_YACC_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    SHOW = 258,                    /* SHOW  */
    TABLES = 259,                  /* TABLES  */
    CREATE = 260,                  /* CREATE  */
    TABLE = 261,                   /* TABLE  */
    DROP = 262,                    /* DROP  */
    DESC = 263,                    /* DESC  */
    INSERT = 264,                  /* INSERT  */
    INTO = 265,                    /* INTO  */
    VALUES = 266,                  /* VALUES  */
    DELETE = 267,                  /* DELETE  */
    FROM = 268,                    /* FROM  */
    ASC = 269,                     /* ASC  */
    ORDER = 270,                   /* ORDER  */
    BY = 271,                      /* BY  */
    LIMIT = 272,                   /* LIMIT  */
    WHERE = 273,                   /* WHERE  */
    UPDATE = 274,                  /* UPDATE  */
    SET = 275,                     /* SET  */
    SELECT = 276,                  /* SELECT  */
    INT = 277,                     /* INT  */
    CHAR = 278,                    /* CHAR  */
    FLOAT = 279,                   /* FLOAT  */
    DATETIME = 280,                /* DATETIME  */
    INDEX = 281,                   /* INDEX  */
    AND = 282,                     /* AND  */
    JOIN = 283,                    /* JOIN  */
    EXIT = 284,                    /* EXIT  */
    HELP = 285,                    /* HELP  */
    TXN_BEGIN = 286,               /* TXN_BEGIN  */
    TXN_COMMIT = 287,              /* TXN_COMMIT  */
    TXN_ABORT = 288,               /* TXN_ABORT  */
    TXN_ROLLBACK = 289,            /* TXN_ROLLBACK  */
    ORDER_BY = 290,                /* ORDER_BY  */
    BIGINT = 291,                  /* BIGINT  */
    COUNT = 292,                   /* COUNT  */
    MAX = 293,                     /* MAX  */
    MIN = 294,                     /* MIN  */
    SUM = 295,                     /* SUM  */
    AS = 296,                      /* AS  */
    LEQ = 297,                     /* LEQ  */
    NEQ = 298,                     /* NEQ  */
    GEQ = 299,                     /* GEQ  */
    T_EOF = 300,                   /* T_EOF  */
    IDENTIFIER = 301,              /* IDENTIFIER  */
    VALUE_STRING = 302,            /* VALUE_STRING  */
    VALUE_INT_BIGINT = 303,        /* VALUE_INT_BIGINT  */
    VALUE_FLOAT = 304              /* VALUE_FLOAT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




int yyparse (void);


#endif /* !YY_YY_HOME_YONCHICY_RMDB_SRC_PARSER_YACC_TAB_H_INCLUDED  */
