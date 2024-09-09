#ifndef UTILS_HH
#define UTILS_HH

#include <string>


const char HEX_ALPHABET[] = "0123456789abcdef";

uint8_t hexValue(char c);
std::string hashSha512(const std::string& s);

uint64_t timeBasedSeed();

#endif
