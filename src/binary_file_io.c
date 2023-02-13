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

#include "binary_file_io.h"
#include "read_binary_file.h"
#include "write_binary_file.h"

//******************************************************************************
/// @brief BINARY FILE END COMPILE CALLBACK
//******************************************************************************
PLI_INT32 binary_end_compile_cb(p_cb_data data)
{
  struct s_process_data *ps_process_data  = NULL;
  
  ps_process_data = (struct s_process_data *)data->user_data;
  
  ps_process_data->p_ringbuffer = initRingBuffer(BUFFSIZE, sizeof(s_vpi_vecval));
  
  if(!ps_process_data->p_ringbuffer)
  {
    vpi_printf("ERROR: %s could not create ringbuffer\n", vpi_get_str(vpiName, ps_process_data->systf_handle));
    
    vpi_control(vpiFinish, 1);
    
    return 0;
  }
  
  return 0;
}

//******************************************************************************
/// @brief BINARY FILE END SIM CALLBACK
//******************************************************************************
PLI_INT32 binary_end_sim_cb(p_cb_data data)
{
  struct s_process_data *ps_process_data  = NULL;
  
  ps_process_data = (struct s_process_data *)data->user_data;
  
  if(ps_process_data->p_ringbuffer) ringBufferEndBlocking(ps_process_data->p_ringbuffer);
  
  if(!ps_process_data->error) pthread_join(ps_process_data->thread, NULL);
  
  if(ps_process_data->p_ringbuffer) freeRingBuffer(&ps_process_data->p_ringbuffer);
  
  if(ps_process_data->p_file) fclose(ps_process_data->p_file);
  
  free(ps_process_data);
  
  return 0;
}

//******************************************************************************
/// @brief  Returns the size, in bits, of the function return type.
//******************************************************************************
PLI_INT32 binary_sizetf(PLI_BYTE8 *user_data)
{
  (void)user_data;
  
  return 32;
}

