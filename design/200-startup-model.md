# LuKOS Startup Model
The startup model is intended to generalize what is needed from a board and processor support standpoint to make the kernel work.

## At Power-On
At power-on, the board and processor specific code needs to put the hardware (especially the interrupt controller and MMU) in a quiscient state so there will be no spurious interrupts and such when the kernel launches. The boot core will setup an initial translation table so the kernel can run in virtual space.

This code will then pass control to kernel main entry.  

## At Kernel Start
The bootup code will pass control to the kernel main entry by calling kernel_entry.
### Kernel Entry Flow
The kernel_entry function will call architecture specific startup code, install handlers for exceptions/interrupts, setup interrupt controllers, MMUs and the finally run the root process with a thread for each core.
### Kernel Entry Signature

```
void kernel_entry(size_t corenum);

```


