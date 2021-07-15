#include "ArduCAM.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "pico/binary_info.h"
#include "ov2640_regs.h"
#include "ov5642_regs.h"

ArduCAM::ArduCAM()
{
  sensor_model = OV7670;
  sensor_addr = 0x42;
}
ArduCAM::ArduCAM(byte model ,int CS)
{
	B_CS = CS;
	*P_CS=CS;
  sbi(P_CS, B_CS);
	sensor_model = model;
	switch (sensor_model)
	{
    case OV2640:
      	sensor_addr = 0x30;
    break;
     case OV5642:
      	sensor_addr = 0x3C;
    break;
		default:
		  sensor_addr = 0x60;
		break;
	}	
}

void ArduCAM::InitCAM()
{
 
  switch (sensor_model)
  {
    case OV2640:
        wrSensorReg8_8(0xff, 0x01);
        wrSensorReg8_8(0x12, 0x80);
        sleep_ms(100);
        if (m_fmt == JPEG)
        {
          wrSensorRegs8_8(OV2640_JPEG_INIT);
          wrSensorRegs8_8(OV2640_YUV422);
          wrSensorRegs8_8(OV2640_JPEG);
          wrSensorReg8_8(0xff, 0x01);
          wrSensorReg8_8(0x15, 0x00);
          wrSensorRegs8_8(OV2640_320x240_JPEG);
        }
        else
        {
          wrSensorRegs8_8(OV2640_QVGA);
        }
        break;
		case OV5642:
		{
					wrSensorReg16_8(0x3008, 0x80);
					if (m_fmt == RAW)
					{
						//Init and set 640x480;
						wrSensorRegs16_8(OV5642_1280x960_RAW);	
						wrSensorRegs16_8(OV5642_640x480_RAW);	
					}
					else
					{	
						wrSensorRegs16_8(OV5642_QVGA_Preview);
						sleep_ms(100);
						if (m_fmt == JPEG)
						{
							sleep_ms(100);
							wrSensorRegs16_8(OV5642_JPEG_Capture_QSXGA);
							wrSensorRegs16_8(ov5642_320x240);
							sleep_ms(100);
							wrSensorReg16_8(0x3818, 0xa8);
							wrSensorReg16_8(0x3621, 0x10);
							wrSensorReg16_8(0x3801, 0xb0);
							wrSensorReg16_8(0x4407, 0x04);
						}
						else
						{
							byte reg_val;
							wrSensorReg16_8(0x4740, 0x21);
							wrSensorReg16_8(0x501e, 0x2a);
							wrSensorReg16_8(0x5002, 0xf8);
							wrSensorReg16_8(0x501f, 0x01);
							wrSensorReg16_8(0x4300, 0x61);
							rdSensorReg16_8(0x3818, &reg_val);
							wrSensorReg16_8(0x3818, (reg_val | 0x60) & 0xff);
							rdSensorReg16_8(0x3621, &reg_val);
							wrSensorReg16_8(0x3621, reg_val & 0xdf);
						}
					}
        break;
		}
    default:
      break;
  }
}

void ArduCAM::CS_HIGH(void)
{
	 sbi(P_CS, B_CS);	
}
void ArduCAM::CS_LOW(void)
{
	 cbi(P_CS, B_CS);	
}

