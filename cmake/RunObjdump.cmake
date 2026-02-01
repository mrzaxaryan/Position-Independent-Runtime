# Run llvm-objdump and redirect output to file
if(NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "INPUT_FILE and OUTPUT_FILE must be defined")
endif()

execute_process(
    COMMAND llvm-objdump -d -s -h -j .text "${INPUT_FILE}"
    OUTPUT_FILE "${OUTPUT_FILE}"
    ERROR_VARIABLE err
    RESULT_VARIABLE res
)

if(NOT res EQUAL 0)
    message(FATAL_ERROR "llvm-objdump failed: ${err}")
endif()
