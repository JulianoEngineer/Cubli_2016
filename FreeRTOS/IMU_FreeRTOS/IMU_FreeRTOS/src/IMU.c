/*
 * IMU.c
 *
 * Created: 03/04/2016 16:45:34
 *  Author: Caio
 */ 

#include "IMU.h"
#include "UART_Comm.h"
#include <string.h>

#define TWI_SPEED		400000	//400KHz Fast-Speed
//#define TWI_BLOCK_TIME	(500/portTICK_RATE_MS)
//#define TWI_BLOCK_TIME	(10/portTICK_RATE_MS)
#define TWI_BLOCK_TIME	(0)
//#define TWI_SEM_WAIT	(100/portTICK_RATE_MS)
#define TWI_SEM_WAIT	(0)

#define CONST_ADXL		(3.9)
#define CONST_ITG		(14.375)

freertos_twi_if freertos_twi;
char buffer[50];

xSemaphoreHandle xseIMU;
xSemaphoreHandle xSemIMUTimer;
xTimerHandle xTimerIMU;

static void vTimerIMU(void *pvParameters){
	xSemaphoreGive(xSemIMUTimer);
}

void IMUTask(void *pvParameters){
	UNUSED(pvParameters);
	status_code_t status;
	
	vSemaphoreCreateBinary(xSemIMUTimer);
	xTimerIMU = xTimerCreate("TimerIMU", TWI_TASK_DELAY , pdTRUE, NULL, vTimerIMU);
	xTimerStart(xTimerIMU, 0);
	
	vSemaphoreCreateBinary(xseIMU);
	configASSERT(xseIMU);
	xSemaphoreTake(xseIMU, 0);
	status = configIMU(xseIMU);
	if (status != STATUS_OK){
		printf_mux("Error IMU!");
		LED_On(LED2_GPIO);
		vTaskDelete(NULL);
	}
	
	float acel[3];
	float gyro[3];
	
	for (;;){
		xSemaphoreTake(xSemIMUTimer, portMAX_DELAY);
		
		memset(acel, 0, sizeof(acel));
		acel[0] = get_acel_value(Axis_X, ADXL_Low, xseIMU) + ACEL_OFFSET_X;
		acel[1] = get_acel_value(Axis_Y, ADXL_Low, xseIMU) + ACEL_OFFSET_Y;	
		acel[2] = get_acel_value(Axis_Z, ADXL_Low, xseIMU) + ACEL_OFFSET_Z;
		
		memset(gyro, 0, sizeof(gyro));
		gyro[0] = get_gyro_value(Axis_X, ITG_Low, xseIMU) + GYRO_OFFSET_X;
		gyro[1] = get_gyro_value(Axis_Y, ITG_Low, xseIMU) + GYRO_OFFSET_Y;
		gyro[2] = get_gyro_value(Axis_Z, ITG_Low, xseIMU) + GYRO_OFFSET_Z;
		
		printf_mux("Acel:");
		printf_mux("X = %0.3f\tY = %0.3f\tZ = %0.3f", acel[0] , acel[1], acel[2]);
		printf_mux("Gyro:");
		printf_mux("X = %0.3f\tY = %0.3f\tZ = %0.3f", gyro[0] , gyro[1], gyro[2]);
		
		/*
		*	To do:
		*	- Create a Task to write on LCD
		*/
		/*ili9225_set_foreground_color(COLOR_WHITE);
		ili9225_draw_filled_rectangle(0,40,ILI9225_LCD_WIDTH,ILI9225_LCD_HEIGHT);
		
		ili9225_set_foreground_color(COLOR_BLACK);
		sprintf(buffer, "Acel:\nX = %0.3f\nY = %0.3f\nZ = %0.3f", acel[0], acel[1], acel[2]);
		ili9225_draw_string(10,50, buffer);
		
		ili9225_set_foreground_color(COLOR_BLACK);
		sprintf(buffer, "Gyro:\nX = %0.3f\nY = %0.3f\nZ = %0.3f", gyro[0], gyro[1], gyro[2]);
		ili9225_draw_string(10,130, buffer);*/
	}
}

