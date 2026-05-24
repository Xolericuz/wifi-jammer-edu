# ============================================
# RF Interference Simulator — WASM Build
# ============================================
# Talablar:
#   - Emscripten SDK (emcc)
#   - Python 3 (local server uchun)
#
# O'rnatish:
#   git clone https://github.com/emscripten-core/emsdk.git
#   cd emsdk && ./emsdk install latest && ./emsdk activate latest
#   source ./emsdk_env.sh
#
# Ishga tushirish:
#   make wasm     # WASM kompilatsiya
#   make serve    # Local serverni ishga tushirish
#   make clean    # Tozalash

EMCC = emcc
CFLAGS = -O3 --closure 1 -s WASM=1 -s MODULARIZE=1 \
         -s EXPORTED_FUNCTIONS='["_calcFSPL","_calcJam","_getRadius","_calcSINR","_deauthFrames","_generateNoise","_calcThroughput"]' \
         -s ALLOW_MEMORY_GROWTH=1 \
         -s EXPORT_NAME='WasmRFSim' \
         --no-heap-copy

SRC = src/sim.cpp
OUT_JS = wasm/sim.js
OUT_WASM = wasm/sim.wasm

.PHONY: all wasm serve clean

all: wasm

wasm:
	@mkdir -p wasm
	$(EMCC) $(SRC) -o $(OUT_JS) $(CFLAGS)
	@echo ""
	@echo "✅ WASM build complete!"
	@echo "   $(OUT_JS)"
	@echo "   $(OUT_WASM)"
	@echo ""
	@echo "   Fayl hajmi:"
	@ls -lh $(OUT_WASM)

serve:
	@echo "🌐 Server running at http://localhost:8000"
	python3 -m http.server 8000

clean:
	rm -rf wasm/
	@echo "🧹 Cleaned"
