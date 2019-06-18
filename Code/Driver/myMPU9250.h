#ifndef _myMPU9250_H
#define _myMPU9250_H

// Constants

/* MPU9250 registers */
#define MPU9250_ACCEL_OUT             0x3B
#define MPU9250_GYRO_OUT              0x43
#define MPU9250_TEMP_OUT              0x41
#define MPU9250_EXT_SENS_DATA_00      0x49
#define MPU9250_ACCEL_CONFIG 	      0x1C
#define MPU9250_ACCEL_FS_SEL_2G       0x00
#define MPU9250_ACCEL_FS_SEL_4G       0x08
#define MPU9250_ACCEL_FS_SEL_8G       0x10
#define MPU9250_ACCEL_FS_SEL_16G      0x18
#define MPU9250_GYRO_CONFIG           0x1B
#define MPU9250_GYRO_FS_SEL_250DPS    0x00
#define MPU9250_GYRO_FS_SEL_500DPS    0x08
#define MPU9250_GYRO_FS_SEL_1000DPS   0x10
#define MPU9250_GYRO_FS_SEL_2000DPS   0x18

#define MPU9250_ACCEL_CONFIG2         0x1D
#define MPU9250_ACCEL_DLPF_184        0x01
#define MPU9250_ACCEL_DLPF_92         0x02
#define MPU9250_ACCEL_DLPF_41         0x03
#define MPU9250_ACCEL_DLPF_20         0x04
#define MPU9250_ACCEL_DLPF_10         0x05
#define MPU9250_ACCEL_DLPF_5          0x06
#define MPU9250_CONFIG                0x1A
#define MPU9250_GYRO_DLPF_184         0x01
#define MPU9250_GYRO_DLPF_92          0x02
#define MPU9250_GYRO_DLPF_41          0x03
#define MPU9250_GYRO_DLPF_20          0x04
#define MPU9250_GYRO_DLPF_10          0x05
#define MPU9250_GYRO_DLPF_5           0x06
#define MPU9250_SMPDIV                0x19
#define MPU9250_INT_PIN_CFG           0x37
#define MPU9250_INT_ENABLE            0x38
#define MPU9250_INT_DISABLE           0x00
#define MPU9250_INT_PULSE_50US        0x00
#define MPU9250_INT_WOM_EN            0x40
#define MPU9250_INT_RAW_RDY_EN        0x01
#define MPU9250_PWR_MGMNT_1           0x6B
#define MPU9250_PWR_CYCLE             0x20
#define MPU9250_PWR_RESET             0x80
#define MPU9250_CLOCK_SEL_PLL         0x01
#define MPU9250_PWR_MGMNT_2           0x6C
#define MPU9250_SEN_ENABLE            0x00
#define MPU9250_DIS_GYRO              0x07
#define MPU9250_USER_CTRL             0x6A
#define MPU9250_I2C_MST_EN            0x20
#define MPU9250_I2C_MST_CLK           0x0D
#define MPU9250_I2C_MST_CTRL          0x24
#define MPU9250_I2C_SLV0_ADDR         0x25
#define MPU9250_I2C_SLV0_REG          0x26
#define MPU9250_I2C_SLV0_DO           0x63
#define MPU9250_I2C_SLV0_CTRL         0x27
#define MPU9250_I2C_SLV0_EN           0x80
#define MPU9250_I2C_READ_FLAG         0x80
#define MPU9250_MOT_DETECT_CTRL       0x69
#define MPU9250_ACCEL_INTEL_EN        0x80
#define MPU9250_ACCEL_INTEL_MODE      0x40
#define MPU9250_LP_ACCEL_ODR          0x1E
#define MPU9250_WOM_THR               0x1F
#define MPU9250_WHO_AM_I              0x75
#define MPU9250_FIFO_EN               0x23
#define MPU9250_FIFO_TEMP             0x80
#define MPU9250_FIFO_GYRO             0x70
#define MPU9250_FIFO_ACCEL            0x08
#define MPU9250_FIFO_MAG              0x01
#define MPU9250_FIFO_COUNT            0x72
#define MPU9250_FIFO_READ             0x74

/* AK8963 registers */
#define MPU9250_AK8963_I2C_ADDR       0x0C
#define MPU9250_AK8963_HXL            0x03
#define MPU9250_AK8963_CNTL1          0x0A
#define MPU9250_AK8963_PWR_DOWN       0x00
#define MPU9250_AK8963_CNT_MEAS1      0x12
#define MPU9250_AK8963_CNT_MEAS2      0x16
#define MPU9250_AK8963_FUSE_ROM       0x0F
#define MPU9250_AK8963_CNTL2          0x0B
#define MPU9250_AK8963_RESET          0x01
#define MPU9250_AK8963_ASA            0x10
#define MPU9250_AK8963_WHO_AM_I       0x00

/* I2C baudrate */
#define MPU9250_I2C_RATE              400000 // 400 kHz

#endif  