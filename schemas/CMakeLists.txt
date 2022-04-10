
set(SCHEMA_HEADER "#ifndef SCHEMA_H\n#define SCHEMA_H\n#include<string>\nnamespace schemas{\n")

function(schema_gen file_input)
    set(schema_path ${CMAKE_CURRENT_LIST_DIR}/${file_input}.schema.json)
    set(out_path ${CMAKE_CURRENT_LIST_DIR}/include_gen/${file_input}.out)
    set(SCHEMA_HEADER "${SCHEMA_HEADER}const nlohmann::json ${file_input}Schema =\n#include \"${out_path}\"\n\;\n" PARENT_SCOPE)
    make_includable(${schema_path} ${out_path})
endfunction()


schema_gen(clientLog)
schema_gen(connectionAdminInfo)
schema_gen(connectionPermission)
schema_gen(connectionPermissionReply)
schema_gen(connectionRequest)
schema_gen(connectionRevoke)
schema_gen(ssgcData)
MESSAGE(STATUS "Generated schemas.")

set(SCHEMA_HEADER "${SCHEMA_HEADER}\n}\n#endif\n")
file(WRITE ${CMAKE_CURRENT_LIST_DIR}/include_gen/schemas.hpp ${SCHEMA_HEADER})
include_directories(${CMAKE_CURRENT_LIST_DIR}/include_gen)
