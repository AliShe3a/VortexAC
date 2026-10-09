#pragma once
#include <windows.h>

class CRC32 {
private:
    static unsigned long* GetTable() {
        static unsigned long table[256];
        static bool initialized = false;
        if (!initialized) {
            unsigned long polynomial = 0xEDB88320;
            for (int i = 0; i < 256; i++) {
                unsigned long entry = (unsigned long)i;
                for (int j = 0; j < 8; j++) {
                    if (entry & 1) entry = (entry >> 1) ^ polynomial;
                    else entry >>= 1;
                }
                table[i] = entry;
            }
            initialized = true;
        }
        return table;
    }

public:
    static unsigned long Compute(unsigned char* buffer, size_t len) {
        unsigned long* table = GetTable();
        unsigned long hash = 0xFFFFFFFF;
        for (size_t i = 0; i < len; i++) {
            hash = (hash >> 8) ^ table[(buffer[i] ^ hash) & 0xFF];
        }
        return ~hash;
    }

    static unsigned long Compute(unsigned long polynomial, unsigned long seed, unsigned char* buffer, size_t len) {
        return Compute(buffer, len);
    }
};