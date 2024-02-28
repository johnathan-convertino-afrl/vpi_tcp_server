//******************************************************************************
/// @file   send_tcp_server.c
/// @author Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date   2024-23-02
/// @brief  Functions for TCP server data send.
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

#include "tcp_server.h"
#include "send_tcp_server.h"

//******************************************************************************
/// @brief SEND TCP SERVER THREAD TO EMPTY RINGBUFFER
//******************************************************************************
void *send_thread(void *data)
{
  PLI_UBYTE8 *p_send_buffer = NULL;
  
  s_vpi_vecval *p_vecval_buffer = NULL;
  
  int *p_index = NULL;

  p_index = (int *)data;

  if(!p_index)
  {
    vpi_printf("ERROR: recv_thread received a NULL pointer for the data argument\n");

    vpi_control(vpiFinish, 1);

    return NULL;
  }
  
  p_send_buffer = malloc(DATACHUNK);
  
  if(!p_send_buffer)
  {
    vpi_printf("ERROR: Could not allocate send_thread buffer.\n");

    vpi_control(vpiFinish, 1);
    
    return NULL;
  }
  
  p_vecval_buffer = malloc(DATACHUNK * sizeof(s_vpi_vecval));
  
  if(!p_vecval_buffer)
  {    
    vpi_printf("ERROR: Could not allocate send_thread buffer.\n");

    vpi_control(vpiFinish, 1);
    
    free(p_send_buffer);
    
    return NULL;
  }
  
  do
  {
    int error             = 0;
    int index             = 0;
    int num_byte_z        = 0;
    int num_byte_read     = 0;
    int num_byte_wrote    = 0;
    int num_ab_pairs_read = 0;
    
    error = poll(&g_send_tcp_server[*p_index].poll_connection, 1, 0);

    if(error <= 0) continue;

    if(g_send_tcp_server[*p_index].poll_connection.revents & POLLHUP) continue;

    if(g_send_tcp_server[*p_index].poll_connection.revents & POLLERR) continue;

    if(g_send_tcp_server[*p_index].poll_connection.revents & POLLOUT)
    {
      memset(p_vecval_buffer, 0, DATACHUNK * sizeof(s_vpi_vecval));

      num_ab_pairs_read = ringBufferRead(g_send_tcp_server[*p_index].send_process_data.p_ringbuffer, p_vecval_buffer, DATACHUNK);

      if(num_ab_pairs_read <= 0) continue;

      num_byte_read = num_ab_pairs_read * sizeof(PLI_INT32);

      //extract aval/bval
      for(index = 0; index < num_byte_read; index++)
      {
        if((p_vecval_buffer[index/sizeof(PLI_INT32)].bval & (0x000000FF << (PLI_INT32)(index%(sizeof(PLI_INT32)) * 8))) || (g_send_tcp_server[*p_index].send_process_data.array_byte_size <= (int)(index%(g_send_tcp_server[*p_index].send_process_data.num_ab_val_pairs*sizeof(PLI_INT32)))))
        {
          num_byte_z++;
        }
        else
        {
          p_send_buffer[index - num_byte_z] = (PLI_UBYTE8)(p_vecval_buffer[index/sizeof(PLI_INT32)].aval >> (PLI_INT32)(index%(sizeof(PLI_INT32)) * 8));
        }
      }

      num_byte_read -= num_byte_z;

      do
      {
        num_byte_wrote += send(g_send_tcp_server[*p_index].poll_connection.fd, p_send_buffer + num_byte_wrote, (size_t)(num_byte_read - num_byte_wrote), MSG_DONTWAIT);
      } while(num_byte_read < num_byte_wrote);

      if(num_byte_wrote <= 0) continue;
    }

  } while(ringBufferStillBlocking(g_send_tcp_server[*p_index].send_process_data.p_ringbuffer) || getRingBufferReadByteSize(g_send_tcp_server[*p_index].send_process_data.p_ringbuffer));
  
  free(p_send_buffer);
  
  free(p_vecval_buffer);
  
  return NULL;
}

