#include "sese/net/http/DynamicTable.h"
#include "sese/net/http/HPACK.h"

#include <limits>

using namespace sese::net::http;

DynamicTable::DynamicTable(size_t max) noexcept {
    this->max = max;
}

void DynamicTable::resize(size_t max) noexcept {
    while (size >= max) {
        decltype(auto) header = queue.back();
        size -= header.first.size() + header.second.size() + 4;
        queue.pop_back();
    }
    this->max = max;
}


bool DynamicTable::set(const std::string &key, const std::string &value) noexcept {
    if (key.size() > std::numeric_limits<size_t>::max() - value.size()) {
        return false;
    }

    auto addition = key.size() + value.size() + 4;

    while (size + addition >= max) {
        decltype(auto) header = queue.back();
        size -= header.first.size() + header.second.size() + 4;
        queue.pop_back();
    }

    size += addition;
    queue.emplace_back(key, value);
    return true;
}

std::optional<DynamicTable::Header> DynamicTable::get(size_t index) const noexcept {
    if (index < PREDEFINED_HEADERS.size()) {
        return PREDEFINED_HEADERS.at(index);
    } else if (index < PREDEFINED_HEADERS.size() + queue.size()) {
        return queue.at(index - PREDEFINED_HEADERS.size());
    }
    return std::nullopt;
}
