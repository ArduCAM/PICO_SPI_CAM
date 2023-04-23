import board
import busio
import bitbangio
import time as utime
import digitalio
from OV2640_reg import *
from OV5642_reg import *

OV2640=0
OV5642=1

MAX_FIFO_SIZE=0x7FFFFF
ARDUCHIP_FRAMES=0x01
ARDUCHIP_TIM=0x03
VSYNC_LEVEL_MASK=0x02
ARDUCHIP_TRIG=0x41
CAP_DONE_MASK=0x08

OV5642_CHIPID_HIGH=0x300a
OV5642_CHIPID_LOW=0x300b

OV2640_160x120  =0
OV2640_176x144  =1
OV2640_320x240  =2
OV2640_352x288  =3
OV2640_640x480  =4
OV2640_800x600  =5
OV2640_1024x768 =6
OV2640_1280x1024=7
OV2640_1600x1200=8

OV5642_320x240  =0
OV5642_640x480  =1
OV5642_1024x768 =2
OV5642_1280x960 =3
OV5642_1600x1200=4
OV5642_2048x1536=5
OV5642_2592x1944=6
OV5642_1920x1080=7

Advanced_AWB =0
Simple_AWB   =1
Manual_day   =2
Manual_A     =3
Manual_cwf   =4
Manual_cloudy=5


degree_180=0
degree_150=1
degree_120=2
degree_90 =3
degree_60 =4
degree_30 =5
degree_0  =6
degree30  =7
degree60  =8
degree90  =9
degree120 =10
degree150 =11

Auto   =0
Sunny  =1
Cloudy =2
Office =3
Home   =4

Antique     =0
Bluish      =1
Greenish    =2
Reddish     =3
BW          =4
Negative    =5
BWnegative  =6
Normal      =7
Sepia       =8
Overexposure=9
Solarize    =10
Blueish     =11
Yellowish   =12

Exposure_17_EV  =0
Exposure_13_EV  =1
Exposure_10_EV  =2
Exposure_07_EV  =3
Exposure_03_EV  =4
Exposure_default=5
Exposure03_EV   =6
Exposure07_EV   =7
Exposure10_EV   =8
Exposure13_EV   =9
Exposure17_EV   =10

Auto_Sharpness_default=0
Auto_Sharpness1       =1
Auto_Sharpness2       =2
Manual_Sharpnessoff   =3
Manual_Sharpness1     =4
Manual_Sharpness2     =5
Manual_Sharpness3     =6
Manual_Sharpness4     =7
Manual_Sharpness5     =8

MIRROR     =0
FLIP       =1
MIRROR_FLIP=2

Saturation4 =0
Saturation3 =1
Saturation2 =2
Saturation1 =3
Saturation0 =4
Saturation_1=5
Saturation_2=6
Saturation_3=7
Saturation_4=8

Brightness4 =0
Brightness3 =1
Brightness2 =2
Brightness1 =3
Brightness0 =4
Brightness_1=5
Brightness_2=6
Brightness_3=7
Brightness_4=8

Contrast4 =0
Contrast3 =1
Contrast2 =2
Contrast1 =3
Contrast0 =4
Contrast_1=5
Contrast_2=6
Contrast_3=7
Contrast_4=8

Antique      = 0
Bluish       = 1
Greenish     = 2
Reddish      = 3
BW           = 4
Negative     = 5
BWnegative   = 6
Normal       = 7
Sepia        = 8
Overexposure = 9
Solarize     = 10
Blueish      = 11
Yellowish    = 12

high_quality   =0
default_quality=1
low_quality    =2

Color_bar   =0
Color_square=1
BW_square   =2
DLI         =3

BMP =0
JPEG=1
RAW =2

