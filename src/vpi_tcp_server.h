//******************************************************************************
/// @file   vpi_tcp_server.h
/// @author Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date   2024-23-02
/// @brief  Functions to create multiple TCP servers
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

#ifndef __VPI_TCP_SERVER
#define __VPI_TCP_SERVER

// Include the VPI library of routines (object based).
#include <vpi_user.h>
#include "tcp_server.h"

#define RECV_NAME  "$recv_tcp_server"
#define SEND_NAME  "$send_tcp_server"
#define SETUP_NAME "$setup_tcp_server"

struct s_vpi_data
{
  PLI_INT32 error;
  PLI_INT32 num_ab_val_pairs;
  PLI_INT32 array_byte_size;
  vpiHandle systf_handle;
  vpiHandle arg1_handle;
  vpiHandle arg2_handle;
};

#endif
