#!/usr/bin/env bash
set -eo pipefail

# Run nodeos if not
get_info=$(cleos get info && echo 1 || echo 0)
if [ $get_info == 0 ]; then
    nodeos -e -p eosio --plugin eosio::chain_api_plugin --delete-all-blocks >> nodeos.log 2>&1 &
    sleep 5
fi

SCRIPT_DIR=$(dirname "$0")
CONTRACTS_DIR=$SCRIPT_DIR/../../build/evm_runtime
HASH_FILE=$CONTRACTS_DIR/hash.txt

if [ ! -d "$CONTRACTS_DIR" ]; then
    echo "Directory $CONTRACTS_DIR does not exist. Please run ./build.sh first."
    exit 1
fi

ABI_FILES=$(find $CONTRACTS_DIR -type f -name "*.abi")
WASM_FILES=$(find $CONTRACTS_DIR -type f -name "*.wasm" -not -name "CMake*")

if [ ! -z "$WASM_FILES" ]; then
    > $HASH_FILE
else
    echo "No .wasm file is found."
    exit 1
fi

for abiFile in $ABI_FILES
do
    CONTRACT_JSON=$(cleos set abi eosio $abiFile -p eosio -d --suppress-duplicate-check -j -s)
    ABIHEX=$(echo $CONTRACT_JSON | jq '.actions[] | select (.name=="setabi")' | jq -r '.data')
    ABIHASH=$(cleos convert unpack_action_data eosio setabi "$ABIHEX" | jq -r '.abi' | xxd -r -p | sha256sum | cut -d " " -f1)

    if [ ! -z "$ABIHASH" ]; then
        echo "`basename $abiFile`: $ABIHASH" >> $HASH_FILE
    fi
done

for file in $WASM_FILES
do
    hash=$(od -An -vtx1 $file | xxd -r -p | sha256sum | cut -d " " -f1)
    name=${file##*/}
    # name=${name%.*}
    if [ ! -z "$hash" ]; then
        echo "$name: $hash" >> $HASH_FILE
    fi
done


echo "-- Contracts code hash have been written to: $HASH_FILE"
pkill nodeos