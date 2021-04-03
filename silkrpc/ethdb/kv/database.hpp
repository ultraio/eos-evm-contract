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

#ifndef SILKRPC_KV_DATABASE_H_
#define SILKRPC_KV_DATABASE_H_

#include <memory>

#include <silkrpc/config.hpp>

#include <asio/awaitable.hpp>

#include <silkrpc/ethdb/kv/transaction.hpp>

namespace silkrpc::ethdb::kv {

class Database {
public:
    Database() = default;
    virtual ~Database() = default;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    virtual asio::awaitable<std::unique_ptr<Transaction>> begin() = 0;
};

} // namespace silkrpc::ethdb::kv

#endif  // SILKRPC_KV_DATABASE_H_
