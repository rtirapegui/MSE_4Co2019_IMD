# Implementación de manejadores de dispositivos (IMD)

## Consigna

- Compilar el kernel linux con soporte para la placa de desarrollo BeagleBoneBlack, booteando desde un directorio de la estación de trabajo, compartido mediante NFS.
- Modificar el device tree por defecto, agregando un nuevo dispositivo.
- Realizar un módulo de kernel que maneje un dispositivo I2C a elección: [MPU-9250](https://github.com/rtirapegui/MSE_4Co2019_IMD/tree/master/Sensor%20I2C)
- Exponer el dispositivo I2C para que pueda ser utilizado desde el espacio de usuario implementando un *char device*.
- Realizar un programa de prueba que interactúe con el módulo mediante el *char device*.

## Entregables

- Código fuente del device driver desarrollado: [myMPU9250.c](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Code/Driver/myMPU9250.c) [myMPU9250.h](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Code/Driver/myMPU9250.h).
- Código fuente de la aplicación de usuario que lo usa: [testMyMPU9250.c](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Code/Test/testMyMPU9250.c).
- Device tree "custom": [am335x-customboneblack.dts](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Code/Device%20tree/am335x-customboneblack.dts).
- Makefiles correspondientes para compilar el driver: [Makefile](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Code/Driver/Makefile).
- Material utilizado para la presentación en clase.
- README explicando cómo se usa el driver.

## Descripción del dispositivo I2C (MPU9250)

MPU-9250 es un módulo multi-chip (MCM) que consiste en dos pastillas de silicio integradas en un mismo encapsulado de tipo QFN.
Una de las pastillas de silicio contiene un giroscopo de 3-ejes, un acelerómetro de 3-ejes y un termómetro. 
La otra pastilla contiene un acelerómetro [AK8963](https://www.akm.com/akm/en/file/datasheet/AK8963C.pdf) fabricado por Asahi Kasei Microdevices Corporation.

El [driver del sensor MPU9250](https://github.com/torvalds/linux/tree/master/drivers/iio/imu/inv_mpu6050) existe en linux, pero a los fines de 
la materia se lo reimplementa centrándose en la configuración y obtención de las aceleraciones (Ax, Ay, Az), ángulos relativos (Gx, Gy, Gz) y 
temperatura.

## Banco de pruebas

El banco de prueba está compuesto por:

* 1 [BeagleBoneBlack Rev C](https://www.amazon.com/Beagleboard-BBONE-BLACK-4G-BeagleBone-Rev-C/dp/B00K7EEX2U).
* 1 [Adaptador USB-TTL UART](https://www.sparkfun.com/products/13830) .
* 1 [Sensor I2C MPU9250](https://www.amazon.com/diymall%C3%82%C2%AE-mpu-9250-nine-axis-attitude-acceleration/dp/b00opnuo9u).

Se muestra a continuación el conexionado de las placas que conforman el banco de prueba:

Conexionado Adaptador USB-TTL UART
:-------------------------:
![](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Pictures/IMG_20190618_204054341.jpg)

Conexionado Sensor I2C MPU9250
:-------------------------:
![](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Pictures/IMG_20190618_204341849.jpg)

## Instrucciones

En base a la [guía de bootlin](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Linux%20Kernel%20Labs/linux-kernel-labs.pdf)
compilar y bootear el kernel linux en la placa BeagleBoneBlack.

Luego:

- En el directorio ~/linux-kernel-labs/modules/nfsroot/root/myMPU9250/
  > Copiar los [archivos del driver](https://github.com/rtirapegui/MSE_4Co2019_IMD/tree/master/Code/Driver)
  
  > Copiar el [archivo de la aplicación de prueba](https://github.com/rtirapegui/MSE_4Co2019_IMD/tree/master/Code/Test)

- En el directorio ~/linux-kernel-labs/src/linux/arch/arm/boot/dts/
  > Copiar el [archivo device tree](https://github.com/rtirapegui/MSE_4Co2019_IMD/tree/master/Code/Device%20tree) junto con el makefile

**Nota**: el makefile agrega la línea *am335x-customboneblack.dtb* con la cual se indica que debe compilarse el device tree am335x-customboneblack.dts.

- En una terminal agregar las variables de entorno $ export ARCH=arm y $ export CROSS_COMPILE=arm-linux-gnueabi-
- Compilar con $ make dtbs desde ~/linux-kernel-labs/src/linux/arch/arm/boot/dts/
- Copiar el archivo .dtb generado junto con zImage en /var/lib/tftpboot/ (tftp server home directory).
- Compilar el driver implementado desde ~/linux-kernel-labs/modules/nfsroot/root/myMPU9250/ con el comando $ make
- Compilar la aplicación de prueba en userspace mediante el comando $ arm-linux-gnueabi-gcc -o test testMyMPU9250.c

- Bootear la BeagleBone con zImage, am335x-customboneblack.dts y filesystem por NFS mediante los [comandos](https://github.com/rtirapegui/MSE_4Co2019_IMD/blob/master/Console/Comandos%20UBoot.txt).

## Pruebas realizadas sobre el hardware

### Inserción del nuevo módulo en el kernel

    # insmod myMPU9250.ko
    [   57.343908] myMPU9250: loading out-of-tree module taints kernel.
    [   57.352666] From Char Init: Initializing the i2cMPU9250Char LKM
    [   57.359018] From Char Init: Registered correctly with major number 246
    [   57.365683] From Char Init: Device class registered correctly
    [   57.374761] From Char Init: Device class created correctly
    [   57.380443] From Probe: Module initialized correctly!
    [   57.408114] From Probe: Who Am I MPU9250 check success!
    [   57.414388] From Probe: Enable I2C master mode success!
    [   57.421282] From Probe: Set the I2C bus speed to 400 kHz success!
    [   57.428475] From Probe: Select clock source to gyroscope success!
    [   57.435569] From Probe: Setting accel range to 16G as default success!
    [   57.443667] From Probe: Setting the gyro range to 2000DPS as default success!
    [   57.451887] From Probe: Setting accel bandwidth to 184Hz as default success!
    [   57.460290] From Probe: Setting gyro bandwidth to 184Hz success!
    [   57.467802] From Probe: Setting the sample rate divider to 0 as default success!
    [   57.476220] From Probe: Enable accelerometer and gyroscope success!

### Verificación de la existencia de los nuevos dispositivos del device tree

    # find /sys/firmware/devicetree/ -name "*myMPU9250*"
    /sys/firmware/devicetree/base/ocp/i2c@4802a000/myMPU9250@68

### Listado del char device

    # cd /dev/
    # ls | grep i2cMPU9250
    i2cMPU9250

### Remoción del módulo del kernel

    # rmmod myMPU9250
    [  381.933145] From Char Exit: Goodbye from the LKM!
    [  381.938160] From Remove: MPU9250 remove success!

### Ejecución del código de prueba

    # ./test
    From TestApp: Starting device test code example..
    [  551.716790] From Dev Open: Device has been opened 3 time(s)

    From TestApp: Press ENTER to read /dev/i2cMPU9250 data or ANY KEY + ENTER to exit

    From TestApp: Reading from the device /dev/i2cMPU9250
    [  553.080101] From Dev Write: Received 1 characters from the user
    [  553.091226] From Dev Write: Written 1 characters to device
    [  553.098427] From Dev Read: Sent 15 characters to the user
    From TestApp: Giroscopo = (-0.044742, 0.026632, -0.037285) [rad/s]
    From TestApp: Acelerometro = (-4.539638, -1.101389, 9.759263) [m/s2]
    From TestApp: Temperatura = 25.741367 [C]

    From TestApp: Press ENTER to read /dev/i2cMPU9250 data or ANY KEY + ENTER to exit
    e
    [  555.788306] From Release: Device successfully closed
    From TestApp: Success to close the device /dev/i2cMPU9250
