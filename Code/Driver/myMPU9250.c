#include <linux/init.h>                 // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>               // Core header for loading LKMs into the kernel
#include <linux/device.h>               // Header to support the kernel Driver Model
#include <linux/kernel.h>               // Contains types, macros, functions for the kernel
#include <linux/fs.h>                   // Header for the Linux file system support
#include <linux/uaccess.h>              // Required for the copy to user function+
#include <linux/i2c.h>                  // Required for the drievr to work
#include "myMPU9250.h"                  // Required to initialize hardware sensor MPU9250

#define  DEVICE_NAME "i2cMPU9250"       ///< The device will appear at /dev/i2cMPU9250 using this value
#define  CLASS_NAME  "i2c"              ///< The device class -- this is a character device driver

#define MESSAGE_SIZE_MAX    256         ///< Kernel buffer size max
#define MIN(a,b) ((a < b) ? (a) : (b))  ///< Macro to get the minimum between two numbers

MODULE_LICENSE("GPL");                                            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Rodrigo A. Tirapegui");                            ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("Linux char driver for the BBB and MPU9250");  ///< The description -- see modinfo
MODULE_VERSION("0.1");                                            ///< A version number to inform users

static int                  g_majorNumber;                        ///< Stores the device number -- determined automatically
static char                 g_message[MESSAGE_SIZE_MAX] = { 0 };  ///< Memory for the string that is passed from userspace
static short                g_sizeOfMessage;                      ///< Used to remember the size of the string stored
static int                  g_numberOpens = 0;                    ///< Counts the number of times the device is opened
static struct class *       g_MPU9250charClass  = NULL;           ///< The device-driver class struct pointer
static struct device *      g_MPU9250charDevice = NULL;           ///< The device-driver device struct pointer
static struct i2c_client *  g_i2cClientHandler;

