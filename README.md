# STM32F401RE ve STM32F407 ile Yazılımsal I²C ve SPI Haberleşmesi

Bu projede bir STM32F401RE’yi ana master olarak kullanıp iki kartla haberleştirdim. Bilgisayardan seri terminale yazılan bir komutla, ikinci bir STM32F401RE ve bir STM32F407 üzerindeki LED’ler kontrol ediliyor. Master, LED komutlarını iki karta SPI üzerinden gönderiyor. Alıcı STM32F401RE ise uyguladığı LED komutunu I²C üzerinden master’a geri bildiriyor.

Master tarafında I²C ve SPI sinyallerini GPIO pinlerini yazılımla değiştirerek ürettim. İki alıcı kartta SPI verisini okumak için STM32’nin donanımsal SPI1 birimini kullandım.

## Kartların görevleri

| Proje | Kart | `main.c` içindeki görevi |
|---|---|---|
| `Software_i2c` | STM32F401RE – ana master | UART komutunu alır, iki alıcıyı ayrı ayrı seçerek SPI verisini gönderir ve I²C geri bildirimini terminale yazar. |
| `stm32f401re_i2c` | STM32F401RE – alıcı | SPI1 üzerinden aldığı komutla LED’i kontrol eder ve yazılımsal I²C ile master’ın okuma isteğini bekler. |
| `stm32f407_software` | STM32F407 – alıcı | SPI1 üzerinden aldığı komutla kendi LED’ini kontrol eder. |

## Ana master’ın `main.c` dosyası nasıl çalışıyor?

Başlangıçta master; GPIO, USART2 ve TIM3 birimlerini yapılandırır. `mikrosaniye_timer_init(&htim3)` ile zamanlayıcıyı başlatır, `i2c_init()` ile SDA ve SCL hatlarını serbest bırakır. Ardından ana döngüde şu işlemler gerçekleşir:

1. **Butonu ve I²C verisini okur.** Önce PC13 üzerindeki butonun seviyesini örnekler. Ardından `i2c_receive(&timer, 0x28)` ile alıcı STM32F401RE’den bir bayt okumayı dener.
2. **Terminal komutunu bekler.** `HAL_UART_Receive()` ile USART2 üzerinden 10 bayt bekler. Bekleme süresi en fazla 1 saniyedir. Bu çağrıdan sonra I²C veri alanındaki değer `0x01` ise terminale `Led Yandi`, `0x02` ise `led yanmiyor` yazar.
3. **Komutu iki SPI aktarımına çevirir.** Geçerli bir UART komutu geldiğinde her alıcı için gönderilecek baytı belirler. İlgili CS pinini düşük seviyeye çeker, `send_spi()` ile bir bayt gönderir ve CS pinini tekrar yüksek seviyeye alır. Böylece iki kart aynı SCK ve MOSI hatlarını paylaşırken farklı komutlar alabilir.
4. **Master üzerindeki LED’i kontrol eder.** PC13 butonunun durum değişimini 20 ms eşiğiyle kontrol eder. Bir basış algıladığında PA5 üzerindeki yerel LED’in durumunu değiştirir.

I²C okuması döngüde yeni SPI komutlarından **önce** yapılır. Bu nedenle terminaldeki LED mesajı daha önce işlenen bir komuta ait olabilir. Ayrıca mevcut kod, mesajı yazdırmadan önce I²C okumasının başarılı olduğunu kontrol etmediği için başarısız bir okumada önceki değer tekrar görünebilir.

## Seri terminalden kullanım

USART2 bağlantısını **115200 baud, 8 veri biti, parite yok, 1 stop biti (8N1)** olarak açın. Satır sonunu **CR+LF** seçin. Örneğin `led on 2` komutunun sekiz karakteri ve iki satır sonu karakteri, master’ın beklediği 10 baytı tamamlar.

