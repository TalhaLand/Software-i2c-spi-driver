#include "main.h"
#include "software_i2c.h"


uint8_t flag = 0;
static uint8_t sayac = 0, first = 0;
static uint8_t j = 0;
uint8_t clock_sayac;
uint8_t i; uint8_t hat_sal = 0;
uint8_t gelen_data_sayac;

uint8_t giden_veri_sayac = 7;
void i2c_init()
{
	// sda ve scl bitleri yüksek seviyeye alındı
	HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);
	HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, SET);

}

void i2c_send(timer_t *timer , uint8_t address, uint8_t message)
{

	uint32_t time = HAL_GetTick();

	timer->zaman_asimi = 0;

	while(1)
	{
		timer->i2c_bekleme = HAL_GetTick();

		if(timer->i2c_bekleme - time >= 25)
		{

			// burada belli süre içinde veri gitmezse salıyoruz
			//tim3 durmus olabilir ya da baska bir zamanlayiici hatası

			HAL_GPIO_WritePin(SCL_GPIO_Port,SCL_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

			timer->veri_iletildi = 0;
			timer->zaman_asimi = 1;
			hat_sal = 0;
			first = 0;
			j = 0;
			clock_sayac = 0;
			sayac = 0;

			flag = 0;

			return;
		}

		if(first == 0)
		{
			 i = 7;

			//sda düşük çektik
			HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);

			// address kaydırdık bir bit
			address = address * 2;

			timer->hata_adres = 0;
			timer->hata_veri = 0;
			timer->veri_iletildi = 0;
			timer->adres_iletildi = 0;
			timer->zaman_asimi = 0;

			first = 1;

		}



		if (first == 1)
		{
		    mikrosaniye_timer(timer, 10);

		    if (timer->sure_doldu)
		    {
		        // Sayaç 0 iken ilk adım SCL'yi düşürmek.
		        // Sonra sırayla yükseltip düşürüyoruz.
		        GPIO_PinState hedef;

		        if (clock_sayac % 2 == 0)
		            hedef = GPIO_PIN_RESET;
		        else
		            hedef = GPIO_PIN_SET;

		        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, hedef);

		        timer->pinDurumu = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);

		        if (timer->pinDurumu == hedef)
		        {
		            clock_sayac++;
		            sayac = 1;
		        }
		        else
		        {
		            sayac = 0;
		        }
		    }
		}




		if(sayac == 1)
		{
			if(timer->pinDurumu == GPIO_PIN_RESET)
			{

				if(j <= 7)
				{

					/// maskeleme ile bit kontrol ettik böylese 0 ve 1 değerlerini gördük
					if(address & (1 << i) )
					{
						HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);

					}
					else
					{
						HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, RESET);
					}

					i--;

					j++;

					sayac = 0;


				}
				else if(j == 8)
				{
					HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);
					j++;
					sayac = 0;

				}
				else if(j >= 10 && j < 18)
				{

					if(message & (1 << i))
					{
						HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);
					}
					else
					{
						HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, RESET);

					}
					i--;

					j++;
					sayac = 0;

				}
				else if(j == 18)
				{
					HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);
					j++;
					sayac = 0;
				}

			}

			if(timer->pinDurumu == GPIO_PIN_SET)
			{
				if(clock_sayac == 18 && sayac == 1)
				{


					timer->ack_nack = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);

					if(timer->ack_nack == 1)
					{
						timer->hata_adres = 1;
						timer->adres_iletildi = 0;

						while(hat_sal < 5)
						{

							timer->i2c_bekleme = HAL_GetTick();
							if(timer->i2c_bekleme - time >= 25)
							{
								// burada belli süre içinde veri gitmezse salıyoruz
								//tim3 durmus olabilir ya da baska bir zamanlayiici hatası

								HAL_GPIO_WritePin(SCL_GPIO_Port,SCL_Pin, GPIO_PIN_SET);
								HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

								timer->veri_iletildi = 0;
								timer->zaman_asimi = 1;
								hat_sal = 0;
								first = 0;
								j = 0;
								clock_sayac = 0;
								sayac = 0;

								flag = 0;

								return;
							}

							mikrosaniye_timer(timer, 5);
							if(timer->sure_doldu == true)
							{
								if(hat_sal == 3 && HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin) == 0)
								{
									hat_sal = 3;
								}
								else
								{
									hat_sal++;
								}
							}

							if(hat_sal == 1)
							{
								HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, RESET);
							}
							if(hat_sal == 2)
							{
								HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, RESET);
							}
							if(hat_sal == 3)
							{

								HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, SET);

							}
							if(hat_sal == 4)
							{
								HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);
							}

						}
						hat_sal = 0;
						first = 0;
						j = 0;
						clock_sayac = 0;



						return;
					}
					else
					{
						timer->hata_adres = 0;
						timer->adres_iletildi = 1;
					}

					i = 7;
					j++;
					sayac = 0;

				}

				if(clock_sayac == 36 && sayac == 1)
				{
					timer->ack_nack2 = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);

					first = 0;
					j = 0;
					clock_sayac = 0;

					if(timer->ack_nack2 == 0 && timer->ack_nack == 0 )
					{
						timer->veri_iletildi = 1;
						timer->hata_veri = 0;

					}
					else
					{
						timer->veri_iletildi = 0;
						timer->hata_veri = 1;
					}

					while(hat_sal < 5)
					{

						timer->i2c_bekleme = HAL_GetTick();
						if(timer->i2c_bekleme - time >= 25)
						{
							// burada belli süre içinde veri gitmezse salıyoruz
							//tim3 durmus olabilir ya da baska bir zamanlayiici hatası

							HAL_GPIO_WritePin(SCL_GPIO_Port,SCL_Pin, GPIO_PIN_SET);
							HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

							timer->veri_iletildi = 0;
							timer->zaman_asimi = 1;
							hat_sal = 0;
							first = 0;
							j = 0;
							clock_sayac = 0;
							sayac = 0;

							flag = 0;

							return;
						}

						mikrosaniye_timer(timer, 5);

						if(timer->sure_doldu == true)
						{
							if(hat_sal == 3 && HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin) == 0)
							{
								hat_sal = 3;
							}
							else
							{
								hat_sal++;
							}
						}

						if(hat_sal == 1)
						{
							HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, RESET);
						}
						if(hat_sal == 2)
						{
							HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, RESET);
						}
						if(hat_sal == 3)
						{
							HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, SET);
						}
						if(hat_sal == 4)
						{
							HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);
						}

					}
					hat_sal = 0;

					return;


				}
			}
		}



	}

}