//******************************************************************************
/// @brief  Compile time call, check the arguments for validity.
//******************************************************************************
PLI_INT32 binary_compiletf(PLI_BYTE8 *user_data)
{
  // keep compiler from whining
  (void)user_data;
  
  //variables
  PLI_INT32 arg_type = 0;
  PLI_INT32 vector_size;
  PLI_INT32 array_byte_size;
  PLI_INT32 num_ab_val_pairs;
  
  char *p_file_name = NULL;
  
  PLI_BYTE8 *p_func_name = NULL;
  
  struct s_process_data *ps_process_data = NULL;
  
  s_vpi_value file_name;
  
  // vpi_register_cb copies the data in these structs per the verilog-2001 standard. cute.
  s_cb_data end_cmp_cb_data;
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
  
  arg_type = vpi_get(vpiConstType, arg1_handle);
  
  if(arg_type != vpiStringConst)
  {
      vpi_printf("ERROR: %s requires first argument be a string.\n", p_func_name);
      
      vpi_free_object(arg_iterate);
      
      vpi_control(vpiFinish, 1);
      
      return 0;
  }
  
  file_name.format = vpiStringVal;
  
  vpi_get_value(arg1_handle, &file_name);

  // check for a second argument, this should return a vector.
  arg2_handle = vpi_scan(arg_iterate);
  
  arg_type = vpi_get(vpiType, arg2_handle);

  if(arg_type != vpiReg)
  switch(arg_type)
  {
    case vpiNet:
    case vpiReg:
      break;
    default:
      vpi_printf("ERROR: %s require a reg or net type for the vector argument.\n", p_func_name);
    
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
  
  // arguments verified, allocate memory for callback structs and such.
  ps_process_data  = malloc(sizeof(struct s_process_data));
  
  if(!ps_process_data)
  {
    vpi_printf("ERROR: %s for file %s, could not allocate argument struct memory.\n", p_func_name, file_name.value.str);
    
    vpi_control(vpiFinish, 1);
    
    return 0;
  }
  
  memset(ps_process_data, 0, sizeof(struct s_process_data));
  
  // get argument size and create buffer for storage
  vector_size = vpi_get(vpiSize, arg2_handle);
  
  if((vector_size%8) != 0)
  {
    vpi_printf("ERROR: %s for file %s, has to have a vector that is some number of bytes.\n", p_func_name, file_name.value.str);
    
    vpi_control(vpiFinish, 1);
    
    return 0;
  }
  
  array_byte_size = ((vector_size-1)/8 + 1);
  
  num_ab_val_pairs = ((array_byte_size-1)/sizeof(PLI_INT32) + 1);
  
  p_file_name = strdup(file_name.value.str);
  
  if(!p_file_name)
  {
    vpi_printf("ERROR: %s could not allocate space for file name %s.\n", p_func_name, file_name.value.str);

    free(ps_process_data);
    
    vpi_control(vpiFinish, 1);
    
    return 0;
  }
  
  ps_process_data->p_file_name     = p_file_name;
  ps_process_data->arg2_handle     = arg2_handle;
  ps_process_data->systf_handle    = systf_handle;
  ps_process_data->num_ab_val_pairs= num_ab_val_pairs;
  ps_process_data->array_byte_size = array_byte_size;
  
  // setup callback for end of compile, well before it starts.
  end_cmp_cb_data.reason    = cbEndOfCompile;
  end_cmp_cb_data.cb_rtn    = binary_end_compile_cb;
  end_cmp_cb_data.obj       = NULL;
  end_cmp_cb_data.time      = NULL;
  end_cmp_cb_data.value     = NULL;
  // add user data argument struct
  end_cmp_cb_data.user_data = (PLI_BYTE8 *)ps_process_data;
  callback_handle = vpi_register_cb(&end_cmp_cb_data);
  vpi_free_object(callback_handle);
               
  // setup callback for start of simulation, well before it starts.
  start_sim_cb_data.reason    = cbStartOfSimulation;
  
  if(!strcmp(vpi_get_str(vpiName, ps_process_data->systf_handle), WRITE_NAME))
  {
    start_sim_cb_data.cb_rtn    = write_binary_start_sim_cb;
  }
  else
  {
    start_sim_cb_data.cb_rtn    = read_binary_start_sim_cb;
  }
  
  start_sim_cb_data.obj       = NULL;
  start_sim_cb_data.time      = NULL;
  start_sim_cb_data.value     = NULL;
  // add user data argument struct
  start_sim_cb_data.user_data = (PLI_BYTE8 *)ps_process_data;
  callback_handle = vpi_register_cb(&start_sim_cb_data);
  vpi_free_object(callback_handle);
  
  // setup callback for end of simulation, well before it starts.
  end_sim_cb_data.reason    = cbEndOfSimulation;
  end_sim_cb_data.cb_rtn    = binary_end_sim_cb;
  end_sim_cb_data.obj       = NULL;
  end_sim_cb_data.time      = NULL;
  end_sim_cb_data.value     = NULL;
  // add user data argument
  end_sim_cb_data.user_data = (PLI_BYTE8 *)ps_process_data;
  callback_handle = vpi_register_cb(&end_sim_cb_data);
  vpi_free_object(callback_handle);
  
  return 0;
}

//******************************************************************************
/// @brief Setup read_binary_file function
//******************************************************************************
void read_binary_reg_systf(void)
{
  s_vpi_systf_data s_systf_data;

  s_systf_data.type      = vpiSysFunc;  
  s_systf_data.tfname    = READ_NAME;
  s_systf_data.calltf    = read_binary_calltf;
  s_systf_data.compiletf = binary_compiletf;
  s_systf_data.sizetf    = binary_sizetf;
  s_systf_data.user_data = 0;
  
  vpi_register_systf(&s_systf_data);
}

//******************************************************************************
/// @brief Setup write_binary_file function
//******************************************************************************
void write_binary_reg_systf(void)
{
  s_vpi_systf_data s_systf_data;

  s_systf_data.type      = vpiSysFunc;  
  s_systf_data.tfname    = WRITE_NAME;
  s_systf_data.calltf    = write_binary_calltf;
  s_systf_data.compiletf = binary_compiletf;
  s_systf_data.sizetf    = binary_sizetf;
  s_systf_data.user_data = 0;
  
  vpi_register_systf(&s_systf_data);
}

//******************************************************************************
/// @brief register the new file functions
//******************************************************************************
void (*vlog_startup_routines[])(void) = {
  read_binary_reg_systf,
  write_binary_reg_systf,
  0
};