//******************************************************************************
/// @brief SEND TCP SERVER DATA END SIM CALLBACK
//******************************************************************************
PLI_INT32 send_tcp_server_end_sim_cb(p_cb_data data)
{
  int *p_index = NULL;

  p_index = (int *)data->user_data;

  g_send_tcp_server[*p_index].kill_thread = 1;

  if(g_send_tcp_server[*p_index].send_process_data.p_ringbuffer)
  {
    ringBufferEndBlocking(g_send_tcp_server[*p_index].send_process_data.p_ringbuffer);

    pthread_join(g_send_tcp_server[*p_index].send_process_data.thread, NULL);

    freeRingBuffer(&g_send_tcp_server[*p_index].send_process_data.p_ringbuffer);
  }

  free(p_index);

  return 0;
}

//******************************************************************************
/// @brief SEND TCP SERVER DATA START SIM CALLBACK
//******************************************************************************
PLI_INT32 send_tcp_server_start_sim_cb(p_cb_data data)
{
  int error = 0;
  int *p_index = NULL;
  s_vpi_value fd;

  p_index = (int *)data->user_data;

  vpi_put_userdata(g_send_tcp_server[*p_index].send_process_data.systf_handle, (void *)p_index);

  g_send_tcp_server[*p_index].send_process_data.p_ringbuffer = initRingBuffer(BUFFSIZE, sizeof(s_vpi_vecval));

  if(!g_send_tcp_server[*p_index].send_process_data.p_ringbuffer)
  {
    vpi_printf("ERROR: %s could not create ringbuffer\n", vpi_get_str(vpiName, g_send_tcp_server[*p_index].send_process_data.systf_handle));

    vpi_control(vpiFinish, 1);

    g_send_tcp_server[*p_index].kill_thread = 1;

    return 0;
  }

  error = pthread_create(&g_send_tcp_server[*p_index].send_process_data.thread, NULL, send_thread, p_index);

  if(error)
  {
    vpi_printf("ERROR: $send_tcp_server failed to create thread for fd %d.\n", *p_index);

    vpi_control(vpiFinish, 1);

    g_send_tcp_server[*p_index].kill_thread = 1;
  }

  return 0;
}

