#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "ArduCAM.h"
#include "hardware/irq.h"
#include "ov2640_regs.h"
// set pin 7 as the slave select for the digital pot:
const int CS = 5;
bool is_header = false;
int mode = 0;
uint8_t start_capture = 0;
ArduCAM myCAM( OV2640, CS );
uint8_t read_fifo_burst(ArduCAM myCAM);

int main() {
	  // put your setup code here, to run once:
	myCAM.Arducam_uart_init(115200);
	printf("start\r\n");	
	#if 0
	 const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
    }
	#endif
  uint8_t vid, pid;
  uint8_t temp;
	   //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  sleep_ms(100);
  myCAM.write_reg(0x07, 0x00);
  sleep_ms(100);
		printf("start1\r\n");
	while (1) {
			//Check if the ArduCAM SPI bus is OK
			myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
			temp = myCAM.read_reg(ARDUCHIP_TEST1);
			if (temp != 0x55) {
				printf(" SPI interface Error!");
				sleep_ms(1000); continue;
			} else {
				printf("ACK CMD SPI interface OK.END"); break;
			}
	}
	while (1) {
		//Check if the camera module type is OV2640
		myCAM.wrSensorReg8_8(0xff, 0x01);
		myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
		myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
		if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
			printf("Can't find OV2640 module!");
			sleep_ms(1000); continue;
		}
		else {
			printf("OV2640 detected.END"); break;
		}
  }

	 //Change to JPEG capture mode and initialize the OV5642 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  sleep_ms(1000);
  myCAM.clear_fifo_flag();
	while (1) {
   int i , count;
    uint8_t value[1024*40];
    //Flush the FIFO
    myCAM.flush_fifo();
    //Start capture
    myCAM.start_capture(); 
    while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK)){;}
    int length = myCAM.read_fifo_length();
    count = length;
    i = 0 ;
    myCAM.CS_LOW();
    myCAM.set_fifo_burst();//Set fifo burst mode
    spi_read_blocking(SPI_PORT, BURST_FIFO_READ,value, length);
    uart_write_blocking(UART_ID, value, length);
    count = 0;
    myCAM.CS_HIGH();
	
	}
	return 0;
}