void ArduCAM::flush_fifo(void)
{
	write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

void ArduCAM::start_capture(void)
{
	write_reg(ARDUCHIP_FIFO, FIFO_START_MASK);
}


void ArduCAM::clear_fifo_flag(void )
{
	write_reg(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}


uint8_t ArduCAM::read_fifo(void)
{
	uint8_t data;
	data = bus_read(SINGLE_FIFO_READ);
	return data;
}


uint8_t ArduCAM::read_reg(uint8_t addr)
{
  uint8_t value = 0;
	addr = addr& 0x7f;
 	cbi(P_CS, B_CS);
	spi_write_blocking(SPI_PORT, &addr, 1);
  spi_read_blocking(SPI_PORT, 0, &value, 1);
  sbi(P_CS, B_CS);
	return value;
}



void ArduCAM::write_reg(uint8_t addr, uint8_t data)
{
    uint8_t buf[2];
    buf[0] = addr|WRITE_BIT ;  // remove read bit as this is a write
    buf[1] = data;
    cbi(P_CS, B_CS);
    spi_write_blocking(SPI_PORT, buf, 2);
    sbi(P_CS, B_CS);
    sleep_ms(1); 
}


uint32_t ArduCAM::read_fifo_length(void)
{
	uint32_t len1,len2,len3,length=0;
	len1 = read_reg(FIFO_SIZE1);
  len2 = read_reg(FIFO_SIZE2);
  len3 = read_reg(FIFO_SIZE3) & 0x7f;
  length = ((len3 << 16) | (len2 << 8) | len1) & 0x07fffff;
	return length;	
}

void ArduCAM::set_fifo_burst()
{
    uint8_t value;
    spi_read_blocking(SPI_PORT, BURST_FIFO_READ, &value, 1);	
}

//Set corresponding bit  
void ArduCAM::set_bit(uint8_t addr, uint8_t bit)
{
	uint8_t temp;
	temp = read_reg(addr);
	write_reg(addr, temp | bit);
}


//Clear corresponding bit 
void ArduCAM::clear_bit(uint8_t addr, uint8_t bit)
{
	uint8_t temp;
	temp = read_reg(addr);
	write_reg(addr, temp & (~bit));
}


//Get corresponding bit status
uint8_t ArduCAM::get_bit(uint8_t addr, uint8_t bit)
{
  uint8_t temp;
  temp = read_reg(addr);
  temp = temp & bit;
  return temp;
}

uint8_t ArduCAM::bus_write(int address,int value)
{	
	cbi(P_CS, B_CS);
		//SPI.transfer(address);
		//SPI.transfer(value);
	sbi(P_CS, B_CS);
	return 1;
}


uint8_t ArduCAM:: bus_read(int address)
{
	uint8_t value;
	cbi(P_CS, B_CS);
	//	  SPI.transfer(address);
//		 value = SPI.transfer(0x00);
	sbi(P_CS, B_CS);
	return value;
}

	// Write 8 bit values to 8 bit register address
int ArduCAM::wrSensorRegs8_8(const struct sensor_reg reglist[])
{
		int err = 0;
	  uint16_t reg_addr = 0;
	  uint16_t reg_val = 0;
	  const struct sensor_reg *next = reglist;
	  while ((reg_addr != 0xff) | (reg_val != 0xff))
	  {
	    reg_addr = next->reg;
	    reg_val = next->val;
	    err = wrSensorReg8_8(reg_addr, reg_val);
	    next++;
	  }
	return 1;
}

	// Write 16 bit values to 8 bit register address
int ArduCAM::wrSensorRegs8_16(const struct sensor_reg reglist[])
{

		int err = 0;
	  unsigned int reg_addr, reg_val;
	  const struct sensor_reg *next = reglist;
	  while ((reg_addr != 0xff) | (reg_val != 0xffff))
	  {

	     reg_addr = next->reg;
	     reg_val = next->val;
	    err = wrSensorReg8_16(reg_addr, reg_val);
	    next++;
	  }
	return 1;
}
// Write 8 bit values to 16 bit register address
int ArduCAM::wrSensorRegs16_8(const struct sensor_reg reglist[])
{
		int err = 0;
	  unsigned int reg_addr;
	  unsigned char reg_val;
	  const struct sensor_reg *next = reglist;
	  while ((reg_addr != 0xffff) | (reg_val != 0xff))
	  {
	     reg_addr = next->reg;
	     reg_val = next->val;
	    err = wrSensorReg16_8(reg_addr, reg_val);
	    next++;
	  }
	return 1;
}

// Read/write 8 bit value to/from 16 bit register address
byte ArduCAM::wrSensorReg16_8(int regID, int regDat)
{
    uint8_t buf[3]={0};
    buf[0]=(regID >> 8)&0xff;
    buf[1]=(regID)&0xff;
    buf[2]=regDat;
    i2c_write_blocking(I2C_PORT, sensor_addr, buf,  3, true );
		sleep_ms(2);
	  return 1;
}

// Read/write 8 bit value to/from 8 bit register address	
byte ArduCAM::wrSensorReg8_8(int regID, int regDat)
{
uint8_t buf[2];
    buf[0] = regID;
    buf[1] = regDat;
    i2c_write_blocking(I2C_PORT, sensor_addr, buf,  2, true );
	return 1;
	
}

	void ArduCAM::OV2640_set_Special_effects(uint8_t Special_effect)
	{
// #if (defined (OV2640_CAM)||defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS))	
		switch(Special_effect)
		{
			case Antique:
	
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x18);
				wrSensorReg8_8(0x7c, 0x05);
				wrSensorReg8_8(0x7d, 0x40);
				wrSensorReg8_8(0x7d, 0xa6);
			break;
			case Bluish:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x18);
				wrSensorReg8_8(0x7c, 0x05);
				wrSensorReg8_8(0x7d, 0xa0);
				wrSensorReg8_8(0x7d, 0x40);
			break;
			case Greenish:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x18);
				wrSensorReg8_8(0x7c, 0x05);
				wrSensorReg8_8(0x7d, 0x40);
				wrSensorReg8_8(0x7d, 0x40);
			break;
			case Reddish:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x18);
				wrSensorReg8_8(0x7c, 0x05);
				wrSensorReg8_8(0x7d, 0x40);
				wrSensorReg8_8(0x7d, 0xc0);
			break;
			case BW:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x18);
				wrSensorReg8_8(0x7c, 0x05);
				wrSensorReg8_8(0x7d, 0x80);
				wrSensorReg8_8(0x7d, 0x80);
			break;
			case Negative:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x40);
				wrSensorReg8_8(0x7c, 0x05);
				wrSensorReg8_8(0x7d, 0x80);
				wrSensorReg8_8(0x7d, 0x80);
			break;
			case BWnegative:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x58);
				wrSensorReg8_8(0x7c, 0x05);
				wrSensorReg8_8(0x7d, 0x80);
			  wrSensorReg8_8(0x7d, 0x80);
	
			break;
			case Normal:
		
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x00);
				wrSensorReg8_8(0x7c, 0x05);
				wrSensorReg8_8(0x7d, 0x80);
				wrSensorReg8_8(0x7d, 0x80);
			
			break;
					
		}
	// #endif
	}


	void ArduCAM::OV2640_set_Contrast(uint8_t Contrast)
	{
//  #if (defined (OV2640_CAM)||defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS))	
		switch(Contrast)
		{
			case Contrast2:
		
			wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x07);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x28);
				wrSensorReg8_8(0x7d, 0x0c);
				wrSensorReg8_8(0x7d, 0x06);
			break;
			case Contrast1:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x07);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x24);
				wrSensorReg8_8(0x7d, 0x16);
				wrSensorReg8_8(0x7d, 0x06); 
			break;
			case Contrast0:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x07);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x06); 
			break;
			case Contrast_1:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x07);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x2a);
		  wrSensorReg8_8(0x7d, 0x06);	
			break;
			case Contrast_2:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x07);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x18);
				wrSensorReg8_8(0x7d, 0x34);
				wrSensorReg8_8(0x7d, 0x06);
			break;
		}
