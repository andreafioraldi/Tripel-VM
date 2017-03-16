cmake_minimum_required (VERSION 3.0.2)

project("Tripel Virtual Machine" VERSION "1.0" LANGUAGES "C")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

#add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/lib")

#enable optimization for gcc
if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
endif(CMAKE_COMPILER_IS_GNUCC)

#set default prefix to /usr in Unix
if (UNIX AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "default install path" FORCE)
endif()

include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if(NOT IS_BIG_ENDIAN)
	set(LITTLE_ENDIAN_DEFINITION "#define LITTLE_ENDIAN")
else()
	set(LITTLE_ENDIAN_DEFINITION "")
endif()
configure_file(from_bytes.h.in from_bytes.h)

include(ExternalProject)
ExternalProject_Add(
    libjit
    URL ${CMAKE_CURRENT_SOURCE_DIR}/libjit.tar.gz
    CONFIGURE_COMMAND 
        COMMAND "${CMAKE_BINARY_DIR}/libjit-prefix/src/libjit/configure" "--prefix=${CMAKE_BINARY_DIR}" --disable-shared
    BUILD_COMMAND make
    #CMAKE_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/libjit/bootstrap
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND make install
)

ExternalProject_Add(
    gc
    URL ${CMAKE_CURRENT_SOURCE_DIR}/gc.tar.gz
    CONFIGURE_COMMAND 
        COMMAND "${CMAKE_BINARY_DIR}/gc-prefix/src/gc/configure" "--prefix=${CMAKE_BINARY_DIR}" --disable-shared
    BUILD_COMMAND make
    #CMAKE_GENERATOR ${CMAKE_CURRENT_SOURCE_DIR}/libjit/bootstrap
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND make install
)

find_package(Threads REQUIRED)

include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_BINARY_DIR}/include/jit")

set(SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/program.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/module.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/function.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/main.c"
)

add_executable(tvm ${SOURCE_FILES})
add_dependencies(tvm libjit)
add_dependencies(tvm gc)

if (UNIX)
	set(LIB_DL dl)
else (UNIX)
	set(LIB_DL "")
endif (UNIX)

target_link_libraries(tvm 
    "${CMAKE_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jit${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CMAKE_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jitdynamic${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CMAKE_BINARY_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gc${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${CMAKE_THREAD_LIBS_INIT}"
    ${LIB_DL}
    "m"
)








