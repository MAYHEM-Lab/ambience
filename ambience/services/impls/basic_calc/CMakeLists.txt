add_service(basic_calc calc impl.cpp)
target_link_libraries(basic_calc PUBLIC log_schema alarm_schema filesystem_schema)

if (TARGET ae_user_space)
    add_service(calc_bench_agent agent calc_bench_agent.cpp)
    target_link_libraries(calc_bench_agent PUBLIC calc_schema ae_user_space)
endif()

# name: basic_calc
# interface: tos::ae::services::calculator
# dependencies:
#   logger: tos::services::logger