static float get_gyro_value(Axis_Op axis, ITG_Addr_Dev dev, xSemaphoreHandle xse){
	status_code_t result;
	float gyro_value = -800;
	uint16_t itg = 0;
	uint8_t b[2];
	memset(b, 0, sizeof(b));
	
	switch (axis)
	{
		case Axis_X:
			result = itg_read(dev, b, ITG_DataX1, sizeof(b), xse);
			break;
		case Axis_Y:
			result = itg_read(dev, b, ITG_DataY1, sizeof(b), xse);
			break;
		case Axis_Z:
			result = itg_read(dev, b, ITG_DataZ1, sizeof(b), xse);
			break;
	}
	
	if (result != STATUS_OK){
		return gyro_value;
	}
	
	xSemaphoreTake(xse, TWI_SEM_WAIT);
	
	itg = (uint16_t)( (b[0] << 8) | b[1] );
	
	if ( !(itg & 0x8000) ){
		gyro_value = ((float)itg) / CONST_ITG;
	} else {
		itg = ( ( (~itg) +1 ) & 0x7FFF);
		gyro_value = -(((float)itg) / CONST_ITG);
	}
	
	return gyro_value;
}

static float get_acel_value(Axis_Op axis, ADXL_Addr_Dev dev, xSemaphoreHandle xse){
	status_code_t result;
	float acel_value = -16000;
	uint16_t adxl = 0;
	uint8_t b[2];
	memset(b, 0, sizeof(b));
	
	switch (axis) {
		case Axis_X:
			result = adxl_read(dev, b, ADXL_DataX0, sizeof(b), xse);
			break;
		case Axis_Y:
			result = adxl_read(dev, b, ADXL_DataY0, sizeof(b), xse);
			break;
		case Axis_Z:
			result = adxl_read(dev, b, ADXL_DataZ0, sizeof(b), xse);
			break;
	}
	
	if (result != STATUS_OK){
		return acel_value;
	}
	
	xSemaphoreTake(xse, TWI_SEM_WAIT);
	
	adxl = (uint16_t)( (b[1] << 8) | b[0] );
	
	if ( !(adxl & 0xFC00) ){
		acel_value = CONST_ADXL * ((float) adxl);
	} else {
		adxl = ( ( (~adxl) +1 ) & 0x03FF);
		acel_value = -(CONST_ADXL * ((float) adxl) );
	}
	
	return acel_value;
}

static status_code_t configIMU(xSemaphoreHandle xse){
	status_code_t status;
	
	if (twi_init() != TWI_SUCCESS){
		printf_mux("TWI init Error!");
		return -1;
	}
	
	status = adxl_init(ADXL_Low, xse);
	if (status != STATUS_OK){
		printf_mux("ADXL init Error!");
		return status;
	}
	
	status = itg_init(ITG_Low, xse);
	if (status != STATUS_OK){
		printf_mux("ITG init Error!");
		return status;
	}
	
	return status;
}

static uint8_t twi_init(void){
	gpio_configure_pin(TWI0_DATA_GPIO, TWI0_DATA_FLAGS);
	gpio_configure_pin(TWI0_CLK_GPIO, TWI0_CLK_FLAGS);
	
	freertos_peripheral_options_t driver_options = {
		//Receiver Buffer
		NULL,
		//Size Receiver Buffer
		0,
		//Priority Interrupt
		(configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY+1),
		TWI_I2C_MASTER,
		( USE_TX_ACCESS_MUTEX | USE_RX_ACCESS_MUTEX )
	};
	
	freertos_twi = freertos_twi_master_init(TWI0, &driver_options);
	
	if (freertos_twi != NULL){
		uint8_t twi_flag = twi_set_speed(TWI0, TWI_SPEED, sysclk_get_cpu_hz());
		return twi_flag;
	}
	
	return (!TWI_SUCCESS);
}

