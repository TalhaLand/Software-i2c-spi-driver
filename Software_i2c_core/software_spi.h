/*
 * software_spi.h
 *
 *  Created on: 6 Kas 2026
 *      Author: Mansu
 */

#ifndef INC_SOFTWARE_SPI_H_
#define INC_SOFTWARE_SPI_H_

#include "main.h"

#include "software_i2c.h"



void send_spi(timer_t *timer,  uint8_t data);

void spi_delay(timer_t *timer);


#endif /* INC_SOFTWARE_SPI_H_ */