// #endif		
	}

	void ArduCAM::OV2640_set_Brightness(uint8_t Brightness)
	{
	// #if (defined (OV2640_CAM)||defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS))
		switch(Brightness)
		{
			case Brightness2:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x09);
				wrSensorReg8_8(0x7d, 0x40);
				wrSensorReg8_8(0x7d, 0x00);
			break;
			case Brightness1:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x09);
				wrSensorReg8_8(0x7d, 0x30);
				wrSensorReg8_8(0x7d, 0x00);
			break;	
			case Brightness0:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x09);
				wrSensorReg8_8(0x7d, 0x20);
				wrSensorReg8_8(0x7d, 0x00);
			break;
			case Brightness_1:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x09);
				wrSensorReg8_8(0x7d, 0x10);
				wrSensorReg8_8(0x7d, 0x00);
			break;
			case Brightness_2:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x04);
				wrSensorReg8_8(0x7c, 0x09);
				wrSensorReg8_8(0x7d, 0x00);
				wrSensorReg8_8(0x7d, 0x00);
			break;	
		}
// #endif	
			
	}

	void ArduCAM::OV2640_set_Color_Saturation(uint8_t Color_Saturation)
	{
	// #if (defined (OV2640_CAM)||defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS))
		switch(Color_Saturation)
		{
			case Saturation2:
			
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x02);
				wrSensorReg8_8(0x7c, 0x03);
				wrSensorReg8_8(0x7d, 0x68);
				wrSensorReg8_8(0x7d, 0x68);
			break;
			case Saturation1:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x02);
				wrSensorReg8_8(0x7c, 0x03);
				wrSensorReg8_8(0x7d, 0x58);
				wrSensorReg8_8(0x7d, 0x58);
			break;
			case Saturation0:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x02);
				wrSensorReg8_8(0x7c, 0x03);
				wrSensorReg8_8(0x7d, 0x48);
				wrSensorReg8_8(0x7d, 0x48);
			break;
			case Saturation_1:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x02);
				wrSensorReg8_8(0x7c, 0x03);
				wrSensorReg8_8(0x7d, 0x38);
				wrSensorReg8_8(0x7d, 0x38);
			break;
			case Saturation_2:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0x7c, 0x00);
				wrSensorReg8_8(0x7d, 0x02);
				wrSensorReg8_8(0x7c, 0x03);
				wrSensorReg8_8(0x7d, 0x28);
				wrSensorReg8_8(0x7d, 0x28);
			break;	
		}
// #endif	
	}

void ArduCAM::OV2640_set_Light_Mode(uint8_t Light_Mode)
	{
//  #if (defined (OV2640_CAM)||defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS))
		 switch(Light_Mode)
		 {
			
			  case Auto:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0xc7, 0x00); //AWB on
			  break;
			  case Sunny:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0xc7, 0x40); //AWB off
			  wrSensorReg8_8(0xcc, 0x5e);
				wrSensorReg8_8(0xcd, 0x41);
				wrSensorReg8_8(0xce, 0x54);
			  break;
			  case Cloudy:
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0xc7, 0x40); //AWB off
				wrSensorReg8_8(0xcc, 0x65);
				wrSensorReg8_8(0xcd, 0x41);
				wrSensorReg8_8(0xce, 0x4f);  
			  break;
			  case Office:
			  wrSensorReg8_8(0xff, 0x00);
			  wrSensorReg8_8(0xc7, 0x40); //AWB off
			  wrSensorReg8_8(0xcc, 0x52);
			  wrSensorReg8_8(0xcd, 0x41);
			  wrSensorReg8_8(0xce, 0x66);
			  break;
			  case Home:
			  wrSensorReg8_8(0xff, 0x00);
			  wrSensorReg8_8(0xc7, 0x40); //AWB off
			  wrSensorReg8_8(0xcc, 0x42);
			  wrSensorReg8_8(0xcd, 0x3f);
			  wrSensorReg8_8(0xce, 0x71);
			  break;
			  default :
				wrSensorReg8_8(0xff, 0x00);
				wrSensorReg8_8(0xc7, 0x00); //AWB on
			  break; 
		 }	
