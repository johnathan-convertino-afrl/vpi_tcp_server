//******************************************************************************
/// @file     recv_tcp_server.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2024-02-22
/// @brief    Functions for TCP server data receive.
/// @details  $recv_tcp_server takes 2 arguments. First the fd returned from
///           $setup_tcp_server, and then a register for data in size bytes.
///           The function returns the number of bytes read.
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

#ifndef __RECV_TCP_SERVER
#define __RECV_TCP_SERVER

// Include the VPI library of routines (object based).
#include <vpi_user.h>

//******************************************************************************
/// @brief RECEIVE TCP SERVER DATA COMPILE SETUP
//******************************************************************************
PLI_INT32 recv_tcp_server_compiletf(PLI_BYTE8 *user_data);

//******************************************************************************
/// @brief  read_binary_calltf is a callback for the recv_tcp_server function.
//******************************************************************************
PLI_INT32 recv_tcp_server_calltf(PLI_BYTE8 *user_data);

#endif
