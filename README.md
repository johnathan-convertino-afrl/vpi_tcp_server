# VPI binary file io
### read and write binary functions for VPI
---

   author: Jay Convertino   
   
   date: 2023.01.01  
   
   details: Read and write binary file functions for VPI. This is threaded and does all file IO outside of the simulation function call.   
   
   license: MIT   
   
---

### Version
#### Current
  - V1.0.0 - initial release

#### Previous
  - none

### IP USAGE
#### INSTRUCTIONS

This library provides two functions.  
* read_binary_file(FILE_NAME, VECTOR)
* write_binary_file(FILE_NAME, VECTOR)

Each instance is a new instance, and will start reading the file from the start.  
The vector has to be in size bytes from 1 to any number of bytes. Each function  
returns the number of bytes read or writen. Z or X place in the vector indicates
bytes not available for read, or do not write these bytes for write. The read funciton
will return a negative number of bytes when the end of file is reached.

##### Dependency include for fusesoc core file
``` 
  dep_vpi:
    depend:
      - AFRL:vpi:binary_file_io:1.0.0
      
targets:
  default: &default
    filesets: [src, dep, dep_vpi]
```

### COMPONENTS
#### SRC

* binary_file_io.c
* binary_file_io.h
* binary_file_io.sft
* read_binary_file.c
* read_binary_file.h
* write_binary_file.c
* write_binary_file.h
  
#### TB

* tb_vpi.v
* count_test.bin
* rand_test.bin

### fusesoc

* fusesoc_info.core created.
* Simulation uses icarus to run data through the core.

#### TARGETS

* RUN WITH: (fusesoc run --target=sim VENDER:CORE:NAME:VERSION)
  - default (for IP integration builds)
  - sim
  - sim_rand_data
  - sim_8bit_count_data
