function(get_sxs_runtime_files out_var)
    set(SXS_FILES
        "include/sxs/types.h"
        "include/sxs/runtime.h"
        "include/sxs/sxs.h"
        "include/sxs/ds/map.h"
        "src/runtime.c"
        "src/ds/map.c"
    )
    set(${out_var} ${SXS_FILES} PARENT_SCOPE)
endfunction()

function(embed_sxs_runtime output_file)
    if(NOT DEFINED SXS_ROOT)
        set(SXS_ROOT "${CMAKE_SOURCE_DIR}/runtime/sxs")
    endif()
    
    get_sxs_runtime_files(SXS_FILES)
    
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