| Terminal komutu | CS / PA9 hattındaki kart | CS2 / PA8 hattındaki kart |
|---|---|---|
| `led on 0` | `0x02` – LED’i kapat | `0x02` – LED’i kapat |
| `led on 1` | `0x01` – LED’i aç | `0x02` – LED’i kapat |
| `led on 2` | `0x02` – LED’i kapat | `0x01` – LED’i aç |
| `led on 3` | `0x01` – LED’i aç | `0x01` – LED’i aç |

`led on 1` ve `led on 2` komutlarının hangi fiziksel kartı yaktığı, o kartın NSS girişini master’daki PA9 veya PA8 pinlerinden hangisine bağladığınıza göre belirlenir.

## Alıcı kartlarda ne oluyor?

### STM32F401RE – SPI komutu ve I²C geri bildirimi

Alıcı STM32F401RE, `HAL_SPI_Receive()` ile SPI1 üzerinden bir bayt bekler. `0x01` aldığında LED çıkışını yüksek seviyeye çeker ve `i2c_slave_send(&timer, 0x01)` çağrısını yapar. `0x02` aldığında LED çıkışını düşük seviyeye çeker ve aynı fonksiyona `0x02` verir.

Master, **7 bitlik `0x28` adresine** okuma isteği gönderdiğinde bu baytı alabilir. Geri bildirim, alıcının LED çıkışına uyguladığı komutu temsil eder.

### STM32F407 – SPI ile LED kontrolü

STM32F407 de `HAL_SPI_Receive()` ile SPI1 üzerinden bir bayt bekler. Alım başarılıysa `0x01` geldiğinde `led1` çıkışını yüksek, `0x02` geldiğinde düşük seviyeye çeker. Bu kartın ana döngüsündeki haberleşme SPI komutunun alınmasıyla tamamlanır.

Her iki alıcının SPI1 ayarları **slave, 8 bit, MSB first, Mode 0 ve donanımsal NSS girişi** şeklindedir.

## Master kartın bağlantıları

| Sinyal | Master pini | Bağlantı |
|---|---|---|
| SPI SCK | PA10 | Her iki alıcının SPI1 saat girişi |
| SPI MOSI | PA12 | Her iki alıcının SPI1 MOSI girişi |
| SPI CS | PA9 | Bir alıcının NSS girişi |
| SPI CS2 | PA8 | Diğer alıcının NSS girişi |
| SPI MISO | PA11 | Giriş olarak ayarlı, örnekte veri okunmuyor |
| I²C SDA / SCL | PA4 / PA6 | Alıcı STM32F401RE’nin yazılımsal I²C veri ve saat hatları |
| USART2 TX / RX | PA2 / PA3 | Bilgisayarla seri terminal bağlantısı |
| LED / Buton | PA5 / PC13 | Master kart üzerindeki yerel LED ve kullanıcı butonu |

Kartların GND uçları ortak bağlanmalıdır. Master üzerinde SDA ve SCL **open-drain** olarak ayarlı ve dahili pull-up dirençleri kapalıdır. Her iki I²C hattında da pull-up bulunmalıdır.

![Master STM32F401RE pin bağlantıları](./11111.png)

![Master GPIO çıkış tipleri ve başlangıç seviyeleri](./333333.png)

## Master saat ve TIM3 ayarları

Master kartta **8 MHz HSE** kaynağından PLL ile **84 MHz sistem saati** elde edilir. APB1 çevre birimi saati **42 MHz**, TIM3’ün giriş saati ise **84 MHz**’dir.

![Master kartın CubeMX saat ağacı](./22222.png)

TIM3 prescaler değeri `84 - 1` olarak ayarlanmıştır. Böylece sayaç **1 MHz** frekansta çalışır ve her **1 µs**’de bir artar. `software_i2c.c` içindeki `mikrosaniye_timer()` fonksiyonu bu sayacı okur. Hem I²C sürücüsü hem de `software_spi.c` içindeki `send_spi()` fonksiyonu, saat geçişleri arasındaki beklemeleri bu şekilde oluşturur.

![Master TIM3 prescaler ve sayaç ayarları](./44444.png)
