serv_instance(
    name="sensor",
    serv=null_weather_sensor,
    deps={
    }
)

serv_instance(
    name="sampler",
    serv=weather_sampler,
    deps={
        "alarm": "alarm.localnode",
        "sensor": "weather_sensor.localnode"
    }
)

# export(
#     service="sampler",
#     networks={
#         "udp-internet": 1234
#     }
# )

export(
    service="sampler",
    networks={
        "xbee-home": 0
    }
)