//******************************************************************************
/// @file   tcp_server.h
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

// c standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// other libs
// threads
#include <pthread.h>
// tcp
#include <unistd.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
// Include the VPI library of routines (object based).
#include <vpi_user.h>
// include ringbuffer library
#include "ringBuffer.h"

//ring buffer sizes
// 4 MB
#define BUFFSIZE  (1 << 23)
// 1 MB
#define DATACHUNK (1 << 21)

#define MAX_CONNECTIONS 256

#define RECV_NAME  "$recv_tcp_server"
#define SEND_NAME  "$send_tcp_server"
#define SETUP_NAME "$setup_tcp_server"

struct s_process_data
{
  // PLI_INT32 error;
  PLI_INT32 num_ab_val_pairs;
  PLI_INT32 array_byte_size;
  
  struct s_ringBuffer *p_ringbuffer;
  
  pthread_t thread;

  vpiHandle systf_handle;
  vpiHandle arg2_handle;
};

struct s_send_tcp_server
{
  int kill_thread;

  struct pollfd poll_connection;
  struct sockaddr_in *p_socket_info;

  pthread_t connection_thread;

  char *p_address;
  unsigned short port;

  vpiHandle systf_handle;

  struct s_process_data recv_process_data;
  struct s_process_data send_process_data;
};

extern struct s_send_tcp_server g_send_tcp_server[MAX_CONNECTIONS];

#endif
