
# enable testing with CTest
include(CTest)

if(NOT DISABLE_CATCH2v3)
    include(FetchContent)
    FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG devel)
    FetchContent_MakeAvailable(Catch2)
endif()

add_subdirectory(src)

