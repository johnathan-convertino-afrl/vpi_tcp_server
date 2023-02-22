//******************************************************************************
/// @file   read_binary_file.c
/// @author Jay Convertino(johnathan.convertino.1@us.af.mil)
/// @date   2022-12-19
/// @brief  Functions to read raw binary files properly in verilog.
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

//******************************************************************************
/// @brief READ BINARY FILE THREAD TO FILL RINGBUFFER
//******************************************************************************
void *read_thread(void *data)
{
  PLI_UBYTE8 all_ones = ~0;
  
  PLI_UBYTE8 *p_file_buffer = NULL;
  
  s_vpi_vecval *p_vecval_buffer = NULL;
  
  struct s_process_data *ps_process_data  = NULL;
  
  ps_process_data = (struct s_process_data *)data;
  
  if(!ps_process_data)
  {
    vpi_printf("ERROR: read_thread received a NULL pointer for the data argument\n");
    
    return NULL;
  }
  
  p_file_buffer = malloc(DATACHUNK);
  
  if(!p_file_buffer)
  {
    vpi_printf("ERROR: Could not allocate read_thread buffer.\n");
    
    return NULL;
  }
  
  p_vecval_buffer = malloc(DATACHUNK * sizeof(s_vpi_vecval));
  
  if(!p_vecval_buffer)
  {    
    vpi_printf("ERROR: Could not allocate read_thread buffer.\n");
    
    free(p_file_buffer);
    
    return NULL;
  }
  
  do
  {
    int index           = 0;
    int num_byte_z      = 0;
    int num_byte_read   = 0;
    int num_elem_wrote  = 0;
    
    PLI_INT32 num_ab_pairs_read = 0;
    
    num_byte_read = fread(p_file_buffer, sizeof(*p_file_buffer), DATACHUNK, ps_process_data->p_file);
    
    memset(p_vecval_buffer, 0, DATACHUNK * sizeof(s_vpi_vecval));
    
    num_ab_pairs_read = (((num_byte_read-1)/ps_process_data->array_byte_size) + 1) * ps_process_data->num_ab_val_pairs;
    
    //set aval/bval values up to the number bytes read
    for(index = 0; index < (num_byte_read + num_byte_z); index++)
    {
      if(ps_process_data->array_byte_size <= (int)(index%(ps_process_data->num_ab_val_pairs*sizeof(PLI_INT32))))
      {
        num_byte_z++;
        
        p_vecval_buffer[index/sizeof(PLI_INT32)].bval |= all_ones << (index%(sizeof(PLI_INT32)) * 8);
      }
      else
      {
        p_vecval_buffer[index/sizeof(PLI_INT32)].aval |= p_file_buffer[index - num_byte_z] << (PLI_INT32)(index%(sizeof(PLI_INT32)) * 8);
      }
    }

    // set any extra bytes for the aval/bval pairs to z
    for(; index < (int)(num_ab_pairs_read * sizeof(PLI_INT32)); index++)
    {
      p_vecval_buffer[index/sizeof(PLI_INT32)].bval |= all_ones << (index%(sizeof(PLI_INT32)) * 8);
    }
    
    do
    {
      num_elem_wrote += ringBufferBlockingWrite(ps_process_data->p_ringbuffer, p_vecval_buffer + num_elem_wrote, num_ab_pairs_read - num_elem_wrote, NULL);
    } while(num_elem_wrote < num_ab_pairs_read);
    
    if(feof(ps_process_data->p_file))
    {
      ringBufferEndBlocking(ps_process_data->p_ringbuffer);
    }
    
  } while(ringBufferStillBlocking(ps_process_data->p_ringbuffer));
  
  free(p_file_buffer);
  
  free(p_vecval_buffer);
  
  return NULL;
}

//******************************************************************************
/// @brief READ BINARY FILE START SIM CALLBACK
//******************************************************************************
PLI_INT32 read_binary_start_sim_cb(p_cb_data data)
{
  int error = 0;
  
  struct s_process_data *ps_process_data  = NULL;
  
  ps_process_data = (struct s_process_data *)data->user_data;
  
  vpi_put_userdata(ps_process_data->systf_handle, (void *)ps_process_data);
  
  ps_process_data->p_file = fopen(ps_process_data->p_file_name, "rb");
  
  if(!ps_process_data->p_file)
  {
    vpi_printf("ERROR: $read_binary_file could not open file %s .\n", ps_process_data->p_file_name);
    
    ps_process_data->error = 1;
    
    vpi_control(vpiFinish, 1);
    
    return 0;
  }
  
  error = pthread_create(&ps_process_data->thread, NULL, read_thread, ps_process_data);
  
  ps_process_data->error = error;
  
  if(error)
  {
    vpi_printf("ERROR: $read_binary_file failed to create thread for file %s.\n", ps_process_data->p_file_name);
    
    vpi_control(vpiFinish, 1);
    
    return 0;
  }
  
  return 0;
}

//******************************************************************************
/// @brief  Called by the simulator, each time it is requested.
///         TODO
//******************************************************************************
PLI_INT32 read_binary_calltf(PLI_BYTE8 *user_data)
{
  (void)user_data;

  int num_pairs_read = 0;
  int num_bytes_read = 0;
  int index = 0;
  
  PLI_UBYTE8 all_ones = ~0;
  
  struct s_process_data *ps_process_data  = NULL;
  
  s_vpi_value return_value; 
  s_vpi_value vector_value;

  vpiHandle systf_handle;

  // Obtain a handle to the argument list
  systf_handle = vpi_handle(vpiSysTfCall, NULL);
  
  ps_process_data = (struct s_process_data *)vpi_get_userdata(systf_handle);
  
  if(!ps_process_data) return 0;
  
  s_vpi_vecval vecval_buffer[ps_process_data->num_ab_val_pairs];
  
  memset(&vecval_buffer, 0, sizeof(s_vpi_vecval) * ps_process_data->num_ab_val_pairs);
  
  num_pairs_read = ringBufferBlockingRead(ps_process_data->p_ringbuffer, &vecval_buffer, ps_process_data->num_ab_val_pairs, NULL);
  
  for(index = num_pairs_read; index < ps_process_data->num_ab_val_pairs; index++)
  {
    vecval_buffer[index].bval = ~0;
  }
  
  vector_value.format = vpiVectorVal;
  
  vector_value.value.vector = vecval_buffer;
  
  vpi_put_value(ps_process_data->arg2_handle, &vector_value, NULL, vpiNoDelay);
  
  return_value.format = vpiIntVal;
  
  for(index = 0; index < ps_process_data->array_byte_size; index++)
  {
     if(!(vecval_buffer[index/sizeof(PLI_INT32)].bval & (all_ones << (index%(sizeof(PLI_INT32)) * 8)))) num_bytes_read++;
  }
  
  return_value.value.integer = num_bytes_read;
  
  if(ringBufferStillBlocking(ps_process_data->p_ringbuffer) || ringBufferIsEmpty(ps_process_data->p_ringbuffer))
  {
    return_value.value.integer *= -1;
  }
  
  vpi_put_value(systf_handle, &return_value, NULL, vpiNoDelay); 
  
  return 0;
}