// #endif
	}

byte ArduCAM::rdSensorReg8_8(uint8_t regID, uint8_t* regDat)
{	
  i2c_write_blocking(I2C_PORT, sensor_addr, &regID, 1, true );
  i2c_read_blocking(I2C_PORT, sensor_addr, regDat,  1, false );
  return 1;
	
}

byte ArduCAM::rdSensorReg16_8(uint16_t regID, uint8_t* regDat)
{
	uint8_t buffer[2]={0};
	buffer[0]=(regID>>8)&0xff;
	buffer[1]=regID&0xff;
	i2c_write_blocking(I2C_PORT, sensor_addr, buffer, 2, true );
//	i2c_write_blocking(I2C_PORT, sensor_addr, &low, 1, true );
	i2c_read_blocking(I2C_PORT, sensor_addr, regDat,  1, false );
	return 1;
}


void ArduCAM::OV2640_set_JPEG_size(uint8_t size)
{
// #if (defined (OV2640_CAM)||defined (OV2640_MINI_2MP)||defined (OV2640_MINI_2MP_PLUS))
	switch(size)
	{
		case OV2640_160x120:
			wrSensorRegs8_8(OV2640_160x120_JPEG);
			break;
		case OV2640_176x144:
			wrSensorRegs8_8(OV2640_176x144_JPEG);
			break;
		case OV2640_320x240:
			wrSensorRegs8_8(OV2640_320x240_JPEG);
			break;
		case OV2640_352x288:
	  	wrSensorRegs8_8(OV2640_352x288_JPEG);
			break;
		case OV2640_640x480:
			wrSensorRegs8_8(OV2640_640x480_JPEG);
			break;
		case OV2640_800x600:
			wrSensorRegs8_8(OV2640_800x600_JPEG);
			break;
		case OV2640_1024x768:
			wrSensorRegs8_8(OV2640_1024x768_JPEG);
			break;
		case OV2640_1280x1024:
			wrSensorRegs8_8(OV2640_1280x1024_JPEG);
			break;
		case OV2640_1600x1200:
			wrSensorRegs8_8(OV2640_1600x1200_JPEG);
			break;
		default:
			wrSensorRegs8_8(OV2640_320x240_JPEG);
			break;
	}
//#endif
}


void ArduCAM::set_format(byte fmt)
{
  if (fmt == BMP)
    m_fmt = BMP;
  else if(fmt == RAW)
    m_fmt = RAW;
  else
    m_fmt = JPEG;
}
unsigned char usart_symbol=0;
unsigned char usart_Command = 0;
// RX interrupt handler
void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
       usart_Command = uart_getc(UART_ID);
       usart_symbol=1;
    }
}
void ArduCAM:: Arducam_init(void)
{
    // This example will use I2C0 on GPIO4 (SDA) and GPIO5 (SCL)
  i2c_init(I2C_PORT, 100 * 1000);
  gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
  gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(PIN_SDA);
  gpio_pull_up(PIN_SCL);
  // Make the I2C pins available to picotool
  bi_decl( bi_2pins_with_func(PIN_SDA, PIN_SCL, GPIO_FUNC_I2C));
    // This example will use SPI0 at 0.5MHz.
  spi_init(SPI_PORT, 4 * 1000*1000);
  gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
  gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
  gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
}


void ArduCAM::OV5642_set_JPEG_size(uint8_t size)
{
//#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)
  uint8_t reg_val;

  switch (size)
  {
    case OV5642_320x240:
      wrSensorRegs16_8(ov5642_320x240);
      break;
    case OV5642_640x480:
      wrSensorRegs16_8(ov5642_640x480);
      break;
    case OV5642_1024x768:
      wrSensorRegs16_8(ov5642_1024x768);
      break;
    case OV5642_1280x960:
      wrSensorRegs16_8(ov5642_1280x960);
      break;
    case OV5642_1600x1200:
      wrSensorRegs16_8(ov5642_1600x1200);
      break;
    case OV5642_2048x1536:
      wrSensorRegs16_8(ov5642_2048x1536);
      break;
    case OV5642_2592x1944:
      wrSensorRegs16_8(ov5642_2592x1944);
      break;
    default:
      wrSensorRegs16_8(ov5642_320x240);
      break;
  }
//#endif
}

