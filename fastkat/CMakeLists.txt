FILE(GLOB game_src_glob *.cc;*.h;*.inl)
set(game_src ${game_src_glob})

set ( src_glew
	../external/glew/GL/glew.c
	../external/glew/GL/glew.h
	../external/glew/GL/wglew.h
)

source_group("" FILES ${game_src})
source_group(glew FILES ${src_glew})

set(all_src ${game_src} ${src_glew})


find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

message(STATUS "Including launcher ${LauncherName} ${LauncherIncludes}")
add_executable(fastkat WIN32 ${all_src})
#if(BUILD_USE_CLANG_FORMAT)
#	add_custom_command(TARGET fastkat
#				   PRE_BUILD
#				   COMMAND ./call-clang-format.py ${CMAKE_CURRENT_SOURCE_DIR}/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/*.h  ${mainsrc}
#				   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#				   COMMENT "Running clang format" VERBATIM)
#endif(BUILD_USE_CLANG_FORMAT)
target_link_libraries(fastkat ${OPENGL_LIBRARIES} ${SDL2_LIBRARY} )# ${ASSIMP_LIBRARIES})

#message(STATUS "Linking with OpenGL ${OPENGL_LIBRARIES}")
#message(STATUS "Linking with AssImp ${ASSIMP_LIBRARIES}")