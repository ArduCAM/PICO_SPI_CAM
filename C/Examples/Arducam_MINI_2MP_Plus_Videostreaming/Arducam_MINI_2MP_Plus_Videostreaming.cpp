#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "ArduCAM.h"
#include "hardware/irq.h"
#include "ov2640_regs.h"
#include <cstdlib>
#include "stdio.h"
#include "bsp/board.h"
#include "tusb.h"
#include "pico/mutex.h"


static mutex_t usb_mutex;
void SerialUsb(uint8_t* buffer,uint32_t lenght);
int SerialUSBAvailable(void);
int SerialUsbRead(void);
#define BMPIMAGEOFFSET 66
uint8_t bmp_header[BMPIMAGEOFFSET] =
{
  0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
  0x00, 0x00
};

// set pin 10 as the slave select for the digital pot:
const uint8_t CS = 5;
bool is_header = false;
int mode = 0;
uint8_t start_capture = 0;
ArduCAM myCAM( OV2640, CS );
uint8_t read_fifo_burst(ArduCAM myCAM);
int main() 
{
  int value=0;
  uint8_t vid, pid;
  uint8_t cameraCommand;
  stdio_init_all();
  tusb_init();
	// put your setup code here, to run once:
	myCAM.Arducam_init();	
  gpio_init(CS);
  gpio_set_dir(CS, GPIO_OUT);
  gpio_put(CS, 1);
    //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  sleep_ms(100);
  myCAM.write_reg(0x07, 0x00);
  sleep_ms(100);
	while (1) {
			//Check if the ArduCAM SPI bus is OK
			myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
			cameraCommand = myCAM.read_reg(ARDUCHIP_TEST1);
			if (cameraCommand != 0x55) {
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

	while (1) 
  {
    uint8_t cameraCommand_last = 0;
    uint8_t is_header = 0;
    
    if(SerialUSBAvailable())
    {
      usart_Command=SerialUsbRead();
      switch (usart_Command)
      {
        case 0:
          myCAM.OV2640_set_JPEG_size(OV2640_160x120);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_160x120 END");
        usart_Command = 0xff;
        break;
        case 1:
          myCAM.OV2640_set_JPEG_size(OV2640_176x144);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_176x144 END");
        usart_Command = 0xff;
        break;
        case 2: 
          myCAM.OV2640_set_JPEG_size(OV2640_320x240);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_320x240 END");
        usart_Command = 0xff;
        break;
        case 3:
        myCAM.OV2640_set_JPEG_size(OV2640_352x288);sleep_ms(1000);
        printf("ACK CMD switch to OV2640_352x288 END");
        usart_Command = 0xff;
        break;
        case 4:
          myCAM.OV2640_set_JPEG_size(OV2640_640x480);sleep_ms(1000);
          printf("ACK CMD switch to OV2640_640x480 END");
        usart_Command = 0xff;
        break;
        case 5:
        myCAM.OV2640_set_JPEG_size(OV2640_800x600);sleep_ms(1000);
        printf("ACK CMD switch to OV2640_800x600 END");
        usart_Command = 0xff;
        break;
        case 6:
        myCAM.OV2640_set_JPEG_size(OV2640_1024x768);sleep_ms(1000);
        printf("ACK CMD switch to OV2640_1024x768 END");
        usart_Command = 0xff;
        break;
        case 7:
        myCAM.OV2640_set_JPEG_size(OV2640_1280x1024);sleep_ms(1000);
        printf("ACK CMD switch to OV2640_1280x1024 END");
        usart_Command = 0xff;
        break;
        case 8:
        myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);sleep_ms(1000);
        printf("ACK CMD switch to OV2640_1600x1200 END");
        usart_Command = 0xff;
        break;
        case 0x10:
        mode = 1;
        usart_Command = 0xff;
        start_capture = 1;
        printf("ACK CMD CAM start single shoot. END");
        break;
        case 0x11: 
        usart_Command = 0xff;
        myCAM.set_format(JPEG);
        myCAM.InitCAM();
        break;
        case 0x20:
        mode = 2;
        usart_Command = 0xff;
        start_capture = 2;
        printf("ACK CMD CAM start video streaming. END");
        break;
        case 0x30:
        mode = 3;
        usart_Command = 0xff;
        start_capture = 3;
        printf("ACK CMD CAM start single shoot. END");
        break;
        case 0x31:
        usart_Command = 0xff;
        myCAM.set_format(BMP);
        myCAM.InitCAM();
        #if !(defined (OV2640_MINI_2MP))        
        myCAM.clear_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
        #endif
        myCAM.wrSensorReg16_8(0x3818, 0x81);
        myCAM.wrSensorReg16_8(0x3621, 0xA7);
        break;
        case 0x40:
        myCAM.OV2640_set_Light_Mode(Auto);usart_Command = 0xff;
        printf("ACK CMD Set to Auto END");break;
        case 0x41:
        myCAM.OV2640_set_Light_Mode(Sunny);usart_Command = 0xff;
        printf("ACK CMD Set to Sunny END");break;
        case 0x42:
        myCAM.OV2640_set_Light_Mode(Cloudy);usart_Command = 0xff;
        printf("ACK CMD Set to Cloudy END");break;
        case 0x43:
        myCAM.OV2640_set_Light_Mode(Office);usart_Command = 0xff;
        printf("ACK CMD Set to Office END");break;
      case 0x44:
        myCAM.OV2640_set_Light_Mode(Home);   usart_Command = 0xff;
        printf("ACK CMD Set to Home END");break;
      case 0x50:
        myCAM.OV2640_set_Color_Saturation(Saturation2); usart_Command = 0xff;
        printf("ACK CMD Set to Saturation+2 END");break;
      case 0x51:
        myCAM.OV2640_set_Color_Saturation(Saturation1); usart_Command = 0xff;
        printf("ACK CMD Set to Saturation+1 END");break;
      case 0x52:
        myCAM.OV2640_set_Color_Saturation(Saturation0); usart_Command = 0xff;
        printf("ACK CMD Set to Saturation+0 END");break;
        case 0x53:
        myCAM. OV2640_set_Color_Saturation(Saturation_1); usart_Command = 0xff;
        printf("ACK CMD Set to Saturation-1 END");break;
        case 0x54:
        myCAM.OV2640_set_Color_Saturation(Saturation_2); usart_Command = 0xff;
        printf("ACK CMD Set to Saturation-2 END");break; 
      case 0x60:
        myCAM.OV2640_set_Brightness(Brightness2); usart_Command = 0xff;
        printf("ACK CMD Set to Brightness+2 END");break;
      case 0x61:
        myCAM.OV2640_set_Brightness(Brightness1); usart_Command = 0xff;
        printf("ACK CMD Set to Brightness+1 END");break;
      case 0x62:
        myCAM.OV2640_set_Brightness(Brightness0); usart_Command = 0xff;
        printf("ACK CMD Set to Brightness+0 END");break;
        case 0x63:
        myCAM. OV2640_set_Brightness(Brightness_1); usart_Command = 0xff;
        printf("ACK CMD Set to Brightness-1 END");break;
        case 0x64:
        myCAM.OV2640_set_Brightness(Brightness_2); usart_Command = 0xff;
        printf("ACK CMD Set to Brightness-2 END");break; 
        case 0x70:
          myCAM.OV2640_set_Contrast(Contrast2);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast+2 END");break; 
        case 0x71:
          myCAM.OV2640_set_Contrast(Contrast1);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast+1 END");break;
        case 0x72:
          myCAM.OV2640_set_Contrast(Contrast0);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast+0 END");break;
        case 0x73:
          myCAM.OV2640_set_Contrast(Contrast_1);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast-1 END");break;
      case 0x74:
          myCAM.OV2640_set_Contrast(Contrast_2);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast-2 END");break;
      case 0x80:
        myCAM.OV2640_set_Special_effects(Antique);usart_Command = 0xff;
        printf("ACK CMD Set to Antique END");break;
      case 0x81:
        myCAM.OV2640_set_Special_effects(Bluish);usart_Command = 0xff;
        printf("ACK CMD Set to Bluish END");break;
      case 0x82:
        myCAM.OV2640_set_Special_effects(Greenish);usart_Command = 0xff;
        printf("ACK CMD Set to Greenish END");break;  
      case 0x83:
        myCAM.OV2640_set_Special_effects(Reddish);usart_Command = 0xff;
        printf("ACK CMD Set to Reddish END");break;  
      case 0x84:
        myCAM.OV2640_set_Special_effects(BW);usart_Command = 0xff;
        printf("ACK CMD Set to BW END");break; 
      case 0x85:
        myCAM.OV2640_set_Special_effects(Negative);usart_Command = 0xff;
        printf("ACK CMD Set to Negative END");break; 
      case 0x86:
        myCAM.OV2640_set_Special_effects(BWnegative);usart_Command = 0xff;
        printf("ACK CMD Set to BWnegative END");break;   
      case 0x87:
        myCAM.OV2640_set_Special_effects(Normal);usart_Command = 0xff;
        printf("ACK CMD Set to Normal END");break;   
      }
    }
    if (mode == 1)
    {
      if (start_capture == 1)
      {
        myCAM.flush_fifo();
        myCAM.clear_fifo_flag();
        //Start capture
        myCAM.start_capture();
        start_capture = 0;
      }
      if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
      {
        printf("ACK CMD CAM Capture Done. END");
        read_fifo_burst(myCAM);
        //Clear the capture done flag
        myCAM.clear_fifo_flag();
      }
    }
    else if (mode == 2)
    {
        while (1)
        {
          if(SerialUSBAvailable())
          {
            usart_Command=SerialUsbRead();
          }
          if (usart_Command == 0x21)
          {
            start_capture = 0;
            mode = 0;
            usart_Command=0xff;
            printf("ACK CMD CAM stop video streaming. END");
            break;
          }
          switch (usart_Command)
          {
              case 0x40:
            myCAM.OV2640_set_Light_Mode(Auto);usart_Command = 0xff;
            printf("ACK CMD Set to Auto END");break;
              case 0x41:
            myCAM.OV2640_set_Light_Mode(Sunny);usart_Command = 0xff;
            printf("ACK CMD Set to Sunny END");break;
              case 0x42:
            myCAM.OV2640_set_Light_Mode(Cloudy);usart_Command = 0xff;
            printf("ACK CMD Set to Cloudy END");break;
              case 0x43:
            myCAM.OV2640_set_Light_Mode(Office);usart_Command = 0xff;
            printf("ACK CMD Set to Office END");break;
            case 0x44:
            myCAM.OV2640_set_Light_Mode(Home);   usart_Command = 0xff;
            printf("ACK CMD Set to Home END");break;
            case 0x50:
            myCAM.OV2640_set_Color_Saturation(Saturation2); usart_Command = 0xff;
              printf("ACK CMD Set to Saturation+2 END");break;
            case 0x51:
              myCAM.OV2640_set_Color_Saturation(Saturation1); usart_Command = 0xff;
              printf("ACK CMD Set to Saturation+1 END");break;
            case 0x52:
            myCAM.OV2640_set_Color_Saturation(Saturation0); usart_Command = 0xff;
              printf("ACK CMD Set to Saturation+0 END");break;
            case 0x53:
            myCAM. OV2640_set_Color_Saturation(Saturation_1); usart_Command = 0xff;
              printf("ACK CMD Set to Saturation-1 END");break;
            case 0x54:
              myCAM.OV2640_set_Color_Saturation(Saturation_2); usart_Command = 0xff;
              printf("ACK CMD Set to Saturation-2 END");break; 
            case 0x60:
            myCAM.OV2640_set_Brightness(Brightness2); usart_Command = 0xff;
              printf("ACK CMD Set to Brightness+2 END");break;
            case 0x61:
              myCAM.OV2640_set_Brightness(Brightness1); usart_Command = 0xff;
              printf("ACK CMD Set to Brightness+1 END");break;
            case 0x62:
            myCAM.OV2640_set_Brightness(Brightness0); usart_Command = 0xff;
              printf("ACK CMD Set to Brightness+0 END");break;
            case 0x63:
            myCAM. OV2640_set_Brightness(Brightness_1); usart_Command = 0xff;
              printf("ACK CMD Set to Brightness-1 END");break;
            case 0x64:
              myCAM.OV2640_set_Brightness(Brightness_2); usart_Command = 0xff;
              printf("ACK CMD Set to Brightness-2 END");break; 
            case 0x70:
              myCAM.OV2640_set_Contrast(Contrast2);usart_Command = 0xff;
              printf("ACK CMD Set to Contrast+2 END");break; 
            case 0x71:
              myCAM.OV2640_set_Contrast(Contrast1);usart_Command = 0xff;
              printf("ACK CMD Set to Contrast+1 END");break;
              case 0x72:
              myCAM.OV2640_set_Contrast(Contrast0);usart_Command = 0xff;
              printf("ACK CMD Set to Contrast+0 END");break;
            case 0x73:
              myCAM.OV2640_set_Contrast(Contrast_1);usart_Command = 0xff;
              printf("ACK CMD Set to Contrast-1 END");break;
            case 0x74:
              myCAM.OV2640_set_Contrast(Contrast_2);usart_Command = 0xff;
              printf("ACK CMD Set to Contrast-2 END");break;
            case 0x80:
            myCAM.OV2640_set_Special_effects(Antique);usart_Command = 0xff;
            printf("ACK CMD Set to Antique END");break;
            case 0x81:
            myCAM.OV2640_set_Special_effects(Bluish);usart_Command = 0xff;
            printf("ACK CMD Set to Bluish END");break;
            case 0x82:
            myCAM.OV2640_set_Special_effects(Greenish);usart_Command = 0xff;
            printf("ACK CMD Set to Greenish END");break;  
            case 0x83:
            myCAM.OV2640_set_Special_effects(Reddish);usart_Command = 0xff;
            printf("ACK CMD Set to Reddish END");break;  
            case 0x84:
            myCAM.OV2640_set_Special_effects(BW);usart_Command = 0xff;
            printf("ACK CMD Set to BW END");break; 
            case 0x85:
            myCAM.OV2640_set_Special_effects(Negative);usart_Command = 0xff;
            printf("ACK CMD Set to Negative END");break; 
            case 0x86:
            myCAM.OV2640_set_Special_effects(BWnegative);usart_Command = 0xff;
            printf("ACK CMD Set to BWnegative END");break;   
            case 0x87:
            myCAM.OV2640_set_Special_effects(Normal);usart_Command = 0xff;
            printf("ACK CMD Set to Normal END");break;  
          }   
          if (start_capture == 2)
          {
            myCAM.flush_fifo();
            myCAM.clear_fifo_flag();
            //Start capture
            myCAM.start_capture();
            start_capture = 0;
          }
          if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
          {
            read_fifo_burst(myCAM);
            start_capture = 2;
          }
         
        }
   }
    else if (mode == 3)
    {
      if (start_capture == 3)
      {
        //Flush the FIFO
        myCAM.flush_fifo();
        myCAM.clear_fifo_flag();
        //Start capture
        myCAM.start_capture();
        start_capture = 0;
      }
      if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
      {
        printf("ACK CMD CAM Capture Done.");
        uint8_t temp, temp_last;
        uint32_t length = 0;
        length = myCAM.read_fifo_length();
        if (length >= MAX_FIFO_SIZE ) 
        {
          printf("ACK CMD Over size.");
          myCAM.clear_fifo_flag();
          return 0;
        }
        if (length == 0 ) //0 kb
        {
          printf("ACK CMD Size is 0.");
          myCAM.clear_fifo_flag();
          return 0;
        }
        uint8_t symbol[2]={0};
        symbol[0]=0xff;
        symbol[1]=0xaa;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();//Set fifo burst mode
        SerialUsb(symbol, sizeof(symbol));
        SerialUsb(bmp_header,BMPIMAGEOFFSET);
        spi_read_blocking(SPI_PORT, BURST_FIFO_READ,symbol, 1);
        uint8_t VH, VL;
        int i = 0, j = 0;
        for (i = 0; i < 240; i++)
        {
          for (j = 0; j < 320; j++)
          {
            spi_read_blocking(SPI_PORT, BURST_FIFO_READ,&VH, 1);
            spi_read_blocking(SPI_PORT, BURST_FIFO_READ,&VL, 1);
            SerialUsb( &VL, 1);
            sleep_us(12);
            SerialUsb( &VH, 1);
            sleep_us(12);
          }
        }
        symbol[0]=0xbb;
        symbol[1]=0xcc;
        SerialUsb(symbol, sizeof(symbol));
        myCAM.CS_HIGH();
        //Clear the capture done flag
        myCAM.clear_fifo_flag();
      }
    }    
  }
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