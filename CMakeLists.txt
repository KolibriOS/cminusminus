cmake_minimum_required (VERSION 3.10)
project (CMINUSMINUS LANGUAGES CXX)

set (CMM_SRC main.cpp misc.cpp toka.cpp tokb.cpp tokc.cpp toke.cpp tokr.cpp errors.cpp debug.cpp
	outobj.cpp outpe.cpp disasm.cpp switch.cpp outle.cpp pointer.cpp new_type.cpp class.cpp res.cpp
	optreg.cpp libobj.cpp)

include_directories(${PROJECT_SOURCE_DIR})
add_executable (c-- ${CMM_SRC})

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
	set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fpermissive -Wno-write-strings")
endif()
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -fpermissive -Wno-write-strings")
endif()

