//******************************************************************************
/// @file     read_binary_file.h
/// @author   Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date     2023-20-1
/// @brief    Functions to write raw binary files properly in verilog.
/// @details  $read_binary_file takes 2 arguments. First the file name, next
///           a register for data in size bytes.
///           The function returns the number of bytes read. If it is a negative
///           number, that indicates EOF. 
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

#ifndef __READ_FILE_IO
#define __READ_FILE_IO

// Include the VPI library of routines (object based).
#include <vpi_user.h>

//******************************************************************************
/// @brief READ BINARY FILE START SIM CALLBACK
//******************************************************************************
PLI_INT32 read_binary_start_sim_cb(p_cb_data data);

//******************************************************************************
/// @brief  read_binary_calltf is a callback for the read_binary_file function.
//******************************************************************************
PLI_INT32 read_binary_calltf(PLI_BYTE8 *user_data);

#endif
