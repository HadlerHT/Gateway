file(GLOB SRC_DIR "../src/*.cpp")

idf_component_register(
    SRCS "main.cpp" ${SRC_DIR}
    INCLUDE_DIRS "." "../include"
)
