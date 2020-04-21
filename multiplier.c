#include <linux/init.h>     // Needed for __init and __exit macros
#include <asm/io.h>         // Needed for IO reads and writes
#include <linux/ioport.h>   // Used for io memory allocation
#include "xparameters.h"    //needed for multiplier
#include "multiplier.h" // needed for device dirver

#define PHY_ADDR XPAR_MULTIPLY_0_S00_AXI_BASEADDR // physical address of multiplier
#define MEMSIZE XPAR_MULTIPLY_0_S00_AXI_HIGHADDR - XPAR_MULTIPLY_0_S00_AXI_BASEADDR + 1 //size of physical address for multiplier


/*function pointer that points to this device diver*/
static struct file_operations fops = {	
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release,
};

static void* virt_addr; // virtual address pointing to multiplier
static int Minor; //minor number of this device diver
static int counter =0;  //statc counter that only gets initialized the first time this function is called 

/*function that is run upon module load.*/
static int __init my_init(void){

	sema_init(&sem, 1); //Initializes the semaphor
	
	msg_bf_Ptr = (char *)kmalloc(BUF_LEN*sizeof(char), GFP_KERNEL); //allocates memory for message pointer
 
	/*prints and returns error if message pointer didnt allocate*/
	if (msg_bf_Ptr == NULL) {
		printk(KERN_ALERT "Unable to allocate needed memory\n");
		return -1; 
	}
	
    printk(KERN_INFO "Mapping virtual address...\n");
    virt_addr = ioremap(PHY_ADDR, MEMSIZE); //maps virtual address to multiplier physical address
	printk(KERN_INFO "Physical Address: %x	,	Virtual Address: %x\n", PHY_ADDR, virt_addr); //prints physical and virtual address
	
	Major = register_chrdev(0, DEVICE_NAME, &fops); //initialises char device dirver
  
	/* prints and returns error if major number negitve */
	if(Major < 0){ //major cant be negitve		
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}

	printk(KERN_INFO "Registered a device with dynamic Major number of %d\n", Major); //prints major number
	printk(KERN_INFO "Create a device file for this device with this command:\n'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

	return 0;		/* success */
}


/* functon to open a device file*/
static int device_open(struct inode *inode, struct file *file){

	/*race condition check*/
	if (down_interruptible (&sem)){
		return -ERESTARTSYS; //restarts
	}
	
	/* checks to if one file is open opens and holds a file one at a time */
	if (Device_Open){
		up(&sem); //releases semaphores on error conditions
		return -EBUSY;	// returns failure to open device
	}
	
	printk("device is open!\n");
	Device_Open++; //Keeps the count of the device thats open
	up(&sem); //resleases semaphore

	counter++; //increments the counter
	
	cur_Ptr = msg_bf_Ptr; //sets a pointer as the current pointer to string
	
	return counter;
}

/* function to close a device file.*/
static int device_release(struct inode *inode, struct file *file){
	
	Device_Open--; //Keeps the count of the device thats open
		
	printk("device is closed!\n");
	
	counter--; //Decrements the counter
		
	return counter;
}


/*function to wirte to a device*/
static ssize_t device_write(struct file *file, const char __user * buffer, size_t length, loff_t * offset)
{
  int i;
  
   printk(KERN_INFO "device_write(%p,%s,%d)", file, buffer, (int)length); 
  
  /* get_user pulls message from userspace into kernel space */
  for (i = 0; i < length ; i++)
    get_user(msg_bf_Ptr[i], &buffer[i]);
  
  /* left one char early from buffer to leave space for null char*/
  msg_bf_Ptr[i] = '\0';

  iowrite32((int) msg_bf_Ptr[0], virt_addr + 0);
  iowrite32((int)msg_bf_Ptr[1],virt_addr + 4);
  return i;
}


/*function to read from a open device*/
static ssize_t device_read(struct file *filp, char *buffer,size_t length, loff_t * offset){
	int bytes_read = 0; //number of bytes written to buffer
	char multiplyReg;
	
	/*reads i*/
	multiplyReg = (char) ioread32(virt_addr+0);
	put_user(buffer[0],&multiplyReg);
	++bytes_read;
	
	/*reads j*/
	multiplyReg = (char) ioread32(virt_addr+4);
	put_user(buffer[1],&multiplyReg);
	++bytes_read;
	
	/*reads the result*/ 
	multiplyReg = (char) ioread32(virt_addr+8);
	put_user(buffer[2],&multiplyReg);
	++bytes_read;
	
  
  return bytes_read; //returns the amount of bytes read
}




/*function that releases all resources used by this module */
static void __exit my_exit(void)
{
	printk(KERN_ALERT "unregistering the device diver...\n");
	unregister_chrdev(Major, DEVICE_NAME);//Unregisters the device
	
	printk(KERN_ALERT "freeing msg_bf_Ptr space...\n");
	kfree(msg_bf_Ptr); //frees memory
	
    printk(KERN_ALERT "unmapping virtual address space...\n");
    iounmap((void*)virt_addr); //unmaps virtual address
}

/* module info*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ECEN449 Michael Mengistu");
MODULE_DESCRIPTION("Simple multiplier module");

/*initialization and cleanup functions*/
module_init(my_init);
module_exit(my_exit);
