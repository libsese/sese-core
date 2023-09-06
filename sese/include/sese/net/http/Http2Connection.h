#pragma once

#include "sese/net/V2Server.h"
#include "sese/net/http/Http2Stream.h"
#include "sese/net/http/DynamicTable.h"

#include <map>
#include <mutex>

namespace sese::net::v2::http {

    class SESE_DEPRECATED Http2Connection {
    public:
        using Ptr = std::shared_ptr<Http2Connection>;

        explicit Http2Connection(v2::IOContext ctx) noexcept;

        void addStream(uint32_t sid, const Http2Stream::Ptr &stream) noexcept;
        Http2Stream::Ptr find(uint32_t sid);

        v2::IOContext context;
        /// 此互斥量用于同步发送进度
        std::mutex mutex;
        std::map<uint8_t, Http2Stream::Ptr> streamMap;
        net::http::DynamicTable dynamicTable4recv;
        net::http::DynamicTable dynamicTable4send;

        // settings
        uint32_t headerTableSize = 4096;
        uint32_t enablePush = 1;
        uint32_t maxConcurrentStream = 0;
        uint32_t windowSize = 65535;
        uint32_t maxFrameSize = 16384;
        uint32_t maxHeaderListSize = 0;
    };

}// namespace sese::net::v2::http

namespace sese::net::http {
    struct API Http2Connection {
        using Ptr = std::shared_ptr<Http2Connection>;

        /// 流映射
        std::map<uint8_t, Http2Stream::Ptr> streamMap;
        /// 发送用动态表
        DynamicTable dynamicTable1;
        /// 接收用动态表
        DynamicTable dynamicTable2;

        bool first = true;

        // settings
        uint32_t headerTableSize = 4096;
        uint32_t enablePush = 1;
        uint32_t maxConcurrentStream = 0;
        uint32_t windowSize = 65535;
        uint32_t maxFrameSize = 16384;
        uint32_t maxHeaderListSize = 0;

        void *data = nullptr;

        void decodeHttp2Settings(const std::string &settings) noexcept;
    };
}// namespace sese::net::http