class ArducamClass(object):
    def __init__(self,Type):
        self.CameraMode=JPEG
        self.CameraType=Type
        self.SPI_CS=digitalio.DigitalInOut(board.GP5)
        self.SPI_CS.direction = digitalio.Direction.OUTPUT
        self.I2cAddress=0x30
        self.spi = busio.SPI(clock=board.GP2, MOSI=board.GP3, MISO=board.GP4)
        while not self.spi.try_lock():
            pass
        self.spi.configure(baudrate=4000000,polarity=0,phase=0,bits=8)
        self.i2c = bitbangio.I2C(scl=board.GP9, sda=board.GP8,frequency=1000000)
        while not self.i2c.try_lock():
            pass
        print(self.i2c.scan())
        self.Spi_write(0x07,0x80)
        utime.sleep(0.1)
        self.Spi_write(0x07,0x00)
        utime.sleep(0.1)
        
    def Camera_Detection(self):
        while True:
            if self.CameraType==OV2640:
                self.I2cAddress=0x30
                self.wrSensorReg8_8(0xff,0x01)
                id_h=self.rdSensorReg8_8(0x0a)
                id_l=self.rdSensorReg8_8(0x0b)
                if((id_h==0x26)and((id_l==0x40)or(id_l==0x42))):
                    print('CameraType is OV2640')
                    break
                else:
                    print('Can\'t find OV2640 module')
            elif self.CameraType==OV5642:
                self.I2cAddress=0x3c
                self.wrSensorReg16_8(0xff,0x01)
                id_h=self.rdSensorReg16_8(OV5642_CHIPID_HIGH)
                id_l=self.rdSensorReg16_8(OV5642_CHIPID_LOW)
                if((id_h==0x56)and(id_l==0x42)):
                    print('CameraType is OV5642')
                    break
                else:
                    print('Can\'t find OV5642 module')
            utime.sleep(1)
            
    def Set_Camera_mode(self,mode):
        self.CameraMode=mode
    
    def wrSensorReg16_8(self,addr,val):
        buffer=bytearray(3)
        buffer[0]=(addr>>8)&0xff
        buffer[1]=addr&0xff
        buffer[2]=val
        self.iic_write(buffer)
        utime.sleep(0.003)

    def rdSensorReg16_8(self,addr):
        buffer=bytearray(2)
        rt=bytearray(1)
        buffer[0]=(addr>>8)&0xff
        buffer[1]=addr&0xff
        self.iic_write(buffer)
        self.iic_readinto(rt)
        return rt[0]
    
    def wrSensorReg8_8(self,addr,val):
        buffer=bytearray(2)
        buffer[0]=addr
        buffer[1]=val
        self.iic_write(buffer)
        
    def iic_write(self, buf, *, start=0, end=None):
        if end is None:
            end = len(buf)
        self.i2c.writeto(self.I2cAddress, buf, start=start, end=end)

    def iic_readinto(self, buf, *, start=0, end=None):
        if end is None:
            end = len(buf)
        self.i2c.readfrom_into(self.I2cAddress, buf, start=start, end=end)
        
    def rdSensorReg8_8(self,addr):
        buffer=bytearray(1)
        buffer[0]=addr
        self.iic_write(buffer)
        self.iic_readinto(buffer)
        return buffer[0]
    def Spi_Test(self):
        while True:
            self.Spi_write(0X00,0X56)
            value=self.Spi_read(0X00)
            if(value[0]==0X56):
                print('SPI interface OK')
                break
            else:
                print('SPI interface Error')
            utime.sleep(1)

    def Camera_Init(self):
        if self.CameraType==OV2640:
            self.wrSensorReg8_8(0xff,0x01)
            self.wrSensorReg8_8(0x12,0x80)
            utime.sleep(0.1)
            self.wrSensorRegs8_8(OV2640_JPEG_INIT);
            self.wrSensorRegs8_8(OV2640_YUV422);
            self.wrSensorRegs8_8(OV2640_JPEG);
            self.wrSensorReg8_8(0xff,0x01)
            self.wrSensorReg8_8(0x15,0x00)
            self.wrSensorRegs8_8(OV2640_320x240_JPEG);
        elif self.CameraType==OV5642:
            self.wrSensorReg16_8(0x3008, 0x80)
            if self.CameraMode == RAW:
                self.wrSensorRegs16_8(OV5642_1280x960_RAW)
                self.wrSensorRegs16_8(OV5642_640x480_RAW)
            else:
                self.wrSensorRegs16_8(OV5642_QVGA_Preview1)
                self.wrSensorRegs16_8(OV5642_QVGA_Preview2)
                utime.sleep(0.1)
                if self.CameraMode == JPEG:
                    utime.sleep(0.1)
                    self.wrSensorRegs16_8(OV5642_JPEG_Capture_QSXGA)
                    self.wrSensorRegs16_8(ov5642_320x240)
                    utime.sleep(0.1)
                    self.wrSensorReg16_8(0x3818, 0xa8)
                    self.wrSensorReg16_8(0x3621, 0x10)
                    self.wrSensorReg16_8(0x3801, 0xb0)
                    self.wrSensorReg16_8(0x4407, 0x04)
                else:
                    reg_val
                    self.wrSensorReg16_8(0x4740, 0x21)
                    self.wrSensorReg16_8(0x501e, 0x2a)
                    self.wrSensorReg16_8(0x5002, 0xf8)
                    self.wrSensorReg16_8(0x501f, 0x01)
                    self.wrSensorReg16_8(0x4300, 0x61)
                    reg_val=self.rdSensorReg16_8(0x3818)
                    self.wrSensorReg16_8(0x3818, (reg_val | 0x60) & 0xff)
                    reg_val=self.rdSensorReg16_8(0x3621)
                    self.wrSensorReg16_8(0x3621, reg_val & 0xdf)            
        else:
            pass
        
    def Spi_write(self,address,value):
        maskbits = 0x80
        buffer=bytearray(2)
        buffer[0]=address | maskbits
        buffer[1]=value
        self.SPI_CS_LOW()
        self.spi_write(buffer)
        self.SPI_CS_HIGH()
        
    def Spi_read(self,address):
        maskbits = 0x7f
        buffer=bytearray(1)
        buffer[0]=address & maskbits
        self.SPI_CS_LOW()
        self.spi_write(buffer)
        self.spi_readinto(buffer)
        self.SPI_CS_HIGH()
        return buffer

    def spi_write(self, buf, *, start=0, end=None):
        if end is None:
            end = len(buf)
        self.spi.write(buf, start=start, end=end)

    def spi_readinto(self, buf, *, start=0, end=None):
        if end is None:
            end = len(buf)
        self.spi.readinto(buf, start=start, end=end)
        
    def get_bit(self,addr,bit):
        value=self.Spi_read(addr)[0]
        return value&bit
  
    def SPI_CS_LOW(self):
        self.SPI_CS.value=False
        
    def SPI_CS_HIGH(self):
        self.SPI_CS.value=True
        
    def set_fifo_burst(self):
        buffer=bytearray(1)
        buffer[0]=0x3c
        self.spi.write(buffer, start=0, end=1)
        
    def clear_fifo_flag(self):
        self.Spi_write(0x04,0x01)
        
    def flush_fifo(self):
        self.Spi_write(0x04,0x01)
        
    def start_capture(self):
        self.Spi_write(0x04,0x02)
        
    def read_fifo_length(self):
        len1=self.Spi_read(0x42)[0]
        len2=self.Spi_read(0x43)[0]
        len3=self.Spi_read(0x44)[0]
        len3=len3 & 0x7f
        lenght=((len3<<16)|(len2<<8)|(len1))& 0x07fffff
        return lenght
    
    def wrSensorRegs8_8(self,reg_value):
        for data in reg_value:
            addr = data[0]
            val = data[1]
            if (addr == 0xff and val == 0xff):
                return
            self.wrSensorReg8_8(addr, val)
            utime.sleep(0.001)
            
    def wrSensorRegs16_8(self,reg_value):
        for data in reg_value:
            addr = data[0]
            val = data[1]
            if (addr == 0xffff and val == 0xff):
                return
            self.wrSensorReg16_8(addr, val)
            
    def set_format(self,mode):
        if mode==BMP or mode==JPEG or mode==RAW:   
            self.CameraMode=mode
            
    def set_bit(self,addr,bit):
        temp=self.Spi_read(addr)[0]
        self.Spi_write(addr,temp&(~bit))
    
    def OV2640_set_JPEG_size(self,size):
        if size==OV2640_160x120:
            self.wrSensorRegs8_8(OV2640_160x120_JPEG)
        elif size==OV2640_176x144:
            self.wrSensorRegs8_8(OV2640_176x144_JPEG)
        elif size==OV2640_320x240:
            self.wrSensorRegs8_8(OV2640_320x240_JPEG)
        elif size==OV2640_352x288:
            self.wrSensorRegs8_8(OV2640_352x288_JPEG)
        elif size==OV2640_640x480:
            self.wrSensorRegs8_8(OV2640_640x480_JPEG)
        elif size==OV2640_800x600:
            self.wrSensorRegs8_8(OV2640_800x600_JPEG)
        elif size==OV2640_1024x768:
            self.wrSensorRegs8_8(OV2640_1024x768_JPEG)
        elif size==OV2640_1280x1024:
            self.wrSensorRegs8_8(OV2640_1280x1024_JPEG)
        elif size==OV2640_1600x1200:
            self.wrSensorRegs8_8(OV2640_1600x1200_JPEG)
        else:
            self.wrSensorRegs8_8(OV2640_320x240_JPEG)
      
      
    def OV2640_set_Light_Mode(self,result):
        if result==Auto:
            self.wrSensorReg8_8(0xff,0x00)
            self.wrSensorReg8_8(0xc7,0x00)
        elif result==Sunny:
            self.wrSensorReg8_8(0xff,0x00)
            self.wrSensorReg8_8(0xc7,0x40)
            self.wrSensorReg8_8(0xcc,0x5e)
            self.wrSensorReg8_8(0xcd,0x41)
            self.wrSensorReg8_8(0xce,0x54)
        elif result==Cloudy:
            self.wrSensorReg8_8(0xff,0x00)
            self.wrSensorReg8_8(0xc7,0x40)
            self.wrSensorReg8_8(0xcc,0x65)
            self.wrSensorReg8_8(0xcd,0x41)
            self.wrSensorReg8_8(0xce,0x4f)
        elif result==Office:
            self.wrSensorReg8_8(0xff,0x00)
            self.wrSensorReg8_8(0xc7,0x40)
            self.wrSensorReg8_8(0xcc,0x52)
            self.wrSensorReg8_8(0xcd,0x41)
            self.wrSensorReg8_8(0xce,0x66)
        elif result==Home:
            self.wrSensorReg8_8(0xff,0x00)
            self.wrSensorReg8_8(0xc7,0x40)
            self.wrSensorReg8_8(0xcc,0x42)
            self.wrSensorReg8_8(0xcd,0x3f)
            self.wrSensorReg8_8(0xce,0x71)
        else:
            self.wrSensorReg8_8(0xff,0x00)
            self.wrSensorReg8_8(0xc7,0x00)
    def OV2640_set_Color_Saturation(self,Saturation):
        if Saturation== Saturation2:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x02)
            self.wrSensorReg8_8(0x7c, 0x03)
            self.wrSensorReg8_8(0x7d, 0x68)
            self.wrSensorReg8_8(0x7d, 0x68)
        elif Saturation== Saturation1:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x02)
            self.wrSensorReg8_8(0x7c, 0x03)
            self.wrSensorReg8_8(0x7d, 0x58)
            self.wrSensorReg8_8(0x7d, 0x58)
        elif Saturation== Saturation0:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x02)
            self.wrSensorReg8_8(0x7c, 0x03)
            self.wrSensorReg8_8(0x7d, 0x48)
            self.wrSensorReg8_8(0x7d, 0x48)
        elif Saturation== Saturation_1:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x02)
            self.wrSensorReg8_8(0x7c, 0x03)
            self.wrSensorReg8_8(0x7d, 0x38)
            self.wrSensorReg8_8(0x7d, 0x38)
        elif Saturation== Saturation_2:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x02)
            self.wrSensorReg8_8(0x7c, 0x03)
            self.wrSensorReg8_8(0x7d, 0x28)
            self.wrSensorReg8_8(0x7d, 0x28)
            
    def OV2640_set_Brightness(self,Brightness):
        if Brightness== Brightness2:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x09)
            self.wrSensorReg8_8(0x7d, 0x40)
            self.wrSensorReg8_8(0x7d, 0x00)
        elif Brightness== Brightness1:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x09)
            self.wrSensorReg8_8(0x7d, 0x30)
            self.wrSensorReg8_8(0x7d, 0x00)
        elif Brightness== Brightness0:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x09)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x00)
        elif Brightness== Brightness_1:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x09)
            self.wrSensorReg8_8(0x7d, 0x10)
            self.wrSensorReg8_8(0x7d, 0x00)
        elif Brightness== Brightness_2:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x09)
            self.wrSensorReg8_8(0x7d, 0x00)
            self.wrSensorReg8_8(0x7d, 0x00)
    def OV2640_set_Contrast(self,Contrast):
        if Contrast== Contrast2:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x07)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x28)
            self.wrSensorReg8_8(0x7d, 0x0c)
            self.wrSensorReg8_8(0x7d, 0x06)
        elif Contrast== Contrast1:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x07)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x24)
            self.wrSensorReg8_8(0x7d, 0x16)
            self.wrSensorReg8_8(0x7d, 0x06) 
        elif Contrast== Contrast0:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x07)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x06) 
        elif Contrast== Contrast_1:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x07)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x2a)
            self.wrSensorReg8_8(0x7d, 0x06)
        elif Contrast== Contrast_2:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x04)
            self.wrSensorReg8_8(0x7c, 0x07)
            self.wrSensorReg8_8(0x7d, 0x20)
            self.wrSensorReg8_8(0x7d, 0x18)
            self.wrSensorReg8_8(0x7d, 0x34)
            self.wrSensorReg8_8(0x7d, 0x06)     
    def OV2640_set_Special_effects(self,Special_effect):
        if Special_effect== Antique:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x18)
            self.wrSensorReg8_8(0x7c, 0x05)
            self.wrSensorReg8_8(0x7d, 0x40)
            self.wrSensorReg8_8(0x7d, 0xa6)
        elif Special_effect== Bluish:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x18)
            self.wrSensorReg8_8(0x7c, 0x05)
            self.wrSensorReg8_8(0x7d, 0xa0)
            self.wrSensorReg8_8(0x7d, 0x40)
        elif Special_effect== Greenish:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x18)
            self.wrSensorReg8_8(0x7c, 0x05)
            self.wrSensorReg8_8(0x7d, 0x40)
            self.wrSensorReg8_8(0x7d, 0x40)
        elif Special_effect== Reddish:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x18)
            self.wrSensorReg8_8(0x7c, 0x05)
            self.wrSensorReg8_8(0x7d, 0x40)
            self.wrSensorReg8_8(0x7d, 0xc0)
        elif Special_effect== BW:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x18)
            self.wrSensorReg8_8(0x7c, 0x05)
            self.wrSensorReg8_8(0x7d, 0x80)
            self.wrSensorReg8_8(0x7d, 0x80)
        elif Special_effect== Negative:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x40)
            self.wrSensorReg8_8(0x7c, 0x05)
            self.wrSensorReg8_8(0x7d, 0x80)
            self.wrSensorReg8_8(0x7d, 0x80)
        elif Special_effect== BWnegative:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x58)
            self.wrSensorReg8_8(0x7c, 0x05)
            self.wrSensorReg8_8(0x7d, 0x80)
            self.wrSensorReg8_8(0x7d, 0x80)
        elif Special_effect== Normal:
            self.wrSensorReg8_8(0xff, 0x00)
            self.wrSensorReg8_8(0x7c, 0x00)
            self.wrSensorReg8_8(0x7d, 0x00)
            self.wrSensorReg8_8(0x7c, 0x05)
            self.wrSensorReg8_8(0x7d, 0x80)
            self.wrSensorReg8_8(0x7d, 0x80)

    def OV5642_set_JPEG_size(self,size):
        if size== OV5642_320x240:
          self.wrSensorRegs16_8(ov5642_320x240)
        elif size== OV5642_640x480:
          self.wrSensorRegs16_8(ov5642_640x480)     
        elif size== OV5642_1024x768:
          self.wrSensorRegs16_8(ov5642_1024x768)      
        elif size== OV5642_1280x960:
          self.wrSensorRegs16_8(ov5642_1280x960)      
        elif size== OV5642_1600x1200:
          self.wrSensorRegs16_8(ov5642_1600x1200)      
        elif size== OV5642_2048x1536:
          self.wrSensorRegs16_8(ov5642_2048x1536)      
        elif size== OV5642_2592x1944:
          self.wrSensorRegs16_8(ov5642_2592x1944)      
        else:
          self.wrSensorRegs16_8(ov5642_320x240)
 
    def OV5642_set_Light_Mode(self,Light_Mode):
        if Light_Mode== Advanced_AWB:
            self.wrSensorReg16_8(0x3406 ,0x0 )
            self.wrSensorReg16_8(0x5192 ,0x04)
            self.wrSensorReg16_8(0x5191 ,0xf8)
            self.wrSensorReg16_8(0x518d ,0x26)
            self.wrSensorReg16_8(0x518f ,0x42)
            self.wrSensorReg16_8(0x518e ,0x2b)
            self.wrSensorReg16_8(0x5190 ,0x42)
            self.wrSensorReg16_8(0x518b ,0xd0)
            self.wrSensorReg16_8(0x518c ,0xbd)
            self.wrSensorReg16_8(0x5187 ,0x18)
            self.wrSensorReg16_8(0x5188 ,0x18)
            self.wrSensorReg16_8(0x5189 ,0x56)
            self.wrSensorReg16_8(0x518a ,0x5c)
            self.wrSensorReg16_8(0x5186 ,0x1c)
            self.wrSensorReg16_8(0x5181 ,0x50)
            self.wrSensorReg16_8(0x5184 ,0x20)
            self.wrSensorReg16_8(0x5182 ,0x11)
            self.wrSensorReg16_8(0x5183 ,0x0 )	
        elif Light_Mode== Simple_AWB:
            self.wrSensorReg16_8(0x3406 ,0x00)
            self.wrSensorReg16_8(0x5183 ,0x80)
            self.wrSensorReg16_8(0x5191 ,0xff)
            self.wrSensorReg16_8(0x5192 ,0x00)
        elif Light_Mode== Manual_day:
            self.wrSensorReg16_8(0x3406 ,0x1 )
            self.wrSensorReg16_8(0x3400 ,0x7 )
            self.wrSensorReg16_8(0x3401 ,0x32)
            self.wrSensorReg16_8(0x3402 ,0x4 )
            self.wrSensorReg16_8(0x3403 ,0x0 )
            self.wrSensorReg16_8(0x3404 ,0x5 )
            self.wrSensorReg16_8(0x3405 ,0x36)
        elif Light_Mode== Manual_A:
            self.wrSensorReg16_8(0x3406 ,0x1 )
            self.wrSensorReg16_8(0x3400 ,0x4 )
            self.wrSensorReg16_8(0x3401 ,0x88)
            self.wrSensorReg16_8(0x3402 ,0x4 )
            self.wrSensorReg16_8(0x3403 ,0x0 )
            self.wrSensorReg16_8(0x3404 ,0x8 )
            self.wrSensorReg16_8(0x3405 ,0xb6)
        elif Light_Mode== Manual_cwf:
            self.wrSensorReg16_8(0x3406 ,0x1 )
            self.wrSensorReg16_8(0x3400 ,0x6 )
            self.wrSensorReg16_8(0x3401 ,0x13)
            self.wrSensorReg16_8(0x3402 ,0x4 )
            self.wrSensorReg16_8(0x3403 ,0x0 )
            self.wrSensorReg16_8(0x3404 ,0x7 )
            self.wrSensorReg16_8(0x3405 ,0xe2)
        elif Light_Mode== Manual_cloudy:
            self.wrSensorReg16_8(0x3406 ,0x1 )
            self.wrSensorReg16_8(0x3400 ,0x7 )
            self.wrSensorReg16_8(0x3401 ,0x88)
            self.wrSensorReg16_8(0x3402 ,0x4 )
            self.wrSensorReg16_8(0x3403 ,0x0 )
            self.wrSensorReg16_8(0x3404 ,0x5 )
            self.wrSensorReg16_8(0x3405 ,0x0)

    def OV5642_set_Color_Saturation(self,Color_Saturation):       
        if Color_Saturation== Saturation4:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x80)
            self.wrSensorReg16_8(0x5584 ,0x80)
            self.wrSensorReg16_8(0x5580 ,0x02)
        elif Color_Saturation== Saturation3:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x70)
            self.wrSensorReg16_8(0x5584 ,0x70)
            self.wrSensorReg16_8(0x5580 ,0x02)
        elif Color_Saturation== Saturation2:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x60)
            self.wrSensorReg16_8(0x5584 ,0x60)
            self.wrSensorReg16_8(0x5580 ,0x02)
        elif Color_Saturation== Saturation1:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x50)
            self.wrSensorReg16_8(0x5584 ,0x50)
            self.wrSensorReg16_8(0x5580 ,0x02)
        elif Color_Saturation== Saturation0:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x40)
            self.wrSensorReg16_8(0x5584 ,0x40)
            self.wrSensorReg16_8(0x5580 ,0x02)
        elif Color_Saturation== Saturation_1:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x30)
            self.wrSensorReg16_8(0x5584 ,0x30)
            self.wrSensorReg16_8(0x5580 ,0x02)
        elif Color_Saturation== Saturation_2:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x20)
            self.wrSensorReg16_8(0x5584 ,0x20)
            self.wrSensorReg16_8(0x5580 ,0x02)
        elif Color_Saturation== Saturation_3:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x10)
            self.wrSensorReg16_8(0x5584 ,0x10)
            self.wrSensorReg16_8(0x5580 ,0x02)
        elif Color_Saturation== Saturation_4:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5583 ,0x00)
            self.wrSensorReg16_8(0x5584 ,0x00)
            self.wrSensorReg16_8(0x5580 ,0x02)
                 
    def OV5642_set_Brightness(self,Brightness):
        if Brightness== Brightness4:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x40)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Brightness== Brightness3:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x30)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Brightness== Brightness2:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x20)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Brightness== Brightness1:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x10)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Brightness== Brightness0:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x00)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Brightness== Brightness_1:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x10)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x08)
        elif Brightness== Brightness_2:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x20)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x08)
        elif Brightness== Brightness_3:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x30)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x08)
        elif Brightness== Brightness_4:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5589 ,0x40)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x558a ,0x08)
            
    def OV5642_set_Contrast(self,Contrast):
        if Contrast== Contrast4:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x30)
            self.wrSensorReg16_8(0x5588 ,0x30)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Contrast== Contrast3:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x2c)
            self.wrSensorReg16_8(0x5588 ,0x2c)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Contrast== Contrast2:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x28)
            self.wrSensorReg16_8(0x5588 ,0x28)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Contrast== Contrast1:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x24)
            self.wrSensorReg16_8(0x5588 ,0x24)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Contrast== Contrast0:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x20)
            self.wrSensorReg16_8(0x5588 ,0x20)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Contrast== Contrast_1:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x1C)
            self.wrSensorReg16_8(0x5588 ,0x1C)
            self.wrSensorReg16_8(0x558a ,0x1C)
        elif Contrast== Contrast_2:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x18)
            self.wrSensorReg16_8(0x5588 ,0x18)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Contrast== Contrast_3:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x14)
            self.wrSensorReg16_8(0x5588 ,0x14)
            self.wrSensorReg16_8(0x558a ,0x00)
        elif Contrast== Contrast_4:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x04)
            self.wrSensorReg16_8(0x5587 ,0x10)
            self.wrSensorReg16_8(0x5588 ,0x10)
            self.wrSensorReg16_8(0x558a ,0x00)
       
    def OV5642_set_hue(self,degree):
        if degree== degree_180:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x80)
            self.wrSensorReg16_8(0x5582 ,0x00)
            self.wrSensorReg16_8(0x558a ,0x32)
        elif degree== degree_150:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x6f)
            self.wrSensorReg16_8(0x5582 ,0x40)
            self.wrSensorReg16_8(0x558a ,0x32)
        elif degree== degree_120:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x40)
            self.wrSensorReg16_8(0x5582 ,0x6f)
            self.wrSensorReg16_8(0x558a ,0x32)
        elif degree== degree_90:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x00)
            self.wrSensorReg16_8(0x5582 ,0x80)
            self.wrSensorReg16_8(0x558a ,0x02)
        elif degree== degree_60:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x40)
            self.wrSensorReg16_8(0x5582 ,0x6f)
            self.wrSensorReg16_8(0x558a ,0x02)
        elif degree== degree_30:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x6f)
            self.wrSensorReg16_8(0x5582 ,0x40)
            self.wrSensorReg16_8(0x558a ,0x02)
        elif degree== degree_0:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x80)
            self.wrSensorReg16_8(0x5582 ,0x00)
            self.wrSensorReg16_8(0x558a ,0x01)
        elif degree== degree30:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x6f)
            self.wrSensorReg16_8(0x5582 ,0x40)
            self.wrSensorReg16_8(0x558a ,0x01)
        elif degree== degree60:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x40)
            self.wrSensorReg16_8(0x5582 ,0x6f)
            self.wrSensorReg16_8(0x558a ,0x01)
        elif degree== degree90:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x00)
            self.wrSensorReg16_8(0x5582 ,0x80)
            self.wrSensorReg16_8(0x558a ,0x31)
        elif degree== degree120:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x40)
            self.wrSensorReg16_8(0x5582 ,0x6f)
            self.wrSensorReg16_8(0x558a ,0x31)
        elif degree== degree150:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x01)
            self.wrSensorReg16_8(0x5581 ,0x6f)
            self.wrSensorReg16_8(0x5582 ,0x40)
            self.wrSensorReg16_8(0x558a ,0x31)
      
    def OV5642_set_Special_effects(self,Special_effect):
        if  Special_effect== Bluish:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x18)
            self.wrSensorReg16_8(0x5585 ,0xa0)
            self.wrSensorReg16_8(0x5586 ,0x40)			
        elif  Special_effect== Greenish:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x18)
            self.wrSensorReg16_8(0x5585 ,0x60)
            self.wrSensorReg16_8(0x5586 ,0x60)			
        elif  Special_effect== Reddish:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x18)
            self.wrSensorReg16_8(0x5585 ,0x80)
            self.wrSensorReg16_8(0x5586 ,0xc0)			
        elif  Special_effect== BW:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x18)
            self.wrSensorReg16_8(0x5585 ,0x80)
            self.wrSensorReg16_8(0x5586 ,0x80)			
        elif  Special_effect== Negative:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x40)
        elif  Special_effect== Sepia:
            self.wrSensorReg16_8(0x5001 ,0xff)
            self.wrSensorReg16_8(0x5580 ,0x18)
            self.wrSensorReg16_8(0x5585 ,0x40)
            self.wrSensorReg16_8(0x5586 ,0xa0)		
        elif  Special_effect== Normal:
            self.wrSensorReg16_8(0x5001 ,0x7f)
            self.wrSensorReg16_8(0x5580 ,0x00)
            
    def OV5642_set_Exposure_level(self,level):
        if level== Exposure_17_EV:
            self.wrSensorReg16_8(0x3a0f ,0x10)
            self.wrSensorReg16_8(0x3a10 ,0x08)
            self.wrSensorReg16_8(0x3a1b ,0x10)
            self.wrSensorReg16_8(0x3a1e ,0x08)
            self.wrSensorReg16_8(0x3a11 ,0x20)
            self.wrSensorReg16_8(0x3a1f ,0x10)
        elif level== Exposure_13_EV:
            self.wrSensorReg16_8(0x3a0f ,0x18)
            self.wrSensorReg16_8(0x3a10 ,0x10)
            self.wrSensorReg16_8(0x3a1b ,0x18)
            self.wrSensorReg16_8(0x3a1e ,0x10)
            self.wrSensorReg16_8(0x3a11 ,0x30)
            self.wrSensorReg16_8(0x3a1f ,0x10)
        elif level== Exposure_10_EV:
            self.wrSensorReg16_8(0x3a0f ,0x20)
            self.wrSensorReg16_8(0x3a10 ,0x18)
            self.wrSensorReg16_8(0x3a11 ,0x41)
            self.wrSensorReg16_8(0x3a1b ,0x20)
            self.wrSensorReg16_8(0x3a1e ,0x18)
            self.wrSensorReg16_8(0x3a1f ,0x10)
        elif level== Exposure_07_EV:
            self.wrSensorReg16_8(0x3a0f ,0x28)
            self.wrSensorReg16_8(0x3a10 ,0x20)
            self.wrSensorReg16_8(0x3a11 ,0x51)
            self.wrSensorReg16_8(0x3a1b ,0x28)
            self.wrSensorReg16_8(0x3a1e ,0x20)
            self.wrSensorReg16_8(0x3a1f ,0x10)
        elif level== Exposure_03_EV:
            self.wrSensorReg16_8(0x3a0f ,0x30)
            self.wrSensorReg16_8(0x3a10 ,0x28)
            self.wrSensorReg16_8(0x3a11 ,0x61)
            self.wrSensorReg16_8(0x3a1b ,0x30)
            self.wrSensorReg16_8(0x3a1e ,0x28)
            self.wrSensorReg16_8(0x3a1f ,0x10)
        elif level== Exposure_default:
            self.wrSensorReg16_8(0x3a0f ,0x38)
            self.wrSensorReg16_8(0x3a10 ,0x30)
            self.wrSensorReg16_8(0x3a11 ,0x61)
            self.wrSensorReg16_8(0x3a1b ,0x38)
            self.wrSensorReg16_8(0x3a1e ,0x30)
            self.wrSensorReg16_8(0x3a1f ,0x10)
        elif level== Exposure03_EV:
            self.wrSensorReg16_8(0x3a0f ,0x40)
            self.wrSensorReg16_8(0x3a10 ,0x38)
            self.wrSensorReg16_8(0x3a11 ,0x71)
            self.wrSensorReg16_8(0x3a1b ,0x40)
            self.wrSensorReg16_8(0x3a1e ,0x38)
            self.wrSensorReg16_8(0x3a1f ,0x10)
        elif level== Exposure07_EV:
            self.wrSensorReg16_8(0x3a0f ,0x48)
            self.wrSensorReg16_8(0x3a10 ,0x40)
            self.wrSensorReg16_8(0x3a11 ,0x80)
            self.wrSensorReg16_8(0x3a1b ,0x48)
            self.wrSensorReg16_8(0x3a1e ,0x40)
            self.wrSensorReg16_8(0x3a1f ,0x20)
        elif level== Exposure10_EV:
            self.wrSensorReg16_8(0x3a0f ,0x50)
            self.wrSensorReg16_8(0x3a10 ,0x48)
            self.wrSensorReg16_8(0x3a11 ,0x90)
            self.wrSensorReg16_8(0x3a1b ,0x50)
            self.wrSensorReg16_8(0x3a1e ,0x48)
            self.wrSensorReg16_8(0x3a1f ,0x20)
        elif level== Exposure13_EV:
            self.wrSensorReg16_8(0x3a0f ,0x58)
            self.wrSensorReg16_8(0x3a10 ,0x50)
            self.wrSensorReg16_8(0x3a11 ,0x91)
            self.wrSensorReg16_8(0x3a1b ,0x58)
            self.wrSensorReg16_8(0x3a1e ,0x50)
            self.wrSensorReg16_8(0x3a1f ,0x20)
        elif level== Exposure17_EV:
            self.wrSensorReg16_8(0x3a0f ,0x60)
            self.wrSensorReg16_8(0x3a10 ,0x58)
            self.wrSensorReg16_8(0x3a11 ,0xa0)
            self.wrSensorReg16_8(0x3a1b ,0x60)
            self.wrSensorReg16_8(0x3a1e ,0x58)
            self.wrSensorReg16_8(0x3a1f ,0x20)
            
    def OV5642_set_Sharpness(self,Sharpness):
        if Sharpness== Auto_Sharpness_default:
            self.wrSensorReg16_8(0x530A ,0x00)
            self.wrSensorReg16_8(0x530c ,0x0 )
            self.wrSensorReg16_8(0x530d ,0xc )
            self.wrSensorReg16_8(0x5312 ,0x40)
        elif Sharpness== Auto_Sharpness1:
            self.wrSensorReg16_8(0x530A ,0x00)
            self.wrSensorReg16_8(0x530c ,0x4 )
            self.wrSensorReg16_8(0x530d ,0x18)
            self.wrSensorReg16_8(0x5312 ,0x20)
        elif Sharpness== Auto_Sharpness2:
            self.wrSensorReg16_8(0x530A ,0x00)
            self.wrSensorReg16_8(0x530c ,0x8 )
            self.wrSensorReg16_8(0x530d ,0x30)
            self.wrSensorReg16_8(0x5312 ,0x10)
        elif Sharpness== Manual_Sharpnessoff:
            self.wrSensorReg16_8(0x530A ,0x08)
            self.wrSensorReg16_8(0x531e ,0x00)
            self.wrSensorReg16_8(0x531f ,0x00)
        elif Sharpness== Manual_Sharpness1:
            self.wrSensorReg16_8(0x530A ,0x08)
            self.wrSensorReg16_8(0x531e ,0x04)
            self.wrSensorReg16_8(0x531f ,0x04)
        elif Sharpness== Manual_Sharpness2:
            self.wrSensorReg16_8(0x530A ,0x08)
            self.wrSensorReg16_8(0x531e ,0x08)
            self.wrSensorReg16_8(0x531f ,0x08)
        elif Sharpness== Manual_Sharpness3:
            self.wrSensorReg16_8(0x530A ,0x08)
            self.wrSensorReg16_8(0x531e ,0x0c)
            self.wrSensorReg16_8(0x531f ,0x0c)
        elif Sharpness== Manual_Sharpness4:
            self.wrSensorReg16_8(0x530A ,0x08)
            self.wrSensorReg16_8(0x531e ,0x0f)
            self.wrSensorReg16_8(0x531f ,0x0f)
        elif Sharpness== Manual_Sharpness5:
            self.wrSensorReg16_8(0x530A ,0x08)
            self.wrSensorReg16_8(0x531e ,0x1f)
            self.wrSensorReg16_8(0x531f ,0x1f)

    def OV5642_set_Mirror_Flip(self,Mirror_Flip):
        if Mirror_Flip== MIRROR:
            reg_val=self.rdSensorReg16_8(0x3818)
            reg_val = reg_val|0x00
            reg_val = reg_val&0x9F
            self.wrSensorReg16_8(0x3818 ,reg_val)
            reg_val=self.rdSensorReg16_8(0x3621)
            reg_val = reg_val|0x20
            self.wrSensorReg16_8(0x3621, reg_val )
        elif Mirror_Flip== FLIP:
            reg_val=self.rdSensorReg16_8(0x3818)
            reg_val = reg_val|0x20
            reg_val = reg_val&0xbF
            self.wrSensorReg16_8(0x3818 ,reg_val)
            reg_val=self.rdSensorReg16_8(0x3621)
            reg_val = reg_val|0x20
            self.wrSensorReg16_8(0x3621, reg_val )
        elif Mirror_Flip== MIRROR_FLIP:
            reg_val=self.rdSensorReg16_8(0x3818)
            reg_val = reg_val|0x60
            reg_val = reg_val&0xFF
            self.wrSensorReg16_8(0x3818 ,reg_val)
            reg_val=self.rdSensorReg16_8(0x3621)
            reg_val = reg_val&0xdf
            self.wrSensorReg16_8(0x3621, reg_val )
        elif Mirror_Flip== Normal:
            reg_val=self.rdSensorReg16_8(0x3818)
            reg_val = reg_val|0x40
            reg_val = reg_val&0xdF;
            self.wrSensorReg16_8(0x3818 ,reg_val)
            reg_val=self.rdSensorReg16_8(0x3621);
            reg_val = reg_val&0xdf;
            self.wrSensorReg16_8(0x3621, reg_val )

    def OV5642_set_Compress_quality(self,quality):
        if quality== high_quality:
            self.wrSensorReg16_8(0x4407, 0x02)
        elif quality== default_quality:
            self.wrSensorReg16_8(0x4407, 0x04)
        elif quality== low_quality:
            self.wrSensorReg16_8(0x4407, 0x08)
    
    def OV5642_Test_Pattern(self,Pattern):
        if Pattern== Color_bar:
            self.wrSensorReg16_8(0x503d , 0x80)
            self.wrSensorReg16_8(0x503e, 0x00)
        elif Pattern== Color_square:
            self.wrSensorReg16_8(0x503d , 0x85)
            self.wrSensorReg16_8(0x503e, 0x12)
        elif Pattern== BW_square:
            self.wrSensorReg16_8(0x503d , 0x85)
            self.wrSensorReg16_8(0x503e, 0x1a)
        elif Pattern== DLI:
            self.wrSensorReg16_8(0x4741 , 0x4)