static const struct i2c_device_id myMPU9250_i2c_id[] = 
{
    { "myMPU9250", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, myMPU9250_i2c_id);

static const struct of_device_id myMPU9250_of_match[] = 
{
    { .compatible = "mse,myMPU9250" },
    { }
};

MODULE_DEVICE_TABLE(of, myMPU9250_of_match);

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. 
 *  The file_operations structure from /linux/fs.h lists the callback functions that 
 *  you wish to associated with your file operations using a C99 syntax structure. 
 *  Char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. 
 *  The __init macro means that for a built-in driver (not a LKM) the function is only 
 *  used at initialization time and that it can be discarded and its memory freed up 
 *  after that point.
 *  @return returns 0 if successful
 */
static int __init i2cMPU9250char_init(void)
{
   pr_info(KERN_INFO "From Char Init: Initializing the i2cMPU9250Char LKM\n");

   /* Try to dynamically allocate a major number for the device -- more difficult but worth it */
   g_majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   
   if (g_majorNumber < 0)
   {
      pr_info(KERN_ALERT "From Char Init: Failed to register a major number\n");
      return g_majorNumber;
   }

   pr_info(KERN_INFO "From Char Init: Registered correctly with major number %d\n", g_majorNumber);

   // Register the device class
   g_MPU9250charClass = class_create(THIS_MODULE, CLASS_NAME);

   if (IS_ERR(g_MPU9250charClass))
   {  // Check for error and clean up if there is
      unregister_chrdev(g_majorNumber, DEVICE_NAME);
      pr_info(KERN_ALERT "From Char Init: Failed to register device class\n");
      
      /* Correct way to return an error on a pointer */
      return PTR_ERR(g_MPU9250charClass);                    
   }

   pr_info(KERN_INFO "From Char Init: Device class registered correctly\n");

   /* Register the device driver */
   g_MPU9250charDevice = device_create(g_MPU9250charClass, NULL, MKDEV(g_majorNumber, 0), NULL, DEVICE_NAME);
   
   if (IS_ERR(g_MPU9250charDevice))
   {  // Clean up if there is an error
      
      /* Repeated code but the alternative is goto statements */
      class_destroy(g_MPU9250charClass);                
      unregister_chrdev(g_majorNumber, DEVICE_NAME);
      pr_info(KERN_ALERT "From Char Init: Failed to create the device\n");
      return PTR_ERR(g_MPU9250charDevice);
   }

   /* Made it! device was initialized */
   pr_info(KERN_INFO "From Char Init: Device class created correctly\n"); 
   
   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. 
 *  The __exit macro notifies that if this code is used for a built-in driver 
 *  (not a LKM) that this function is not required.
 */
static void __exit i2cMPU9250char_exit(void)
{
   /* Remove the device */
   device_destroy(g_MPU9250charClass, MKDEV(g_majorNumber, 0));     
   
   /* Unregister the device class */
   class_unregister(g_MPU9250charClass);                          
   
   /* Remove the device class */ 
   class_destroy(g_MPU9250charClass);                             
   
   /* Unregister the major number */
   unregister_chrdev(g_majorNumber, DEVICE_NAME);             
   
   pr_info(KERN_INFO "From Char Exit: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep)
{
   /* Increment the g_numberOpens counter */
   g_numberOpens++;
   
   pr_info(KERN_INFO "From Dev Open: Device has been opened %d time(s)\n", g_numberOpens);

   return 0;
}

/** @brief This function is called whenever device is being read from user space 
 *         i.e. data is being sent from the device to the user. 
 *  In this case is uses the copy_to_user() function to send the buffer string to 
 *  the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
   int rv;
   int errCnt = 0;

   /* Read data from MPU9250 */
   rv = i2c_master_recv(g_i2cClientHandler, g_message, MIN(sizeof(g_message), len));

   if(0 < rv)
   {
       /* Copy_to_user has the format ( *to, *from, size) and returns 0 on success */
       errCnt = copy_to_user(buffer, g_message, rv);

       if (0 != errCnt)
       {  // If true then have success
        
          pr_info(KERN_INFO "From Dev Read: Failed to send %d characters to the user\n", errCnt);
        
          /* Failed -- return a bad address message (i.e. -14) */
          return -EFAULT;              
       }

       pr_info(KERN_INFO "From Dev Read: Sent %d characters to the user\n", rv);

       /* Clear the position to the start and return 0 */
       g_sizeOfMessage = 0;  
   }

   return rv;
}

/** @brief This function is called whenever the device is being written to from user space 
 *         i.e. data is sent to the device from the user. 
 *  The data is copied to the message[] array in this LKM using the sprintf() 
 *  function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   int rv;
   int errCnt = 0;

   /* Set size of message to write */
   g_sizeOfMessage = MIN(sizeof(g_message), len);

   /* Copy message from user to kernel space */
   errCnt = copy_from_user(g_message, buffer, g_sizeOfMessage);
   
   if(0 != errCnt)
   {
      pr_info(KERN_INFO "From Dev Write: Failed to received %d characters from the user\n", errCnt);

      /* Failed -- return a bad address message (i.e. -14) */
      return -EFAULT;  
   }

   pr_info(KERN_INFO "From Dev Write: Received %u characters from the user\n", g_sizeOfMessage);
   
   /* Write data to device */
   rv = i2c_master_send(g_i2cClientHandler, g_message, g_sizeOfMessage);

   if(0 < rv)
   {
      pr_info(KERN_INFO "From Dev Write: Written %u characters to device\n", rv);
   }
   
   return rv;
}

/** @brief The device release function that is called whenever the device is closed/released 
 *         by the userspace program.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep)
{
    pr_info(KERN_INFO "From Release: Device successfully closed\n");
    
   return 0;
}

/*****************************************************************************************/
static int mpu9250ReadRegister(struct i2c_client *client, char subAddress, char *rxBuff, char count)
{
    int rv;
    
    /* Read back the register */
    rv = i2c_master_send(client, &subAddress, 1);
    
    if(0 < rv)
    {
        rv = i2c_master_recv(client, rxBuff, 1);
    }

    return rv;
}
static int mpu9250WriteRegister(struct i2c_client *client, char subAddress, char data)
{
    int rv;
	char txBuff[2];
    char rx;

	txBuff[0] = subAddress;
	txBuff[1] = data;
	
    /* Write register */
    rv = i2c_master_send(client, txBuff, 2);

    if(0 < rv)
    {
        /* Read back the register */
        rv = i2c_master_send(client, &subAddress, 1);
        
        if(0 < rv)
        {
            rv = i2c_master_recv(client, &rx, 1);

            if(0 < rv)
            {
                /* Check the read back register against the written register */
                if(data == rx)
                    return 1;
            }
        }
    }

    return -1;
}
static int mpu9250WhoAmI(struct i2c_client *client)
{
    char rx;

	/* Read the WHO AM I register */
	if (0 > mpu9250ReadRegister(client, MPU9250_WHO_AM_I, &rx, 1)) 
    {
		return -1;
	}
	
    /* Return the register value */
	return rx;
}

/** @brief Execute when LKM is installed
 * 
 *  It initialized the hardware sensor MPU9250
 */
static int myMPU9250_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    /* Initialize module */
    i2cMPU9250char_init();

    /* Save i2c client handler */
    g_i2cClientHandler = client;
    
    pr_info("From Probe: Module initialized correctly!\n");

	/* Check the WHO AM I byte, expected value is 0x71 (decimal 113) or 0x73 (decimal 115) */
	if ((113 != mpu9250WhoAmI(client)) && (115 != mpu9250WhoAmI(client))) 
    {
        pr_info("From Probe: Who Am I MPU9250 check fail.\n");
		return -8;
	}

    pr_info("From Probe: Who Am I MPU9250 check success!\n");

    /* Enable I2C master mode */
	if (0 > mpu9250WriteRegister(client, MPU9250_USER_CTRL, MPU9250_I2C_MST_EN)) 
    {
        pr_info("From Probe: Enable I2C master mode fail.\n");
		return -2;
	}

    pr_info("From Probe: Enable I2C master mode success!\n");
	
    /* Set the I2C bus speed to 400 kHz */ 
	if (0 > mpu9250WriteRegister(client, MPU9250_I2C_MST_CTRL, MPU9250_I2C_MST_CLK)) 
    {
        pr_info("From Probe: Set the I2C bus speed to 400 kHz fail.\n");
		return -3;
	}

    pr_info("From Probe: Set the I2C bus speed to 400 kHz success!\n");

    /* Select clock source to gyroscope */
	if (0 > mpu9250WriteRegister(client, MPU9250_PWR_MGMNT_1, MPU9250_CLOCK_SEL_PLL)) 
    {
        pr_info("From Probe: Select clock source to gyroscope fail.\n");
		return -1;
	}

    pr_info("From Probe: Select clock source to gyroscope success!\n");

	/* Setting accel range to 16G as default */
	if (0 > mpu9250WriteRegister(client, MPU9250_ACCEL_CONFIG, MPU9250_ACCEL_FS_SEL_16G)) 
    {
        pr_info("From Probe: Setting accel range to 16G as default fail.\n");
		return -7;
	}

    pr_info("From Probe: Setting accel range to 16G as default success!\n");

	/* Setting the gyro range to 2000DPS as default */
	if (0 > mpu9250WriteRegister(client, MPU9250_GYRO_CONFIG, MPU9250_GYRO_FS_SEL_2000DPS)) 
    {
        pr_info("From Probe: Setting the gyro range to 2000DPS as default fail.\n");
		return -8;
	}

    pr_info("From Probe: Setting the gyro range to 2000DPS as default success!\n");
   
	/* Setting accel bandwidth to 184Hz as default */
	if (0 > mpu9250WriteRegister(client, MPU9250_ACCEL_CONFIG2, MPU9250_ACCEL_DLPF_184)) 
    {
        pr_info("From Probe: Setting accel bandwidth to 184Hz as default fail.\n");
		return -9;
	}

    pr_info("From Probe: Setting accel bandwidth to 184Hz as default success!\n");
   
    /* Setting gyro bandwidth to 184Hz */
	if (0 > mpu9250WriteRegister(client, MPU9250_CONFIG, MPU9250_GYRO_DLPF_184)) 
    { 
        pr_info("From Probe: Setting gyro bandwidth to 184Hz fail.\n");
		return -10;
	}

    pr_info("From Probe: Setting gyro bandwidth to 184Hz success!\n");

	/* Setting the sample rate divider to 0 as default */
	if (0 > mpu9250WriteRegister(client, MPU9250_SMPDIV, 0x00)) 
    {
        pr_info("From Probe: Setting the sample rate divider to 0 as default fail.\n");
		return -11;
	}

    pr_info("From Probe: Setting the sample rate divider to 0 as default success!\n");

    /* Enable accelerometer and gyroscope */
	if (0 > mpu9250WriteRegister(client, MPU9250_PWR_MGMNT_2, MPU9250_SEN_ENABLE)) 
    {
        pr_info("From Probe: Enable accelerometer and gyroscope fail.\n");
		return -9;
	}

    pr_info("From Probe: Enable accelerometer and gyroscope success!\n");

    return 0;
}

/** @brief Execute when LKM is removed
 * 
 *  It removes the hardware sensor MPU9250
 */
static int myMPU9250_remove(struct i2c_client *client)
{
    /* Exit module */
    i2cMPU9250char_exit();

    pr_info("From Remove: MPU9250 remove success!\n");

    return 0;
}

static struct i2c_driver myMPU9250_i2c_driver = 
{
    .driver = 
    {
        .name = "myMPU9250",
        .of_match_table = myMPU9250_of_match,
    },
    .probe = myMPU9250_probe,
    .remove = myMPU9250_remove,
    .id_table = myMPU9250_i2c_id
};

module_i2c_driver(myMPU9250_i2c_driver);
