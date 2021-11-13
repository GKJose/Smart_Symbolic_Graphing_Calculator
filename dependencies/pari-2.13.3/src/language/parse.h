/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
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

#ifndef YY_PARI_SRC_LANGUAGE_PARSE_H_INCLUDED
# define YY_PARI_SRC_LANGUAGE_PARSE_H_INCLUDED
/* Debug traces.  */
#ifndef PARI_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define PARI_DEBUG 1
#  else
#   define PARI_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define PARI_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined PARI_DEBUG */
#if PARI_DEBUG
extern int pari_debug;
#endif

/* Token type.  */
#ifndef PARI_TOKENTYPE
# define PARI_TOKENTYPE
  enum pari_tokentype
  {
    KPARROW = 258,
    KARROW = 259,
    KDOTDOT = 260,
    KPE = 261,
    KSE = 262,
    KME = 263,
    KDE = 264,
    KDRE = 265,
    KEUCE = 266,
    KMODE = 267,
    KAND = 268,
    KOR = 269,
    KID = 270,
    KEQ = 271,
    KNE = 272,
    KGE = 273,
    KLE = 274,
    KSRE = 275,
    KSLE = 276,
    KSR = 277,
    KSL = 278,
    KDR = 279,
    KPP = 280,
    KSS = 281,
    KINTEGER = 282,
    KREAL = 283,
    KENTRY = 284,
    KSTRING = 285,
    SEQ = 286,
    DEFFUNC = 287,
    INT = 288,
    LVAL = 289,
    SIGN = 290,
    DERIV = 291
  };
#endif

/* Value type.  */

/* Location type.  */
#if ! defined PARI_LTYPE && ! defined PARI_LTYPE_IS_DECLARED
typedef struct PARI_LTYPE PARI_LTYPE;
struct PARI_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define PARI_LTYPE_IS_DECLARED 1
# define PARI_LTYPE_IS_TRIVIAL 1
#endif



int pari_parse (char **lex);

#endif /* !YY_PARI_SRC_LANGUAGE_PARSE_H_INCLUDED  */
