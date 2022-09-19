set(srcs
src/LoRaMac.c
src/aes.c
src/LoRaMacCrypto.c
src/Mcu.S
src/gpio.c
src/board.c
src/delay.c
src/gpio-board.c
src/cmac.c
src/OLEDDisplay.cpp
src/fifo.c
src/timer.S
src/region/RegionUS915-Hybrid.c
src/region/RegionAU915.c
src/region/RegionCN470.c
src/region/RegionAS923.c
src/region/Region.c
src/region/RegionCN779.c
src/region/RegionLA915.c
src/region/RegionEU868.c
src/region/RegionKR920.c
src/region/RegionEU433.c
src/region/RegionCommon.c
src/region/RegionIN865.c
src/region/RegionUS915.c
src/rtc-board.S
src/sensor/HDC1080.cpp
src/ESP32_LoRaWAN.cpp
src/utilities.c
src/OLEDDisplayUi.cpp
src/sx1276-board.c
src/sx1276.c
src/LoRaMacConfirmQueue.c
  )

set(includedirs
    src
    src/region
    src/sensor
  )

set(priv_includes )
set(requires arduino)
set(priv_requires )

idf_component_register(INCLUDE_DIRS ${includedirs} PRIV_INCLUDE_DIRS ${priv_includes} SRCS ${srcs} REQUIRES ${requires} PRIV_REQUIRES ${priv_requires})

function(maybe_add_component component_name)
    idf_build_get_property(components BUILD_COMPONENTS)
    if (${component_name} IN_LIST components)
        idf_component_get_property(lib_name ${component_name} COMPONENT_LIB)
        target_link_libraries(${COMPONENT_LIB} PUBLIC ${lib_name})
    endif()
endfunction()

maybe_add_component(arduino)

target_compile_options(${COMPONENT_TARGET} PUBLIC
    -DESP32 -DLORAWAN_PREAMBLE_LENGTH=${CONFIG_LORAWAN_PREAMBLE_LENGTH}
)
