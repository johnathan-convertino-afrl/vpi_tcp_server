//******************************************************************************
/// @file   vpi_tcp_server.h
/// @author Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date   2024-22-02
/// @brief  Functions to write raw binary files properly in verilog.
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

#include "send_tcp_server.h"
#include "recv_tcp_server.h"
#include "vpi_tcp_server.h"

//******************************************************************************
/// @brief SETUP TCP SERVER DATA START SIM CALLBACK
//******************************************************************************
PLI_INT32 setup_tcp_server_start_sim_cb(p_cb_data data)
{
  int *p_index = NULL;

  p_index = (int *)data->user_data;

  return start_tcp_server(p_index);
}

//******************************************************************************
/// @brief SETUP TCP SERVER END SIM CALLBACK
//******************************************************************************
PLI_INT32 setup_tcp_server_end_sim_cb(p_cb_data data)
{
  int *p_index = NULL;

  p_index = (int *)data->user_data;

  return end_tcp_server(p_index);
}

//******************************************************************************
/// @brief  Returns the size, in bits, of the function return type.
//******************************************************************************
PLI_INT32 tcp_server_sizetf(PLI_BYTE8 *user_data)
{
  (void)user_data;
  
  return 32;
}

//******************************************************************************
/// @brief  Compile time call, check the arguments for validity.
//******************************************************************************
PLI_INT32 setup_tcp_server_compiletf(PLI_BYTE8 *user_data)
{
  // keep compiler from whining
  (void)user_data;
  
  //variables
  int *p_index = NULL;

  PLI_INT32 arg_type = 0;
  // PLI_INT32 vector_size;
  // PLI_INT32 array_byte_size;
  // PLI_INT32 num_ab_val_pairs;
  
  PLI_BYTE8 *p_func_name = NULL;
  
  s_vpi_value address;
  s_vpi_value port;
  
  // vpi_register_cb copies the data in these structs per the verilog-2001 standard. cute.
  s_cb_data start_sim_cb_data;
  s_cb_data end_sim_cb_data;
  
  vpiHandle systf_handle, arg_iterate, arg1_handle, arg2_handle, arg_handle, callback_handle;
  
  // obtain a handle to the argument list
  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  
  p_func_name = vpi_get_str(vpiName, systf_handle);
  
  // pull the argument object handles, if no handles, no arguments
  arg_iterate = vpi_iterate(vpiArgument, systf_handle);
  
  if(!arg_iterate)
  {
    vpi_printf("ERROR: %s requires two arguments.\n", p_func_name);

    vpi_control(vpiFinish, 1);

    return 0;
  }
  
  // check the type of the first handle, if it doesn't match, quit.
  arg1_handle = vpi_scan(arg_iterate);

  if(!arg1_handle)
  {
    vpi_printf("ERROR: %s requires a first argument.\n", p_func_name);

    vpi_free_object(arg_iterate);

    vpi_control(vpiFinish, 1);

    return 0;
  }
  
  arg_type = vpi_get(vpiConstType, arg1_handle);
  
  if(arg_type != vpiStringConst)
  {
      vpi_printf("ERROR: %s requires first argument be a string.\n", p_func_name);
      
      vpi_free_object(arg_iterate);
      
      vpi_control(vpiFinish, 1);
      
      return 0;
  }
  
  address.format = vpiStringVal;
  
  vpi_get_value(arg1_handle, &address);

  // check for a second argument, this should return a vector.
  arg2_handle = vpi_scan(arg_iterate);

  if(!arg2_handle)
  {
    vpi_printf("ERROR: %s requires a second argument.\n", p_func_name);

    vpi_free_object(arg_iterate);

    vpi_control(vpiFinish, 1);

    return 0;
  }
  
  arg_type = vpi_get(vpiType, arg2_handle);

  if(arg_type != vpiConstant)
  {
      vpi_printf("ERROR: %s requires second argument be a int.\n", p_func_name);

      vpi_free_object(arg_iterate);

      vpi_control(vpiFinish, 1);

      return 0;
  }
  
  // check for a any more arguments, if they exist, error out.
  arg_handle = vpi_scan(arg_iterate);
  
  if(arg_handle)
  {
    vpi_printf("ERROR: %s require only two arguments.\n", p_func_name);
    
    vpi_free_object(arg_iterate);
    
    vpi_control(vpiFinish, 1);
    
    return 0;
  }

  port.format = vpiIntVal;

  vpi_get_value(arg2_handle, &port);

  p_index = setup_tcp_server(address.value.str, port.value.integer);

  if(!p_index)
  {
    vpi_printf("ERROR: %s failed to setup tcp server.\n", p_func_name);

    vpi_control(vpiFinish, 1);

    return 0;
  }

  // setup callback for start of simulation, well before it starts.
  start_sim_cb_data.reason    = cbStartOfSimulation;
  start_sim_cb_data.cb_rtn    = setup_tcp_server_start_sim_cb;
  start_sim_cb_data.obj       = NULL;
  start_sim_cb_data.time      = NULL;
  start_sim_cb_data.value     = NULL;
  // add user data argument struct
  start_sim_cb_data.user_data = (PLI_BYTE8 *)p_index;
  callback_handle = vpi_register_cb(&start_sim_cb_data);
  vpi_free_object(callback_handle);

  // setup callback for end of simulation, well before it starts.
  end_sim_cb_data.reason    = cbEndOfSimulation;
  end_sim_cb_data.cb_rtn    = setup_tcp_server_end_sim_cb;
  end_sim_cb_data.obj       = NULL;
  end_sim_cb_data.time      = NULL;
  end_sim_cb_data.value     = NULL;
  // add user data argument
  end_sim_cb_data.user_data = (PLI_BYTE8 *)p_index;
  callback_handle = vpi_register_cb(&end_sim_cb_data);
  vpi_free_object(callback_handle);
  
  return 0;
}

