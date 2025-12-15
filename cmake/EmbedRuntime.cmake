function(read_file_as_string filepath varname)
    file(READ "${filepath}" file_content)
    string(REPLACE "\\" "\\\\" file_content "${file_content}")
    string(REPLACE "\"" "\\\"" file_content "${file_content}")
    set(${varname} "${file_content}" PARENT_SCOPE)
endfunction()

function(embed_sxs_runtime output_file)
    set(SXS_ROOT "${CMAKE_SOURCE_DIR}/runtime/sxs")
    
    set(SXS_FILES
        "include/sxs/types.h"
        "include/sxs/runtime.h"
        "include/sxs/sxs.h"
        "include/sxs/ds/buffer.h"
        "include/sxs/ds/map.h"
        "include/sxs/ds/scanner.h"
        "src/runtime.c"
        "src/ds/buffer.c"
        "src/ds/map.c"
        "src/ds/scanner.c"
    )
    
    set(GENERATED_CONTENT "#pragma once\n\n")
    string(APPEND GENERATED_CONTENT "#include <string>\n")
    string(APPEND GENERATED_CONTENT "#include <unordered_map>\n\n")
    string(APPEND GENERATED_CONTENT "namespace truk::emitc::embedded {\n\n")
    
    string(APPEND GENERATED_CONTENT "struct runtime_file_s {\n")
    string(APPEND GENERATED_CONTENT "    std::string content;\n")
    string(APPEND GENERATED_CONTENT "    bool is_header;\n")
    string(APPEND GENERATED_CONTENT "    bool for_application;\n")
    string(APPEND GENERATED_CONTENT "    bool for_library;\n")
    string(APPEND GENERATED_CONTENT "};\n\n")
    
    string(APPEND GENERATED_CONTENT "inline const std::unordered_map<std::string, runtime_file_s> runtime_files = {\n")
    
    foreach(rel_path ${SXS_FILES})
        set(full_path "${SXS_ROOT}/${rel_path}")
        
        if(NOT EXISTS "${full_path}")
            message(FATAL_ERROR "Runtime file not found: ${full_path}")
        endif()
        
        file(READ "${full_path}" file_content)
        
        get_filename_component(ext "${rel_path}" EXT)
        if(ext STREQUAL ".h")
            set(is_header "true")
        else()
            set(is_header "false")
        endif()
        
        set(for_app "true")
        set(for_lib "true")
        
        string(APPEND GENERATED_CONTENT "    {\"${rel_path}\", {R\"EMBED_DELIM(\n")
        string(APPEND GENERATED_CONTENT "${file_content}")
        string(APPEND GENERATED_CONTENT ")EMBED_DELIM\", ${is_header}, ${for_app}, ${for_lib}}},\n")
    endforeach()
    
    string(APPEND GENERATED_CONTENT "};\n\n")
    string(APPEND GENERATED_CONTENT "} // namespace truk::emitc::embedded\n")
    
    file(WRITE "${output_file}" "${GENERATED_CONTENT}")
    message(STATUS "Generated embedded runtime: ${output_file}")
endfunction()
