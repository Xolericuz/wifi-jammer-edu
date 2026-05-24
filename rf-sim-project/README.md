# RF Interference Simulator v2.0

**Full pipeline:** HTML + CSS + JS → Three.js → WASM (C++)

802.11 protokoli, FSPL (Free Space Path Loss) matematik modeli va RF interferensiya mexanizmlarini o'rganish uchun ta'lim platformasi.

## Architecture

```
HTML + CSS + JS    ← Frontend UI
    ↓
Three.js           ← 3D Visualization (WebGL2)
    ↓
WASM (C++)         ← Computation Core (Emscripten)
    ↓
FSPL Model         ← RF Physics Engine
```

## Project Structure

```
rf-sim-project/
├── index.html       # Main entry point
├── package.json     # npm scripts
├── Makefile         # WASM build commands
├── src/
│   └── sim.cpp      # C++ WASM source (FSPL, SINR, throughput)
└── wasm/            # Compiled WASM output (generated)
```

## WASM Build

```bash
# 1. Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh

# 2. Build WASM
cd rf-sim-project
make wasm

# 3. Run local server
make serve
# → http://localhost:8000
```

## C++ WASM Exports

| Function | Parameters | Return | Description |
|----------|-----------|--------|-------------|
| `calcJam` | pwr, dist, freq | float (dBm) | Signal strength at distance |
| `getRadius` | pwr, freq, threshold | float (m) | Effective jamming radius |
| `calcSINR` | signal, jammer, noise | float (dB) | Signal-to-Interference+Noise Ratio |
| `deauthFrames` | seconds, rate | int | Frame count |
| `generateNoise` | mean, stddev | float | Gaussian noise |
| `calcThroughput` | sinr, bw | float (Mbps) | Throughput estimation |

## 802.11 Deauth Frame Structure

```
struct DeauthFrame {
    uint8_t frame_control[2] = {0xC0, 0x00};
    uint8_t duration[2];
    uint8_t dst_addr[6];      // FF:FF:FF:FF:FF:FF (broadcast)
    uint8_t src_addr[6];
    uint8_t bssid[6];
    uint8_t seq_num[2];
    uint8_t reason_code[2] = {0x07, 0x00};
    // FCS — hardware auto-calculated
};
```

## License

Educational purpose only. No real RF signal transmitted.
