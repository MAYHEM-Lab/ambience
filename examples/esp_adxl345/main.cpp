//
// Created by fatih on 12/23/18.
//

#include <tos/ft.hpp>
#include <common/adxl345.hpp>
#include <arch/lx106/drivers.hpp>
#include <tos/print.hpp>
#include <common/gpio.hpp>
#include <common/inet/tcp_stream.hpp>
#include <cwpack.hpp>

struct vec3i {
  int32_t x, y, z;
};

tos::fixed_fifo<vec3i, 20> fifo;

tos::stack_storage<1024> accel_stack;
auto task = [] {
  tos::esp82::twim twim{ {5}, {4} };

  tos::adxl345 sensor{twim};
  sensor.powerOn();
  sensor.setRangeSetting(2);

  auto tmr = tos::open(tos::devs::timer<0>);
  auto alarm = tos::open(tos::devs::alarm, tmr);

  while (true)
  {
      auto [x, y, z] = sensor.read();

      fifo.push({x, y, z});

      using namespace std::chrono_literals;
      alarm->sleep_for(200ms);
  }
};

auto wifi_connect()
{
  tos::esp82::wifi w;

  conn_:
  auto res = w.connect("cs190b", "cs190bcs190b");
  if (!res) goto conn_;

  auto& wconn = force_get(res);

  wconn.wait_for_dhcp();
  lwip_init();

  return std::make_pair(w, std::move(wconn));
}

auto wifi_task = []{
  auto [w, conn] = wifi_connect();

  while (true)
  {
    auto try_conn = tos::esp82::connect(conn, {{169, 231, 9, 60}}, {9993});
    if (!try_conn) continue;

    auto& conn = force_get(try_conn);

    auto elem = fifo.pop();
    std::array<char, 32> buf;
    tos::msgpack::packer p{buf};
    auto arr = p.insert_arr(3);
    arr.insert(elem.x);
    arr.insert(elem.y);
    arr.insert(elem.z);
    auto sp = p.get();
    auto schema = "bakir/adxl";
    std::strncpy(buf.begin() + sp.size(), schema, buf.size() - sp.size());
    buf[sp.size() + strlen(schema)] = strlen(schema);

    tos::tcp_stream<tos::esp82::tcp_endpoint> stream{std::move(conn)};

    stream->write(tos::span(buf).slice(0, sp.size() + strlen(schema) + 1));
  }
};

void tos_main()
{
  tos::launch(accel_stack, task);
  tos::launch(tos::alloc_stack, wifi_task);
}
