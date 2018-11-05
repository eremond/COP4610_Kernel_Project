#include <linux/module.h> // Dynamic loading of modules into the kernel.
#include <linux/init.h> // Macros for module initialization.
#include <linux/prof_fs.h> // File system structure and calls.
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <syscalls.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("An elevator simulator utilizing first-in, first-out scheduling.");

#define MODULE_NAME "elevator"
#define MODULE_PERMISSIONS 0644
#define MODULE_PARENT NULL

#define OFFLINE 0
#define IDLE 1
#define UP 2
#define DOWN 3
#define LOADING 4

extern int stop_s;
extern int mainDirection;
extern int nextDirection;
extern int currFloor;
extern int nextFloor;
extern int numPassengers;
extern int numWeight;
extern int waitPassengers;
extern int passengersServiced;
extern int numFloors = 10;
extern int passengersServFloor[numFloors];

int rp;
char * message;

struct mutex passengerQueueMutex;
struct mutex elevatorListMutex;
struct *task_struct elevator_thread;

static struct file_operations fileOperations; // Points to proc file definitions.

static int InitializeModule(void) {
    printk(KERN_NOTICE "Creating /proc/%s.\n", MODULE_NAME);

    mainDirection = OFFLINE;  // initialize mainDirection
    nextDirection = UP;
    stop_s = 1;
    currFloor = 1;
    nextFloor = 1;
    numPassengers = 0;
    numWeight = 0;
    int i;
    for(i = 0; i < 10; i++) {
      passengersServFloor[i] = 0;
    }
    initQueue();
    elevator_syscalls_create();
    mutex_init(&passengerQueueMutex);
    mutex_init(&elevatorListMutex);
    elevator_thread = kthread_run(elevatorRun, NULL, "Elevator thread");
    if(IS_ERR(elevator_thread)) {
      printk("Error: ElevatorRun\n");
      return PTR_ERR(elevator_thread);
    }
    if(!proc_create(MODULE_NAME, MODULE_PERMISSIONS, NULL, fileOperations)) {
      printk("Error: proc_create\n");
      remove_proc_entry(MODULE_NAME, NULL);
      return -ENOMEM;
    }
    return 0;
}

static int OpenModule(void) {
   printk(KERN_NOTICE "/proc/ called open\n");
   rp = 1;
   message = kmalloc(sizeof(char) * 2048, __GFP_RECLAIM, __GFP_IO, __GFP_FS);
   if(message == NULL) {
     printk("Error: open");
     return -ENOMEM;
   }
   return 0;
}

static size_t ReadModule(struct file *sp_file, char __usr *buff, size_t size, loff_t *offset) {
  printk(KERN_NOTICE "here's the elevator...");
}

static void ExitModule(void) {
    int r;
    remove_proc_entry(MODULE_NAME, NULL);
    elevator_syscalls_remove();
    printk(KERN_NOTICE "Removing /proc/%s.\n", MODULE_NAME);
    r = kthread_stop(elevator_thread);
    if(r != -EINTR) {
      prink("Elevator stopped...\n");
    }
}

module_init(InitializeModule);
module_exit(ExitModule);