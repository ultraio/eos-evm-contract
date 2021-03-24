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

#include <silkrpc/config.hpp>

#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <asio/awaitable.hpp>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <boost/process/environment.hpp>
#include <grpcpp/grpcpp.h>

#include <silkrpc/common/constants.hpp>
#include <silkrpc/common/log.hpp>
#include <silkrpc/http/server.hpp>
#include <silkrpc/ethdb/kv/remote_database.hpp>

ABSL_FLAG(std::string, chaindata, silkrpc::common::kEmptyChainData, "chain data path as string");
ABSL_FLAG(std::string, local, silkrpc::common::kDefaultLocal, "HTTP JSON local binding as string <address>:<port>");
ABSL_FLAG(std::string, target, silkrpc::common::kDefaultTarget, "TG Core gRPC service location as string <address>:<port>");
ABSL_FLAG(uint32_t, timeout, silkrpc::common::kDefaultTimeout.count(), "gRPC call timeout as 32-bit integer");
ABSL_FLAG(silkrpc::LogLevel, logLevel, silkrpc::LogLevel::LogCritical, "logging level");

int main(int argc, char* argv[]) {
    const auto pid = boost::this_process::get_id();
    const auto tid = std::this_thread::get_id();

    using namespace silkrpc;
    using silkrpc::common::kAddressPortSeparator;

    try {
        absl::SetProgramUsageMessage("Seek Turbo-Geth/Silkworm Key-Value (KV) remote interface to database");
        absl::ParseCommandLine(argc, argv);

        auto chaindata{absl::GetFlag(FLAGS_chaindata)};
        if (!chaindata.empty() && !std::filesystem::exists(chaindata)) {
            SILKRPC_ERROR << "Parameter chaindata is invalid: [" << chaindata << "]\n";
            SILKRPC_ERROR << "Use --chaindata flag to specify the path of Turbo-Geth database\n";
            return -1;
        }

        auto local{absl::GetFlag(FLAGS_local)};
        if (!local.empty() && local.find(kAddressPortSeparator) == std::string::npos) {
            SILKRPC_ERROR << "Parameter local is invalid: [" << local << "]\n";
            SILKRPC_ERROR << "Use --local flag to specify the local binding for HTTP JSON server\n";
            return -1;
        }

        auto target{absl::GetFlag(FLAGS_target)};
        if (!target.empty() && target.find(":") == std::string::npos) {
            SILKRPC_ERROR << "Parameter target is invalid: [" << target << "]\n";
            SILKRPC_ERROR << "Use --target flag to specify the location of Turbo-Geth running instance\n";
            return -1;
        }

        if (chaindata.empty() && target.empty()) {
            SILKRPC_ERROR << "Parameters chaindata and target cannot be both empty, specify one of them\n";
            SILKRPC_ERROR << "Use --chaindata or --target flag to specify the path or the location of Turbo-Geth instance\n";
            return -1;
        }

        auto timeout{absl::GetFlag(FLAGS_timeout)};
        if (timeout < 0) {
            SILKRPC_ERROR << "Parameter timeout is invalid: [" << timeout << "]\n";
            SILKRPC_ERROR << "Use --timeout flag to specify the timeout in msecs for Turbo-Geth KV gRPC I/F\n";
            return -1;
        }

        SILKRPC_LOG_VERBOSITY(absl::GetFlag(FLAGS_logLevel));

        asio::io_context context{1};
        asio::signal_set signals{context, SIGINT, SIGTERM};

        // TODO: handle also secure channel for remote
        auto grpc_channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
        // TODO: handle also local (shared-memory) database
        std::unique_ptr<silkrpc::ethdb::kv::Database> database =
            std::make_unique<silkrpc::ethdb::kv::RemoteDatabase>(context, grpc_channel);

        const auto http_host = local.substr(0, local.find(kAddressPortSeparator));
        const auto http_port = local.substr(local.find(kAddressPortSeparator) + 1, std::string::npos);
        silkrpc::http::Server http_server{context, http_host, http_port, database};

        signals.async_wait([&](const asio::system_error& error, int signal_number) {
            std::cout << "\n";
            SILKRPC_INFO << "Signal caught, error: " << error.what() << " number: " << signal_number << "\n" << std::flush;
            context.stop();
            http_server.stop();
        });

        asio::co_spawn(context, http_server.start(), [&](std::exception_ptr eptr) {
            if (eptr) std::rethrow_exception(eptr);
        });

        SILKRPC_LOG << "Silkrpc running [pid=" << pid << ", main thread: " << tid << "]\n" << std::flush;

        context.run();
    } catch (const std::exception& e) {
        SILKRPC_CRIT << "Exception: " << e.what() << "\n" << std::flush;
    } catch (...) {
        SILKRPC_CRIT << "Unexpected exception\n" << std::flush;
    }

    SILKRPC_LOG << "Silkrpc exiting [pid=" << pid << ", main thread: " << tid << "]\n" << std::flush;

    return 0;
}