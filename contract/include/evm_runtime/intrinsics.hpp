#pragma once
namespace eosio {
   namespace internal_use_do_not_use {
      extern "C" {
         __attribute__((eosio_wasm_import))
         uint32_t get_block_num();

        #ifdef WITH_LOGTIME
        __attribute__((eosio_wasm_import))
         void logtime(const char*);
        #endif
      }
   }
}