add_executable(Arducam_MINI_5MP_Plus_Videostreaing
        Arducam_MINI_5MP_Plus_Videostreaing.cpp 
        )

# Pull in our pico_stdlib which pulls in commonly used features
include_directories(../../ArduCAM)
target_link_libraries(Arducam_MINI_5MP_Plus_Videostreaing
	pico_stdlib
	hardware_dma
	hardware_i2c
	hardware_pwm
	ArduCAM
)
pico_enable_stdio_usb(Arducam_MINI_5MP_Plus_Videostreaing 1)
pico_enable_stdio_uart(Arducam_MINI_5MP_Plus_Videostreaing 0)
# create map/bin/hex file etc.
pico_add_extra_outputs(Arducam_MINI_5MP_Plus_Videostreaing)


