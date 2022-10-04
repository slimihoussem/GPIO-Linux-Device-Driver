# GPIO-Linux-Device-Driver

- install the kernel header using sudo apt install raspberrypi-kernel-headers
- clone this repository
- Build the driver by using Makefile (sudo make)
- Load the driver using sudo insmod GPIO_Module.ko GP="choice your GPIO" IO="0:input ; 1:output"

Testing the Device Driver
Since we are exporting the GPIO to the sysfs using gpio_export(), the below steps also should work:
- use sudo su and enter the password if required to get the root permission
- echo 1 > /sys/class/gpio/gpio21/value [This must turn ON the GPIO].
- echo 0 > /sys/class/gpio/gpio21/value [This must turn OFF the GPIO].
- cat /sys/class/gpio/gpio21/value [Read the GPIO value].