//******************************************************************************
/// @brief  setup_tcp_server_calltf is the callback for the setup_tcp_server function.
//******************************************************************************
PLI_INT32 setup_tcp_server_calltf(PLI_BYTE8 *user_data)
{
  (void)user_data;

  return 0;
}

//******************************************************************************
/// @brief Setup recv_tcp_server function
//******************************************************************************
void recv_tcp_server_reg_systf(void)
{
  s_vpi_systf_data s_systf_data;

  s_systf_data.type      = vpiSysFunc;
  s_systf_data.tfname    = RECV_NAME;
  s_systf_data.calltf    = recv_tcp_server_calltf;
  s_systf_data.compiletf = recv_tcp_server_compiletf;
  s_systf_data.sizetf    = tcp_server_sizetf;
  s_systf_data.user_data = 0;

  vpi_register_systf(&s_systf_data);
}

//******************************************************************************
/// @brief Setup send_tcp_server function
//******************************************************************************
void send_tcp_server_reg_systf(void)
{
  s_vpi_systf_data s_systf_data;

  s_systf_data.type      = vpiSysFunc;
  s_systf_data.tfname    = SEND_NAME;
  s_systf_data.calltf    = send_tcp_server_calltf;
  s_systf_data.compiletf = send_tcp_server_compiletf;
  s_systf_data.sizetf    = tcp_server_sizetf;
  s_systf_data.user_data = 0;

  vpi_register_systf(&s_systf_data);
}

//******************************************************************************
/// @brief Setup setup_tcp_server function
//******************************************************************************
void setup_tcp_server_reg_systf(void)
{
  s_vpi_systf_data s_systf_data;

  s_systf_data.type      = vpiSysTask;
  s_systf_data.tfname    = SETUP_NAME;
  s_systf_data.calltf    = setup_tcp_server_calltf;
  s_systf_data.compiletf = setup_tcp_server_compiletf;
  s_systf_data.sizetf    = 0;
  s_systf_data.user_data = 0;

  vpi_register_systf(&s_systf_data);
}

//******************************************************************************
/// @brief register the new file functions
//******************************************************************************
void (*vlog_startup_routines[])(void) = {
  recv_tcp_server_reg_systf,
  send_tcp_server_reg_systf,
  setup_tcp_server_reg_systf,
  0
};
