add_lidlc(social_media_schema sm_common.lidl analysis.lidl posts.lidl accounts.lidl)

add_service(basic_analyzer social_media basic_analyzer.cpp)
add_service(posts_manager social_media posts_manager.cpp)

if (TARGET ae_user_space)
    add_service(posts_bench_agent agent posts_bench_agent.cpp)
    target_link_libraries(posts_bench_agent PUBLIC social_media_schema ae_user_space)
endif()