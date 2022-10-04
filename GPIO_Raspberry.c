#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>


/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("houssem ");
MODULE_DESCRIPTION("A hello world Psuedo device driver");

int GP = 0;
module_param(GP, int, 0664);
int IO = 0;
module_param(IO, int, 0664);

#define GPIO_21 GP

dev_t device_number;
static struct cdev st_characterdevice;
static struct class *myclass;
static struct device *mydevice;

static int __init hellodriver_init(void);
static void __exit hellodriver_exit(void);


static int driver_open(struct inode *device_file, struct file *instance)
{
printk("%s dev_nr - open was called!\n", __FUNCTION__);
return 0;
}
static int driver_close(struct inode *device_file, struct file *instance)
{
printk("dev_nr - close was called!\n");
return 0;
}

// this function will be called when open the device file
static ssize_t driver_read(struct file *filp,
                char __user *buf, size_t len, loff_t *off)
{
uint8_t gpio_state = 0;
//reading GPIO value

gpio_state = gpio_get_value(GPIO_21);

//write to user
len = 1;
if( copy_to_user(buf, &gpio_state, len) > 0) {
	printk("ERROR: Not all the bytes have been copied to user\n");
}
printk("Read function : GPIO_21 = %d \n", gpio_state);
return 0;
}

// this function will be called when we write device file
static ssize_t driver_write(struct file *filp,
                const char __user *buf, size_t len, loff_t *off)
{
uint8_t rec_buf[10] = {0};

if( copy_from_user( rec_buf, buf, len ) > 0) {
printk("ERROR: Not all the bytes have been copied from user\n");
}

printk("Write Function : GPIO_21 Set = %c\n", rec_buf[0]);

if (rec_buf[0]=='1') {
//set the GPIO value to HIGH
gpio_set_value(GPIO_21, 1);
}
 else if (rec_buf[0]=='0') {
//set the GPIO value to LOW
gpio_set_value(GPIO_21, 0);
} else {
	printk("Unknown command : Please provide either 1 or 0 \n");
}

return len;
}

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
	.write = driver_write,
};
static int __init hellodriver_init(void)
{
int retval;

printk("hello kernel\n");
//1- allocate device number
retval = alloc_chrdev_region(&device_number, 0, 1, "test_devicenumber");
if(retval==0)
{
	printk("%s retval=0 - registered device number Major: %d, minor: %d\n", __FUNCTION__, MAJOR(device_number), MINOR(device_number));
}else
{
	printk("Could not register device number!\n");
	return -1;
}
//2- define driver character or block or network
cdev_init(&st_characterdevice, &fops);
retval = cdev_add(&st_characterdevice, device_number, 1);
if(retval !=0)
{
	unregister_chrdev_region(device_number, 1);
	printk("Registering of device to kernel failed!\n");
	return -1;
}
//3- generate file (class -device)
if ((myclass = class_create(THIS_MODULE, "testclass")) == NULL)
{
 	printk("Device class can not be created!\n");
	cdev_del(&st_characterdevice);
	unregister_chrdev_region(device_number, 1);
	return -1;
}
mydevice = device_create(myclass, NULL, device_number, NULL, "testfile");
if (mydevice == NULL)
{
	printk("Device class can not be created!\n");
	cdev_del(&st_characterdevice);
	class_destroy(myclass);
	unregister_chrdev_region(device_number, 1);
	return -1;
}
//-4 checking the GPIO is valid or not
if (gpio_is_valid(GPIO_21) == false)
{
	printk("GPIO %d is not valid\n",GPIO_21);
	cdev_del(&st_characterdevice);
	device_destroy(myclass, device_number);
	class_destroy(myclass);
	unregister_chrdev_region(device_number, 1);
	return -1;
}
//-5 Requesting the GPIO
if (gpio_request(GPIO_21, "GPIO_21") < 0 )
{
	printk("ERROR: GPIO %d request\n", GPIO_21);
	gpio_free(GPIO_21);
	cdev_del(&st_characterdevice);
	device_destroy(myclass, device_number);
	class_destroy(myclass);
	unregister_chrdev_region(device_number, 1);
	return -1;
}
//-6 configure the GPIO as output or input
if (IO == '1')
{
gpio_direction_output(GPIO_21, 0);
}
else if (IO == '0')
{
gpio_direction_input(GPIO_21);
}

gpio_export(GPIO_21, false);
printk("device driver is created\n");
return 0;
}

static void __exit hellodriver_exit(void)
{
 gpio_unexport(GPIO_21);
 gpio_free(GPIO_21);
 cdev_del(&st_characterdevice);
 device_destroy(myclass, device_number);
 class_destroy(myclass);
 unregister_chrdev_region(device_number, 1);
 printk("good bye\n");
}

module_init(hellodriver_init);
module_exit(hellodriver_exit);
// lsmod
// insmod # modprobe
// rmmod
// modinf