//******************************************************************************
/// @brief  Compile time call, check the arguments for validity.
//******************************************************************************
PLI_INT32 send_tcp_server_compiletf(PLI_BYTE8 *user_data)
{
  // keep compiler from whining
  (void)user_data;

  //variables
  int *p_index = NULL;

  PLI_INT32 arg_type = 0;
  PLI_INT32 vector_size;
  PLI_INT32 array_byte_size;
  PLI_INT32 num_ab_val_pairs;

  s_vpi_value fd;

  PLI_BYTE8 *p_func_name = NULL;

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

  // check the type of the first handle, if it doesn't match as a int, quit.
  arg1_handle = vpi_scan(arg_iterate);

  if(arg_type != arg1_handle)
  {
      vpi_printf("ERROR: %s requires two arguments.\n", p_func_name);

      vpi_free_object(arg_iterate);

      vpi_control(vpiFinish, 1);

      return 0;
  }

  arg_type = vpi_get(vpiType, arg1_handle);

  if(arg_type != vpiConstant)
  {
      vpi_printf("ERROR: %s requires first argument is a port number.\n", p_func_name);

      vpi_free_object(arg_iterate);

      vpi_control(vpiFinish, 1);

      return 0;
  }

  fd.format = vpiIntVal;

  vpi_get_value(arg1_handle, &fd);

  // check for a second argument, this should return a vector.
  arg2_handle = vpi_scan(arg_iterate);

  if(arg_type != arg2_handle)
  {
      vpi_printf("ERROR: %s requires two arguments.\n", p_func_name);

      vpi_free_object(arg_iterate);

      vpi_control(vpiFinish, 1);

      return 0;
  }

  arg_type = vpi_get(vpiType, arg2_handle);

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

  // get argument size and create buffer for storage
  vector_size = vpi_get(vpiSize, arg2_handle);

  if((vector_size%8) != 0)
  {
    vpi_printf("ERROR: %s for server, has to have a vector that is some number of bytes.\n", p_func_name);

    vpi_control(vpiFinish, 1);

    return 0;
  }


  array_byte_size = ((vector_size-1)/8 + 1);

  num_ab_val_pairs = ((array_byte_size-1)/sizeof(PLI_INT32) + 1);

  p_index = malloc(sizeof(int));

  if(!p_index)
  {
    vpi_printf("ERROR: malloc failed.\n");

    vpi_control(vpiFinish, 1);

    return 0;
  }

  *p_index = -1;

  for(int index = 0; index < MAX_CONNECTIONS; index++)
  {
    if(g_send_tcp_server[index].port == fd.value.integer)
      *p_index = index;
  }

  if(*p_index < 0)
  {
    vpi_printf("ERROR: %s has to setup_tcp_server with a matching port before call.\n", p_func_name);

    vpi_control(vpiFinish, 1);

    return 0;
  }

  g_send_tcp_server[*p_index].send_process_data.systf_handle = systf_handle;
  g_send_tcp_server[*p_index].send_process_data.arg2_handle = arg2_handle;

  g_send_tcp_server[*p_index].send_process_data.array_byte_size = array_byte_size;
  g_send_tcp_server[*p_index].send_process_data.num_ab_val_pairs = num_ab_val_pairs;

  // setup callback for start of simulation, well before it starts.
  start_sim_cb_data.reason    = cbStartOfSimulation;
  start_sim_cb_data.cb_rtn    = send_tcp_server_start_sim_cb;
  start_sim_cb_data.obj       = NULL;
  start_sim_cb_data.time      = NULL;
  start_sim_cb_data.value     = NULL;
  // add user data argument struct
  start_sim_cb_data.user_data = (PLI_BYTE8 *)p_index;
  callback_handle = vpi_register_cb(&start_sim_cb_data);
  vpi_free_object(callback_handle);

  // setup callback for end of simulation, well before it starts.
  end_sim_cb_data.reason    = cbEndOfSimulation;
  end_sim_cb_data.cb_rtn    = send_tcp_server_end_sim_cb;
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
/// @brief  Called by the simulator, each time it is requested.
//******************************************************************************
PLI_INT32 send_tcp_server_calltf(PLI_BYTE8 *user_data)
{
  (void)user_data;
  
  int index = 0;  
  int num_bytes_deleted = 0;
  int *p_index = NULL;
  
  PLI_UBYTE8 all_ones = ~0;
  
  struct t_vpi_value return_value; 
  
  s_vpi_value vector_value;

  vpiHandle systf_handle;

  // Obtain a handle to the argument list
  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  
  p_index = (int *)vpi_get_userdata(systf_handle);

  if(!p_index) return 0;
  
  vector_value.format = vpiVectorVal;
  
  vpi_get_value(g_send_tcp_server[*p_index].send_process_data.arg2_handle, &vector_value);
  
  for(index = 0; index < (int)(g_send_tcp_server[*p_index].send_process_data.num_ab_val_pairs*sizeof(PLI_INT32)); index++)
  {
    if((vector_value.value.vector[index/sizeof(PLI_INT32)].bval & (all_ones << (index%(sizeof(PLI_INT32)) * 8))))
    {
      num_bytes_deleted++;
    }
  }

  ringBufferBlockingWrite(g_send_tcp_server[*p_index].send_process_data.p_ringbuffer, vector_value.value.vector, g_send_tcp_server[*p_index].send_process_data.num_ab_val_pairs, NULL);
  
  return_value.format = vpiIntVal;
  
  return_value.value.integer = g_send_tcp_server[*p_index].send_process_data.array_byte_size - num_bytes_deleted;
  
  vpi_put_value(systf_handle, &return_value, NULL, vpiNoDelay); 
  
  return 0;
}
