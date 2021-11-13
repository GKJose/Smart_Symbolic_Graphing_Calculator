/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mpconfig.h"
#include "nlr.h"
#include "misc.h"
#include "qstr.h"
#include "obj.h"
#include "input.h"

const char * console_input(const char * msg1,const char * msg2,bool numeric,int ypos);

char *prompt(const char *p) {
  return strdup(console_input(p,"? ",false,185));
}

STATIC mp_obj_t mp_builtin_input(size_t n_args, const mp_obj_t *args) {
  const char * s="";
  if (n_args==1 && MP_OBJ_IS_STR(args[0])) 
    s=mp_obj_str_get_str(args[0]);
  char *line = prompt(s);
  if (line == NULL) {
    nlr_raise(mp_obj_new_exception(&mp_type_EOFError));
  }
  mp_obj_t o = mp_obj_new_str(line, strlen(line));
  free(line);
  return o;
}

MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mp_builtin_input_obj, 0, 1, mp_builtin_input);
