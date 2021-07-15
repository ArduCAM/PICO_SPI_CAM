#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "ArduCAM.h"
#include "hardware/irq.h"
#include "ov2640_regs.h"
#include <cstdlib>
#include "bsp/board.h"
#include "tusb.h"
#include "pico/mutex.h"


static mutex_t usb_mutex;
void SerialUsb(uint8_t* buffer,uint32_t lenght);
int SerialUSBAvailable(void);
int SerialUsbRead(void);

const int CS1 = 5;
const int CS2 = 6;
const int CS3 = 7;
const int CS4 = 10;

uint8_t start_capture = 0;
ArduCAM myCAM1(OV2640, CS1);
ArduCAM myCAM2(OV2640, CS2);
ArduCAM myCAM3(OV2640, CS3);
ArduCAM myCAM4(OV2640, CS4);
uint8_t read_fifo_burst(ArduCAM myCAM);
uint8_t flag[5]={0xFF,0xAA,0x01,0xFF,0x55};

int main(void)
{
  uint8_t cam1=1,cam2=1,cam3=1,cam4=1;
  uint8_t vid, pid;
  uint8_t cameraCommand;
  stdio_init_all();
  tusb_init();
  myCAM1.Arducam_init();	
  gpio_init(CS1);
  gpio_set_dir(CS1, GPIO_OUT);
  gpio_put(CS1, 1);
  gpio_init(CS2);
  gpio_set_dir(CS2, GPIO_OUT);
  gpio_put(CS2, 1);
  gpio_init(CS3);
  gpio_set_dir(CS3, GPIO_OUT);
  gpio_put(CS3, 1);
  gpio_init(CS4);
  gpio_set_dir(CS4, GPIO_OUT);
  gpio_put(CS4, 1);
  myCAM1.write_reg(0x07, 0x80);
  sleep_ms(100);
  myCAM1.write_reg(0x07, 0x00);
  sleep_ms(100);
  myCAM2.write_reg(0x07, 0x80);
  sleep_ms(100);
  myCAM2.write_reg(0x07, 0x00);
  sleep_ms(100);
  myCAM3.write_reg(0x07, 0x80);
  sleep_ms(100);
  myCAM3.write_reg(0x07, 0x00);
  sleep_ms(100);
  myCAM4.write_reg(0x07, 0x80);
  sleep_ms(100);
  myCAM4.write_reg(0x07, 0x00);
  sleep_ms(100);
    while (1) 
    {
//			Check if the ArduCAM SPI bus is OK
			myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
			cameraCommand = myCAM1.read_reg(ARDUCHIP_TEST1);
			if (cameraCommand != 0x55)
      {
        cam1=0;
				printf(" SPI1 interface Error!");
			} 
			myCAM2.write_reg(ARDUCHIP_TEST1, 0x55);
			cameraCommand = myCAM2.read_reg(ARDUCHIP_TEST1);
			if (cameraCommand != 0x55)
      {
        cam2=0;
				printf(" SPI2 interface Error!");
			} 

			myCAM3.write_reg(ARDUCHIP_TEST1, 0x55);
			cameraCommand = myCAM3.read_reg(ARDUCHIP_TEST1);
			if (cameraCommand != 0x55)
      {
        cam3=0;
				printf(" SPI3 interface Error!");
			} 

			myCAM4.write_reg(ARDUCHIP_TEST1, 0x55);
			cameraCommand = myCAM4.read_reg(ARDUCHIP_TEST1);
			if (cameraCommand != 0x55)
      {
        cam4=0;
				printf(" SPI4 interface Error!");
			} 
      if(!(cam1||cam2||cam3||cam4))
      {
        sleep_ms(1000);continue;
      }
      else
      {
        break;
      }
    }
    while (1) 
    {
      //Check if the camera module type is OV2640
      myCAM1.wrSensorReg8_8(0xff, 0x01);
      myCAM1.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
      myCAM1.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
      if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
        printf("Can't find OV2640 module!");
        sleep_ms(1000); continue;
      }
      else {
        printf("OV2640 detected.END"); break;
      }
    }
    myCAM1.set_format(JPEG);
    myCAM1.InitCAM();
    myCAM1.OV2640_set_JPEG_size(OV2640_320x240);
    sleep_ms(1000);
    myCAM1.clear_fifo_flag();
    myCAM2.clear_fifo_flag();
    myCAM3.clear_fifo_flag();
    myCAM4.clear_fifo_flag();
  while(1)
  {
    uint8_t temp = 0xff, temp_last = 0;
    uint8_t start_capture = 0;
    uint8_t finish_count;
    if(SerialUSBAvailable())
    {
      usart_Command=SerialUsbRead();
      switch (usart_Command)
      {
         case 0:
          myCAM1.OV2640_set_JPEG_size(OV2640_160x120);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_160x120 END");
          usart_Command = 0xff;
        break;
        case 1:
          myCAM1.OV2640_set_JPEG_size(OV2640_176x144);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_176x144 END");
          usart_Command = 0xff;
        break;
        case 2: 
          myCAM1.OV2640_set_JPEG_size(OV2640_320x240);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_320x240 END");
          usart_Command = 0xff;
        break;
        case 3:
          usart_Command = 0xff;
          myCAM1.OV2640_set_JPEG_size(OV2640_352x288);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_352x288 END");
        break;
        case 4:
          usart_Command = 0xff;
          myCAM1.OV2640_set_JPEG_size(OV2640_640x480);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_640x480 END");
        break;
        case 5:
          usart_Command = 0xff;
          myCAM1.OV2640_set_JPEG_size(OV2640_800x600);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_800x600 END");
        break;
        case 6:
          usart_Command = 0xff;
          myCAM1.OV2640_set_JPEG_size(OV2640_1024x768);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_1024x768 END");
        break;
        case 7:
        usart_Command = 0xff;
        myCAM1.OV2640_set_JPEG_size(OV2640_1280x1024);sleep_ms(1000);
        printf("ACK CMD switch to OV2640_1280x1024 END");
        break;
        case 8:
        usart_Command = 0xff;
        myCAM1.OV2640_set_JPEG_size(OV2640_1600x1200);sleep_ms(1000);
        printf("ACK CMD switch to OV2640_1600x1200 END");
        break;
      case 0x10: 
      if(cam1)
      {
        flag[2]=0x01;//flag of cam1
        sleep_ms(1);
        SerialUsb(flag, 5);
        read_fifo_burst(myCAM1);
      }
      if(cam2)
      {
        flag[2]=0x02;//flag of cam1
        sleep_ms(1);
        SerialUsb(flag, 5);
        read_fifo_burst(myCAM2);
      }
      if(cam3)
      {
        flag[2]=0x03;//flag of cam1
        sleep_ms(1);
        SerialUsb(flag, 5);
        read_fifo_burst(myCAM3); 
      } 
      if(cam4)
      {
        flag[2]=0x04;//flag of cam1
        sleep_ms(1);
        SerialUsb(flag, 5);
        read_fifo_burst(myCAM4);

      }  
      break;
      case 0x20: 
      while(1)
      {
        if(SerialUSBAvailable())
        {
          usart_Command=SerialUsbRead();
        }
        if (usart_Command == 0x21)
        {
          usart_Command=0xff;
          break;
        }
        
        if(cam1)
        {
          flag[2]=0x01;//flag of cam1
          sleep_ms(1);
          SerialUsb(flag, 5);
          read_fifo_burst(myCAM1);
        }
        if(cam2)
        {
          flag[2]=0x02;//flag of cam1
          sleep_ms(1);
          SerialUsb(flag, 5);
          read_fifo_burst(myCAM2); 
        }
        if(cam3)
        {
          flag[2]=0x03;//flag of cam1
          sleep_ms(1);
          SerialUsb(flag, 5);
          read_fifo_burst(myCAM3); 
        } 
        if(cam4)
        {
          flag[2]=0x04;//flag of cam1
          sleep_ms(1);
          SerialUsb(flag, 5);
          read_fifo_burst(myCAM4);
        }  
      }
        break;
      }
   }
   
  }
   return 0;
}