void ArduCAM::OV5642_set_Light_Mode(uint8_t Light_Mode)
{
//#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)
		switch(Light_Mode)
		{
		
			case Advanced_AWB:
			wrSensorReg16_8(0x3406 ,0x0 );
			wrSensorReg16_8(0x5192 ,0x04);
			wrSensorReg16_8(0x5191 ,0xf8);
			wrSensorReg16_8(0x518d ,0x26);
			wrSensorReg16_8(0x518f ,0x42);
			wrSensorReg16_8(0x518e ,0x2b);
			wrSensorReg16_8(0x5190 ,0x42);
			wrSensorReg16_8(0x518b ,0xd0);
			wrSensorReg16_8(0x518c ,0xbd);
			wrSensorReg16_8(0x5187 ,0x18);
			wrSensorReg16_8(0x5188 ,0x18);
			wrSensorReg16_8(0x5189 ,0x56);
			wrSensorReg16_8(0x518a ,0x5c);
			wrSensorReg16_8(0x5186 ,0x1c);
			wrSensorReg16_8(0x5181 ,0x50);
			wrSensorReg16_8(0x5184 ,0x20);
			wrSensorReg16_8(0x5182 ,0x11);
			wrSensorReg16_8(0x5183 ,0x0 );	
			break;
			case Simple_AWB:
			wrSensorReg16_8(0x3406 ,0x00);
			wrSensorReg16_8(0x5183 ,0x80);
			wrSensorReg16_8(0x5191 ,0xff);
			wrSensorReg16_8(0x5192 ,0x00);
			break;
			case Manual_day:
			wrSensorReg16_8(0x3406 ,0x1 );
			wrSensorReg16_8(0x3400 ,0x7 );
			wrSensorReg16_8(0x3401 ,0x32);
			wrSensorReg16_8(0x3402 ,0x4 );
			wrSensorReg16_8(0x3403 ,0x0 );
			wrSensorReg16_8(0x3404 ,0x5 );
			wrSensorReg16_8(0x3405 ,0x36);
			break;
			case Manual_A:
			wrSensorReg16_8(0x3406 ,0x1 );
			wrSensorReg16_8(0x3400 ,0x4 );
			wrSensorReg16_8(0x3401 ,0x88);
			wrSensorReg16_8(0x3402 ,0x4 );
			wrSensorReg16_8(0x3403 ,0x0 );
			wrSensorReg16_8(0x3404 ,0x8 );
			wrSensorReg16_8(0x3405 ,0xb6);
			break;
			case Manual_cwf:
			wrSensorReg16_8(0x3406 ,0x1 );
			wrSensorReg16_8(0x3400 ,0x6 );
			wrSensorReg16_8(0x3401 ,0x13);
			wrSensorReg16_8(0x3402 ,0x4 );
			wrSensorReg16_8(0x3403 ,0x0 );
			wrSensorReg16_8(0x3404 ,0x7 );
			wrSensorReg16_8(0x3405 ,0xe2);
			break;
			case Manual_cloudy:
			wrSensorReg16_8(0x3406 ,0x1 );
			wrSensorReg16_8(0x3400 ,0x7 );
			wrSensorReg16_8(0x3401 ,0x88);
			wrSensorReg16_8(0x3402 ,0x4 );
			wrSensorReg16_8(0x3403 ,0x0 );
			wrSensorReg16_8(0x3404 ,0x5 );
			wrSensorReg16_8(0x3405 ,0x0);
			break;
			default :
			break; 
		}	
//#endif
}


void ArduCAM::OV5642_set_Color_Saturation(uint8_t Color_Saturation)
{
//#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)
	
		switch(Color_Saturation)
		{
			case Saturation4:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x80);
				wrSensorReg16_8(0x5584 ,0x80);
				wrSensorReg16_8(0x5580 ,0x02);
			break;
			case Saturation3:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x70);
				wrSensorReg16_8(0x5584 ,0x70);
				wrSensorReg16_8(0x5580 ,0x02);
			break;
			case Saturation2:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x60);
				wrSensorReg16_8(0x5584 ,0x60);
				wrSensorReg16_8(0x5580 ,0x02);
			break;
			case Saturation1:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x50);
				wrSensorReg16_8(0x5584 ,0x50);
				wrSensorReg16_8(0x5580 ,0x02);
			break;
			case Saturation0:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x40);
				wrSensorReg16_8(0x5584 ,0x40);
				wrSensorReg16_8(0x5580 ,0x02);
			break;		
			case Saturation_1:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x30);
				wrSensorReg16_8(0x5584 ,0x30);
				wrSensorReg16_8(0x5580 ,0x02);
			break;
				case Saturation_2:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x20);
				wrSensorReg16_8(0x5584 ,0x20);
				wrSensorReg16_8(0x5580 ,0x02);
			break;
				case Saturation_3:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x10);
				wrSensorReg16_8(0x5584 ,0x10);
				wrSensorReg16_8(0x5580 ,0x02);
			break;
				case Saturation_4:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5583 ,0x00);
				wrSensorReg16_8(0x5584 ,0x00);
				wrSensorReg16_8(0x5580 ,0x02);
			break;
		}
//#endif	
}


