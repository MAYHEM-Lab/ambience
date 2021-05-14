add_service(basic_calc calc impl.cpp)
target_link_libraries(basic_calc PUBLIC log_schema)

# name: basic_calc
# interface: tos::ae::services::calculator
# dependencies:
#   logger: tos::services::logger