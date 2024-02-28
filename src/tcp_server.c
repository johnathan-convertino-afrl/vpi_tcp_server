//******************************************************************************
/// @file   tcp_server.h
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
#include "tcp_server.h"

unsigned int g_num_of_connections = 0;

struct s_send_tcp_server g_send_tcp_server[MAX_CONNECTIONS];

// PRIVATE FUNCTIONS //
void* connection_keep_alive(void *p_data);

//******************************************************************************
/// @brief SETUP TCP SERVER DATA START SIM CALLBACK
//******************************************************************************
PLI_INT32 setup_tcp_server_start_sim_cb(p_cb_data data)
{
  int *p_index = NULL;

  p_index = (int *)data->user_data;

  vpi_put_userdata(g_send_tcp_server[*p_index].systf_handle, (void *)p_index);

  // open socket discriptor for client/server
  g_send_tcp_server[*p_index].p_socket_info = malloc(sizeof(struct sockaddr_in));

  if(!g_send_tcp_server[*p_index].p_socket_info)
  {
    vpi_printf("ERROR: TCP SERVER, sockaddr_in llocation Issue\n");

    vpi_control(vpiFinish, 1);

    return ~0;
  }

  g_send_tcp_server[*p_index].kill_thread = 0;

  g_send_tcp_server[*p_index].p_socket_info->sin_family = AF_INET;

  g_send_tcp_server[*p_index].p_socket_info->sin_port = htons(g_send_tcp_server[*p_index].port);

  g_send_tcp_server[*p_index].p_socket_info->sin_addr.s_addr = inet_addr(g_send_tcp_server[*p_index].p_address);

  //launch keep_alive_connection thread to connect and keep connection alive.
  pthread_create(&g_send_tcp_server[*p_index].connection_thread, NULL, connection_keep_alive, p_index);

  return 0;
}

