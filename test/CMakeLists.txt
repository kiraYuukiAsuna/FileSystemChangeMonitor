cmake_minimum_required (VERSION 3.8)

project ("Test")


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
    "../include/"
)

if(WIN32)
    set(SOURCES
    "test.cpp"
    "../source/filesystemchangemonitor.cpp"
    "../source/filesystemchangemonitorwindows.cpp"
    )
elseif(UNIX)
    set(SOURCES
    "test.cpp"
    "../source/filesystemchangemonitor.cpp"
    "../source/filesystemchangemonitorlinux.cpp"
    )
endif()

include_directories(
    
)

link_directories(

)

add_executable (Test ${HEADERS} ${SOURCES})

if(WIN32)
	
elseif(UNIX)
	#target_link_libraries(AsyncForwarderServer pthread dl)
endif()