void ArduCAM::OV5642_set_Brightness(uint8_t Brightness)
{
//	#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)
	
		switch(Brightness)
		{
			case Brightness4:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x40);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Brightness3:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x30);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x00);
			break;	
			case Brightness2:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x20);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Brightness1:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x10);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Brightness0:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x00);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x00);
			break;	
			case Brightness_1:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x10);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x08);
			break;	
			case Brightness_2:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x20);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x08);
			break;	
			case Brightness_3:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x30);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x08);
			break;	
			case Brightness_4:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5589 ,0x40);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x558a ,0x08);
			break;	
		}
//#endif	
			
}


void ArduCAM::OV5642_set_Contrast(uint8_t Contrast)
{
//#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)	
		switch(Contrast)
		{
			case Contrast4:
			wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x30);
				wrSensorReg16_8(0x5588 ,0x30);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Contrast3:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x2c);
				wrSensorReg16_8(0x5588 ,0x2c);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Contrast2:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x28);
				wrSensorReg16_8(0x5588 ,0x28);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Contrast1:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x24);
				wrSensorReg16_8(0x5588 ,0x24);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Contrast0:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x20);
				wrSensorReg16_8(0x5588 ,0x20);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Contrast_1:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x1C);
				wrSensorReg16_8(0x5588 ,0x1C);
				wrSensorReg16_8(0x558a ,0x1C);
			break;
			case Contrast_2:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x18);
				wrSensorReg16_8(0x5588 ,0x18);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Contrast_3:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x14);
				wrSensorReg16_8(0x5588 ,0x14);
				wrSensorReg16_8(0x558a ,0x00);
			break;
			case Contrast_4:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x04);
				wrSensorReg16_8(0x5587 ,0x10);
				wrSensorReg16_8(0x5588 ,0x10);
				wrSensorReg16_8(0x558a ,0x00);
			break;
		}
//#endif		
}


void ArduCAM::OV5642_set_hue(uint8_t degree)
{
//	#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)	
		switch(degree)
		{
			case degree_180:
			wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x80);
				wrSensorReg16_8(0x5582 ,0x00);
				wrSensorReg16_8(0x558a ,0x32);
			break;
			case degree_150:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x6f);
				wrSensorReg16_8(0x5582 ,0x40);
				wrSensorReg16_8(0x558a ,0x32);
			break;
			case degree_120:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x40);
				wrSensorReg16_8(0x5582 ,0x6f);
				wrSensorReg16_8(0x558a ,0x32);
			break;
			case degree_90:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x00);
				wrSensorReg16_8(0x5582 ,0x80);
				wrSensorReg16_8(0x558a ,0x02);
			break;
			case degree_60:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x40);
				wrSensorReg16_8(0x5582 ,0x6f);
				wrSensorReg16_8(0x558a ,0x02);
			break;
			case degree_30:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x6f);
				wrSensorReg16_8(0x5582 ,0x40);
				wrSensorReg16_8(0x558a ,0x02);
			break;
			case degree_0:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x80);
				wrSensorReg16_8(0x5582 ,0x00);
				wrSensorReg16_8(0x558a ,0x01);
			break;
			case degree30:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x6f);
				wrSensorReg16_8(0x5582 ,0x40);
				wrSensorReg16_8(0x558a ,0x01);
			break;
			case degree60:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x40);
				wrSensorReg16_8(0x5582 ,0x6f);
				wrSensorReg16_8(0x558a ,0x01);
			break;
			case degree90:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x00);
				wrSensorReg16_8(0x5582 ,0x80);
				wrSensorReg16_8(0x558a ,0x31);
			break;
			case degree120:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x40);
				wrSensorReg16_8(0x5582 ,0x6f);
				wrSensorReg16_8(0x558a ,0x31);
			break;
			case degree150:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x01);
				wrSensorReg16_8(0x5581 ,0x6f);
				wrSensorReg16_8(0x5582 ,0x40);
				wrSensorReg16_8(0x558a ,0x31);
			break;
		}
//#endif	
		
}


void ArduCAM::OV5642_set_Special_effects(uint8_t Special_effect)
{
//	#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)	
		switch(Special_effect)
		{
			case Bluish:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x18);
				wrSensorReg16_8(0x5585 ,0xa0);
				wrSensorReg16_8(0x5586 ,0x40);
			break;
			case Greenish:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x18);
				wrSensorReg16_8(0x5585 ,0x60);
				wrSensorReg16_8(0x5586 ,0x60);
			break;
			case Reddish:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x18);
				wrSensorReg16_8(0x5585 ,0x80);
				wrSensorReg16_8(0x5586 ,0xc0);
			break;
			case BW:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x18);
				wrSensorReg16_8(0x5585 ,0x80);
				wrSensorReg16_8(0x5586 ,0x80);
			break;
			case Negative:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x40);
			break;
			
				case Sepia:
				wrSensorReg16_8(0x5001 ,0xff);
				wrSensorReg16_8(0x5580 ,0x18);
				wrSensorReg16_8(0x5585 ,0x40);
				wrSensorReg16_8(0x5586 ,0xa0);
			break;
			case Normal:
				wrSensorReg16_8(0x5001 ,0x7f);
				wrSensorReg16_8(0x5580 ,0x00);		
			break;		
		}
//	#endif
}