static status_code_t adxl_init(ADXL_Addr_Dev ADXL_Dev, xSemaphoreHandle xSem){
	status_code_t result;
	
	//result = adxl_write(ADXL_Dev, 0x0B, ADXL_DataFormat, xSem);	//16g, 13-bit mode
	result = adxl_write(ADXL_Dev, 0x08, ADXL_DataFormat, xSem);	//2g, 10-bit mode
	if (result != STATUS_OK) return result;
	xSemaphoreTake(xSem, TWI_BLOCK_TIME);
	
	result = adxl_write(ADXL_Dev, 0x09, ADXL_BWRate, xSem);		//Sample rate = 50Hz = 20ms LPF = 25Hz
	if (result != STATUS_OK) return result;
	xSemaphoreTake(xSem, TWI_BLOCK_TIME);
	
	result = adxl_write(ADXL_Dev, 0x08, ADXL_PowerCtl, xSem);	//Start Measurement
	if (result != STATUS_OK) return result;
	xSemaphoreTake(xSem, TWI_BLOCK_TIME);
	
	return result;
}

static status_code_t itg_init(ITG_Addr_Dev ITG_Dev, xSemaphoreHandle xSem){
	status_code_t result;
	
	result = itg_write(ITG_Dev, 0x1B, ITG_DLPF_FS, xSem);		//2000�/s - Sample Rate: 1KHz - LPF: 42Hz
	if (result != STATUS_OK) return result;
	xSemaphoreTake(xSem, TWI_BLOCK_TIME);
	
	result = itg_write(ITG_Dev, 0, ITG_SMPLRT_Div, xSem);		//Fsample = 8KHz/(0 + 1) = 8KHz : 0,125ms
	if (result != STATUS_OK) return result;
	xSemaphoreTake(xSem, TWI_BLOCK_TIME);
	
	result = itg_write(ITG_Dev, 0x00, ITG_PWR_Mgm, xSem);		//Clock Source: Internal Oscillator
	if (result != STATUS_OK) return result;
	xSemaphoreTake(xSem, TWI_BLOCK_TIME);
	
	return result;
}

static status_code_t itg_write(ITG_Addr_Dev itg_addr, uint8_t value, ITG_Addr_Reg itg_reg, xSemaphoreHandle xSem){
	twi_packet_t tx = {
		.addr[0]		= ((uint8_t) itg_reg),
		.addr_length	= 1,
		.buffer			= &value,
		.length			= 1,
		.chip			= ((uint8_t) itg_addr)
	};
	
	status_code_t result;
	
	result = freertos_twi_write_packet_async(freertos_twi, &tx, TWI_BLOCK_TIME, xSem);
	
	return result;
}

static status_code_t itg_read (ITG_Addr_Dev itg_addr, uint8_t *value, ITG_Addr_Reg itg_reg, uint8_t len, xSemaphoreHandle xSem){
	twi_packet_t rx = {
		.addr[0]		= ((uint8_t) itg_reg),
		.addr_length	= 1,
		.buffer			= value,
		.length			= len,
		.chip			= ((uint8_t) itg_addr)
	};
	
	status_code_t result;
	
	result = freertos_twi_read_packet_async(freertos_twi, &rx, TWI_BLOCK_TIME, xSem);
	
	return result;
}

static status_code_t adxl_write(ADXL_Addr_Dev adxl_addr, uint8_t value, ADXL_Addr_Reg adxl_reg, xSemaphoreHandle xSem){
	twi_packet_t tx = {
		.addr[0]		= ((uint8_t) adxl_reg),
		.addr_length	= 1,
		.buffer			= &value,
		.length			= 1,
		.chip			= ((uint8_t) adxl_addr)
	};
	
	status_code_t result;
	
	result = freertos_twi_write_packet_async(freertos_twi, &tx, TWI_BLOCK_TIME, xSem);
	
	return result;
}

static status_code_t adxl_read (ADXL_Addr_Dev adxl_addr, uint8_t *value, ADXL_Addr_Reg adxl_reg, uint8_t len, xSemaphoreHandle xSem){
	twi_packet_t rx = {
		.addr[0]		= ((uint8_t) adxl_reg),
		.addr_length	= 1,
		.buffer			= value,
		.length			= len,
		.chip			= ((uint8_t) adxl_addr)
	};
	
	status_code_t result = STATUS_ERR_TIMEOUT;
	
	result = freertos_twi_read_packet_async(freertos_twi, &rx, TWI_BLOCK_TIME, xSem);
	
	return result;
}