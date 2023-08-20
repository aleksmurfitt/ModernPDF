set(TARGET pdflib)

set(SCRIPTS_FILE ${PROJECT_BINARY_DIR}/include/iso15924.txt)
file(DOWNLOAD https://unicode.org/iso15924/iso15924.txt ${SCRIPTS_FILE})
file(READ ${SCRIPTS_FILE} DATA)
set(DELIM "c++_fragment")
set(DATA "R\"${DELIM}(\n${DATA})${DELIM}\"")
file(WRITE ${SCRIPTS_FILE} "${DATA}")

FETCHCONTENT_DECLARE(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
)
FETCHCONTENT_MAKEAVAILABLE(fmt)

configure_file(${PROJECT_SOURCE_DIR}/cmake/config.hpp.in ${PROJECT_BINARY_DIR}/include/config.hpp)
add_library(${TARGET} STATIC face.cpp font.cpp fontManager.cpp)
target_compile_features(${TARGET} PUBLIC cxx_std_20)
target_link_libraries(${TARGET} PUBLIC ${LIBS} qpdf::libqpdf fmt::fmt)
target_compile_options(${TARGET} PUBLIC ${CFLAGS})
target_include_directories(${TARGET} PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR}/include ${INCLUDE_DIRS})
