#ifndef UTILS_HH
#define UTILS_HH

#include <string>
#include <vector>
#include <gmp.h>
#include <gmpxx.h>

const char HEX_ALPHABET[] = "0123456789abcdef";

uint8_t hexValue(char c);
std::string hashSha512(const std::string& s);
std::string hashShake256(const std::string& s, size_t len = 0);

uint64_t timeBasedSeed();

// This number is only for the primes generated with initPrimes().
// Additional primes are added as hardcoded numbers.
const size_t N_SAFEPRIMES = 256;

// Sophie-Germain primes
inline mpz_t SG_PRIMES[N_SAFEPRIMES + 5];

// Safe primes
inline mpz_t SAFE_PRIMES[N_SAFEPRIMES + 5];

void initPrimes();
mpz_class getSafePrime(size_t i);
// Binary representation of n.
// Note that the in the resulting vector,
// LSB is at index 0, and MSB is at index length - 1,
// hence a reverted representation.
std::vector<bool> toBinary(unsigned n, unsigned length);

// Copy from https://stackoverflow.com/a/17299623/15279018.
template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>>& v) {
  std::size_t total_size = 0;
  for (const auto& sub : v)
    total_size += sub.size();
  std::vector<T> result;
  result.reserve(total_size);
  for (const auto& sub : v)
    result.insert(result.end(), sub.begin(), sub.end());
  return result;
}

// The following numbers are taken from RFC 2409:
// https://www.ietf.org/rfc/rfc2409.txt

// The prime is: 2^768 - 2^704 - 1 + 2^64 * { [2^638 pi] + 149686 }
const char MODP_768_PRIME[] =
  "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1"
  "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD"
  "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245"
  "E485B576 625E7EC6 F44C42E9 A63A3620 FFFFFFFF FFFFFFFF";

// The prime is 2^1024 - 2^960 - 1 + 2^64 * { [2^894 pi] + 129093 }.
const char MODP_1024_PRIME[] =
  "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1"
  "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD"
  "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245"
  "E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED"
  "EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE65381"
  "FFFFFFFF FFFFFFFF";


// The following numbers are taken from RFC 3526:
// https://www.ietf.org/rfc/rfc3526.txt

// The prime is: 2^1536 - 2^1472 - 1 + 2^64 * { [2^1406 pi] + 741804 }
const char MODP_1536_PRIME[] =
  "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1"
  "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD"
  "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245"
  "E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED"
  "EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE45B3D"
  "C2007CB8 A163BF05 98DA4836 1C55D39A 69163FA8 FD24CF5F"
  "83655D23 DCA3AD96 1C62F356 208552BB 9ED52907 7096966D"
  "670C354E 4ABC9804 F1746C08 CA237327 FFFFFFFF FFFFFFFF";

// This prime is: 2^2048 - 2^1984 - 1 + 2^64 * { [2^1918 pi] + 124476 }
const char MODP_2048_PRIME[] =
  "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1"
  "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD"
  "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245"
  "E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED"
  "EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE45B3D"
  "C2007CB8 A163BF05 98DA4836 1C55D39A 69163FA8 FD24CF5F"
  "83655D23 DCA3AD96 1C62F356 208552BB 9ED52907 7096966D"
  "670C354E 4ABC9804 F1746C08 CA18217C 32905E46 2E36CE3B"
  "E39E772C 180E8603 9B2783A2 EC07A28F B5C55DF0 6F4C52C9"
  "DE2BCBF6 95581718 3995497C EA956AE5 15D22618 98FA0510"
  "15728E5A 8AACAA68 FFFFFFFF FFFFFFFF";

// This prime is: 2^3072 - 2^3008 - 1 + 2^64 * { [2^2942 pi] + 1690314 }
const char MODP_3072_PRIME[] =
  "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1"
  "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD"
  "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245"
  "E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED"
  "EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE45B3D"
  "C2007CB8 A163BF05 98DA4836 1C55D39A 69163FA8 FD24CF5F"
  "83655D23 DCA3AD96 1C62F356 208552BB 9ED52907 7096966D"
  "670C354E 4ABC9804 F1746C08 CA18217C 32905E46 2E36CE3B"
  "E39E772C 180E8603 9B2783A2 EC07A28F B5C55DF0 6F4C52C9"
  "DE2BCBF6 95581718 3995497C EA956AE5 15D22618 98FA0510"
  "15728E5A 8AAAC42D AD33170D 04507A33 A85521AB DF1CBA64"
  "ECFB8504 58DBEF0A 8AEA7157 5D060C7D B3970F85 A6E1E4C7"
  "ABF5AE8C DB0933D7 1E8C94E0 4A25619D CEE3D226 1AD2EE6B"
  "F12FFA06 D98A0864 D8760273 3EC86A64 521F2B18 177B200C"
  "BBE11757 7A615D6C 770988C0 BAD946E2 08E24FA0 74E5AB31"
  "43DB5BFC E0FD108E 4B82D120 A93AD2CA FFFFFFFF FFFFFFFF";

// This prime is: 2^4096 - 2^4032 - 1 + 2^64 * { [2^3966 pi] + 240904 }
const char MODP_4096_PRIME[] =
  "FFFFFFFF FFFFFFFF C90FDAA2 2168C234 C4C6628B 80DC1CD1"
  "29024E08 8A67CC74 020BBEA6 3B139B22 514A0879 8E3404DD"
  "EF9519B3 CD3A431B 302B0A6D F25F1437 4FE1356D 6D51C245"
  "E485B576 625E7EC6 F44C42E9 A637ED6B 0BFF5CB6 F406B7ED"
  "EE386BFB 5A899FA5 AE9F2411 7C4B1FE6 49286651 ECE45B3D"
  "C2007CB8 A163BF05 98DA4836 1C55D39A 69163FA8 FD24CF5F"
  "83655D23 DCA3AD96 1C62F356 208552BB 9ED52907 7096966D"
  "670C354E 4ABC9804 F1746C08 CA18217C 32905E46 2E36CE3B"
  "E39E772C 180E8603 9B2783A2 EC07A28F B5C55DF0 6F4C52C9"
  "DE2BCBF6 95581718 3995497C EA956AE5 15D22618 98FA0510"
  "15728E5A 8AAAC42D AD33170D 04507A33 A85521AB DF1CBA64"
  "ECFB8504 58DBEF0A 8AEA7157 5D060C7D B3970F85 A6E1E4C7"
  "ABF5AE8C DB0933D7 1E8C94E0 4A25619D CEE3D226 1AD2EE6B"
  "F12FFA06 D98A0864 D8760273 3EC86A64 521F2B18 177B200C"
  "BBE11757 7A615D6C 770988C0 BAD946E2 08E24FA0 74E5AB31"
  "43DB5BFC E0FD108E 4B82D120 A9210801 1A723C12 A787E6D7"
  "88719A10 BDBA5B26 99C32718 6AF4E23C 1A946834 B6150BDA"
  "2583E9CA 2AD44CE8 DBBBC2DB 04DE8EF9 2E8EFC14 1FBECAA6"
  "287C5947 4E6BC05D 99B2964F A090C3A2 233BA186 515BE7ED"
  "1F612970 CEE2D7AF B81BDD76 2170481C D0069127 D5B05AA9"
  "93B4EA98 8D8FDDC1 86FFB7DC 90A6C08F 4DF435C9 34063199"
  "FFFFFFFF FFFFFFFF";

#endif
