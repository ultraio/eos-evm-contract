/*
   Copyright 2020 The Silkrpc Authors

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef SILKRPC_KV_REMOTE_CURSOR_H_
#define SILKRPC_KV_REMOTE_CURSOR_H_

#include <silkrpc/config.hpp>

#include <memory>

#include <asio/awaitable.hpp>
#include <asio/io_context.hpp>
#include <asio/use_awaitable.hpp>

#include <silkworm/common/util.hpp>
#include <silkrpc/common/clock_time.hpp>
#include <silkrpc/common/log.hpp>
#include <silkrpc/common/util.hpp>
#include <silkrpc/ethdb/kv/awaitables.hpp>
#include <silkrpc/ethdb/kv/cursor.hpp>

namespace silkrpc::ethdb::kv {

class RemoteCursor : public Cursor {
public:
    RemoteCursor(KvAsioAwaitable<asio::io_context::executor_type>& kv_awaitable)
    : kv_awaitable_(kv_awaitable), cursor_id_{0} {}

    RemoteCursor(const RemoteCursor&) = delete;
    RemoteCursor& operator=(const RemoteCursor&) = delete;

    uint32_t cursor_id() const override { return cursor_id_; };

    asio::awaitable<void> open_cursor(const std::string& table_name) override;

    asio::awaitable<silkrpc::common::KeyValue> seek(const silkworm::ByteView& seek_key) override;

    asio::awaitable<silkrpc::common::KeyValue> next() override;

    asio::awaitable<void> close_cursor() override;

private:
    KvAsioAwaitable<asio::io_context::executor_type>& kv_awaitable_;
    uint32_t cursor_id_;
};

} // namespace silkrpc::ethdb::kv

#endif  // SILKRPC_KV_REMOTE_CURSOR_H_