void i2c_receive(timer_t *timer, uint8_t address)
{
	uint32_t time = HAL_GetTick();

		timer->zaman_asimi = 0;

		while(1)
		{
			timer->i2c_bekleme = HAL_GetTick();

			if(timer->i2c_bekleme - time >= 25)
			{

				// burada belli süre içinde veri gitmezse salıyoruz
				//tim3 durmus olabilir ya da baska bir zamanlayiici hatası

				HAL_GPIO_WritePin(SCL_GPIO_Port,SCL_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

				timer->veri_iletildi = 0;
				timer->zaman_asimi = 1;
				hat_sal = 0;
				first = 0;
				j = 0;
				clock_sayac = 0;
				sayac = 0;

				flag = 0;

				return;
			}

			if(first == 0)
			{
				 i = 7;

				//sda düşük çektik
				HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);

				// address kaydırdık bir bit
				address = (address << 1) | 1;

				timer->hata_adres = 0;
				timer->hata_veri = 0;

				timer->adres_iletildi = 0;
				timer->zaman_asimi = 0;

				first = 1;

			}



			if (first == 1)
			{
			    mikrosaniye_timer(timer, 10);

			    if (timer->sure_doldu)
			    {
			        // Sayaç 0 iken ilk adım SCL'yi düşürmek.
			        // Sonra sırayla yükseltip düşürüyoruz.
			        GPIO_PinState hedef;

			        if (clock_sayac % 2 == 0)
			            hedef = GPIO_PIN_RESET;
			        else
			            hedef = GPIO_PIN_SET;

			        HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, hedef);

			        timer->pinDurumu = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);

			        if (timer->pinDurumu == hedef)
			        {
			            clock_sayac++;
			            sayac = 1;
			        }
			        else
			        {
			            sayac = 0;
			        }
			    }
			}

			if(sayac == 1)
			{
				if(timer->pinDurumu == GPIO_PIN_RESET)
				{

					if(j <= 7)
					{

						/// maskeleme ile bit kontrol ettik böylese 0 ve 1 değerlerini gördük
						if(address & (1 << i) )
						{
							HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);

						}
						else
						{
							HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, RESET);
						}

						i--;

						j++;

						sayac = 0;


					}
					else if(j == 8)
					{
						HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, SET);
						j++;
						sayac = 0;

					}

				}



				if(clock_sayac == 18 && timer->pinDurumu == GPIO_PIN_SET)
				{
					timer->ack_nack = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);

					sayac = 0;

					if(timer->ack_nack == GPIO_PIN_RESET)
					{
						timer->adres_iletildi = 1;
						gelen_data_sayac = 0;
						timer->gelen_veri = 0;
						j++;
					}
					else
					{
						HAL_GPIO_WritePin(SCL_GPIO_Port,SCL_Pin, GPIO_PIN_SET);
						HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

						timer->veri_iletildi = 0;

						hat_sal = 0;
						first = 0;
						j = 0;
						clock_sayac = 0;
						sayac = 0;

						flag = 0;

						timer->i2c_ack_hata = 1;
						return;
					}

				}

				if(clock_sayac > 18 && clock_sayac < 35 && timer->ack_nack == GPIO_PIN_RESET && clock_sayac % 2 == 0)
				{
					timer->gelen_bit = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);

					sayac = 0;

					if(timer->gelen_bit == GPIO_PIN_SET)
						timer->gelen_veri = (timer->gelen_veri<<1) | 1;
					else
						timer->gelen_veri = (timer->gelen_veri << 1);
				}
				else if(clock_sayac == 35)
				{
					HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, 1);

				}
				else if(clock_sayac == 37)
				{
				    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);

				    flag = 0;
				    do {
				        mikrosaniye_timer(timer, 10);
				    } while(!timer->sure_doldu);

				    HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET);

				    flag = 0;
				    do {
				        mikrosaniye_timer(timer, 10);
				    } while(!timer->sure_doldu);

				    HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);

				    first = 0;
				    j = 0;
				    clock_sayac = 0;
				    sayac = 0;
				    hat_sal = 0;
				    flag = 0;

				    return;
				}

			}





		}

}

