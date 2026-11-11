
#ifndef INC_SOFTWARE_I2C_H_
#define INC_SOFTWARE_I2C_H_

#include "software_i2c.h"
#include <stdint.h>
#include <stdbool.h>

extern uint8_t flag;
typedef struct
{
	uint32_t baslangic_zaman;

	uint32_t guncel_zaman;

	GPIO_PinState pinDurumu;
	GPIO_PinState ack_nack;
	GPIO_PinState ack_nack2;
	GPIO_PinState gelen_bit;
	GPIO_PinState scl_durum, sda_durum;

	bool sure_doldu;

	uint8_t hata_veri,hata_adres, veri_iletildi, adres_iletildi, zaman_asimi , i2c_ack_hata;

	uint8_t spi_hata_veri, spii_hata_time, spi_veri_ietildi;

	uint32_t i2c_bekleme;
	uint8_t address_receive, slave_address_error;
	uint8_t gelen_veri, gelen_address;


}timer_t;


void i2c_init();

void i2c_send(timer_t *timer , uint8_t address, uint8_t message);

void mikrosaniye_timer(timer_t *timer, uint32_t zaman);

void mikrosaniye_timer_init(TIM_HandleTypeDef *htim);

void i2c_receive(timer_t *timer, uint8_t address);

uint8_t i2c_slave_sck_sda_control(timer_t *timer);

void i2c_slave_send(timer_t *timer, uint8_t data);
#endif /* INC_SOFTWARE_I2C_H_ */
