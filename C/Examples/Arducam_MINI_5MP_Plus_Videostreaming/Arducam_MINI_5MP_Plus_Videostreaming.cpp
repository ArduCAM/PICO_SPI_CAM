#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "ArduCAM.h"
#include "hardware/irq.h"
#include "ov5642_regs.h"
#include <cstdlib>
#include "stdio.h"
#include "bsp/board.h"
#include "tusb.h"
#include "pico/mutex.h"

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
// set pin 7 as the slave select for the digital pot:
// set pin 7 as the slave select for the digital pot:
const uint8_t CS = 5;
bool is_header = false;
int mode = 0;
uint8_t start_capture = 0;
ArduCAM myCAM( OV5642, CS );
uint8_t read_fifo_burst(ArduCAM myCAM);

int main() 
{
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
	while (1) 
  {
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

  
  while (1) 
  {
    //Check if the camera module type is OV5640
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if((vid != 0x56) || (pid != 0x42))
    {
      printf("Can't find OV5642 module!");
      sleep_ms(1000); continue;
    }
    else 
    {
      printf("OV5642 detected.END"); break;
    }
  }

  //Change to JPEG capture mode and initialize the OV5642 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
  sleep_ms(1000);
  myCAM.clear_fifo_flag();
  myCAM.write_reg(ARDUCHIP_FRAMES,0x00);
	while(1) 
  {
    uint8_t cameraCommand_last = 0;
    uint8_t is_header = 0;
    if(SerialUSBAvailable())
    {
      usart_Command=SerialUsbRead();
      switch (usart_Command)
      {
        case 0:
        myCAM.OV5642_set_JPEG_size(OV5642_320x240);sleep_ms(1000);
        printf("ACK CMD switch to OV5642_320x240");
        usart_Command = 0xff;
        break;
        case 1:
        myCAM.OV5642_set_JPEG_size(OV5642_640x480);sleep_ms(1000);
        printf("ACK CMD switch to OV5642_640x480");
        usart_Command = 0xff;
        break;
        case 2: 
        myCAM.OV5642_set_JPEG_size(OV5642_1024x768);sleep_ms(1000);
        printf("ACK CMD switch to OV5642_1024x768");
        usart_Command = 0xff;
        break;
        case 3:
        usart_Command = 0xff;
        myCAM.OV5642_set_JPEG_size(OV5642_1280x960);sleep_ms(1000);
        printf("ACK CMD switch to OV5642_1280x960");
        break;
        case 4:
        usart_Command = 0xff;
        myCAM.OV5642_set_JPEG_size(OV5642_1600x1200);sleep_ms(1000);
        printf("ACK CMD switch to OV5642_1600x1200");
        break;
        case 5:
        usart_Command = 0xff;
        myCAM.OV5642_set_JPEG_size(OV5642_2048x1536);sleep_ms(1000);
        printf("ACK CMD switch to OV5642_2048x1536");
        break;
        case 6:
        usart_Command = 0xff;
        myCAM.OV5642_set_JPEG_size(OV5642_2592x1944);sleep_ms(1000);
        printf("ACK CMD switch to OV5642_2592x1944");
        break;
        case 0x10:
        mode = 1;
        usart_Command = 0xff;
        start_capture = 1;
  //      printf("ACK CMD CAM start single shoot.");
        break;
        case 0x11: 
        usart_Command = 0xff;
        myCAM.set_format(JPEG);
        myCAM.InitCAM();
        // #if !(defined (OV2640_MINI_2MP))
        myCAM.set_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
       // #endif
        break;
        case 0x20:
        mode = 2;
        usart_Command = 0xff;
        start_capture = 2;
        printf("ACK CMD CAM start video streaming.");
        break;
        case 0x30:
        mode = 3;
        usart_Command = 0xff;
        start_capture = 3;
        printf("CAM start single shoot.");
        break;
        case 0x31:
        usart_Command = 0xff;
        myCAM.set_format(BMP);
        myCAM.InitCAM();     
        myCAM.clear_bit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
        myCAM.wrSensorReg16_8(0x3818, 0x81);
        myCAM.wrSensorReg16_8(0x3621, 0xA7);
        break;
        case 0x40:
        myCAM.OV5642_set_Light_Mode(Advanced_AWB);usart_Command = 0xff;
        printf("ACK CMD Set to Advanced_AWB");break;
        case 0x41:
        myCAM.OV5642_set_Light_Mode(Simple_AWB);usart_Command = 0xff;
        printf("ACK CMD Set to Simple_AWB");break;
        case 0x42:
        myCAM.OV5642_set_Light_Mode(Manual_day);usart_Command = 0xff;
        printf("ACK CMD Set to Manual_day");break;
        case 0x43:
        myCAM.OV5642_set_Light_Mode(Manual_A);usart_Command = 0xff;
        printf("ACK CMD Set to Manual_A");break;
        case 0x44:
        myCAM.OV5642_set_Light_Mode(Manual_cwf);usart_Command = 0xff;
        printf("ACK CMD Set to Manual_cwf");break;
        case 0x45:
        myCAM.OV5642_set_Light_Mode(Manual_cloudy);usart_Command = 0xff;
        printf("ACK CMD Set to Manual_cloudy");break;
        case 0x50:
        myCAM.OV5642_set_Color_Saturation(Saturation4);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation+4");break;
        case 0x51:
        myCAM.OV5642_set_Color_Saturation(Saturation3);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation+3");break;
        case 0x52:
        myCAM.OV5642_set_Color_Saturation(Saturation2);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation+2");break;
        case 0x53:
        myCAM.OV5642_set_Color_Saturation(Saturation1);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation+1");break;
        case 0x54:
        myCAM.OV5642_set_Color_Saturation(Saturation0);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation+0");break;
        case 0x55:
        myCAM.OV5642_set_Color_Saturation(Saturation_1);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation-1");break;
        case 0x56:
        myCAM.OV5642_set_Color_Saturation(Saturation_2);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation-2");break;
        case 0x57:
        myCAM.OV5642_set_Color_Saturation(Saturation_3);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation-3");break;
        case 0x58:
        myCAM.OV5642_set_Light_Mode(Saturation_4);usart_Command = 0xff;
        printf("ACK CMD Set to Saturation-4");break; 
        case 0x60:
        myCAM.OV5642_set_Brightness(Brightness4);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness+4");break;
        case 0x61:
        myCAM.OV5642_set_Brightness(Brightness3);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness+3");break; 
        case 0x62:
        myCAM.OV5642_set_Brightness(Brightness2);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness+2");break; 
        case 0x63:
        myCAM.OV5642_set_Brightness(Brightness1);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness+1");break; 
        case 0x64:
        myCAM.OV5642_set_Brightness(Brightness0);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness+0");break; 
        case 0x65:
        myCAM.OV5642_set_Brightness(Brightness_1);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness-1");break; 
        case 0x66:
        myCAM.OV5642_set_Brightness(Brightness_2);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness-2");break; 
        case 0x67:
        myCAM.OV5642_set_Brightness(Brightness_3);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness-3");break; 
        case 0x68:
        myCAM.OV5642_set_Brightness(Brightness_4);usart_Command = 0xff;
        printf("ACK CMD Set to Brightness-4");break;
        case 0x70:
        myCAM.OV5642_set_Contrast(Contrast4);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast+4");break;
        case 0x71:
        myCAM.OV5642_set_Contrast(Contrast3);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast+3");break; 
        case 0x72:
        myCAM.OV5642_set_Contrast(Contrast2);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast+2");break; 
        case 0x73:
        myCAM.OV5642_set_Contrast(Contrast1);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast+1");break; 
        case 0x74:
        myCAM.OV5642_set_Contrast(Contrast0);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast+0");break; 
        case 0x75:
        myCAM.OV5642_set_Contrast(Contrast_1);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast-1");break; 
        case 0x76:
        myCAM.OV5642_set_Contrast(Contrast_2);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast-2");break; 
        case 0x77:
        myCAM.OV5642_set_Contrast(Contrast_3);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast-3");break; 
        case 0x78:
        myCAM.OV5642_set_Contrast(Contrast_4);usart_Command = 0xff;
        printf("ACK CMD Set to Contrast-4");break;
        case 0x80: 
        myCAM.OV5642_set_hue(degree_180);usart_Command = 0xff;
        printf("ACK CMD Set to -180 degree");break;   
        case 0x81: 
        myCAM.OV5642_set_hue(degree_150);usart_Command = 0xff;
        printf("ACK CMD Set to -150 degree");break;  
        case 0x82: 
        myCAM.OV5642_set_hue(degree_120);usart_Command = 0xff;
        printf("ACK CMD Set to -120 degree");break;  
        case 0x83: 
        myCAM.OV5642_set_hue(degree_90);usart_Command = 0xff;
        printf("ACK CMD Set to -90 degree");break;   
        case 0x84: 
        myCAM.OV5642_set_hue(degree_60);usart_Command = 0xff;
        printf("ACK CMD Set to -60 degree");break;   
        case 0x85: 
        myCAM.OV5642_set_hue(degree_30);usart_Command = 0xff;
        printf("ACK CMD Set to -30 degree");break;  
        case 0x86: 
        myCAM.OV5642_set_hue(degree_0);usart_Command = 0xff;
        printf("ACK CMD Set to 0 degree");break; 
        case 0x87: 
        myCAM.OV5642_set_hue(degree30);usart_Command = 0xff;
        printf("ACK CMD Set to 30 degree");break;
        case 0x88: 
        myCAM.OV5642_set_hue(degree60);usart_Command = 0xff;
        printf("ACK CMD Set to 60 degree");break;
        case 0x89: 
        myCAM.OV5642_set_hue(degree90);usart_Command = 0xff;
        printf("ACK CMD Set to 90 degree");break;
        case 0x8a: 
        myCAM.OV5642_set_hue(degree120);usart_Command = 0xff;
        printf("ACK CMD Set to 120 degree");break ; 
        case 0x8b: 
        myCAM.OV5642_set_hue(degree150);usart_Command = 0xff;
        printf("ACK CMD Set to 150 degree");break ;
        case 0x90: 
        myCAM.OV5642_set_Special_effects(Normal);usart_Command = 0xff;
        printf("ACK CMD Set to Normal");break ;
        case 0x91: 
        myCAM.OV5642_set_Special_effects(BW);usart_Command = 0xff;
        printf("ACK CMD Set to BW");break ;
        case 0x92: 
        myCAM.OV5642_set_Special_effects(Bluish);usart_Command = 0xff;
        printf("ACK CMD Set to Bluish");break ;
        case 0x93: 
        myCAM.OV5642_set_Special_effects(Sepia);usart_Command = 0xff;
        printf("ACK CMD Set to Sepia");break ;
        case 0x94: 
        myCAM.OV5642_set_Special_effects(Reddish);usart_Command = 0xff;
        printf("ACK CMD Set to Reddish");break ;
        case 0x95: 
        myCAM.OV5642_set_Special_effects(Greenish);usart_Command = 0xff;
        printf("ACK CMD Set to Greenish");break ;
        case 0x96: 
        myCAM.OV5642_set_Special_effects(Negative);usart_Command = 0xff;
        printf("ACK CMD Set to Negative");break ;
        case 0xA0: 
        myCAM.OV5642_set_Exposure_level(Exposure_17_EV);usart_Command = 0xff;
        printf("ACK CMD Set to -1.7EV");break ;  
        case 0xA1: 
        myCAM.OV5642_set_Exposure_level(Exposure_13_EV);usart_Command = 0xff;
        printf("ACK CMD Set to -1.3EV");break ;
        case 0xA2: 
        myCAM.OV5642_set_Exposure_level(Exposure_10_EV);usart_Command = 0xff;
        printf("ACK CMD Set to -1.0EV");break ; 
        case 0xA3: 
        myCAM.OV5642_set_Exposure_level(Exposure_07_EV);usart_Command = 0xff;
        printf("ACK CMD Set to -0.7EV");break ;
        case 0xA4: 
        myCAM.OV5642_set_Exposure_level(Exposure_03_EV);usart_Command = 0xff;
        printf("ACK CMD Set to -0.3EV");break ;
        case 0xA5: 
        myCAM.OV5642_set_Exposure_level(Exposure_default);usart_Command = 0xff;
        printf("ACK CMD Set to -Exposure_default");break ;
        case 0xA6: 
        myCAM.OV5642_set_Exposure_level(Exposure07_EV);usart_Command = 0xff;
        printf("ACK CMD Set to 0.7EV");break ;  
        case 0xA7: 
        myCAM.OV5642_set_Exposure_level(Exposure10_EV);usart_Command = 0xff;
        printf("ACK CMD Set to 1.0EV");break ;
        case 0xA8: 
        myCAM.OV5642_set_Exposure_level(Exposure13_EV);usart_Command = 0xff;
        printf("ACK CMD Set to 1.3EV");break ; 
        case 0xA9: 
        myCAM.OV5642_set_Exposure_level(Exposure17_EV);usart_Command = 0xff;
        printf("ACK CMD Set to 1.7EV");break ; 
        case 0xB0: 
        myCAM.OV5642_set_Sharpness(Auto_Sharpness_default);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Sharpness default");break ; 
        case 0xB1: 
        myCAM.OV5642_set_Sharpness(Auto_Sharpness1);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Sharpness +1");break ; 
        case 0xB2: 
        myCAM.OV5642_set_Sharpness(Auto_Sharpness2);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Sharpness +2");break ; 
        case 0xB3: 
        myCAM.OV5642_set_Sharpness(Manual_Sharpnessoff);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Manual Sharpness off");break ; 
        case 0xB4: 
        myCAM.OV5642_set_Sharpness(Manual_Sharpness1);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Manual Sharpness +1");break ;
        case 0xB5: 
        myCAM.OV5642_set_Sharpness(Manual_Sharpness2);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Manual Sharpness +2");break ; 
        case 0xB6: 
        myCAM.OV5642_set_Sharpness(Manual_Sharpness3);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Manual Sharpness +3");break ;
        case 0xB7: 
        myCAM.OV5642_set_Sharpness(Manual_Sharpness4);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Manual Sharpness +4");break ;
        case 0xB8: 
        myCAM.OV5642_set_Sharpness(Manual_Sharpness5);usart_Command = 0xff;
        printf("ACK CMD Set to Auto Manual Sharpness +5");break ;  
        case 0xC0: 
        myCAM.OV5642_set_Mirror_Flip(MIRROR);usart_Command = 0xff;
        printf("ACK CMD Set to MIRROR");break ;  
        case 0xC1: 
        myCAM.OV5642_set_Mirror_Flip(FLIP);usart_Command = 0xff;
        printf("ACK CMD Set to FLIP");break ; 
        case 0xC2: 
        myCAM.OV5642_set_Mirror_Flip(MIRROR_FLIP);usart_Command = 0xff;
        printf("ACK CMD Set to MIRROR&FLIP");break ;
        case 0xC3: 
        myCAM.OV5642_set_Mirror_Flip(Normal);usart_Command = 0xff;
        printf("ACK CMD Set to Normal");break ;
        case 0xD0: 
        myCAM.OV5642_set_Compress_quality(high_quality);usart_Command = 0xff;
        printf("ACK CMD Set to high quality");break ;
        case 0xD1: 
        myCAM.OV5642_set_Compress_quality(default_quality);usart_Command = 0xff;
        printf("ACK CMD Set to default quality");break ;
        case 0xD2: 
        myCAM.OV5642_set_Compress_quality(low_quality);usart_Command = 0xff;
        printf("ACK CMD Set to low quality");break ;

        case 0xE0: 
        myCAM.OV5642_Test_Pattern(Color_bar);usart_Command = 0xff;
        printf("ACK CMD Set to Color bar");break ;
        case 0xE1: 
        myCAM.OV5642_Test_Pattern(Color_square);usart_Command = 0xff;
        printf("ACK CMD Set to Color square");break ;
        case 0xE2: 
        myCAM.OV5642_Test_Pattern(BW_square);usart_Command = 0xff;
        printf("ACK CMD Set to BW square");break ;
        case 0xE3: 
        myCAM.OV5642_Test_Pattern(DLI);usart_Command = 0xff;
        printf("ACK CMD Set to DLI");break ;
        default:
        break;
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
        // printf("ACK CMD CAM Capture Done.");
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
            printf("ACK CMD CAM stop video streaming. END");
            break;
          }
          switch (usart_Command)
          {
            case 0x40:
            myCAM.OV5642_set_Light_Mode(Advanced_AWB);usart_Command = 0xff;
            printf("ACK CMD Set to Advanced_AWB");break;
            case 0x41:
            myCAM.OV5642_set_Light_Mode(Simple_AWB);usart_Command = 0xff;
            printf("ACK CMD Set to Simple_AWB");break;
            case 0x42:
            myCAM.OV5642_set_Light_Mode(Manual_day);usart_Command = 0xff;
            printf("ACK CMD Set to Manual_day");break;
            case 0x43:
            myCAM.OV5642_set_Light_Mode(Manual_A);usart_Command = 0xff;
            printf("ACK CMD Set to Manual_A");break;
            case 0x44:
            myCAM.OV5642_set_Light_Mode(Manual_cwf);usart_Command = 0xff;
            printf("ACK CMD Set to Manual_cwf");break;
            case 0x45:
            myCAM.OV5642_set_Light_Mode(Manual_cloudy);usart_Command = 0xff;
            printf("ACK CMD Set to Manual_cloudy");break;
            case 0x50:
            myCAM.OV5642_set_Color_Saturation(Saturation4);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation+4");break;
            case 0x51:
            myCAM.OV5642_set_Color_Saturation(Saturation3);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation+3");break;
            case 0x52:
            myCAM.OV5642_set_Color_Saturation(Saturation2);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation+2");break;
            case 0x53:
            myCAM.OV5642_set_Color_Saturation(Saturation1);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation+1");break;
            case 0x54:
            myCAM.OV5642_set_Color_Saturation(Saturation0);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation+0");break;
            case 0x55:
            myCAM.OV5642_set_Color_Saturation(Saturation_1);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation-1");break;
            case 0x56:
            myCAM.OV5642_set_Color_Saturation(Saturation_2);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation-2");break;
            case 0x57:
            myCAM.OV5642_set_Color_Saturation(Saturation_3);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation-3");break;
            case 0x58:
            myCAM.OV5642_set_Light_Mode(Saturation_4);usart_Command = 0xff;
            printf("ACK CMD Set to Saturation-4");break; 
            case 0x60:
            myCAM.OV5642_set_Brightness(Brightness4);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness+4");break;
            case 0x61:
            myCAM.OV5642_set_Brightness(Brightness3);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness+3");break; 
            case 0x62:
            myCAM.OV5642_set_Brightness(Brightness2);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness+2");break; 
            case 0x63:
            myCAM.OV5642_set_Brightness(Brightness1);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness+1");break; 
            case 0x64:
            myCAM.OV5642_set_Brightness(Brightness0);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness+0");break; 
            case 0x65:
            myCAM.OV5642_set_Brightness(Brightness_1);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness-1");break; 
            case 0x66:
            myCAM.OV5642_set_Brightness(Brightness_2);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness-2");break; 
            case 0x67:
            myCAM.OV5642_set_Brightness(Brightness_3);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness-3");break; 
            case 0x68:
            myCAM.OV5642_set_Brightness(Brightness_4);usart_Command = 0xff;
            printf("ACK CMD Set to Brightness-4");break;
            case 0x70:
            myCAM.OV5642_set_Contrast(Contrast4);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast+4");break;
            case 0x71:
            myCAM.OV5642_set_Contrast(Contrast3);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast+3");break; 
            case 0x72:
            myCAM.OV5642_set_Contrast(Contrast2);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast+2");break; 
            case 0x73:
            myCAM.OV5642_set_Contrast(Contrast1);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast+1");break; 
            case 0x74:
            myCAM.OV5642_set_Contrast(Contrast0);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast+0");break; 
            case 0x75:
            myCAM.OV5642_set_Contrast(Contrast_1);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast-1");break; 
            case 0x76:
            myCAM.OV5642_set_Contrast(Contrast_2);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast-2");break; 
            case 0x77:
            myCAM.OV5642_set_Contrast(Contrast_3);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast-3");break; 
            case 0x78:
            myCAM.OV5642_set_Contrast(Contrast_4);usart_Command = 0xff;
            printf("ACK CMD Set to Contrast-4");break;
            case 0x80: 
            myCAM.OV5642_set_hue(degree_180);usart_Command = 0xff;
            printf("ACK CMD Set to -180 degree");break;   
            case 0x81: 
            myCAM.OV5642_set_hue(degree_150);usart_Command = 0xff;
            printf("ACK CMD Set to -150 degree");break;  
            case 0x82: 
            myCAM.OV5642_set_hue(degree_120);usart_Command = 0xff;
            printf("ACK CMD Set to -120 degree");break;  
            case 0x83: 
            myCAM.OV5642_set_hue(degree_90);usart_Command = 0xff;
            printf("ACK CMD Set to -90 degree");break;   
            case 0x84: 
            myCAM.OV5642_set_hue(degree_60);usart_Command = 0xff;
            printf("ACK CMD Set to -60 degree");break;   
            case 0x85: 
            myCAM.OV5642_set_hue(degree_30);usart_Command = 0xff;
            printf("ACK CMD Set to -30 degree");break;  
            case 0x86: 
            myCAM.OV5642_set_hue(degree_0);usart_Command = 0xff;
            printf("ACK CMD Set to 0 degree");break; 
            case 0x87: 
            myCAM.OV5642_set_hue(degree30);usart_Command = 0xff;
            printf("ACK CMD Set to 30 degree");break;
            case 0x88: 
            myCAM.OV5642_set_hue(degree60);usart_Command = 0xff;
            printf("ACK CMD Set to 60 degree");break;
            case 0x89: 
            myCAM.OV5642_set_hue(degree90);usart_Command = 0xff;
            printf("ACK CMD Set to 90 degree");break;
            case 0x8a: 
            myCAM.OV5642_set_hue(degree120);usart_Command = 0xff;
            printf("ACK CMD Set to 120 degree");break; 
            case 0x8b: 
            myCAM.OV5642_set_hue(degree150);usart_Command = 0xff;
            printf("ACK CMD Set to 150 degree");break;
            case 0x90: 
            myCAM.OV5642_set_Special_effects(Normal);usart_Command = 0xff;
            printf("ACK CMD Set to Normal");break;
            case 0x91: 
            myCAM.OV5642_set_Special_effects(BW);usart_Command = 0xff;
            printf("ACK CMD Set to BW");break;
            case 0x92: 
            myCAM.OV5642_set_Special_effects(Bluish);usart_Command = 0xff;
            printf("ACK CMD Set to Bluish");break;
            case 0x93: 
            myCAM.OV5642_set_Special_effects(Sepia);usart_Command = 0xff;
            printf("ACK CMD Set to Sepia");break;
            case 0x94: 
            myCAM.OV5642_set_Special_effects(Reddish);usart_Command = 0xff;
            printf("ACK CMD Set to Reddish");break;
            case 0x95: 
            myCAM.OV5642_set_Special_effects(Greenish);usart_Command = 0xff;
            printf("ACK CMD Set to Greenish");break;
            case 0x96: 
            myCAM.OV5642_set_Special_effects(Negative);usart_Command = 0xff;
            printf("ACK CMD Set to Negative");break;
            case 0xA0: 
            myCAM.OV5642_set_Exposure_level(Exposure_17_EV);usart_Command = 0xff;
            printf("ACK CMD Set to -1.7EV");break;  
            case 0xA1: 
            myCAM.OV5642_set_Exposure_level(Exposure_13_EV);usart_Command = 0xff;
            printf("ACK CMD Set to -1.3EV");break;
            case 0xA2: 
            myCAM.OV5642_set_Exposure_level(Exposure_10_EV);usart_Command = 0xff;
            printf("ACK CMD Set to -1.0EV");break; 
            case 0xA3: 
            myCAM.OV5642_set_Exposure_level(Exposure_07_EV);usart_Command = 0xff;
            printf("ACK CMD Set to -0.7EV");break;
            case 0xA4: 
            myCAM.OV5642_set_Exposure_level(Exposure_03_EV);usart_Command = 0xff;
            printf("ACK CMD Set to -0.3EV");break;
            case 0xA5: 
            myCAM.OV5642_set_Exposure_level(Exposure_default);usart_Command = 0xff;
            printf("ACK CMD Set to -Exposure_default");break;
            case 0xA6: 
            myCAM.OV5642_set_Exposure_level(Exposure07_EV);usart_Command = 0xff;
            printf("ACK CMD Set to 0.7EV");break;  
            case 0xA7: 
            myCAM.OV5642_set_Exposure_level(Exposure10_EV);usart_Command = 0xff;
            printf("ACK CMD Set to 1.0EV");break;
            case 0xA8: 
            myCAM.OV5642_set_Exposure_level(Exposure13_EV);usart_Command = 0xff;
            printf("ACK CMD Set to 1.3EV");break; 
            case 0xA9: 
            myCAM.OV5642_set_Exposure_level(Exposure17_EV);usart_Command = 0xff;
            printf("ACK CMD Set to 1.7EV");break; 
            case 0xB0: 
            myCAM.OV5642_set_Sharpness(Auto_Sharpness_default);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Sharpness default");break; 
            case 0xB1: 
            myCAM.OV5642_set_Sharpness(Auto_Sharpness1);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Sharpness +1");break; 
            case 0xB2: 
            myCAM.OV5642_set_Sharpness(Auto_Sharpness2);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Sharpness +2");break; 
            case 0xB3: 
            myCAM.OV5642_set_Sharpness(Manual_Sharpnessoff);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Manual Sharpness off");break; 
            case 0xB4: 
            myCAM.OV5642_set_Sharpness(Manual_Sharpness1);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Manual Sharpness +1");break;
            case 0xB5: 
            myCAM.OV5642_set_Sharpness(Manual_Sharpness2);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Manual Sharpness +2");break; 
            case 0xB6: 
            myCAM.OV5642_set_Sharpness(Manual_Sharpness3);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Manual Sharpness +3");break;
            case 0xB7: 
            myCAM.OV5642_set_Sharpness(Manual_Sharpness4);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Manual Sharpness +4");break;
            case 0xB8: 
            myCAM.OV5642_set_Sharpness(Manual_Sharpness5);usart_Command = 0xff;
            printf("ACK CMD Set to Auto Manual Sharpness +5");break;  
            case 0xC0: 
            myCAM.OV5642_set_Mirror_Flip(MIRROR);usart_Command = 0xff;
            printf("ACK CMD Set to MIRROR");break;  
            case 0xC1: 
            myCAM.OV5642_set_Mirror_Flip(FLIP);usart_Command = 0xff;
            printf("ACK CMD Set to FLIP");break; 
            case 0xC2: 
            myCAM.OV5642_set_Mirror_Flip(MIRROR_FLIP);usart_Command = 0xff;
            printf("ACK CMD Set to MIRROR&FLIP");break;
            case 0xC3: 
            myCAM.OV5642_set_Mirror_Flip(Normal);usart_Command = 0xff;
            printf("ACK CMD Set to Normal");break;
            case 0xD0: 
            myCAM.OV5642_set_Compress_quality(high_quality);usart_Command = 0xff;
            printf("ACK CMD Set to high quality");break;
            case 0xD1: 
            myCAM.OV5642_set_Compress_quality(default_quality);usart_Command = 0xff;
            printf("ACK CMD Set to default quality");break;
            case 0xD2: 
            myCAM.OV5642_set_Compress_quality(low_quality);usart_Command = 0xff;
            printf("ACK CMD Set to low quality");break;

            case 0xE0: 
            myCAM.OV5642_Test_Pattern(Color_bar);usart_Command = 0xff;
            printf("ACK CMD Set to Color bar");break;
            case 0xE1: 
            myCAM.OV5642_Test_Pattern(Color_square);usart_Command = 0xff;
            printf("ACK CMD Set to Color square");break;
            case 0xE2: 
            myCAM.OV5642_Test_Pattern(BW_square);usart_Command = 0xff;
            printf("ACK CMD Set to BW square");break;
            case 0xE3: 
            myCAM.OV5642_Test_Pattern(DLI);usart_Command = 0xff;
            printf("ACK CMD Set to DLI");break;
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