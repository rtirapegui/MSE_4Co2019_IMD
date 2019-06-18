/**
 * @file   testMyMPU9250.c
 * @author Rodrigo A. Tirapegui
 * @date   15 June 2019
 * @version 0.1
 * @brief  A Linux user space program that communicates with the myMPU9250.c LKM. 
 *         It passes a string to the LKM and reads the response from the LKM. 
 * 
 * For this example to work the device must be called /dev/i2cMPU9250.
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "myMPU9250.h"

// Constants
#define DEVICE_UNDER_TEST   "/dev/i2cMPU9250"   ///< Device under test
#define BUFFER_LENGTH       256                 ///< The buffer length

// Types
typedef struct 
{
   /* Data counts */
   short _axcounts, _aycounts, _azcounts;
   short _gxcounts, _gycounts, _gzcounts;
   short _tcounts;

   /* Data buffer */
   float _ax, _ay, _az;
   float _gx, _gy, _gz;
   float _t;

   /* Gyro bias estimation */
   float _gxb, _gyb, _gzb;

   /* Accel bias and scale factor estimation */
   float _axb, _ayb, _azb;
   float _axs, _ays, _azs;

   /* Scale factors */
   float _accelScale;
   float _gyroScale;
   float _tempScale;
   float _tempOffset;

   /* Transformation matrix */
   short tX[3];
   short tY[3];
   short tZ[3];

   /* Track success of interacting with sensor */
   char _status;

} MPU9250_Control_t;

// Variables
static char                         g_txData[BUFFER_LENGTH];    ///< The transmit buffer to the LKM
static char                         g_rxData[BUFFER_LENGTH];    ///< The receive buffer from the LKM
static MPU9250_Control_t   g_MPU9250Control =  {                   
                                                ._tempScale = 333.87f,
                                                ._tempOffset = 21.0f,
                                                ._axs = 1.0f,
                                                ._ays = 1.0f,
                                                ._azs = 1.0f,
                                                ._accelScale = 9.807f * (16.0f / 32767.5f),
                                                ._gyroScale = 2000.0f / 32767.5f * (3.14159265359f/180.0f),
                                                .tX[0] = 0,
                                                .tX[1] = 1,
                                                .tX[2] = 0,
                                                .tY[0] = 1,
                                                .tY[1] = 0,
                                                .tY[2] = 0,
                                                .tZ[0] = 0,
                                                .tZ[1] = 0,
                                                .tZ[2] = -1,
                                                };             ///< Control data structure of hardware sensor MPU9250

