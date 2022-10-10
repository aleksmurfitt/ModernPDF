set(TARGET epdf)
add_executable(${TARGET} test.cpp)
target_link_libraries(${TARGET} PRIVATE pdflib)