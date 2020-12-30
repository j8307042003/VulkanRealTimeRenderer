cmake_minimum_required(VERSION 3.1)

project(spirv-reflect)

add_library(spirv-reflect 
      spirv_reflect.h spirv_reflect.c
      )




target_include_directories(spirv-reflect INTERFACE ..)



if(WIN32)
    target_compile_definitions(spirv-reflect PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()


