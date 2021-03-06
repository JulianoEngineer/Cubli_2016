/*
 * HAL_UART.h
 *
 * Created: 02/08/2016 21:09:31
 *  Author: Caio
 */ 


#ifndef HAL_UART_H_
#define HAL_UART_H_

#include <asf.h>

status_code_t send_uart(char * buf, size_t len);
uint32_t read_uart(char *buf, uint32_t len);
status_code_t printf_mux( const char * format, ... ) __attribute__ ((format(gnu_printf, 1, 2)));
void configure_console(void);

#endif /* HAL_UART_H_ */