uint8_t read_fifo_burst(ArduCAM myCAM)
{
  int i , count;
  int length = myCAM.read_fifo_length();
  uint8_t * imageBuf =(uint8_t *) malloc(length*sizeof(uint8_t));
  i = 0 ;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  spi_read_blocking(SPI_PORT, BURST_FIFO_READ,imageBuf, length);
  myCAM.CS_HIGH();
  SerialUsb(imageBuf, length);
  free(imageBuf);
  return 1;
}


void SerialUsb(uint8_t* buf,uint32_t length)
{
    static uint64_t last_avail_time;
    int i = 0;
    if (tud_cdc_connected()) 
    {
        for (int i = 0; i < length;) 
        {
            int n = length - i;
            int avail = tud_cdc_write_available();
            if (n > avail) n = avail;
            if (n) 
            {
                int n2 = tud_cdc_write(buf + i, n);
                tud_task();
                tud_cdc_write_flush();
                i += n2;
                last_avail_time = time_us_64();
            } 
            else 
            {
                tud_task();
                tud_cdc_write_flush();
                if (!tud_cdc_connected() ||
                    (!tud_cdc_write_available() && time_us_64() > last_avail_time + 1000000 /* 1 second */)) {
                    break;
                }
            }
        }
    } 
    else 
    {
        // reset our timeout
        last_avail_time = 0;
    }
}

int SerialUSBAvailable(void)
{
  return tud_cdc_available();
} 

int SerialUsbRead(void) 
{
  if (tud_cdc_connected() && tud_cdc_available()) 
  {
    return tud_cdc_read_char();
  }
  return -1;
}