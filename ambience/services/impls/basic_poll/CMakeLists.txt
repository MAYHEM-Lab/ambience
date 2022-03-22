add_service(basic_poll poll poll.cpp)
target_link_libraries(basic_poll PUBLIC ae_user_space)

if (TARGET ae_user_space)
    add_service(poll_bench_agent agent poll_bench_agent.cpp)
    target_link_libraries(poll_bench_agent PUBLIC poll_schema ae_user_space)
endif()