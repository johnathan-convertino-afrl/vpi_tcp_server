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

#include "tcp_server.h"
#include "messages.h"

unsigned int g_num_of_connections = 0;

struct s_send_tcp_server g_send_tcp_server[MAX_CONNECTIONS];

// FUNCTIONS //
int *setup_tcp_server(char *p_address, int port);
int  start_tcp_server(int *p_index);
int  end_tcp_server(int *p_index);

// PRIVATE FUNCTIONS //
void* connection_keep_alive(void *p_data);

//******************************************************************************
/// @brief SETUP TCP SERVER
//******************************************************************************
int *setup_tcp_server(char *p_address, int port)
{
  int *p_index = NULL;

  p_index = malloc(sizeof(int));

  if(!p_index)
  {
    //vpi switchable
    return NULL;
  }

  *p_index = g_num_of_connections;

  g_send_tcp_server[*p_index].p_address = strdup(p_address);
  g_send_tcp_server[*p_index].port = port;
  g_send_tcp_server[*p_index].p_socket_info = NULL;

  // open socket discriptor for client/server
  g_send_tcp_server[*p_index].p_socket_info = malloc(sizeof(struct sockaddr_in));

  if(!g_send_tcp_server[*p_index].p_socket_info)
  {
    print_error("TCP SERVER, sockaddr_in location Issue\n");

    free(p_index);

    return NULL;
  }

  g_send_tcp_server[*p_index].kill_thread = 0;

  g_send_tcp_server[*p_index].p_socket_info->sin_family = AF_INET;

  g_send_tcp_server[*p_index].p_socket_info->sin_port = htons(port);

  g_send_tcp_server[*p_index].p_socket_info->sin_addr.s_addr = inet_addr(p_address);

  g_num_of_connections++;

  print_info("TCP SERVER ADDRESS: %s PORT: %d\n", p_address, port);

  return p_index;
}

//******************************************************************************
/// @brief  START TCP SERVER
//******************************************************************************
int start_tcp_server(int *p_index)
{
  //launch keep_alive_connection thread to connect and keep connection alive.
  pthread_create(&g_send_tcp_server[*p_index].connection_thread, NULL, connection_keep_alive, p_index);

  return 0;
}

//******************************************************************************
/// @brief END TCP SERVER
//******************************************************************************
int end_tcp_server(int *p_index)
{
  if(!p_index) return ~0;

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
    print_error("TCP SERVER, get index.\n");

    g_send_tcp_server[*p_index].kill_thread = 1;

    return NULL;
  }

  struct pollfd poll_socket;

  struct sockaddr_in client_socket_info;

  poll_socket.fd = socket(g_send_tcp_server[*p_index].p_socket_info->sin_family, SOCK_STREAM || SOCK_NONBLOCK, 0);

  if(poll_socket.fd == -1)
  {
    print_error("TCP SERVER, Failed to create socket\n");

    g_send_tcp_server[*p_index].kill_thread = 1;

    return NULL;
  }

  error = bind(poll_socket.fd, (struct sockaddr *)g_send_tcp_server[*p_index].p_socket_info, sizeof(struct sockaddr_in));

  if(error == -1)
  {
    print_error("TCP SERVER, Failed to bind\n");

    close(poll_socket.fd);

    g_send_tcp_server[*p_index].kill_thread = 1;

    return NULL;
  }

  error = listen(poll_socket.fd, 1);

  if(error == -1)
  {
    print_error("TCP SERVER, Failed to listen\n");

    close(poll_socket.fd);

    g_send_tcp_server[*p_index].kill_thread = 1;

    return NULL;
  }

  poll_socket.events = POLLIN;

  print_info("TCP SERVER STARTED\n");

  print_info("TCP SERVER WAITING FOR CLIENT\n");

  do
  {
    error = poll(&poll_socket, 1, 0);

    if(error < 0)
    {
      print_error("TCP SERVER, Poll failed\n");

      g_send_tcp_server[*p_index].kill_thread = 1;

      break;
    }

    if(error == 0) continue;

    if(poll_socket.revents == POLLIN)
    {
      g_send_tcp_server[*p_index].poll_connection.fd = accept(poll_socket.fd, (struct sockaddr *)&client_socket_info, &socket_len);

      if(g_send_tcp_server[*p_index].poll_connection.fd < 0)
      {
        print_error("TCP SERVER, Accept failed\n");

        g_send_tcp_server[*p_index].kill_thread = 1;

        break;
      }

      g_send_tcp_server[*p_index].poll_connection.events = POLLIN | POLLOUT;
    }

    print_info("TCP CLIENT CONNECTED TO %s : %d\n", g_send_tcp_server[*p_index].p_address, g_send_tcp_server[*p_index].port);

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

    print_info("TCP SERVER DISCONNECTED\n");

    close(g_send_tcp_server[*p_index].poll_connection.fd);

    if(!g_send_tcp_server[*p_index].kill_thread) print_info("TCP SERVER WAITING FOR CLIENT\n");
  }
  while(!g_send_tcp_server[*p_index].kill_thread);

  print_info("TCP SERVER SHUTTING DOWN\n");

  close(poll_socket.fd);

  return NULL;
}
