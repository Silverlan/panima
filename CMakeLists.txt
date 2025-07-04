include(${CMAKE_SOURCE_DIR}/cmake/pr_common.cmake)

set(PROJ_NAME panima)
pr_add_library(${PROJ_NAME} STATIC)

pr_add_dependency(${PROJ_NAME} util_udm TARGET PUBLIC)
pr_add_dependency(${PROJ_NAME} cppbezierfit TARGET PUBLIC)
pr_add_third_party_dependency(${PROJ_NAME} exprtk HEADER_ONLY)

pr_init_module(${PROJ_NAME})

# Required for exprtk
include(CheckCXXCompilerFlag)
if(NOT MSVC)
	check_cxx_compiler_flag("-Wa,-mbig-obj" EXPRTK_HAS_MBIG_OBJ)
	if(${EXPRTK_HAS_MBIG_OBJ})
		target_compile_options(${PROJ_NAME} PRIVATE -Wa,-mbig-obj)
	endif()
endif ()

pr_finalize(${PROJ_NAME})
