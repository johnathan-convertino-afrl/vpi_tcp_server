# VPI TCP server
### TCP server functions for VPI
---

   author: Jay Convertino   
   
   date: 2024.02.24
   
   details: TCP server functions for VPI VPI. This is threaded and does all recv/send IO outside of the simulation function call.
   
   license: MIT   
   
---

### Version
#### Current
  - V1.0.0 - initial release

#### Previous
  - none

### IP USAGE
#### INSTRUCTIONS

This library provides three functions.
* setup_tcp_server(ADDRESS, PORT)
  * RETURNS File Descriptor (FD)
* recv_tcp_server(FD, VECTOR)
  * RETURNS number of bytes received (non-blocking, 0 is nothing available)
* send_tcp_server(FD, VECTOR)
  * RETURNS number of bytes send (non-blocking, 0 is nothing written)

Library supports up to 256 TCP server instances. Each instance is setup by
setup_tcp_server. This returns a descriptor for that instance. Then that descriptor
is used with recv_tcp_server to obtain data received by the server, or used with
send_tcp_server to provide data to the server to send. Same descriptor can be used
for both functions. Testbench is a simple loopback test of two servers.

##### Dependency include for fusesoc core file
``` 
  dep_vpi:
    depend:
      - AFRL:vpi:tcp_server:1.0.0
      
targets:
  default: &default
    filesets: [src, dep, dep_vpi]
```

### COMPONENTS
#### SRC

* tcp_server.c
* tcp_server.h
* tcp_server.sft
* recv_tcp_server.c
* recv_tcp_server.h
* send_tcp_server.c
* send_tcp_server.h
  
#### TB

* tb_vpi.v

### fusesoc

* fusesoc_info.core created.
* Simulation uses icarus to run data through the core.

#### TARGETS

* RUN WITH: (fusesoc run --target=sim VENDER:CORE:NAME:VERSION)
  - default (for IP integration builds)
  - sim
