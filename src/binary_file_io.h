//******************************************************************************
/// @file   binary_file_io.h
/// @author Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date   2023-20-1
/// @brief  Functions to write raw binary files properly in verilog.
///
/// @LICENSE MIT
///  Copyright 2023 Jay Convertino
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

#ifndef __BINARY_FILE_IO
#define __BINARY_FILE_IO

// c standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// other libs
#include <pthread.h>
// Include the VPI library of routines (object based).
#include <vpi_user.h>
// include ringbuffer library
#include "ringBuffer.h"

//ring buffer sizes
// 4 MB
#define BUFFSIZE  (1 << 23)
// 1 MB
#define DATACHUNK (1 << 21)

#define READ_NAME   "$read_binary_file"
#define WRITE_NAME  "$write_binary_file"

struct s_process_data
{
  PLI_INT32 error;
  PLI_INT32 num_ab_val_pairs;
  PLI_INT32 array_byte_size;
  
  char * p_file_name;
  
  struct s_ringBuffer *p_ringbuffer;
  
  FILE *p_file;
  
  pthread_t thread;
  
  vpiHandle systf_handle;
  vpiHandle arg2_handle;
};

//******************************************************************************
/// @brief BINARY FILE END COMPILE CALLBACK
//******************************************************************************
PLI_INT32 binary_end_compile_cb(p_cb_data data);

//******************************************************************************
/// @brief BINARY FILE END SIM CALLBACK
//******************************************************************************
PLI_INT32 binary_end_sim_cb(p_cb_data data);

//******************************************************************************
/// @brief  Returns the size, in bits, of the function return type.
//******************************************************************************
PLI_INT32 binary_sizetf(PLI_BYTE8 *user_data);

#endif
