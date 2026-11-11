
#include "software_spi.h"


uint8_t baslangic = 0;
uint8_t k;
uint8_t sayac2;

extern uint8_t flag;


void send_spi(timer_t *timer,  uint8_t data)
{

	uint8_t sayac;
	uint32_t clock_sayac = 0;
	GPIO_PinState sck_pin_durum;
	uint32_t spi_time_break = HAL_GetTick();

	while(1)
	{
		uint32_t gecen_spi_zaman = HAL_GetTick();

		if(gecen_spi_zaman - spi_time_break >= 25)
		{


			HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, 0);
			HAL_GPIO_WritePin(MOSI_GPIO_Port, MOSI_Pin, 0);

			timer->spii_hata_time = 1;
			timer->spi_hata_veri = 1;
			baslangic = 0;
			flag = 0;
			timer->sure_doldu = 0;
			k = 7;

			return;
		}

		if(baslangic == 0)
		{

			timer->spii_hata_time = 0;
			timer->spi_hata_veri = 0;
			timer->spi_veri_ietildi = 0;
			HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, 0);
			HAL_GPIO_WritePin(MOSI_GPIO_Port, MOSI_Pin, 0);

			k = 7;

			clock_sayac = 0;

			sayac2 = 0;

			if(data & (1 << k))
				HAL_GPIO_WritePin(MOSI_GPIO_Port, MOSI_Pin, 1);
			else
				HAL_GPIO_WritePin(MOSI_GPIO_Port, MOSI_Pin, 0);

			k--;
			sayac2++;

			sayac = 0;

			baslangic = 1;
		}


		if(baslangic == 1)
		{
			GPIO_PinState sck_hedef;

			mikrosaniye_timer(timer, 10);


			if(timer->sure_doldu == true)
			{
				if(clock_sayac % 2 == 0)
					sck_hedef =GPIO_PIN_SET;
				else
					sck_hedef = GPIO_PIN_RESET;

				HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, sck_hedef);

				GPIO_PinState sck_pin_hedef = HAL_GPIO_ReadPin(SCK_GPIO_Port, SCK_Pin);

		        if (sck_pin_hedef == sck_hedef)
		        {
		            clock_sayac++;
		            sayac = 1;
		        }
		        else
		        {
		            sayac = 0;
		        }

			}

			/***********************************************************************************************************/

			if(sayac == 1)
			{

				if(sayac2 < 8)
				{
				sayac = 0;

				sck_pin_durum = HAL_GPIO_ReadPin(SCK_GPIO_Port,SCK_Pin);

					if(sck_pin_durum == 0)
					{
						if(data & (1 << k))
							HAL_GPIO_WritePin(MOSI_GPIO_Port, MOSI_Pin, 1);
						else
							HAL_GPIO_WritePin(MOSI_GPIO_Port, MOSI_Pin, 0);

						k--;
						sayac2++;
					}

				}
				else
				{
					sayac = 0;
					sck_pin_durum = HAL_GPIO_ReadPin(SCK_GPIO_Port, SCK_Pin);

					if(sck_pin_durum == 0)
					{

						baslangic = 0;
						k = 7;
						sayac2 = 0;
						timer->spi_veri_ietildi = 1;
						return;
					}


				}



			}



		}


	}

}