// Private functions
static int unit_test(void)
{
   int ret, fd, c;
   
   printf("From TestApp: Starting device test code example..\n");
   
   /* Open the device with read/write access */
   fd = open(DEVICE_UNDER_TEST, O_RDWR);             
   
   if (0 > fd)
   {
      printf("From TestApp: Failed to open the device %s\n", DEVICE_UNDER_TEST);
      
      return errno;
   }
   
   /* Repeat forever */
   while(1)
   {
      printf("From TestApp: Press ENTER to read %s data or ANY KEY + ENTER to exit\n", DEVICE_UNDER_TEST);
      
      /* Wait until enter is pressed by user */
      c = getchar();

      if('\n' != c)
        break;

      printf("From TestApp: Reading from the device %s\n", DEVICE_UNDER_TEST);

      /* Fill txBuffer with MPU9250 register */
      g_txData[0] = MPU9250_ACCEL_OUT;

      /* Send the data to the LKM */
      ret = write(fd, g_txData, 1); 

      if(0 > ret)
      {
         printf("From TestApp: Failed to write the message to the device.\n");
         
         return errno;
      }

      /* Read the response from the LKM */
      ret = read(fd, g_rxData, 15);
      
      if(0 > ret)
      {
         printf("From TestApp: Failed to read the message from the device.\n");

         return errno;
      }

      /* Parse response from LKM */
      g_MPU9250Control._axcounts = (((short)g_rxData[0]) << 8)  | g_rxData[1];
      g_MPU9250Control._aycounts = (((short)g_rxData[2]) << 8)  | g_rxData[3];
      g_MPU9250Control._azcounts = (((short)g_rxData[4]) << 8)  | g_rxData[5];
      g_MPU9250Control._tcounts  = (((short)g_rxData[6]) << 8)  | g_rxData[7];
      g_MPU9250Control._gxcounts = (((short)g_rxData[8]) << 8)  | g_rxData[9];
      g_MPU9250Control._gycounts = (((short)g_rxData[10]) << 8) | g_rxData[11];
      g_MPU9250Control._gzcounts = (((short)g_rxData[12]) << 8) | g_rxData[13];

      /* Transform and convert to float values */
	  g_MPU9250Control._ax = (((float)(g_MPU9250Control.tX[0]*g_MPU9250Control._axcounts + g_MPU9250Control.tX[1]*g_MPU9250Control._aycounts + g_MPU9250Control.tX[2]*g_MPU9250Control._azcounts) * g_MPU9250Control._accelScale) - g_MPU9250Control._axb)*g_MPU9250Control._axs;
	  g_MPU9250Control._ay = (((float)(g_MPU9250Control.tY[0]*g_MPU9250Control._axcounts + g_MPU9250Control.tY[1]*g_MPU9250Control._aycounts + g_MPU9250Control.tY[2]*g_MPU9250Control._azcounts) * g_MPU9250Control._accelScale) - g_MPU9250Control._ayb)*g_MPU9250Control._ays;
	  g_MPU9250Control._az = (((float)(g_MPU9250Control.tZ[0]*g_MPU9250Control._axcounts + g_MPU9250Control.tZ[1]*g_MPU9250Control._aycounts + g_MPU9250Control.tZ[2]*g_MPU9250Control._azcounts) * g_MPU9250Control._accelScale) - g_MPU9250Control._azb)*g_MPU9250Control._azs;
	  g_MPU9250Control._gx = ((float) (g_MPU9250Control.tX[0]*g_MPU9250Control._gxcounts + g_MPU9250Control.tX[1]*g_MPU9250Control._gycounts + g_MPU9250Control.tX[2]*g_MPU9250Control._gzcounts) * g_MPU9250Control._gyroScale) -  g_MPU9250Control._gxb;
	  g_MPU9250Control._gy = ((float) (g_MPU9250Control.tY[0]*g_MPU9250Control._gxcounts + g_MPU9250Control.tY[1]*g_MPU9250Control._gycounts + g_MPU9250Control.tY[2]*g_MPU9250Control._gzcounts) * g_MPU9250Control._gyroScale) -  g_MPU9250Control._gyb;
	  g_MPU9250Control._gz = ((float) (g_MPU9250Control.tZ[0]*g_MPU9250Control._gxcounts + g_MPU9250Control.tZ[1]*g_MPU9250Control._gycounts + g_MPU9250Control.tZ[2]*g_MPU9250Control._gzcounts) * g_MPU9250Control._gyroScale) -  g_MPU9250Control._gzb;
	  g_MPU9250Control._t = ((((float) g_MPU9250Control._tcounts) - g_MPU9250Control._tempOffset)/ g_MPU9250Control._tempScale) + g_MPU9250Control._tempOffset;

      /* Print results */
      printf("From TestApp: Giroscopo = (%f, %f, %f) [rad/s]\r\n", g_MPU9250Control._gx,
                                                                   g_MPU9250Control._gy,
                                                                   g_MPU9250Control._gz );

	  printf("From TestApp: Acelerometro = (%f, %f, %f) [m/s2]\r\n", g_MPU9250Control._ax,
                                                                     g_MPU9250Control._ay,
                                                                     g_MPU9250Control._az );

	  printf("From TestApp: Temperatura = %f [C]\r\n\r\n", g_MPU9250Control._t);
   }

   /* Close the device with read/write access */
   ret = close(fd);             
   
   if (0 > ret)
   {
      printf("From TestApp: Failed to close the device %s\n", DEVICE_UNDER_TEST);
      
      return errno;
   }

   printf("From TestApp: Success to close the device %s\n", DEVICE_UNDER_TEST);

   return 0;
}

// Public functions
int main(void)
{
   return unit_test();
}