void ArduCAM::OV5642_set_Exposure_level(uint8_t level)
{
//	#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)	
		switch(level)
		{
			case Exposure_17_EV:
			  wrSensorReg16_8(0x3a0f ,0x10);
				wrSensorReg16_8(0x3a10 ,0x08);
				wrSensorReg16_8(0x3a1b ,0x10);
				wrSensorReg16_8(0x3a1e ,0x08);
				wrSensorReg16_8(0x3a11 ,0x20);
				wrSensorReg16_8(0x3a1f ,0x10);
			break;
			case Exposure_13_EV:
				wrSensorReg16_8(0x3a0f ,0x18);
				wrSensorReg16_8(0x3a10 ,0x10);
				wrSensorReg16_8(0x3a1b ,0x18);
				wrSensorReg16_8(0x3a1e ,0x10);
				wrSensorReg16_8(0x3a11 ,0x30);
				wrSensorReg16_8(0x3a1f ,0x10);
			break;
			case Exposure_10_EV:
				wrSensorReg16_8(0x3a0f ,0x20);
				wrSensorReg16_8(0x3a10 ,0x18);
				wrSensorReg16_8(0x3a11 ,0x41);
				wrSensorReg16_8(0x3a1b ,0x20);
				wrSensorReg16_8(0x3a1e ,0x18);
				wrSensorReg16_8(0x3a1f ,0x10);
			break;
			case Exposure_07_EV:
				wrSensorReg16_8(0x3a0f ,0x28);
				wrSensorReg16_8(0x3a10 ,0x20);
				wrSensorReg16_8(0x3a11 ,0x51);
				wrSensorReg16_8(0x3a1b ,0x28);
				wrSensorReg16_8(0x3a1e ,0x20);
				wrSensorReg16_8(0x3a1f ,0x10);
			break;
			case Exposure_03_EV:
				wrSensorReg16_8(0x3a0f ,0x30);
				wrSensorReg16_8(0x3a10 ,0x28);
				wrSensorReg16_8(0x3a11 ,0x61);
				wrSensorReg16_8(0x3a1b ,0x30);
				wrSensorReg16_8(0x3a1e ,0x28);
				wrSensorReg16_8(0x3a1f ,0x10);
			break;
			case Exposure_default:
				wrSensorReg16_8(0x3a0f ,0x38);
				wrSensorReg16_8(0x3a10 ,0x30);
				wrSensorReg16_8(0x3a11 ,0x61);
				wrSensorReg16_8(0x3a1b ,0x38);
				wrSensorReg16_8(0x3a1e ,0x30);
				wrSensorReg16_8(0x3a1f ,0x10);
			break;
			case Exposure03_EV:
				wrSensorReg16_8(0x3a0f ,0x40);
				wrSensorReg16_8(0x3a10 ,0x38);
				wrSensorReg16_8(0x3a11 ,0x71);
				wrSensorReg16_8(0x3a1b ,0x40);
				wrSensorReg16_8(0x3a1e ,0x38);
				wrSensorReg16_8(0x3a1f ,0x10);
			break;
			case Exposure07_EV:
				wrSensorReg16_8(0x3a0f ,0x48);
				wrSensorReg16_8(0x3a10 ,0x40);
				wrSensorReg16_8(0x3a11 ,0x80);
				wrSensorReg16_8(0x3a1b ,0x48);
				wrSensorReg16_8(0x3a1e ,0x40);
				wrSensorReg16_8(0x3a1f ,0x20);
			break;
			case Exposure10_EV:
				wrSensorReg16_8(0x3a0f ,0x50);
				wrSensorReg16_8(0x3a10 ,0x48);
				wrSensorReg16_8(0x3a11 ,0x90);
				wrSensorReg16_8(0x3a1b ,0x50);
				wrSensorReg16_8(0x3a1e ,0x48);
				wrSensorReg16_8(0x3a1f ,0x20);
			break;
			case Exposure13_EV:
				wrSensorReg16_8(0x3a0f ,0x58);
				wrSensorReg16_8(0x3a10 ,0x50);
				wrSensorReg16_8(0x3a11 ,0x91);
				wrSensorReg16_8(0x3a1b ,0x58);
				wrSensorReg16_8(0x3a1e ,0x50);
				wrSensorReg16_8(0x3a1f ,0x20);
			break;
			case Exposure17_EV:
				wrSensorReg16_8(0x3a0f ,0x60);
				wrSensorReg16_8(0x3a10 ,0x58);
				wrSensorReg16_8(0x3a11 ,0xa0);
				wrSensorReg16_8(0x3a1b ,0x60);
				wrSensorReg16_8(0x3a1e ,0x58);
				wrSensorReg16_8(0x3a1f ,0x20);
			break;
		}
//#endif	
}


