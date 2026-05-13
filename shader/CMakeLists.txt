find_program(GLSLANG_VALIDATOR glslangValidator)
if(NOT GLSLANG_VALIDATOR)
	message(WARNING "glslangValidator not found, shader compilation will be skipped.")
endif()

set(SHADER_SRC_DIR ${CMAKE_SOURCE_DIR}/shader)
set(SHADER_DST_DIR ${CMAKE_SOURCE_DIR}/resource/shader)

file(MAKE_DIRECTORY ${SHADER_DST_DIR})

file(GLOB SHADER_FILES
	${SHADER_SRC_DIR}/*.vert
	${SHADER_SRC_DIR}/*.frag
)

set(SHADER_OUTPUTS "")
foreach(SHADER ${SHADER_FILES})
	get_filename_component(SHADER_NAME ${SHADER} NAME)
	set(OUTPUT_SPV ${SHADER_DST_DIR}/${SHADER_NAME}.spv)

	if(GLSLANG_VALIDATOR)
		add_custom_command(
			OUTPUT ${OUTPUT_SPV}
			COMMAND ${GLSLANG_VALIDATOR} -V ${SHADER} -o ${OUTPUT_SPV}
			DEPENDS ${SHADER}
			COMMENT "Compiling ${SHADER_NAME} to SPIR-V"
			VERBATIM
		)
	else()
		add_custom_command(
			OUTPUT ${OUTPUT_SPV}
			COMMAND ${CMAKE_COMMAND} -E echo "Warning: glslangValidator missing, cannot compile ${SHADER_NAME}"
			COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT_SPV}
			DEPENDS ${SHADER}
			COMMENT "Skipping shader compilation (glslangValidator not found)"
		)
	endif()
	list(APPEND SHADER_OUTPUTS ${OUTPUT_SPV})
endforeach()

add_custom_target(CompileShaders DEPENDS ${SHADER_OUTPUTS})

add_dependencies(${CMAKE_PROJECT_NAME} CompileShaders)