

find_package(Yuni COMPONENTS core)
if(Yuni_FOUND)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Yuni_CXXFLAGS}")
	link_libraries("${Yuni_LIBS}")

	message(STATUS "Sample: Core / Command line Options (GetOpt) / Simple option")
	add_executable(getopt_00_simple main.cpp)
endif(Yuni_FOUND)

