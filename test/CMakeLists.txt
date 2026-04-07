cmake_minimum_required(VERSION 3.15)

set(TARGETS arithmetic_types arithmetic_ops application)
set(SANITIZERS undefined address)


list(APPEND CMAKE_PREFIX_PATH /usr/local)
find_package(PkgConfig REQUIRED)
pkg_search_module(SIRIUS sirius)


list(TRANSFORM TARGETS PREPEND "test_" OUTPUT_VARIABLE TARGET_LIST)
foreach(TEST_TARGET IN LISTS TARGET_LIST)
    add_executable(${TEST_TARGET} "${TEST_TARGET}.cpp")
    add_dependencies(${TEST_TARGET} sirius)

    #target_compile_options(${TEST_TARGET} PUBLIC "-march=native")
    target_compile_options(${TEST_TARGET} PUBLIC "-fno-rtti" "-fno-exceptions" "-fopenmp" "-Wall" "-Wextra" "$<$<CONFIG:RELEASE>:-Werror>")
    target_compile_options(${TEST_TARGET} PUBLIC "$<$<CONFIG:RELEASE>:-Werror>" "$<$<CONFIG:RELEASE>:-O3>" "$<$<CONFIG:RELEASE>:-ffast-math>" "$<$<CONFIG:RELEASE>:-march=native>")
    target_compile_options(${TEST_TARGET} PUBLIC "$<$<CONFIG:DEBUG>:-O0>" "$<$<CONFIG:DEBUG>:-g>")
   
    #target_link_options(${TEST_TARGET} PUBLIC "-fuse-ld=mold")
    target_link_options(${TEST_TARGET} PUBLIC "-v")

	if(${SIRIUS_FOUND})
		target_link_libraries(${TEST_TARGET} PUBLIC ${SIRIUS_LIBRARIES})
		target_link_directories(${TEST_TARGET} PUBLIC ${SIRIUS_LIBRARY_DIRS})
		target_link_options(${TEST_TARGET} PUBLIC ${SIRIUS_LDFLAGS})
		target_include_directories(${TEST_TARGET} PUBLIC ${SIRIUS_INCLUDE_DIRS})
		target_compile_options(${TEST_TARGET} PUBLIC ${SIRIUS_CFLAGS})
	else()
		target_link_libraries(${TEST_TARGET} PUBLIC sirius)
	endif()

    foreach(SAN IN LISTS SANITIZERS)
        target_compile_options(${TEST_TARGET} PUBLIC "$<$<CONFIG:DEBUG>:-fsanitize=${SAN}>")
        target_link_options(${TEST_TARGET} PUBLIC "$<$<CONFIG:DEBUG>:-fsanitize=${SAN}>")
    endforeach()


    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${TEST_TARGET} PUBLIC "-fsized-deallocation")
    endif() 
endforeach()