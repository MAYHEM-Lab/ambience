#include <caps/emsha_signer.hpp>

namespace caps::emsha {
void merge_into(sign_t& s, const hash_t& h) {
    auto i = s.signature()->GetMutablePointer(0);
    auto i_end = s.signature()->data() + s.signature()->size();
    auto j = std::begin(h.buf);
    for (; i != i_end; ++i, ++j) {
        *i ^= *j;
    }
    ::emsha::sha256_digest(s.signature()->data(),
                           s.signature()->size(),
                           s.signature()->GetMutablePointer(0));
}
} // namespace caps