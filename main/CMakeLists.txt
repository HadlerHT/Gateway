file(GLOB SRC_DIR "../src/*.c")

idf_component_register(
    SRCS "main.c" ${SRC_DIR}
    INCLUDE_DIRS "." "../include"
)
