Service(
    name="basic_calc",
    interface="tos::ae::services::calculator",
    type="async",
    dependencies={
        "logger": "tos::ae::services::calculator",
        "alarm": "tos::ae::services::alarm",
        "fs": "tos::ae::services::file_system"
    },
    cmake_target="basic_calc",
)
