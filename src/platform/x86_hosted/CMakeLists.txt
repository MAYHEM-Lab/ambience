project(tos CXX ASM)

add_library(x86_hosted_core x86_core.cpp include/tos/platform.hpp)
# Without this flag, the runtime detects us jumping into the middle of a stack and complains.
# Crazy, right?
target_compile_options(x86_hosted_core PUBLIC "-U_FORTIFY_SOURCE")
target_compile_features(x86_hosted_core PUBLIC cxx_std_17)

set(Boost_USE_STATIC_LIBS ON)
# we use boost.asio for asynchronous operations
find_package(Boost COMPONENTS system)

add_library(x86_hosted_impl x86_impl.cpp)
target_link_libraries(x86_hosted_impl PUBLIC x86_hosted_core)
target_link_libraries(x86_hosted_impl PUBLIC tos_interrupt)
target_link_libraries(x86_hosted_impl PUBLIC tos_jobs)

find_package(Threads REQUIRED)
target_link_libraries(x86_hosted_impl PUBLIC Threads::Threads)


if (TARGET Boost::boost)
    target_link_libraries(x86_hosted_impl PUBLIC Boost::boost)
endif()
if (TARGET Boost::system)
    target_link_libraries(x86_hosted_impl PUBLIC Boost::system)
endif()

tos_install(x86_hosted_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(x86_hosted_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
