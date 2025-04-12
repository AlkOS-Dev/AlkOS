#######################################################################
#                                uACPI                                #
#######################################################################

FetchContent_Declare(
        uacpi
        GIT_REPOSITORY https://github.com/UltraOS/uACPI.git
        GIT_TAG 2.0.0
)

FetchContent_MakeAvailable(uacpi)

# Include uACPI's CMake configuration
include(${uacpi_SOURCE_DIR}/uacpi.cmake)

############################## Preparing targets ##############################

add_library(uacpi STATIC ${UACPI_SOURCES})

############################# Adding Definitions ##############################

set(OVERRIDE_DEFINITIONS
        UACPI_OVERRIDE_LIBC=1
)

target_compile_definitions(uacpi PRIVATE ${OVERRIDE_DEFINITIONS})

############################### Adding Headers ###############################

target_include_directories(uacpi PUBLIC ${UACPI_INCLUDES})
target_include_directories(uacpi PRIVATE .)

############################# Linking libraries ##############################

target_link_libraries(uacpi PRIVATE libk)

######################### Setting Custom Properties ##########################

target_compile_options(uacpi PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-mcmodel=kernel>"
        "$<$<COMPILE_LANGUAGE:C>:-mcmodel=kernel>"
)

# Propagate the uACPI library to the parent scope
set(THIRD_PARTY_LIBRARIES ${THIRD_PARTY_LIBRARIES} uacpi PARENT_SCOPE)
