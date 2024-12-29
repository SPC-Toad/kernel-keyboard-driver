Sangyun Kim 
Primer PS/2 Keyboard driver 
09/26/24

IMPORTANT!

To build, run:
```sh 
$ make
```
To clean:
```sh
$ make clean
```

This is interrupt driven
This does not disable the native keyboard. 
This captures the string if you press ctrl+r, then output key capture if ctrl+p. Shift doesnt work properly(works like caps)

keyboard_test.c is the user level program
ioctl_module.c is the kernel level program, which later will become a "*.ko" file. 

In order to load the kernel module into the kernel, you need to use:
```sh
    $ insmod ioctl_module.ko
```

In order to test how it works, you need to run the executable file:
```sh
    $ ./keyboard_test
```

To remove or dump the kernel module we added, you need to use:
```sh
    $ rmmod ioctl_module # do not ad ".ko" at the end.
```

Improvements:
- I think instead of using wait queue, just use while(flag); in the ioctl_module's pseudo_device_ioctl.
- dont have to use wait_event_header

- Also disabling the original ps/2 keyboard driver use irq_to_desc. Note you need to kallsyms and find the vma pointer to the  irq_to_desc. Then you need to call it by using assembly or c language to get the ps/2 native keyboard driver. 
Check out:
https://elixir.bootlin.com/linux/v2.6.33.2/source/include/linux/interrupt.h#L95 
https://elixir.bootlin.com/linux/v2.6.33.2/source/include/linux/irqnr.h
https://wiki.osdev.org/PS/2_Keyboard 
https://wiki.osdev.org/%228042%22_PS/2_Controller#Data_Port
https://tldp.org/LDP/lkmpg/2.6/html/x1256.html
- Better naming convension.
