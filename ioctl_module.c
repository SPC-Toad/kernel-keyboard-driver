/*
 *  ioctl test module -- Rich West.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h> /* error codes */
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/irqnr.h>
#include <linux/irq.h>

MODULE_LICENSE("GPL");

/* attribute structures */
struct ioctl_test_t {
  int field1;
  char field2;
};

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)

#define KEYBOARD_TEST _IOR(0, 7, char)

static inline unsigned char inb( unsigned short usPort ) {
    unsigned char uch;
   
    asm volatile( "inb %1,%0":"=a"(uch):"Nd"(usPort));
    return uch;
}

static inline void outb( unsigned char uch, unsigned short usPort ) {
    asm volatile("outb %0,%1"::"a"(uch),"Nd"(usPort));
}

static irqreturn_t irq_handler(int irq, void *dev_id);

static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations pseudo_dev_proc_operations;

static struct proc_dir_entry *proc_entry;

static struct proc_dir_entry *proc_keyboard_entry;

static unsigned char key_buffer;
static int key_buffer_full = 0;
static wait_queue_head_t wait_queue;
static spinlock_t buffer_lock;
static int dev_id = 0;


static int __init initialization_routine(void) {
  int ret;
  printk("<1> Loading module\n");
  init_waitqueue_head(&wait_queue);
  spin_lock_init(&buffer_lock);

  // typedef struct irq_desc *(*irq_to_desc_func_t)(unsigned int);

  // irq_to_desc_func_t irq_to_desc_ptr = (irq_to_desc_func_t)0xc104728c;
  // struct irq_desc *desc = irq_to_desc_ptr(1);

  // printk("<1> Handler: %pS\n", desc->action ? desc->action->handler : NULL);
  // printk(KERN_INFO "Affinity: %*pbl\n", cpumask_pr_args(desc->irq_common_data.affinity));

  // Keyboard request_irq(1)
  disable_irq(1);
  free_irq(1, NULL);
  
  ret = request_irq(1, irq_handler, IRQF_SHARED, "my_keyboard_handler", &dev_id);
  if (ret) {
      printk(KERN_ERR "Failed to register IRQ handler. Error %d\n", ret);
      return ret;
  }

  
  pseudo_dev_proc_operations.ioctl = pseudo_device_ioctl;

  /* Start create proc entry */
  // This is for ioctl_test
  proc_entry = create_proc_entry("ioctl_test", 0444, NULL);
  if(!proc_entry)
  {
    printk("<1> Error creating /proc entry.\n");
    return 1;
  }
  
  // This is for the keyboard_test
  proc_keyboard_entry = create_proc_entry("keyboard_test", 0444, NULL);
  if(!proc_keyboard_entry)
  {
    printk("<1> Error creating /proc entry.\n");
    return 1;
  }

  //proc_entry->owner = THIS_MODULE; <-- This is now deprecated
  proc_entry->proc_fops = &pseudo_dev_proc_operations;
  proc_keyboard_entry->proc_fops = &pseudo_dev_proc_operations;

  enable_irq(1);

  // disable_ioctl_queue for disabling keyboard driver
  printk("<1> Loading successful\n");
  return 0;
}

/* 'printk' version that prints to active tty. */
void my_printk(char *string)
{
  struct tty_struct *my_tty;

  my_tty = current->signal->tty;

  if (my_tty != NULL) {
    (*my_tty->driver->ops->write)(my_tty, string, strlen(string));
    (*my_tty->driver->ops->write)(my_tty, "\015\012", 2);
  }
} 

static void __exit cleanup_routine(void) {

  printk("<1> Dumping module\n"); 
  remove_proc_entry("ioctl_test", NULL);
  remove_proc_entry("keyboard_test", NULL);
  printk("<1> Dumped module\n"); 
  free_irq(1, &dev_id);
  return;
}

static irqreturn_t irq_handler(int irq, void *dev_id)
{
  unsigned char status;
  unsigned char scancode_value;
  status = inb(0x64);
  if (!(status & 0x1)) {
      scancode_value = inb(0x60);
      if (!(scancode_value & 0x80)) {
          // scancode_value = (scancode_value & 0x7F);
          spin_lock(&buffer_lock);
          key_buffer = scancode_value;
          key_buffer_full = 1;
          spin_unlock(&buffer_lock);

          wake_up_interruptible(&wait_queue);
      }
  }
  return IRQ_HANDLED;
}

/*
  wait.h

  wait_queue_interruptable 


*/


/***
 * ioctl() entry point...
 */
static int pseudo_device_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
  struct ioctl_test_t ioc;
  unsigned char ch;
  int ret;
  
  switch (cmd){
    case IOCTL_TEST:
      copy_from_user(&ioc, (struct ioctl_test_t *)arg, sizeof(struct ioctl_test_t));
      printk("<1> ioctl: call to IOCTL_TEST (%d,%c)!\n", ioc.field1, ioc.field2);
      my_printk ("Got msg in kernel\n");
      break;
    // Add a case for test_keyboard.c
    case KEYBOARD_TEST:
      // add_to_wait_queue(event)
      // Make the interrupt handle 
        // Wait queue here
        // And get the char 
        // End the wait queue
        // Return the char
      // Hijack the keyboard? Look at "cat /proc/kallsym"
      // Wait until a key is available
      ret = wait_event_interruptible(wait_queue, key_buffer_full);
      if (ret == -ERESTARTSYS) {
          printk("<1> wait_event_interruptible was interrupted by a signal\n");
          return -ERESTARTSYS;
      }
      // Get the character
      spin_lock(&buffer_lock);
      ch = key_buffer;
      key_buffer_full = 0; // Clear the buffer
      spin_unlock(&buffer_lock);
      printk("<1> this is kernel character %02x\n", ch);
      if (copy_to_user((char *)arg, &ch, sizeof(unsigned char))) {
        return -EFAULT; 
      }
      printk("<1> ioctl: call to KEYBOARD_TEST, char = %c!\n", ch);
      
      break;
  default:
    return -EINVAL;
    break;
  }
  
  return 0;
}

module_init(initialization_routine); 
module_exit(cleanup_routine); 