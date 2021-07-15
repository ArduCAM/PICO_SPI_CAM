add_executable(ArduCAM_Mini_2MP_Plus_4CAM_VideoStreaming
        ArduCAM_Mini_2MP_Plus_4CAM_VideoStreaming.cpp 
        )

# Pull in our pico_stdlib which pulls in commonly used features
include_directories(../../ArduCAM)
target_link_libraries(ArduCAM_Mini_2MP_Plus_4CAM_VideoStreaming
	pico_stdlib
	hardware_dma
	hardware_i2c
	hardware_pwm
	ArduCAM
)
pico_enable_stdio_usb(ArduCAM_Mini_2MP_Plus_4CAM_VideoStreaming 1)
pico_enable_stdio_uart(ArduCAM_Mini_2MP_Plus_4CAM_VideoStreaming 0)
# create map/bin/hex file etc.
pico_add_extra_outputs(ArduCAM_Mini_2MP_Plus_4CAM_VideoStreaming)


