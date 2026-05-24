/*
 * RF Interference Simulator — C++ WASM Core
 * ==========================================
 * Compile: emcc sim.cpp -o sim.js -s WASM=1 -s MODULARIZE=1 \
 *           -s EXPORTED_FUNCTIONS='["_calcJam","_getRadius","_calcSINR","_deauthFrames","_generateNoise"]' \
 *           -s ALLOW_MEMORY_GROWTH=1 -O3 --closure 1
 *
 * Bu modul brauzerda ishlaydigan WebAssembly (WASM) yadrosidir.
 * Barcha hisob-kitoblar C++ da bajariladi va JavaScript orqali chaqiriladi.
 */

#include <cmath>
#include <cstdint>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

/*
 * FSPL — Free Space Path Loss
 * Formula: FSPL(dB) = 20·log10(distance) + 20·log10(frequency) - 27.55
 * 
 * Parametrlar:
 *   d — masofa (metr)
 *   f — chastota (MHz)
 * 
 * Return: path loss dB da
 */
extern "C" {
EMSCRIPTEN_KEEPALIVE
float calcFSPL(float d, float f) {
    if (d <= 0.0f || f <= 0.0f) return 0.0f;
    return 20.0f * log10f(d) + 20.0f * log10f(f) - 27.55f;
}

/*
 * calcJam — Jam signal kuchini hisoblash
 *   pwr_dbm — uzatish quvvati (dBm)
 *   d — masofa (metr)
 *   f — chastota (MHz)
 * 
 * Return: qabul qiluvchi nuqtadagi signal (dBm)
 */
EMSCRIPTEN_KEEPALIVE
float calcJam(float pwr_dbm, float d, float f) {
    return pwr_dbm - calcFSPL(d, f);
}

/*
 * getRadius — Samara doirasini hisoblash
 *   pwr — uzatish quvvati (dBm)
 *   f — chastota (MHz)
 *   threshold — minimal signal chegarasi (dBm, default -85)
 * 
 * Return: maksimal samaradorlik radiusi (metr)
 */
EMSCRIPTEN_KEEPALIVE
float getRadius(float pwr, float f, float threshold) {
    // Binary search: 0.5m dan 500m gacha
    float lo = 0.5f, hi = 500.0f, mid;
    for (int i = 0; i < 50; i++) {
        mid = (lo + hi) / 2.0f;
        float sig = calcJam(pwr, mid, f);
        if (sig >= threshold)
            lo = mid;
        else
            hi = mid;
    }
    return lo;
}

/*
 * calcSINR — Signal-to-Interference+Noise Ratio
 *   signal_dbm — foydali signal (dBm)
 *   jam_dbm — jammer signali (dBm)
 *   noise_dbm — shovqin darajasi (dBm, default -95)
 * 
 * Return: SINR dB da. SINR < 0 dB → aloqa uziladi.
 */
EMSCRIPTEN_KEEPALIVE
float calcSINR(float signal_dbm, float jam_dbm, float noise_dbm) {
    float sig = powf(10.0f, signal_dbm / 10.0f);
    float jam = powf(10.0f, jam_dbm / 10.0f);
    float noise = powf(10.0f, noise_dbm / 10.0f);
    return 10.0f * log10f(sig / (jam + noise));
}

/*
 * deauthFrames — Deauthentication frame hisoblagich
 *   seconds — vaqt (sekund)
 *   rate — sekundiga frame tezligi
 * 
 * Return: jami frame soni
 */
EMSCRIPTEN_KEEPALIVE
int deauthFrames(float seconds, float rate) {
    return (int)(seconds * rate);
}

/*
 * generateNoise — Gauss shovqin generatsiyasi
 *   mean — o'rtacha qiymat
 *   stddev — standart og'ish
 * 
 * Return: Gauss taqsimotli tasodifiy son
 * Box-Muller transformatsiyasi asosida
 */
EMSCRIPTEN_KEEPALIVE
float generateNoise(float mean, float stddev) {
    static float z0, z1;
    static bool generate = false;
    
    if (!generate) {
        float u1 = (float)rand() / (float)RAND_MAX;
        float u2 = (float)rand() / (float)RAND_MAX;
        if (u1 < 0.0001f) u1 = 0.0001f;
        z0 = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * M_PI * u2);
        z1 = sqrtf(-2.0f * logf(u1)) * sinf(2.0f * M_PI * u2);
        generate = true;
        return mean + z0 * stddev;
    } else {
        generate = false;
        return mean + z1 * stddev;
    }
}

/*
 * calcThroughput — O'tkazish qobiliyatini hisoblash
 *   sinr — Signal-to-Interference+Noise Ratio (dB)
 *   bw — kanal kengligi (MHz, default 20)
 * 
 * Return: throughput (Mbps)
 * 802.11ac/g modulyatsiyasi asosida
 */
EMSCRIPTEN_KEEPALIVE
float calcThroughput(float sinr, float bw) {
    if (sinr < -5.0f) return 0.0f;
    if (sinr < 5.0f)  return bw * 0.3f * (sinr + 5.0f) / 10.0f;
    if (sinr < 15.0f) return bw * 0.3f + bw * 0.5f * (sinr - 5.0f) / 10.0f;
    if (sinr < 25.0f) return bw * 0.8f + bw * 0.2f * (sinr - 15.0f) / 10.0f;
    return bw;
}

/*
 * 802.11 Frame tuzilishi (ma'lumot uchun)
 * =======================================
 * struct DeauthFrame {
 *     uint8_t frame_control[2] = {0xC0, 0x00};  // Type: Management, Subtype: Deauth
 *     uint8_t duration[2];                        // Duration/ID
 *     uint8_t dst_addr[6];                        // Destination MAC (broadcast: FF:FF:FF:FF:FF:FF)
 *     uint8_t src_addr[6];                        // Source MAC
 *     uint8_t bssid[6];                           // BSSID
 *     uint8_t seq_num[2];                         // Sequence number
 *     uint8_t reason_code[2] = {0x07, 0x00};      // Reason: Class 3 frame from nonassociated STA
 *     // Frame Check Sequence (FCS) — hardware tomonidan qo'shiladi
 * };
 * 
 * 802.11 kanallari (2.4 GHz):
 *   CH1: 2412 MHz   CH2: 2417 MHz   CH3: 2422 MHz
 *   CH4: 2427 MHz   CH5: 2432 MHz   CH6: 2437 MHz
 *   CH7: 2442 MHz   CH8: 2447 MHz   CH9: 2452 MHz
 *   CH10: 2457 MHz  CH11: 2462 MHz  CH12: 2467 MHz
 *   CH13: 2472 MHz
 */

} // extern "C"
