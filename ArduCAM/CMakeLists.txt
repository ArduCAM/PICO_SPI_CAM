if (NOT TARGET ArduCAM)
    add_library(ArduCAM INTERFACE)
    target_sources(ArduCAM INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/ArduCAM.cpp 
    )
    target_link_libraries(ArduCAM INTERFACE pico_stdlib hardware_i2c hardware_spi hardware_irq)
endif()
   