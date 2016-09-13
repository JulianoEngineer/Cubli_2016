/*
 * LCD.h
 *
 * Created: 12/04/2016 21:55:44
 *  Author: Caio
 */ 


#ifndef LCD_H_
#define LCD_H_

#include <asf.h>

void config_lcd(void);
void LCDTask(void *pvParameters);

extern xTaskHandle xLCDHandler;

#endif /* LCD_H_ */