//******************************************************************************
/// @brief SETUP TCP SERVER END SIM CALLBACK
//******************************************************************************
PLI_INT32 setup_tcp_server_end_sim_cb(p_cb_data data)
{
  int *p_index = NULL;

  p_index = (int *)data->user_data;

  g_send_tcp_server[*p_index].kill_thread = 1;

  if(!g_send_tcp_server[*p_index].p_socket_info)
  {
    pthread_join(g_send_tcp_server[*p_index].connection_thread, NULL);

    free(g_send_tcp_server[*p_index].p_socket_info);
  }

  g_send_tcp_server[*p_index].p_socket_info = NULL;

  free(g_send_tcp_server[*p_index].p_address);

  free(p_index);

  g_num_of_connections--;

  return 0;
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

  p_index = malloc(sizeof(int));

  if(!p_index)
  {
    vpi_printf("ERROR: %s malloc failed for index\n", p_func_name);

    vpi_control(vpiFinish, 1);

    return 0;
  }

  *p_index = g_num_of_connections;

  g_num_of_connections++;

  vpi_printf("TCP SERVER ADDRESS: %s PORT: %d\n", address.value.str, port.value.integer);

  g_send_tcp_server[*p_index].p_address = strdup(address.value.str);
  g_send_tcp_server[*p_index].port = port.value.integer;
  g_send_tcp_server[*p_index].systf_handle = systf_handle;
  g_send_tcp_server[*p_index].p_socket_info = NULL;

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

  int *p_index = NULL;

  s_vpi_value return_value;

  vpiHandle systf_handle;

  // Obtain a handle to the argument list
  systf_handle = vpi_handle(vpiSysTfCall, NULL);

  p_index = (int *)vpi_get_userdata(systf_handle);

  if(!p_index) return -1;

  return_value.format = vpiIntVal;

  return_value.value.integer = *p_index;

  vpi_put_value(systf_handle, &return_value, NULL, vpiNoDelay);

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

  s_systf_data.type      = vpiSysFunc;
  s_systf_data.tfname    = SETUP_NAME;
  s_systf_data.calltf    = setup_tcp_server_calltf;
  s_systf_data.compiletf = setup_tcp_server_compiletf;
  s_systf_data.sizetf    = tcp_server_sizetf;
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

void* connection_keep_alive(void *p_data)
{
  int error           = 0;
  int prev_revents    = 0;
  unsigned socket_len = 0;

  int *p_index = NULL;

  char p_buffer[16];

  p_index = (int *)p_data;

  // vpi_printf("INDEX IS: %d\n", *p_index);

  if(!p_index)
  {
    vpi_printf("ERROR: TCP SERVER, get index.\n");

    g_send_tcp_server[*p_index].kill_thread = 1;

    vpi_control(vpiFinish, 1);

    return NULL;
  }

  struct pollfd poll_socket;

  struct sockaddr_in client_socket_info;

  poll_socket.fd = socket(g_send_tcp_server[*p_index].p_socket_info->sin_family, SOCK_STREAM || SOCK_NONBLOCK, 0);

  if(poll_socket.fd == -1)
  {
    vpi_printf("ERROR: TCP SERVER, Failed to create socket\n");

    g_send_tcp_server[*p_index].kill_thread = 1;

    vpi_control(vpiFinish, 1);

    return NULL;
  }

  error = bind(poll_socket.fd, (struct sockaddr *)g_send_tcp_server[*p_index].p_socket_info, sizeof(struct sockaddr_in));

  if(error == -1)
  {
    vpi_printf("ERROR: TCP SERVER, Failed to bind\n");

    close(poll_socket.fd);

    g_send_tcp_server[*p_index].kill_thread = 1;

    vpi_control(vpiFinish, 1);

    return NULL;
  }

  error = listen(poll_socket.fd, 1);

  if(error == -1)
  {
    vpi_printf("ERROR: TCP SERVER, Failed to listen\n");

    close(poll_socket.fd);

    g_send_tcp_server[*p_index].kill_thread = 1;

    vpi_control(vpiFinish, 1);

    return NULL;
  }

  poll_socket.events = POLLIN;

  vpi_printf("TCP SERVER STARTED\n");

  vpi_printf("TCP SERVER WAITING FOR CLIENT\n");

  do
  {
    error = poll(&poll_socket, 1, 0);

    if(error < 0)
    {
      vpi_printf("ERROR: TCP SERVER, Poll failed\n");

      g_send_tcp_server[*p_index].kill_thread = 1;

      vpi_control(vpiFinish, 1);

      continue;
    }

    if(error == 0) continue;

    if(poll_socket.revents == POLLIN)
    {
      g_send_tcp_server[*p_index].poll_connection.fd = accept(poll_socket.fd, (struct sockaddr *)&client_socket_info, &socket_len);

      if(g_send_tcp_server[*p_index].poll_connection.fd < 0)
      {
        vpi_printf("ERROR: TCP SERVER, Accept failed\n");

        g_send_tcp_server[*p_index].kill_thread = 1;

        vpi_control(vpiFinish, 1);

        continue;
      }

      g_send_tcp_server[*p_index].poll_connection.events = POLLIN | POLLOUT;
    }

    vpi_printf("TCP CLIENT CONNECTED TO %s : %d\n", g_send_tcp_server[*p_index].p_address, g_send_tcp_server[*p_index].port);

    prev_revents = g_send_tcp_server[*p_index].poll_connection.revents;

    // while connected, just wait till dissconnect or kill_thread
    for(;;)
    {
      int num_bytes_read = 0;

      error = poll(&g_send_tcp_server[*p_index].poll_connection, 1, 0);

      if(error <= 0) continue;

      if(g_send_tcp_server[*p_index].poll_connection.revents & POLLHUP) break;
      if(g_send_tcp_server[*p_index].poll_connection.revents & POLLERR) break;
      if(g_send_tcp_server[*p_index].kill_thread) break;

      if(prev_revents != g_send_tcp_server[*p_index].poll_connection.revents)
      {
        prev_revents = g_send_tcp_server[*p_index].poll_connection.revents;

        num_bytes_read = recv(g_send_tcp_server[*p_index].poll_connection.fd, p_buffer, 16, MSG_PEEK);

        if(num_bytes_read <= 0) break;
      }
    };

    vpi_printf("TCP SERVER DISCONNECTED\n");

    close(g_send_tcp_server[*p_index].poll_connection.fd);

    if(!g_send_tcp_server[*p_index].kill_thread) vpi_printf("TCP SERVER WAITING FOR CLIENT\n");
  }
  while(!g_send_tcp_server[*p_index].kill_thread);

  vpi_printf("TCP SERVER SHUTTING DOWN\n");

  close(poll_socket.fd);

  return NULL;
}
