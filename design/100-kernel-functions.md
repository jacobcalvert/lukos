# LuKOS Kernel Functions
This microkernel will provide the classic bare minimum functions of a microkernel, plus some supporting functions which will be described later. 

## Primary Functions
### Process Management (PM)
The kernel will provide the capability to start a process in a virtual memory space and maintain the processes's threads of execution. It will also provide scheduling to these processes and threads. The kernel will provide a mechanism to duplicate processes with all their thread states. See the [Process Management](120-process-model.md) section.

### Virtual Memory Management (VMM)
The kernel will provide the capability to map/unmap virtual memory to support processes and other services in userland. See the [Virtual Memory](110-virtual-memory.md) section.

### Inter-Process Communication Management(IPCM)
The kernel will provide the capability to communicate between processes in different virtual memory address spaces. See the [IPC Model](130-ipc-model.md) section.

### Interrupt Management (INTM)
The kernel will provide the capability to register for, receive, acknowledge, and deliver interrupts to processes. See the [Interrupt Management](140-interrupt-management.md) section. 

