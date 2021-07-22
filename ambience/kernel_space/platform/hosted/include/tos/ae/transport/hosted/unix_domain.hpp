#pragma once

#include <arch/unix_socket.hpp>
#include <lidlrt/transport/common.hpp>
#include <tos/ae/exporter.hpp>
#include <tos/ae/importer.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/semaphore.hpp>

namespace tos::ae {

using unix_domain_import_args = std::string;

struct unix_domain_importer : importer::sync_server {
    struct transport : lidl::verbatim_transform {
        explicit transport(std::string path)
            : m_sock_cache_sem(200) {
            for (int i = 0; i < 200; ++i) {
                m_sock_cache.push_back(*new sock_state(path));
            }
        }

        std::vector<uint8_t> get_buffer();
        std::vector<uint8_t> send_receive(tos::span<uint8_t> buffer);

        struct sock_state : list_node<sock_state> {
            sock_state(const std::string& path)
                : m_sock{force_get(hosted::connect(path))} {
            }
            std::unique_ptr<hosted::unix_socket> m_sock;
        };

        tos::semaphore m_sock_cache_sem;
        tos::intrusive_list<sock_state> m_sock_cache;
    };

    template<class Service>
    static typename Service::sync_server*
    import_service(const unix_domain_import_args& args) {
        return new typename Service::template stub_client<transport>(args);
    }

    int64_t number_of_calls() override {
        return 0;
    }
};

struct unix_domain_export_args : export_args {
    std::string path;
};

template<class ServT>
struct unix_export {
    unix_export(const ServT& serv, std::string path)
        : m_serv{serv}
        , m_listener{std::make_unique<hosted::unix_listener>(path)} {
        tos::launch(tos::alloc_stack, [this] { accept_thread(); });
    }

    void accept_thread();

    ServT m_serv;
    std::unique_ptr<hosted::unix_listener> m_listener;
};

struct unix_domain_exporter : service_exporter {
    void export_service(const sync_service_host& host, const export_args& args) override;
    void export_service(const async_service_host& host, const export_args& args) override;
    int64_t number_of_calls() override {
        return 0;
    }
};

extern template struct unix_export<sync_service_host>;
extern template struct unix_export<async_service_host>;
} // namespace tos::ae