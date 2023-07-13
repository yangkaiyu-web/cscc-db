<<<<<<< HEAD
/* A Bison parser, made by GNU Bison 3.7.6.  */
=======
/* A Bison parser, made by GNU Bison 3.5.1.  */
>>>>>>> origin/p4

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

<<<<<<< HEAD
#ifndef YY_YY_HOME_ONE_DB2023_SRC_PARSER_YACC_TAB_H_INCLUDED
# define YY_YY_HOME_ONE_DB2023_SRC_PARSER_YACC_TAB_H_INCLUDED
=======
#ifndef YY_YY_HOME_DEVILHEART_RMDB_SRC_PARSER_YACC_TAB_H_INCLUDED
# define YY_YY_HOME_DEVILHEART_RMDB_SRC_PARSER_YACC_TAB_H_INCLUDED
>>>>>>> origin/p4
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
<<<<<<< HEAD
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
    WHERE = 272,                   /* WHERE  */
    UPDATE = 273,                  /* UPDATE  */
    SET = 274,                     /* SET  */
    SELECT = 275,                  /* SELECT  */
    INT = 276,                     /* INT  */
    CHAR = 277,                    /* CHAR  */
    FLOAT = 278,                   /* FLOAT  */
    INDEX = 279,                   /* INDEX  */
    AND = 280,                     /* AND  */
    JOIN = 281,                    /* JOIN  */
    EXIT = 282,                    /* EXIT  */
    HELP = 283,                    /* HELP  */
    TXN_BEGIN = 284,               /* TXN_BEGIN  */
    TXN_COMMIT = 285,              /* TXN_COMMIT  */
    TXN_ABORT = 286,               /* TXN_ABORT  */
    TXN_ROLLBACK = 287,            /* TXN_ROLLBACK  */
    ORDER_BY = 288,                /* ORDER_BY  */
    BIGINT = 289,                  /* BIGINT  */
    LEQ = 290,                     /* LEQ  */
    NEQ = 291,                     /* NEQ  */
    GEQ = 292,                     /* GEQ  */
    T_EOF = 293,                   /* T_EOF  */
    IDENTIFIER = 294,              /* IDENTIFIER  */
    VALUE_STRING = 295,            /* VALUE_STRING  */
    VALUE_INT_BIGINT = 296,        /* VALUE_INT_BIGINT  */
    VALUE_FLOAT = 297              /* VALUE_FLOAT  */
=======
    SHOW = 258,
    TABLES = 259,
    CREATE = 260,
    TABLE = 261,
    DROP = 262,
    DESC = 263,
    INSERT = 264,
    INTO = 265,
    VALUES = 266,
    DELETE = 267,
    FROM = 268,
    ASC = 269,
    ORDER = 270,
    BY = 271,
    WHERE = 272,
    UPDATE = 273,
    SET = 274,
    SELECT = 275,
    INT = 276,
    CHAR = 277,
    FLOAT = 278,
    DATETIME = 279,
    INDEX = 280,
    AND = 281,
    JOIN = 282,
    EXIT = 283,
    HELP = 284,
    TXN_BEGIN = 285,
    TXN_COMMIT = 286,
    TXN_ABORT = 287,
    TXN_ROLLBACK = 288,
    ORDER_BY = 289,
    LEQ = 290,
    NEQ = 291,
    GEQ = 292,
    T_EOF = 293,
    IDENTIFIER = 294,
    VALUE_STRING = 295,
    VALUE_DATETIME = 296,
    VALUE_INT = 297,
    VALUE_FLOAT = 298
>>>>>>> origin/p4
  };
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

<<<<<<< HEAD
#endif /* !YY_YY_HOME_ONE_DB2023_SRC_PARSER_YACC_TAB_H_INCLUDED  */
=======
#endif /* !YY_YY_HOME_DEVILHEART_RMDB_SRC_PARSER_YACC_TAB_H_INCLUDED  */
>>>>>>> origin/p4
