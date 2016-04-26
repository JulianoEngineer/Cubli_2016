/*
 * IMU.h
 *
 * Created: 03/04/2016 16:45:52
 *  Author: Caio
 */ 


#ifndef IMU_H_
#define IMU_H_

#include <asf.h>

#define TWI_TASK_DELAY		(100/portTICK_RATE_MS)

#define ACEL_OFFSET_X		(350.0)
#define ACEL_OFFSET_Y		(1050.0)
#define ACEL_OFFSET_Z		(0.0)

#define GYRO_OFFSET_X		(5.0)
#define GYRO_OFFSET_Y		(0.0)
#define GYRO_OFFSET_Z		(0.0)

static const float offsetAcel[] = {
	ACEL_OFFSET_X,
	ACEL_OFFSET_Y,
	ACEL_OFFSET_Z
};

static const float offsetGyro[] = {
	GYRO_OFFSET_X,
	GYRO_OFFSET_Y,
	GYRO_OFFSET_Z
};

enum ADXL_Addr_Reg_t {
	ADXL_OffsetX	= 0x1E,
	ADXL_OffsetY,
	ADXL_OffsetZ,
	
	ADXL_BWRate		= 0x2C,
	ADXL_PowerCtl,
	ADXL_IntEnable,
	ADXL_InitMap,
	ADXL_InitSouce,
	ADXL_DataFormat,
	ADXL_DataX0,
	ADXL_DataX1,
	ADXL_DataY0,
	ADXL_DataY1,
	ADXL_DataZ0,
	ADXL_DataZ1,
	ADXL_FIFOCtl,
	ADXL_FIFOStatus
};

enum ADXL_Addr_Dev_t {
	ADXL_High		= 0x1D,
	ADXL_Low		= 0x53		//Standard Address
};

enum Axis_t {
	Axis_X,
	Axis_Y,
	Axis_Z
};

enum ITG_Addr_Reg_t {
	ITG_WhoAmI		= 0x00,
	
	ITG_SMPLRT_Div	= 0x15,
	ITG_DLPF_FS,
	ITG_Int_Cfg,
	
	ITG_Int_Status	= 0x1A,
	ITG_Temp_H,
	ITH_Temp_L,
	ITG_DataX1,
	ITG_DataX0,
	ITG_DataY1,
	ITG_DataY0,
	ITG_DataZ1,
	ITG_DataZ0,
	
	ITG_PWR_Mgm		= 0x3E
};
	
enum ITG_Addr_Dev_t {
	ITG_Low			= 0x68,		//Standard Address
	ITG_High
};

typedef enum ADXL_Addr_Reg_t	ADXL_Addr_Reg;
typedef enum ADXL_Addr_Dev_t	ADXL_Addr_Dev;
typedef enum Axis_t				Axis_Op;
typedef enum ITG_Addr_Reg_t		ITG_Addr_Reg;
typedef enum ITG_Addr_Dev_t		ITG_Addr_Dev;

void IMUTask(void *pvParameters);

static void vTimerIMU(void *pvParameters);
static void getAllAcelValue(ADXL_Addr_Dev dev, float *acel);
static void getAllGyroValue(ITG_Addr_Dev dev, float *gyro);
static float get_gyro_value(Axis_Op axis, ITG_Addr_Dev dev);
static float get_acel_value(Axis_Op axis, ADXL_Addr_Dev dev);
static status_code_t configIMU();
uint8_t twi_init();
static status_code_t adxl_init(ADXL_Addr_Dev ADXL_Dev);
static status_code_t itg_init(ITG_Addr_Dev ITG_Dev);
static status_code_t itg_write(ITG_Addr_Dev itg_addr, uint8_t value, ITG_Addr_Reg itg_reg);
static status_code_t itg_read (ITG_Addr_Dev itg_addr, uint8_t *value, ITG_Addr_Reg itg_reg, uint8_t len);
static status_code_t adxl_write(ADXL_Addr_Dev adxl_addr, uint8_t value, ADXL_Addr_Reg adxl_reg);
static status_code_t adxl_read (ADXL_Addr_Dev adxl_addr, uint8_t *value, ADXL_Addr_Reg adxl_reg, uint8_t len);

#endif /* IMU_H_ */