uint8_t i2c_slave_sck_sda_control(timer_t *timer)
{

	GPIO_PinState first_scl = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);
	GPIO_PinState first_sda = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);

	uint32_t ilk_zaman = HAL_GetTick();

	while(1)
	{
		uint32_t ikinci_zaman = HAL_GetTick();

		if(ikinci_zaman - ilk_zaman >= 25)
		{
			timer->zaman_asimi = 1;
			return 0;
		}

		timer->scl_durum = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);

		timer->sda_durum = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);


		if(first_scl == GPIO_PIN_SET && timer->scl_durum == GPIO_PIN_SET &&
		   first_sda == GPIO_PIN_SET && timer->sda_durum == GPIO_PIN_RESET)
		{
		    return 1;
		}
		else
		{
			first_scl = timer->scl_durum;
			first_sda = timer->sda_durum;
		}

	}

}

void i2c_slave_send(timer_t *timer, uint8_t data)
{

	timer->gelen_address = 0x00;

	uint8_t fck = 0;
	uint8_t slave_flag = i2c_slave_sck_sda_control(timer);

	GPIO_PinState scl_ilk = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);

	uint8_t clock_sayac2 = 0;

	uint8_t address_sayac = 0;

	if(slave_flag == 1)
	{
		while(1)
		{
			timer->scl_durum = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);


			if(timer->scl_durum == GPIO_PIN_SET && scl_ilk != GPIO_PIN_SET && address_sayac < 8)
			{



				timer->sda_durum = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);
				if(timer->sda_durum == GPIO_PIN_SET)
					timer->gelen_address = (timer->gelen_address << 1) | 1;
				else
					timer->gelen_address = (timer->gelen_address << 1);

				address_sayac++;

				clock_sayac2++;
				scl_ilk = timer->scl_durum;

				if(timer->gelen_address != 0x51 && address_sayac == 8)
				{
					timer->hata_adres = 1;
					timer->adres_iletildi = 0;
					timer->address_receive = 0;
					return;
				}
			}

			if(address_sayac == 8 && timer->gelen_address == 0x51)
			{
				uint8_t yeter = 0;
				giden_veri_sayac = 7;
				while(1)
				{
					timer->scl_durum = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);

					if(timer->scl_durum == GPIO_PIN_RESET && yeter == 0)
					{
						timer->adres_iletildi = 1;
						timer->address_receive = 1;
						HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, 0);
						yeter++;


					}
					else if(yeter == 1)
					{
						timer->scl_durum = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);
						if(timer->scl_durum == GPIO_PIN_SET)
						{
							yeter++;
						}
					}
					else if(yeter == 2)
					{
						timer->scl_durum = HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin);
						if(timer->scl_durum == GPIO_PIN_RESET)
						{
							yeter++;
						}
					}
					else if(yeter == 3 && timer->scl_durum == GPIO_PIN_RESET)
					{
					    // SCL düşük: sıradaki biti SDA'ya koy.
					    if(data & (1 << giden_veri_sayac))
					        HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
					    else
					        HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);

					    fck++;
					    yeter = 4;
					}
					else if(yeter == 4 && timer->scl_durum == GPIO_PIN_SET)
					{
					    // Master bu biti okuyor. SDA aynı kalsın.
					    yeter = 5;
					}
					else if(yeter == 5 && timer->scl_durum == GPIO_PIN_RESET)
					{
					    if(fck < 8)
					    {
					        giden_veri_sayac--;
					        yeter = 3;
					    }
					    else
					    {
					        // Sekizinci bit okundu. Master'ın cevabı için SDA'yı bırak.
					        HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
					        yeter = 6;
					    }
					}
					else if(yeter == 6 && timer->scl_durum == GPIO_PIN_SET)
					{
					    // Dokuzuncu darbe: tek baytlık okumada master NACK verir.
					    timer->ack_nack = HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin);
					    return;
					}



				}
			}


			scl_ilk = timer->scl_durum;
		}
	}
}




/************************************************************************************/
//
//	Timer Kısmı 1uS timer
//
/*************************************************************************************/
void mikrosaniye_timer_init(TIM_HandleTypeDef *htim)
{
	HAL_TIM_Base_Start(htim);

}

void mikrosaniye_timer(timer_t *timer, uint32_t zaman)
{

	timer->guncel_zaman = TIM3->CNT;

	if(flag == 0)
	{

		timer->baslangic_zaman = timer->guncel_zaman ;
		timer->sure_doldu = false;

		flag = 1;
	}

	if(timer->baslangic_zaman > timer->guncel_zaman)
	{
		if(((65535 - timer->baslangic_zaman ) + timer->guncel_zaman ) >= zaman)
		{
			flag = 0;
			timer->sure_doldu = true;

		}
		else
		{
			return;
		}

	}
	else
	{
		if((timer->guncel_zaman - timer->baslangic_zaman) >= zaman)
		{
			flag = 0;
			timer->sure_doldu = true;

		}
		else
		{
			return;
		}
	}


}