void ArduCAM::OV5642_set_Sharpness(uint8_t Sharpness)
{
//	#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)	
		switch(Sharpness)
		{
			case Auto_Sharpness_default:
			wrSensorReg16_8(0x530A ,0x00);
				wrSensorReg16_8(0x530c ,0x0 );
				wrSensorReg16_8(0x530d ,0xc );
				wrSensorReg16_8(0x5312 ,0x40);
			break;
			case Auto_Sharpness1:
				wrSensorReg16_8(0x530A ,0x00);
				wrSensorReg16_8(0x530c ,0x4 );
				wrSensorReg16_8(0x530d ,0x18);
				wrSensorReg16_8(0x5312 ,0x20);
			break;
			case Auto_Sharpness2:
				wrSensorReg16_8(0x530A ,0x00);
				wrSensorReg16_8(0x530c ,0x8 );
				wrSensorReg16_8(0x530d ,0x30);
				wrSensorReg16_8(0x5312 ,0x10);
			break;
			case Manual_Sharpnessoff:
				wrSensorReg16_8(0x530A ,0x08);
				wrSensorReg16_8(0x531e ,0x00);
				wrSensorReg16_8(0x531f ,0x00);
			break;
			case Manual_Sharpness1:
				wrSensorReg16_8(0x530A ,0x08);
				wrSensorReg16_8(0x531e ,0x04);
				wrSensorReg16_8(0x531f ,0x04);
			break;
			case Manual_Sharpness2:
				wrSensorReg16_8(0x530A ,0x08);
				wrSensorReg16_8(0x531e ,0x08);
				wrSensorReg16_8(0x531f ,0x08);
			break;
			case Manual_Sharpness3:
				wrSensorReg16_8(0x530A ,0x08);
				wrSensorReg16_8(0x531e ,0x0c);
				wrSensorReg16_8(0x531f ,0x0c);
			break;
			case Manual_Sharpness4:
				wrSensorReg16_8(0x530A ,0x08);
				wrSensorReg16_8(0x531e ,0x0f);
				wrSensorReg16_8(0x531f ,0x0f);
			break;
			case Manual_Sharpness5:
				wrSensorReg16_8(0x530A ,0x08);
				wrSensorReg16_8(0x531e ,0x1f);
				wrSensorReg16_8(0x531f ,0x1f);
			break;
		}
//#endif
}


void ArduCAM::OV5642_set_Mirror_Flip(uint8_t Mirror_Flip)
{
//	#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)	
			 uint8_t reg_val;
	switch(Mirror_Flip)
		{
			case MIRROR:
				rdSensorReg16_8(0x3818,&reg_val);
				reg_val = reg_val|0x00;
				reg_val = reg_val&0x9F;
			wrSensorReg16_8(0x3818 ,reg_val);
			rdSensorReg16_8(0x3621,&reg_val);
				reg_val = reg_val|0x20;
				wrSensorReg16_8(0x3621, reg_val );
			
			break;
			case FLIP:
				rdSensorReg16_8(0x3818,&reg_val);
				reg_val = reg_val|0x20;
				reg_val = reg_val&0xbF;
			wrSensorReg16_8(0x3818 ,reg_val);
			rdSensorReg16_8(0x3621,&reg_val);
				reg_val = reg_val|0x20;
				wrSensorReg16_8(0x3621, reg_val );
			break;
			case MIRROR_FLIP:
			 rdSensorReg16_8(0x3818,&reg_val);
				reg_val = reg_val|0x60;
				reg_val = reg_val&0xFF;
			wrSensorReg16_8(0x3818 ,reg_val);
			rdSensorReg16_8(0x3621,&reg_val);
				reg_val = reg_val&0xdf;
				wrSensorReg16_8(0x3621, reg_val );
			break;
			case Normal:
				  rdSensorReg16_8(0x3818,&reg_val);
				reg_val = reg_val|0x40;
				reg_val = reg_val&0xdF;
			wrSensorReg16_8(0x3818 ,reg_val);
			rdSensorReg16_8(0x3621,&reg_val);
				reg_val = reg_val&0xdf;
				wrSensorReg16_8(0x3621, reg_val );
			break;
		}
//	#endif
}


void ArduCAM::OV5642_set_Compress_quality(uint8_t quality)
{
//#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)	
	switch(quality)
		{
			case high_quality:
				wrSensorReg16_8(0x4407, 0x02);
				break;
			case default_quality:
				wrSensorReg16_8(0x4407, 0x04);
				break;
			case low_quality:
				wrSensorReg16_8(0x4407, 0x08);
				break;
		}
//#endif
}


void ArduCAM::OV5642_Test_Pattern(uint8_t Pattern)
{
//	#if defined(OV5642_CAM) || defined(OV5642_CAM_BIT_ROTATION_FIXED)|| defined(OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_PLUS)	
	  switch(Pattern)
		{
			case Color_bar:
				wrSensorReg16_8(0x503d , 0x80);
				wrSensorReg16_8(0x503e, 0x00);
				break;
			case Color_square:
				wrSensorReg16_8(0x503d , 0x85);
				wrSensorReg16_8(0x503e, 0x12);
				break;
			case BW_square:
				wrSensorReg16_8(0x503d , 0x85);
				wrSensorReg16_8(0x503e, 0x1a);
				break;
			case DLI:
				wrSensorReg16_8(0x4741 , 0x4);
				break;
		}
//#endif
}