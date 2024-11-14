#pragma once

#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/asset.hpp>
#include <eosio/symbol.hpp>
#include <intx/intx.hpp>
#include <evmc/evmc.hpp>
#include <ethash/hash_types.hpp>

namespace evm_runtime {
   using intx::operator""_u256;
   static constexpr uint32_t ninety_percent = 90'000;
   static constexpr uint32_t hundred_percent = 100'000;
   static constexpr uint64_t one_gwei = 1'000'000'000ull;
   static constexpr uint64_t gas_sset_min = 2900;
   static constexpr uint64_t grace_period_seconds = 180;

   uint64_t pow10_const(int v);

   constexpr unsigned evm_precision = 18;
   constexpr eosio::name default_token_account = "eosio.token"_n;

   typedef intx::uint<256>         uint256;
   typedef intx::uint<512>         uint512;
   typedef std::vector<char>       bytes;
   typedef evmc::address           address;
   typedef ethash::hash256         hash256;
   typedef evmc::bytes32           bytes32;
   typedef evmc::bytes32           uint256be;

   eosio::checksum256 make_key(bytes data);
   eosio::checksum256 make_key(const evmc::address& addr);
   eosio::checksum256 make_key(const evmc::bytes32& data);

   bytes to_bytes(const uint256& val);
   bytes to_bytes(const evmc::bytes32& val);
   bytes to_bytes(const evmc::address& addr);

   evmc::address to_address(const bytes& addr);
   evmc::bytes32 to_bytes32(const bytes& data);
   uint256 to_uint256(const bytes& value);

   struct exec_input {
      std::optional<bytes> context;
      std::optional<bytes> from;
      bytes                to;
      bytes                data;
      std::optional<bytes> value;

      EOSLIB_SERIALIZE(exec_input, (context)(from)(to)(data)(value));
   };

   struct exec_callback {
      eosio::name contract;
      eosio::name action;

      EOSLIB_SERIALIZE(exec_callback, (contract)(action));
   };

   struct exec_output {
      int32_t              status;
      bytes                data;
      std::optional<bytes> context;

      EOSLIB_SERIALIZE(exec_output, (status)(data)(context));
   };

   struct bridge_message_v0 {
      eosio::name        receiver;
      bytes              sender;
      eosio::time_point  timestamp;
      bytes              value;
      bytes              data;

      EOSLIB_SERIALIZE(bridge_message_v0, (receiver)(sender)(timestamp)(value)(data));
   };

   using bridge_message = std::variant<bridge_message_v0>;

   struct evmtx_base {
      uint64_t  eos_evm_version;
      bytes     rlptx;
      EOSLIB_SERIALIZE(evmtx_base, (eos_evm_version)(rlptx));
   };

   struct evmtx_v1 : evmtx_base {
      uint64_t  base_fee_per_gas;
      EOSLIB_SERIALIZE_DERIVED(evmtx_v1, evmtx_base, (base_fee_per_gas));
   };

   struct evmtx_v3 : evmtx_base {
      uint64_t overhead_price;
      uint64_t storage_price;
      EOSLIB_SERIALIZE_DERIVED(evmtx_v3, evmtx_base, (overhead_price)(storage_price));
   };

   using evmtx_type = std::variant<evmtx_v1, evmtx_v3>;

   struct fee_parameters
   {
      std::optional<uint64_t> gas_price; ///< Minimum gas price (in 10^-18 EOS, aka wei) that is enforced on all
                                          ///< transactions. Required during initialization.

      std::optional<uint32_t> miner_cut; ///< Percentage cut (maximum allowed value of 100,000 which equals 100%) of the
                                          ///< gas fee collected for a transaction that is sent to the indicated miner of
                                          ///< that transaction. Required during initialization.

      std::optional<eosio::asset> ingress_bridge_fee; ///< Fee (in EOS) deducted from ingress transfers of EOS across bridge.
                                             ///< Symbol must be in EOS and quantity must be non-negative. If not
                                             ///< provided during initialization, the default fee of 0 will be used.
   };

} //namespace evm_runtime

namespace eosio {

template<typename Stream>
inline datastream<Stream>& operator>>(datastream<Stream>& ds, evm_runtime::uint256& v) {
   evm_runtime::bytes buffer(32,0);
   ds.read((char*)buffer.data(), 32);
   v = intx::be::unsafe::load<evm_runtime::uint256>((const uint8_t *)buffer.data());
   return ds;
}

} //namespace eosio

namespace std {
template <typename DataStream>
DataStream& operator<<(DataStream& ds, const std::basic_string<uint8_t>& bs)
{
   ds << (eosio::unsigned_int)bs.size();
   if (bs.size())
      ds.write((const char*)bs.data(), bs.size());
   return ds;
}
} // namespace std