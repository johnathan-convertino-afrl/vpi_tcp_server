//******************************************************************************
/// @file   vpi_messages.c
/// @author Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date   2024-03-02
/// @brief  Functions print messages using the vpi interface.
///
/// @LICENSE MIT
///  Copyright 2024 Jay Convertino
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to 
///  deal in the Software without restriction, including without limitation the
///  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
///  sell copies of the Software, and to permit persons to whom the Software is 
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in 
///  all copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
///  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
///  IN THE SOFTWARE.
//******************************************************************************

// c standard libraries
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <vpi_user.h>
#include "messages.h"

// FUNCTIONS //
int print_error(const char *format, ...)
{
  char p_string[strlen(format) + strlen("ERROR: ")];

  va_list args;
  va_start(args, format);

  strcpy(p_string, "ERROR: ");

  vpi_vprintf(strcat(p_string, format), args);

  vpi_control(vpiFinish, 1);

  va_end(args);

  return 0;
}

int print_info(const char *format, ...)
{
  char p_string[strlen(format) + strlen("INFO: ")];

  va_list args;
  va_start(args, format);

  strcpy(p_string, "INFO: ");

  vpi_vprintf(strcat(p_string, format), args);

  va_end(args);

  return 0;
}
