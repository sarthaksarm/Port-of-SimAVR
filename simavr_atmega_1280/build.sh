#!/bin/bash

emmake make
cp simavr/run_avr simavr/run_avr.bc
emcc -s ASM_JS=1 -s WASM=0 -s LINKABLE=1 -s ASSERTIONS=1 -s FORCE_FILESYSTEM=1 -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap","ALLOC_NORMAL"]' -o simavr.js simavr/run_avr.bc -O2
