/*
* Botan 3.0.0-alpha0 Amalgamation
* (C) 1999-2020 The Botan Authors
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include "botan_all.h"

#include <chrono>
#include <functional>
#include <iosfwd>
#include <istream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <vector>


namespace Botan {

/**
* If top bit of arg is set, return ~0. Otherwise return 0.
*/
template<typename T>
inline constexpr T expand_top_bit(T a)
   {
   return static_cast<T>(0) - (a >> (sizeof(T)*8-1));
   }

/**
* If arg is zero, return ~0. Otherwise return 0
*/
template<typename T>
inline constexpr T ct_is_zero(T x)
   {
   return expand_top_bit<T>(~x & (x - 1));
   }

/**
* Power of 2 test. T should be an unsigned integer type
* @param arg an integer value
* @return true iff arg is 2^n for some n > 0
*/
template<typename T>
inline constexpr bool is_power_of_2(T arg)
   {
   return (arg != 0) && (arg != 1) && ((arg & static_cast<T>(arg-1)) == 0);
   }

/**
* Return the index of the highest set bit
* T is an unsigned integer type
* @param n an integer value
* @return index of the highest set bit in n
*/
template<typename T>
inline constexpr size_t high_bit(T n)
   {
   size_t hb = 0;

   for(size_t s = 8*sizeof(T) / 2; s > 0; s /= 2)
      {
      const size_t z = s * ((~ct_is_zero(n >> s)) & 1);
      hb += z;
      n >>= z;
      }

   hb += n;

   return hb;
   }

/**
* Return the number of significant bytes in n
* @param n an integer value
* @return number of significant bytes in n
*/
template<typename T>
inline constexpr size_t significant_bytes(T n)
   {
   size_t b = 0;

   for(size_t s = 8*sizeof(n) / 2; s >= 8; s /= 2)
      {
      const size_t z = s * (~ct_is_zero(n >> s) & 1);
      b += z/8;
      n >>= z;
      }

   b += (n != 0);

   return b;
   }

/**
* Count the trailing zero bits in n
* @param n an integer value
* @return maximum x st 2^x divides n
*/
template<typename T>
inline constexpr size_t ctz(T n)
   {
   /*
   * If n == 0 then this function will compute 8*sizeof(T)-1, so
   * initialize lb to 1 if n == 0 to produce the expected result.
   */
   size_t lb = ct_is_zero(n) & 1;

   for(size_t s = 8*sizeof(T) / 2; s > 0; s /= 2)
      {
      const T mask = (static_cast<T>(1) << s) - 1;
      const size_t z = s * (ct_is_zero(n & mask) & 1);
      lb += z;
      n >>= z;
      }

   return lb;
   }

template<typename T>
constexpr uint8_t ceil_log2(T x)
   {
   static_assert(sizeof(T) < 32, "Abnormally large scalar");

   if(x >> (sizeof(T)*8-1))
      return sizeof(T)*8;

   uint8_t result = 0;
   T compare = 1;

   while(compare < x)
      {
      compare <<= 1;
      result++;
      }

   return result;
   }

// Potentially variable time ctz used for OCB
inline constexpr size_t var_ctz32(uint32_t n)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)
   if(n == 0)
      return 32;
   return __builtin_ctz(n);
#else
   return ctz<uint32_t>(n);
#endif
   }

template<typename T>
inline constexpr T bit_permute_step(T x, T mask, size_t shift)
   {
   /*
   See https://reflectionsonsecurity.wordpress.com/2014/05/11/efficient-bit-permutation-using-delta-swaps/
   and http://programming.sirrida.de/bit_perm.html
   */
   const T swap = ((x >> shift) ^ x) & mask;
   return (x ^ swap) ^ (swap << shift);
   }

template<typename T>
inline constexpr void swap_bits(T& x, T& y, T mask, size_t shift)
   {
   const T swap = ((x >> shift) ^ y) & mask;
   x ^= swap << shift;
   y ^= swap;
   }

template<typename T>
inline constexpr T choose(T mask, T a, T b)
   {
   //return (mask & a) | (~mask & b);
   return (b ^ (mask & (a ^ b)));
   }

template<typename T>
inline constexpr T majority(T a, T b, T c)
   {
   /*
   Considering each bit of a, b, c individually

   If a xor b is set, then c is the deciding vote.

   If a xor b is not set then either a and b are both set or both unset.
   In either case the value of c doesn't matter, and examining b (or a)
   allows us to determine which case we are in.
   */
   return choose(a ^ b, c, b);
   }

}

namespace Botan {

class BLAKE2bMAC;

/**
* BLAKE2B
*/
class BLAKE2b final : public HashFunction, public SymmetricAlgorithm
   {
   public:
      /**
      * @param output_bits the output size of BLAKE2b in bits
      */
      explicit BLAKE2b(size_t output_bits = 512);

      size_t hash_block_size() const override { return 128; }
      size_t output_length() const override { return m_output_bits / 8; }
      size_t key_size() const { return m_key_size; }

      Key_Length_Specification key_spec() const override;

      std::unique_ptr<HashFunction> new_object() const override;
      std::string name() const override;
      void clear() override;

      std::unique_ptr<HashFunction> copy_state() const override;

   protected:
      friend class BLAKE2bMAC;

      void key_schedule(const uint8_t key[], size_t length) override;

      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t out[]) override;

   private:
      void state_init();
      void compress(const uint8_t* data, size_t blocks, uint64_t increment);

      const size_t m_output_bits;

      secure_vector<uint8_t> m_buffer;
      size_t m_bufpos;

      secure_vector<uint64_t> m_H;
      uint64_t m_T[2];
      uint64_t m_F[2];

      size_t m_key_size;
      secure_vector<uint8_t> m_padded_key_buffer;
   };

typedef BLAKE2b Blake2b;

}

namespace Botan {

/**
* Swap a 16 bit integer
*/
inline constexpr uint16_t reverse_bytes(uint16_t x)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG) || defined(BOTAN_BUILD_COMPILER_IS_XLC)
   return __builtin_bswap16(x);
#else
   return static_cast<uint16_t>((x << 8) | (x >> 8));
#endif
   }

/**
* Swap a 32 bit integer
*
* We cannot use MSVC's _byteswap_ulong because it does not consider
* the builtin to be constexpr.
*/
inline constexpr uint32_t reverse_bytes(uint32_t x)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG) || defined(BOTAN_BUILD_COMPILER_IS_XLC)
   return __builtin_bswap32(x);
#else
   // MSVC at least recognizes this as a bswap
   return ((x & 0x000000FF) << 24) |
          ((x & 0x0000FF00) <<  8) |
          ((x & 0x00FF0000) >>  8) |
          ((x & 0xFF000000) >> 24);
#endif
   }

/**
* Swap a 64 bit integer
*
* We cannot use MSVC's _byteswap_uint64 because it does not consider
* the builtin to be constexpr.
*/
inline constexpr uint64_t reverse_bytes(uint64_t x)
   {
#if defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG) || defined(BOTAN_BUILD_COMPILER_IS_XLC)
   return __builtin_bswap64(x);
#else
   uint32_t hi = static_cast<uint32_t>(x >> 32);
   uint32_t lo = static_cast<uint32_t>(x);

   hi = reverse_bytes(hi);
   lo = reverse_bytes(lo);

   return (static_cast<uint64_t>(lo) << 32) | hi;
#endif
   }

/**
* Swap 4 Ts in an array
*/
template<typename T>
inline constexpr void bswap_4(T x[4])
   {
   x[0] = reverse_bytes(x[0]);
   x[1] = reverse_bytes(x[1]);
   x[2] = reverse_bytes(x[2]);
   x[3] = reverse_bytes(x[3]);
   }

}

namespace Botan {

/**
* Struct representing a particular date and time
*/
class BOTAN_TEST_API calendar_point
   {
   public:

      /** The year */
      uint32_t year() const { return m_year; }

      /** The month, 1 through 12 for Jan to Dec */
      uint32_t month() const { return m_month; }

      /** The day of the month, 1 through 31 (or 28 or 30 based on month */
      uint32_t day() const { return m_day; }

      /** Hour in 24-hour form, 0 to 23 */
      uint32_t hour() const { return m_hour; }

      /** Minutes in the hour, 0 to 60 */
      uint32_t minutes() const { return m_minutes; }

      /** Seconds in the minute, 0 to 60, but might be slightly
      larger to deal with leap seconds on some systems
      */
      uint32_t seconds() const { return m_seconds; }

      /**
      * Initialize a calendar_point
      * @param y the year
      * @param mon the month
      * @param d the day
      * @param h the hour
      * @param min the minute
      * @param sec the second
      */
      calendar_point(uint32_t y, uint32_t mon, uint32_t d, uint32_t h, uint32_t min, uint32_t sec) :
        m_year(y), m_month(mon), m_day(d), m_hour(h), m_minutes(min), m_seconds(sec) {}

      /**
      * Convert a time_point to a calendar_point
      * @param time_point a time point from the system clock
      */
      calendar_point(const std::chrono::system_clock::time_point& time_point);

      /**
      * Returns an STL timepoint object
      */
      std::chrono::system_clock::time_point to_std_timepoint() const;

      /**
      * Returns a human readable string of the struct's components.
      * Formatting might change over time. Currently it is RFC339 'iso-date-time'.
      */
      std::string to_string() const;

   private:
      uint32_t m_year;
      uint32_t m_month;
      uint32_t m_day;
      uint32_t m_hour;
      uint32_t m_minutes;
      uint32_t m_seconds;
   };

}

namespace Botan {

/**
* Convert a sequence of UCS-2 (big endian) characters to a UTF-8 string
* This is used for ASN.1 BMPString type
* @param ucs2 the sequence of UCS-2 characters
* @param len length of ucs2 in bytes, must be a multiple of 2
*/
BOTAN_TEST_API std::string ucs2_to_utf8(const uint8_t ucs2[], size_t len);

/**
* Convert a sequence of UCS-4 (big endian) characters to a UTF-8 string
* This is used for ASN.1 UniversalString type
* @param ucs4 the sequence of UCS-4 characters
* @param len length of ucs4 in bytes, must be a multiple of 4
*/
BOTAN_TEST_API std::string ucs4_to_utf8(const uint8_t ucs4[], size_t len);

BOTAN_TEST_API std::string latin1_to_utf8(const uint8_t latin1[], size_t len);

}

namespace Botan {

/**
* Perform encoding using the base provided
* @param base object giving access to the encodings specifications
* @param output an array of at least base.encode_max_output bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding chars will be applied if needed
* @return number of bytes written to output
*/
template <class Base>
size_t base_encode(Base&& base,
                   char output[],
                   const uint8_t input[],
                   size_t input_length,
                   size_t& input_consumed,
                   bool final_inputs)
   {
   input_consumed = 0;

   const size_t encoding_bytes_in = base.encoding_bytes_in();
   const size_t encoding_bytes_out = base.encoding_bytes_out();

   size_t input_remaining = input_length;
   size_t output_produced = 0;

   while(input_remaining >= encoding_bytes_in)
      {
      base.encode(output + output_produced, input + input_consumed);

      input_consumed += encoding_bytes_in;
      output_produced += encoding_bytes_out;
      input_remaining -= encoding_bytes_in;
      }

   if(final_inputs && input_remaining)
      {
      std::vector<uint8_t> remainder(encoding_bytes_in, 0);
      for(size_t i = 0; i != input_remaining; ++i)
         { remainder[i] = input[input_consumed + i]; }

      base.encode(output + output_produced, remainder.data());

      const size_t bits_consumed = base.bits_consumed();
      const size_t remaining_bits_before_padding = base.remaining_bits_before_padding();

      size_t empty_bits = 8 * (encoding_bytes_in - input_remaining);
      size_t index = output_produced + encoding_bytes_out - 1;
      while(empty_bits >= remaining_bits_before_padding)
         {
         output[index--] = '=';
         empty_bits -= bits_consumed;
         }

      input_consumed += input_remaining;
      output_produced += encoding_bytes_out;
      }

   return output_produced;
   }


template <typename Base>
std::string base_encode_to_string(Base&& base, const uint8_t input[], size_t input_length)
   {
   const size_t output_length = base.encode_max_output(input_length);
   std::string output(output_length, 0);

   size_t consumed = 0;
   size_t produced = 0;

   if(output_length > 0)
      {
      produced = base_encode(base, &output.front(),
                                   input, input_length,
                                   consumed, true);
      }

   BOTAN_ASSERT_EQUAL(consumed, input_length, "Consumed the entire input");
   BOTAN_ASSERT_EQUAL(produced, output.size(), "Produced expected size");

   return output;
   }

/**
* Perform decoding using the base provided
* @param base object giving access to the encodings specifications
* @param output an array of at least Base::decode_max_output bytes
* @param input some base input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding is allowed
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
template <typename Base>
size_t base_decode(Base&& base,
                   uint8_t output[],
                   const char input[],
                   size_t input_length,
                   size_t& input_consumed,
                   bool final_inputs,
                   bool ignore_ws = true)
   {
   const size_t decoding_bytes_in = base.decoding_bytes_in();
   const size_t decoding_bytes_out = base.decoding_bytes_out();

   uint8_t* out_ptr = output;
   std::vector<uint8_t> decode_buf(decoding_bytes_in, 0);
   size_t decode_buf_pos = 0;
   size_t final_truncate = 0;

   clear_mem(output, base.decode_max_output(input_length));

   for(size_t i = 0; i != input_length; ++i)
      {
      const uint8_t bin = base.lookup_binary_value(input[i]);

      if(base.check_bad_char(bin, input[i], ignore_ws)) // May throw Invalid_Argument
         {
         decode_buf[decode_buf_pos] = bin;
         ++decode_buf_pos;
         }

      /*
      * If we're at the end of the input, pad with 0s and truncate
      */
      if(final_inputs && (i == input_length - 1))
         {
         if(decode_buf_pos)
            {
            for(size_t j = decode_buf_pos; j < decoding_bytes_in; ++j)
               { decode_buf[j] = 0; }

            final_truncate = decoding_bytes_in - decode_buf_pos;
            decode_buf_pos = decoding_bytes_in;
            }
         }

      if(decode_buf_pos == decoding_bytes_in)
         {
         base.decode(out_ptr, decode_buf.data());

         out_ptr += decoding_bytes_out;
         decode_buf_pos = 0;
         input_consumed = i+1;
         }
      }

   while(input_consumed < input_length &&
         base.lookup_binary_value(input[input_consumed]) == 0x80)
      {
      ++input_consumed;
      }

   size_t written = (out_ptr - output) - base.bytes_to_remove(final_truncate);

   return written;
   }

template<typename Base>
size_t base_decode_full(Base&& base, uint8_t output[], const char input[], size_t input_length, bool ignore_ws)
   {
   size_t consumed = 0;
   const size_t written = base_decode(base, output, input, input_length, consumed, true, ignore_ws);

   if(consumed != input_length)
      {
      throw Invalid_Argument(base.name() + " decoding failed, input did not have full bytes");
      }

   return written;
   }

template<typename Vector, typename Base>
Vector base_decode_to_vec(Base&& base,
                          const char input[],
                          size_t input_length,
                          bool ignore_ws)
   {
   const size_t output_length = base.decode_max_output(input_length);
   Vector bin(output_length);

   const size_t written =
      base_decode_full(base, bin.data(), input, input_length, ignore_ws);

   bin.resize(written);
   return bin;
   }

}

namespace Botan {

/**
* A class handling runtime CPU feature detection. It is limited to
* just the features necessary to implement CPU specific code in Botan,
* rather than being a general purpose utility.
*
* This class supports:
*
*  - x86 features using CPUID. x86 is also the only processor with
*    accurate cache line detection currently.
*
*  - PowerPC AltiVec detection on Linux, NetBSD, OpenBSD, and macOS
*
*  - ARM NEON and crypto extensions detection. On Linux and Android
*    systems which support getauxval, that is used to access CPU
*    feature information. Otherwise a relatively portable but
*    thread-unsafe mechanism involving executing probe functions which
*    catching SIGILL signal is used.
*/
class BOTAN_TEST_API CPUID final
   {
   public:
      /**
      * Probe the CPU and see what extensions are supported
      */
      static void initialize();

      static bool has_simd_32();

      /**
      * Return a possibly empty string containing list of known CPU
      * extensions. Each name will be seperated by a space, and the ordering
      * will be arbitrary. This list only contains values that are useful to
      * Botan (for example FMA instructions are not checked).
      *
      * Example outputs "sse2 ssse3 rdtsc", "neon arm_aes", "altivec"
      */
      static std::string to_string();

      /**
      * Return a best guess of the cache line size
      */
      static size_t cache_line_size()
         {
         return state().cache_line_size();
         }

      static bool is_little_endian()
         {
#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
         return true;
#elif defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
         return false;
#else
         return state().endian_status() == Endian_Status::Little;
#endif
         }

      static bool is_big_endian()
         {
#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
         return true;
#elif defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
         return false;
#else
         return state().endian_status() == Endian_Status::Big;
#endif
         }

      enum CPUID_bits : uint64_t {
#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
         // These values have no relation to cpuid bitfields

         // SIMD instruction sets
         CPUID_SSE2_BIT       = (1ULL << 0),
         CPUID_SSSE3_BIT      = (1ULL << 1),
         CPUID_SSE41_BIT      = (1ULL << 2),
         CPUID_SSE42_BIT      = (1ULL << 3),
         CPUID_AVX2_BIT       = (1ULL << 4),
         CPUID_AVX512F_BIT    = (1ULL << 5),

         CPUID_AVX512DQ_BIT   = (1ULL << 6),
         CPUID_AVX512BW_BIT   = (1ULL << 7),

         // Ice Lake profile: AVX-512 F, DQ, BW, VL, IFMA, VBMI, VBMI2, BITALG
         CPUID_AVX512_ICL_BIT = (1ULL << 11),

         // Crypto-specific ISAs
         CPUID_AESNI_BIT        = (1ULL << 16),
         CPUID_CLMUL_BIT        = (1ULL << 17),
         CPUID_RDRAND_BIT       = (1ULL << 18),
         CPUID_RDSEED_BIT       = (1ULL << 19),
         CPUID_SHA_BIT          = (1ULL << 20),
         CPUID_AVX512_AES_BIT   = (1ULL << 21),
         CPUID_AVX512_CLMUL_BIT = (1ULL << 22),

         // Misc useful instructions
         CPUID_RDTSC_BIT      = (1ULL << 48),
         CPUID_ADX_BIT        = (1ULL << 49),
         CPUID_BMI1_BIT       = (1ULL << 50),
         CPUID_BMI2_BIT       = (1ULL << 51),
         CPUID_FAST_PDEP_BIT  = (1ULL << 52),
#endif

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
         CPUID_ALTIVEC_BIT    = (1ULL << 0),
         CPUID_POWER_CRYPTO_BIT = (1ULL << 1),
         CPUID_DARN_BIT       = (1ULL << 2),
#endif

#if defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
         CPUID_ARM_NEON_BIT      = (1ULL << 0),
         CPUID_ARM_SVE_BIT       = (1ULL << 1),
         CPUID_ARM_AES_BIT       = (1ULL << 16),
         CPUID_ARM_PMULL_BIT     = (1ULL << 17),
         CPUID_ARM_SHA1_BIT      = (1ULL << 18),
         CPUID_ARM_SHA2_BIT      = (1ULL << 19),
         CPUID_ARM_SHA3_BIT      = (1ULL << 20),
         CPUID_ARM_SHA2_512_BIT  = (1ULL << 21),
         CPUID_ARM_SM3_BIT       = (1ULL << 22),
         CPUID_ARM_SM4_BIT       = (1ULL << 23),
#endif

         CPUID_INITIALIZED_BIT = (1ULL << 63)
      };

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
      /**
      * Check if the processor supports AltiVec/VMX
      */
      static bool has_altivec()
         { return has_cpuid_bit(CPUID_ALTIVEC_BIT); }

      /**
      * Check if the processor supports POWER8 crypto extensions
      */
      static bool has_power_crypto()
         { return has_cpuid_bit(CPUID_POWER_CRYPTO_BIT); }

      /**
      * Check if the processor supports POWER9 DARN RNG
      */
      static bool has_darn_rng()
         { return has_cpuid_bit(CPUID_DARN_BIT); }

#endif

#if defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
      /**
      * Check if the processor supports NEON SIMD
      */
      static bool has_neon()
         { return has_cpuid_bit(CPUID_ARM_NEON_BIT); }

      /**
      * Check if the processor supports ARMv8 SVE
      */
      static bool has_arm_sve()
         { return has_cpuid_bit(CPUID_ARM_SVE_BIT); }

      /**
      * Check if the processor supports ARMv8 SHA1
      */
      static bool has_arm_sha1()
         { return has_cpuid_bit(CPUID_ARM_SHA1_BIT); }

      /**
      * Check if the processor supports ARMv8 SHA2
      */
      static bool has_arm_sha2()
         { return has_cpuid_bit(CPUID_ARM_SHA2_BIT); }

      /**
      * Check if the processor supports ARMv8 AES
      */
      static bool has_arm_aes()
         { return has_cpuid_bit(CPUID_ARM_AES_BIT); }

      /**
      * Check if the processor supports ARMv8 PMULL
      */
      static bool has_arm_pmull()
         { return has_cpuid_bit(CPUID_ARM_PMULL_BIT); }

      /**
      * Check if the processor supports ARMv8 SHA-512
      */
      static bool has_arm_sha2_512()
         { return has_cpuid_bit(CPUID_ARM_SHA2_512_BIT); }

      /**
      * Check if the processor supports ARMv8 SHA-3
      */
      static bool has_arm_sha3()
         { return has_cpuid_bit(CPUID_ARM_SHA3_BIT); }

      /**
      * Check if the processor supports ARMv8 SM3
      */
      static bool has_arm_sm3()
         { return has_cpuid_bit(CPUID_ARM_SM3_BIT); }

      /**
      * Check if the processor supports ARMv8 SM4
      */
      static bool has_arm_sm4()
         { return has_cpuid_bit(CPUID_ARM_SM4_BIT); }

#endif

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

      /**
      * Check if the processor supports RDTSC
      */
      static bool has_rdtsc()
         { return has_cpuid_bit(CPUID_RDTSC_BIT); }

      /**
      * Check if the processor supports SSE2
      */
      static bool has_sse2()
         { return has_cpuid_bit(CPUID_SSE2_BIT); }

      /**
      * Check if the processor supports SSSE3
      */
      static bool has_ssse3()
         { return has_cpuid_bit(CPUID_SSSE3_BIT); }

      /**
      * Check if the processor supports SSE4.1
      */
      static bool has_sse41()
         { return has_cpuid_bit(CPUID_SSE41_BIT); }

      /**
      * Check if the processor supports SSE4.2
      */
      static bool has_sse42()
         { return has_cpuid_bit(CPUID_SSE42_BIT); }

      /**
      * Check if the processor supports AVX2
      */
      static bool has_avx2()
         { return has_cpuid_bit(CPUID_AVX2_BIT); }

      /**
      * Check if the processor supports AVX-512F
      */
      static bool has_avx512f()
         { return has_cpuid_bit(CPUID_AVX512F_BIT); }

      /**
      * Check if the processor supports AVX-512DQ
      */
      static bool has_avx512dq()
         { return has_cpuid_bit(CPUID_AVX512DQ_BIT); }

      /**
      * Check if the processor supports AVX-512BW
      */
      static bool has_avx512bw()
         { return has_cpuid_bit(CPUID_AVX512BW_BIT); }

      /**
      * Check if the processor supports AVX-512 Ice Lake profile
      */
      static bool has_avx512_icelake()
         { return has_cpuid_bit(CPUID_AVX512_ICL_BIT); }

      /**
      * Check if the processor supports AVX-512 AES (VAES)
      */
      static bool has_avx512_aes()
         { return has_cpuid_bit(CPUID_AVX512_AES_BIT); }

      /**
      * Check if the processor supports AVX-512 VPCLMULQDQ
      */
      static bool has_avx512_clmul()
         { return has_cpuid_bit(CPUID_AVX512_CLMUL_BIT); }

      /**
      * Check if the processor supports BMI1
      */
      static bool has_bmi1()
         { return has_cpuid_bit(CPUID_BMI1_BIT); }

      /**
      * Check if the processor supports BMI2
      */
      static bool has_bmi2()
         { return has_cpuid_bit(CPUID_BMI2_BIT); }

      /**
      * Check if the processor supports fast PDEP/PEXT from BMI2
      */
      static bool has_fast_pdep()
         { return has_cpuid_bit(CPUID_FAST_PDEP_BIT); }

      /**
      * Check if the processor supports AES-NI
      */
      static bool has_aes_ni()
         { return has_cpuid_bit(CPUID_AESNI_BIT); }

      /**
      * Check if the processor supports CLMUL
      */
      static bool has_clmul()
         { return has_cpuid_bit(CPUID_CLMUL_BIT); }

      /**
      * Check if the processor supports Intel SHA extension
      */
      static bool has_intel_sha()
         { return has_cpuid_bit(CPUID_SHA_BIT); }

      /**
      * Check if the processor supports ADX extension
      */
      static bool has_adx()
         { return has_cpuid_bit(CPUID_ADX_BIT); }

      /**
      * Check if the processor supports RDRAND
      */
      static bool has_rdrand()
         { return has_cpuid_bit(CPUID_RDRAND_BIT); }

      /**
      * Check if the processor supports RDSEED
      */
      static bool has_rdseed()
         { return has_cpuid_bit(CPUID_RDSEED_BIT); }
#endif

      /**
      * Check if the processor supports byte-level vector permutes
      * (SSSE3, NEON, Altivec)
      */
      static bool has_vperm()
         {
#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
         return has_ssse3();
#elif defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
         return has_neon();
#elif defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
         return has_altivec();
#else
         return false;
#endif
         }

      /**
      * Check if the processor supports hardware AES instructions
      */
      static bool has_hw_aes()
         {
#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
         return has_aes_ni();
#elif defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
         return has_arm_aes();
#elif defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
         return has_power_crypto();
#else
         return false;
#endif
         }

      /**
      * Check if the processor supports carryless multiply
      * (CLMUL, PMULL)
      */
      static bool has_carryless_multiply()
         {
#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
         return has_clmul();
#elif defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
         return has_arm_pmull();
#elif defined(BOTAN_TARGET_ARCH_IS_PPC64)
         return has_power_crypto();
#else
         return false;
#endif
         }

      /*
      * Clear a CPUID bit
      * Call CPUID::initialize to reset
      *
      * This is only exposed for testing, don't use unless you know
      * what you are doing.
      */
      static void clear_cpuid_bit(CPUID_bits bit)
         {
         state().clear_cpuid_bit(static_cast<uint64_t>(bit));
         }

      /*
      * Don't call this function, use CPUID::has_xxx above
      * It is only exposed for the tests.
      */
      static bool has_cpuid_bit(CPUID_bits elem)
         {
         const uint64_t elem64 = static_cast<uint64_t>(elem);
         return state().has_bit(elem64);
         }

      static std::vector<CPUID::CPUID_bits> bit_from_string(const std::string& tok);
   private:
      enum class Endian_Status : uint32_t {
         Unknown = 0x00000000,
         Big     = 0x01234567,
         Little  = 0x67452301,
      };

      struct CPUID_Data
         {
         public:
            CPUID_Data();

            CPUID_Data(const CPUID_Data& other) = default;
            CPUID_Data& operator=(const CPUID_Data& other) = default;

            void clear_cpuid_bit(uint64_t bit)
               {
               m_processor_features &= ~bit;
               }

            bool has_bit(uint64_t bit) const
               {
               return (m_processor_features & bit) == bit;
               }

            uint64_t processor_features() const { return m_processor_features; }
            Endian_Status endian_status() const { return m_endian_status; }
            size_t cache_line_size() const { return m_cache_line_size; }

         private:
            static Endian_Status runtime_check_endian();

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY) || \
    defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY) || \
    defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

            static uint64_t detect_cpu_features(size_t* cache_line_size);

#endif
            uint64_t m_processor_features;
            size_t m_cache_line_size;
            Endian_Status m_endian_status;
         };

      static CPUID_Data& state()
         {
         static CPUID::CPUID_Data g_cpuid;
         return g_cpuid;
         }
   };

}

#if defined(BOTAN_HAS_VALGRIND)
  #include <valgrind/memcheck.h>
#endif

namespace Botan {

namespace CT {

/**
* Use valgrind to mark the contents of memory as being undefined.
* Valgrind will accept operations which manipulate undefined values,
* but will warn if an undefined value is used to decided a conditional
* jump or a load/store address. So if we poison all of our inputs we
* can confirm that the operations in question are truly const time
* when compiled by whatever compiler is in use.
*
* Even better, the VALGRIND_MAKE_MEM_* macros work even when the
* program is not run under valgrind (though with a few cycles of
* overhead, which is unfortunate in final binaries as these
* annotations tend to be used in fairly important loops).
*
* This approach was first used in ctgrind (https://github.com/agl/ctgrind)
* but calling the valgrind mecheck API directly works just as well and
* doesn't require a custom patched valgrind.
*/
template<typename T>
inline void poison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_UNDEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p, n);
#endif
   }

template<typename T>
inline void unpoison(const T* p, size_t n)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(p, n * sizeof(T));
#else
   BOTAN_UNUSED(p, n);
#endif
   }

template<typename T>
inline void unpoison(T& p)
   {
#if defined(BOTAN_HAS_VALGRIND)
   VALGRIND_MAKE_MEM_DEFINED(&p, sizeof(T));
#else
   BOTAN_UNUSED(p);
#endif
   }

/**
* A Mask type used for constant-time operations. A Mask<T> always has value
* either 0 (all bits cleared) or ~0 (all bits set). All operations in a Mask<T>
* are intended to compile to code which does not contain conditional jumps.
* This must be verified with tooling (eg binary disassembly or using valgrind)
* since you never know what a compiler might do.
*/
template<typename T>
class Mask
   {
   public:
      static_assert(std::is_unsigned<T>::value, "CT::Mask only defined for unsigned integer types");

      Mask(const Mask<T>& other) = default;
      Mask<T>& operator=(const Mask<T>& other) = default;

      /**
      * Derive a Mask from a Mask of a larger type
      */
      template<typename U>
      Mask(Mask<U> o) : m_mask(static_cast<T>(o.value()))
         {
         static_assert(sizeof(U) > sizeof(T), "sizes ok");
         }

      /**
      * Return a Mask<T> with all bits set
      */
      static Mask<T> set()
         {
         return Mask<T>(static_cast<T>(~0));
         }

      /**
      * Return a Mask<T> with all bits cleared
      */
      static Mask<T> cleared()
         {
         return Mask<T>(0);
         }

      /**
      * Return a Mask<T> which is set if v is != 0
      */
      static Mask<T> expand(T v)
         {
         return ~Mask<T>::is_zero(v);
         }

      /**
      * Return a Mask<T> which is set if m is set
      */
      template<typename U>
      static Mask<T> expand(Mask<U> m)
         {
         static_assert(sizeof(U) < sizeof(T), "sizes ok");
         return ~Mask<T>::is_zero(m.value());
         }

      /**
      * Return a Mask<T> which is set if v is == 0 or cleared otherwise
      */
      static Mask<T> is_zero(T x)
         {
         return Mask<T>(ct_is_zero<T>(x));
         }

      /**
      * Return a Mask<T> which is set if x == y
      */
      static Mask<T> is_equal(T x, T y)
         {
         return Mask<T>::is_zero(static_cast<T>(x ^ y));
         }

      /**
      * Return a Mask<T> which is set if x < y
      */
      static Mask<T> is_lt(T x, T y)
         {
         return Mask<T>(expand_top_bit<T>(x^((x^y) | ((x-y)^x))));
         }

      /**
      * Return a Mask<T> which is set if x > y
      */
      static Mask<T> is_gt(T x, T y)
         {
         return Mask<T>::is_lt(y, x);
         }

      /**
      * Return a Mask<T> which is set if x <= y
      */
      static Mask<T> is_lte(T x, T y)
         {
         return ~Mask<T>::is_gt(x, y);
         }

      /**
      * Return a Mask<T> which is set if x >= y
      */
      static Mask<T> is_gte(T x, T y)
         {
         return ~Mask<T>::is_lt(x, y);
         }

      static Mask<T> is_within_range(T v, T l, T u)
         {
         //return Mask<T>::is_gte(v, l) & Mask<T>::is_lte(v, u);

         const T v_lt_l = v^((v^l) | ((v-l)^v));
         const T v_gt_u = u^((u^v) | ((u-v)^u));
         const T either = v_lt_l | v_gt_u;
         return ~Mask<T>(expand_top_bit(either));
         }

      static Mask<T> is_any_of(T v, std::initializer_list<T> accepted)
         {
         T accept = 0;

         for(auto a: accepted)
            {
            const T diff = a ^ v;
            const T eq_zero = ~diff & (diff - 1);
            accept |= eq_zero;
            }

         return Mask<T>(expand_top_bit(accept));
         }

      /**
      * AND-combine two masks
      */
      Mask<T>& operator&=(Mask<T> o)
         {
         m_mask &= o.value();
         return (*this);
         }

      /**
      * XOR-combine two masks
      */
      Mask<T>& operator^=(Mask<T> o)
         {
         m_mask ^= o.value();
         return (*this);
         }

      /**
      * OR-combine two masks
      */
      Mask<T>& operator|=(Mask<T> o)
         {
         m_mask |= o.value();
         return (*this);
         }

      /**
      * AND-combine two masks
      */
      friend Mask<T> operator&(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() & y.value());
         }

      /**
      * XOR-combine two masks
      */
      friend Mask<T> operator^(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() ^ y.value());
         }

      /**
      * OR-combine two masks
      */
      friend Mask<T> operator|(Mask<T> x, Mask<T> y)
         {
         return Mask<T>(x.value() | y.value());
         }

      /**
      * Negate this mask
      */
      Mask<T> operator~() const
         {
         return Mask<T>(~value());
         }

      /**
      * Return x if the mask is set, or otherwise zero
      */
      T if_set_return(T x) const
         {
         return m_mask & x;
         }

      /**
      * Return x if the mask is cleared, or otherwise zero
      */
      T if_not_set_return(T x) const
         {
         return ~m_mask & x;
         }

      /**
      * If this mask is set, return x, otherwise return y
      */
      T select(T x, T y) const
         {
         return choose(value(), x, y);
         }

      T select_and_unpoison(T x, T y) const
         {
         T r = this->select(x, y);
         CT::unpoison(r);
         return r;
         }

      /**
      * If this mask is set, return x, otherwise return y
      */
      Mask<T> select_mask(Mask<T> x, Mask<T> y) const
         {
         return Mask<T>(select(x.value(), y.value()));
         }

      /**
      * Conditionally set output to x or y, depending on if mask is set or
      * cleared (resp)
      */
      void select_n(T output[], const T x[], const T y[], size_t len) const
         {
         for(size_t i = 0; i != len; ++i)
            output[i] = this->select(x[i], y[i]);
         }

      /**
      * If this mask is set, zero out buf, otherwise do nothing
      */
      void if_set_zero_out(T buf[], size_t elems)
         {
         for(size_t i = 0; i != elems; ++i)
            {
            buf[i] = this->if_not_set_return(buf[i]);
            }
         }

      /**
      * Return the value of the mask, unpoisoned
      */
      T unpoisoned_value() const
         {
         T r = value();
         CT::unpoison(r);
         return r;
         }

      /**
      * Return true iff this mask is set
      */
      bool is_set() const
         {
         return unpoisoned_value() != 0;
         }

      /**
      * Return the underlying value of the mask
      */
      T value() const
         {
         return m_mask;
         }

   private:
      Mask(T m) : m_mask(m) {}

      T m_mask;
   };

template<typename T>
inline Mask<T> conditional_copy_mem(T cnd,
                                    T* to,
                                    const T* from0,
                                    const T* from1,
                                    size_t elems)
   {
   const auto mask = CT::Mask<T>::expand(cnd);
   mask.select_n(to, from0, from1, elems);
   return mask;
   }

template<typename T>
inline void conditional_swap(bool cnd, T& x, T& y)
   {
   const auto swap = CT::Mask<T>::expand(cnd);

   T t0 = swap.select(y, x);
   T t1 = swap.select(x, y);
   x = t0;
   y = t1;
   }

template<typename T>
inline void conditional_swap_ptr(bool cnd, T& x, T& y)
   {
   uintptr_t xp = reinterpret_cast<uintptr_t>(x);
   uintptr_t yp = reinterpret_cast<uintptr_t>(y);

   conditional_swap<uintptr_t>(cnd, xp, yp);

   x = reinterpret_cast<T>(xp);
   y = reinterpret_cast<T>(yp);
   }

/**
* If bad_input is unset, return input[offset:input_length] copied to new
* buffer. If bad_input is set, return an empty vector. In all cases, the capacity
* of the vector is equal to input_length
*
* This function attempts to avoid leaking the following:
*  - if bad_input was set or not
*  - the value of offset
*  - the values in input[]
*
* This function leaks the value of input_length
*/
BOTAN_TEST_API
secure_vector<uint8_t> copy_output(CT::Mask<uint8_t> bad_input,
                                   const uint8_t input[],
                                   size_t input_length,
                                   size_t offset);

secure_vector<uint8_t> strip_leading_zeros(const uint8_t in[], size_t length);

inline secure_vector<uint8_t> strip_leading_zeros(const secure_vector<uint8_t>& in)
   {
   return strip_leading_zeros(in.data(), in.size());
   }

}

}

#if defined(BOTAN_BUILD_COMPILER_IS_MSVC) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)
  #include <intrin.h>
  #pragma intrinsic(_umul128)
#endif

namespace Botan {

#if defined(__SIZEOF_INT128__) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)
   #define BOTAN_TARGET_HAS_NATIVE_UINT128

   // Prefer TI mode over __int128 as GCC rejects the latter in pendantic mode
   #if defined(__GNUG__)
     typedef unsigned int uint128_t __attribute__((mode(TI)));
   #else
     typedef unsigned __int128 uint128_t;
   #endif
#endif

/**
* Perform a 64x64->128 bit multiplication
*/
inline void mul64x64_128(uint64_t a, uint64_t b, uint64_t* lo, uint64_t* hi)
   {
#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)

   const uint128_t r = static_cast<uint128_t>(a) * b;
   *hi = (r >> 64) & 0xFFFFFFFFFFFFFFFF;
   *lo = (r      ) & 0xFFFFFFFFFFFFFFFF;

#elif defined(BOTAN_BUILD_COMPILER_IS_MSVC) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)
   *lo = _umul128(a, b, hi);

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_X86_64)
   asm("mulq %3"
       : "=d" (*hi), "=a" (*lo)
       : "a" (a), "rm" (b)
       : "cc");

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_PPC64)
   asm("mulhdu %0,%1,%2"
       : "=r" (*hi)
       : "r" (a), "r" (b)
       : "cc");
   *lo = a * b;

#else

   /*
   * Do a 64x64->128 multiply using four 32x32->64 multiplies plus
   * some adds and shifts. Last resort for CPUs like UltraSPARC (with
   * 64-bit registers/ALU, but no 64x64->128 multiply) or 32-bit CPUs.
   */
   const size_t HWORD_BITS = 32;
   const uint32_t HWORD_MASK = 0xFFFFFFFF;

   const uint32_t a_hi = (a >> HWORD_BITS);
   const uint32_t a_lo = (a  & HWORD_MASK);
   const uint32_t b_hi = (b >> HWORD_BITS);
   const uint32_t b_lo = (b  & HWORD_MASK);

   uint64_t x0 = static_cast<uint64_t>(a_hi) * b_hi;
   uint64_t x1 = static_cast<uint64_t>(a_lo) * b_hi;
   uint64_t x2 = static_cast<uint64_t>(a_hi) * b_lo;
   uint64_t x3 = static_cast<uint64_t>(a_lo) * b_lo;

   // this cannot overflow as (2^32-1)^2 + 2^32-1 < 2^64-1
   x2 += x3 >> HWORD_BITS;

   // this one can overflow
   x2 += x1;

   // propagate the carry if any
   x0 += static_cast<uint64_t>(static_cast<bool>(x2 < x1)) << HWORD_BITS;

   *hi = x0 + (x2 >> HWORD_BITS);
   *lo  = ((x2 & HWORD_MASK) << HWORD_BITS) + (x3 & HWORD_MASK);
#endif
   }

}

namespace Botan {

class donna128 final
   {
   public:
      donna128(uint64_t ll = 0, uint64_t hh = 0) { l = ll; h = hh; }

      donna128(const donna128&) = default;
      donna128& operator=(const donna128&) = default;

      friend donna128 operator>>(const donna128& x, size_t shift)
         {
         donna128 z = x;
         if(shift > 0)
            {
            const uint64_t carry = z.h << (64 - shift);
            z.h = (z.h >> shift);
            z.l = (z.l >> shift) | carry;
            }
         return z;
         }

      friend donna128 operator<<(const donna128& x, size_t shift)
         {
         donna128 z = x;
         if(shift > 0)
            {
            const uint64_t carry = z.l >> (64 - shift);
            z.l = (z.l << shift);
            z.h = (z.h << shift) | carry;
            }
         return z;
         }

      friend uint64_t operator&(const donna128& x, uint64_t mask)
         {
         return x.l & mask;
         }

      uint64_t operator&=(uint64_t mask)
         {
         h = 0;
         l &= mask;
         return l;
         }

      donna128& operator+=(const donna128& x)
         {
         l += x.l;
         h += x.h;

         const uint64_t carry = (l < x.l);
         h += carry;
         return *this;
         }

      donna128& operator+=(uint64_t x)
         {
         l += x;
         const uint64_t carry = (l < x);
         h += carry;
         return *this;
         }

      uint64_t lo() const { return l; }
      uint64_t hi() const { return h; }
   private:
      uint64_t h = 0, l = 0;
   };

inline donna128 operator*(const donna128& x, uint64_t y)
   {
   BOTAN_ARG_CHECK(x.hi() == 0, "High 64 bits of donna128 set to zero during multiply");

   uint64_t lo = 0, hi = 0;
   mul64x64_128(x.lo(), y, &lo, &hi);
   return donna128(lo, hi);
   }

inline donna128 operator*(uint64_t y, const donna128& x)
   {
   return x * y;
   }

inline donna128 operator+(const donna128& x, const donna128& y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator+(const donna128& x, uint64_t y)
   {
   donna128 z = x;
   z += y;
   return z;
   }

inline donna128 operator|(const donna128& x, const donna128& y)
   {
   return donna128(x.lo() | y.lo(), x.hi() | y.hi());
   }

inline uint64_t carry_shift(const donna128& a, size_t shift)
   {
   return (a >> shift).lo();
   }

inline uint64_t combine_lower(const donna128& a, size_t s1,
                              const donna128& b, size_t s2)
   {
   donna128 z = (a >> s1) | (b << s2);
   return z.lo();
   }

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)
inline uint64_t carry_shift(const uint128_t a, size_t shift)
   {
   return static_cast<uint64_t>(a >> shift);
   }

inline uint64_t combine_lower(const uint128_t a, size_t s1,
                              const uint128_t b, size_t s2)
   {
   return static_cast<uint64_t>((a >> s1) | (b << s2));
   }
#endif

}

namespace Botan {

/**
* Represents a DLL or shared object
*/
class BOTAN_TEST_API Dynamically_Loaded_Library final
   {
   public:
      /**
      * Load a DLL (or fail with an exception)
      * @param lib_name name or path to a library
      *
      * If you don't use a full path, the search order will be defined
      * by whatever the system linker does by default. Always using fully
      * qualified pathnames can help prevent code injection attacks (eg
      * via manipulation of LD_LIBRARY_PATH on Linux)
      */
      Dynamically_Loaded_Library(const std::string& lib_name);

      /**
      * Unload the DLL
      * @warning Any pointers returned by resolve()/resolve_symbol()
      * should not be used after this destructor runs.
      */
      ~Dynamically_Loaded_Library();

      /**
      * Load a symbol (or fail with an exception)
      * @param symbol names the symbol to load
      * @return address of the loaded symbol
      */
      void* resolve_symbol(const std::string& symbol);

      /**
      * Convenience function for casting symbol to the right type
      * @param symbol names the symbol to load
      * @return address of the loaded symbol
      */
      template<typename T>
      T resolve(const std::string& symbol)
         {
         return reinterpret_cast<T>(resolve_symbol(symbol));
         }

   private:
      Dynamically_Loaded_Library(const Dynamically_Loaded_Library&);
      Dynamically_Loaded_Library& operator=(const Dynamically_Loaded_Library&);

      std::string m_lib_name;
      void* m_lib;
   };

}

namespace Botan {

/**
* No_Filesystem_Access Exception
*/
class No_Filesystem_Access final : public Exception
   {
   public:
      No_Filesystem_Access() : Exception("No filesystem access enabled.")
         {}
   };

BOTAN_TEST_API bool has_filesystem_impl();

BOTAN_TEST_API std::vector<std::string> get_files_recursive(const std::string& dir);

}

namespace Botan {

/**
* HMAC
*/
class HMAC final : public MessageAuthenticationCode
   {
   public:
      void clear() override;
      std::string name() const override;
      std::unique_ptr<MessageAuthenticationCode> new_object() const override;

      size_t output_length() const override;

      Key_Length_Specification key_spec() const override;

      /**
      * @param hash the hash to use for HMACing
      */
      explicit HMAC(std::unique_ptr<HashFunction> hash);

      HMAC(const HMAC&) = delete;
      HMAC& operator=(const HMAC&) = delete;
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void key_schedule(const uint8_t[], size_t) override;

      std::unique_ptr<HashFunction> m_hash;
      secure_vector<uint8_t> m_ikey, m_okey;
      size_t m_hash_output_length;
      size_t m_hash_block_size;
   };

}

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
   #define BOTAN_ENDIAN_N2L(x) reverse_bytes(x)
   #define BOTAN_ENDIAN_L2N(x) reverse_bytes(x)
   #define BOTAN_ENDIAN_N2B(x) (x)
   #define BOTAN_ENDIAN_B2N(x) (x)

#elif defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
   #define BOTAN_ENDIAN_N2L(x) (x)
   #define BOTAN_ENDIAN_L2N(x) (x)
   #define BOTAN_ENDIAN_N2B(x) reverse_bytes(x)
   #define BOTAN_ENDIAN_B2N(x) reverse_bytes(x)

#endif

namespace Botan {

/**
* Byte extraction
* @param byte_num which byte to extract, 0 == highest byte
* @param input the value to extract from
* @return byte byte_num of input
*/
template<typename T> inline constexpr uint8_t get_byte_var(size_t byte_num, T input)
   {
   return static_cast<uint8_t>(
      input >> (((~byte_num)&(sizeof(T)-1)) << 3)
      );
   }

/**
* Byte extraction
* @param byte_num which byte to extract, 0 == highest byte
* @param input the value to extract from
* @return byte byte_num of input
*/
template<size_t B, typename T> inline constexpr uint8_t get_byte(T input)
   {
   static_assert(B < sizeof(T), "Valid byte offset");

   const size_t shift = ((~B) & (sizeof(T) - 1)) << 3;
   return static_cast<uint8_t>((input >> shift) & 0xFF);
   }

/**
* Make a uint16_t from two bytes
* @param i0 the first byte
* @param i1 the second byte
* @return i0 || i1
*/
inline constexpr uint16_t make_uint16(uint8_t i0, uint8_t i1)
   {
   return static_cast<uint16_t>((static_cast<uint16_t>(i0) << 8) | i1);
   }

/**
* Make a uint32_t from four bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @return i0 || i1 || i2 || i3
*/
inline constexpr uint32_t make_uint32(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3)
   {
   return ((static_cast<uint32_t>(i0) << 24) |
           (static_cast<uint32_t>(i1) << 16) |
           (static_cast<uint32_t>(i2) <<  8) |
           (static_cast<uint32_t>(i3)));
   }

/**
* Make a uint64_t from eight bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @param i4 the fifth byte
* @param i5 the sixth byte
* @param i6 the seventh byte
* @param i7 the eighth byte
* @return i0 || i1 || i2 || i3 || i4 || i5 || i6 || i7
*/
inline constexpr uint64_t make_uint64(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3,
                                      uint8_t i4, uint8_t i5, uint8_t i6, uint8_t i7)
    {
   return ((static_cast<uint64_t>(i0) << 56) |
           (static_cast<uint64_t>(i1) << 48) |
           (static_cast<uint64_t>(i2) << 40) |
           (static_cast<uint64_t>(i3) << 32) |
           (static_cast<uint64_t>(i4) << 24) |
           (static_cast<uint64_t>(i5) << 16) |
           (static_cast<uint64_t>(i6) <<  8) |
           (static_cast<uint64_t>(i7)));
    }

/**
* Load a big-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a big-endian value
*/
template<typename T>
inline constexpr T load_be(const uint8_t in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = static_cast<T>((out << 8) | in[i]);
   return out;
   }

/**
* Load a little-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a litte-endian value
*/
template<typename T>
inline constexpr T load_le(const uint8_t in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[sizeof(T)-1-i];
   return out;
   }

/**
* Load a big-endian uint16_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint16_t of in, as a big-endian value
*/
template<>
inline constexpr uint16_t load_be<uint16_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint16_t);

#if defined(BOTAN_ENDIAN_N2B)
   uint16_t x = 0;
   typecast_copy(x, in);
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_uint16(in[0], in[1]);
#endif
   }

/**
* Load a little-endian uint16_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint16_t of in, as a little-endian value
*/
template<>
inline constexpr uint16_t load_le<uint16_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint16_t);

#if defined(BOTAN_ENDIAN_N2L)
   uint16_t x = 0;
   typecast_copy(x, in);
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_uint16(in[1], in[0]);
#endif
   }

/**
* Load a big-endian uint32_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint32_t of in, as a big-endian value
*/
template<>
inline constexpr uint32_t load_be<uint32_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint32_t);
#if defined(BOTAN_ENDIAN_N2B)
   uint32_t x = 0;
   typecast_copy(x, in);
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_uint32(in[0], in[1], in[2], in[3]);
#endif
   }

/**
* Load a little-endian uint32_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint32_t of in, as a little-endian value
*/
template<>
inline constexpr uint32_t load_le<uint32_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint32_t);
#if defined(BOTAN_ENDIAN_N2L)
   uint32_t x = 0;
   typecast_copy(x, in);
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_uint32(in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load a big-endian uint64_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint64_t of in, as a big-endian value
*/
template<>
inline constexpr uint64_t load_be<uint64_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint64_t);
#if defined(BOTAN_ENDIAN_N2B)
   uint64_t x = 0;
   typecast_copy(x, in);
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_uint64(in[0], in[1], in[2], in[3],
                      in[4], in[5], in[6], in[7]);
#endif
   }

/**
* Load a little-endian uint64_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint64_t of in, as a little-endian value
*/
template<>
inline constexpr uint64_t load_le<uint64_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint64_t);
#if defined(BOTAN_ENDIAN_N2L)
   uint64_t x = 0;
   typecast_copy(x, in);
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_uint64(in[7], in[6], in[5], in[4],
                      in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load two little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline constexpr void load_le(const uint8_t in[], T& x0, T& x1)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   }

/**
* Load four little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline constexpr void load_le(const uint8_t in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   }

/**
* Load eight little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline constexpr void load_le(const uint8_t in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   x4 = load_le<T>(in, 4);
   x5 = load_le<T>(in, 5);
   x6 = load_le<T>(in, 6);
   x7 = load_le<T>(in, 7);
   }

/**
* Load a variable number of little-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline constexpr void load_le(T out[],
                    const uint8_t in[],
                    size_t count)
   {
   if(count > 0)
      {
#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
      typecast_copy(out, in, count);

#elif defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
      typecast_copy(out, in, count);

      const size_t blocks = count - (count % 4);
      const size_t left = count - blocks;

      for(size_t i = 0; i != blocks; i += 4)
         bswap_4(out + i);

      for(size_t i = 0; i != left; ++i)
         out[blocks+i] = reverse_bytes(out[blocks+i]);
#else
      for(size_t i = 0; i != count; ++i)
         out[i] = load_le<T>(in, i);
#endif
      }
   }

/**
* Load two big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline constexpr void load_be(const uint8_t in[], T& x0, T& x1)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   }

/**
* Load four big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline constexpr void load_be(const uint8_t in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   }

/**
* Load eight big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline constexpr void load_be(const uint8_t in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   x4 = load_be<T>(in, 4);
   x5 = load_be<T>(in, 5);
   x6 = load_be<T>(in, 6);
   x7 = load_be<T>(in, 7);
   }

/**
* Load a variable number of big-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline constexpr void load_be(T out[],
                    const uint8_t in[],
                    size_t count)
   {
   if(count > 0)
      {
#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
      typecast_copy(out, in, count);

#elif defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
      typecast_copy(out, in, count);
      const size_t blocks = count - (count % 4);
      const size_t left = count - blocks;

      for(size_t i = 0; i != blocks; i += 4)
         bswap_4(out + i);

      for(size_t i = 0; i != left; ++i)
         out[blocks+i] = reverse_bytes(out[blocks+i]);
#else
      for(size_t i = 0; i != count; ++i)
         out[i] = load_be<T>(in, i);
#endif
      }
   }

/**
* Store a big-endian uint16_t
* @param in the input uint16_t
* @param out the byte array to write to
*/
inline constexpr void store_be(uint16_t in, uint8_t out[2])
   {
#if defined(BOTAN_ENDIAN_N2B)
   uint16_t o = BOTAN_ENDIAN_N2B(in);
   typecast_copy(out, o);
#else
   out[0] = get_byte<0>(in);
   out[1] = get_byte<1>(in);
#endif
   }

/**
* Store a little-endian uint16_t
* @param in the input uint16_t
* @param out the byte array to write to
*/
inline constexpr void store_le(uint16_t in, uint8_t out[2])
   {
#if defined(BOTAN_ENDIAN_N2L)
   uint16_t o = BOTAN_ENDIAN_N2L(in);
   typecast_copy(out, o);
#else
   out[0] = get_byte<1>(in);
   out[1] = get_byte<0>(in);
#endif
   }

/**
* Store a big-endian uint32_t
* @param in the input uint32_t
* @param out the byte array to write to
*/
inline constexpr void store_be(uint32_t in, uint8_t out[4])
   {
#if defined(BOTAN_ENDIAN_B2N)
   uint32_t o = BOTAN_ENDIAN_B2N(in);
   typecast_copy(out, o);
#else
   out[0] = get_byte<0>(in);
   out[1] = get_byte<1>(in);
   out[2] = get_byte<2>(in);
   out[3] = get_byte<3>(in);
#endif
   }

/**
* Store a little-endian uint32_t
* @param in the input uint32_t
* @param out the byte array to write to
*/
inline constexpr void store_le(uint32_t in, uint8_t out[4])
   {
#if defined(BOTAN_ENDIAN_L2N)
   uint32_t o = BOTAN_ENDIAN_L2N(in);
   typecast_copy(out, o);
#else
   out[0] = get_byte<3>(in);
   out[1] = get_byte<2>(in);
   out[2] = get_byte<1>(in);
   out[3] = get_byte<0>(in);
#endif
   }

/**
* Store a big-endian uint64_t
* @param in the input uint64_t
* @param out the byte array to write to
*/
inline constexpr void store_be(uint64_t in, uint8_t out[8])
   {
#if defined(BOTAN_ENDIAN_B2N)
   uint64_t o = BOTAN_ENDIAN_B2N(in);
   typecast_copy(out, o);
#else
   out[0] = get_byte<0>(in);
   out[1] = get_byte<1>(in);
   out[2] = get_byte<2>(in);
   out[3] = get_byte<3>(in);
   out[4] = get_byte<4>(in);
   out[5] = get_byte<5>(in);
   out[6] = get_byte<6>(in);
   out[7] = get_byte<7>(in);
#endif
   }

/**
* Store a little-endian uint64_t
* @param in the input uint64_t
* @param out the byte array to write to
*/
inline constexpr void store_le(uint64_t in, uint8_t out[8])
   {
#if defined(BOTAN_ENDIAN_L2N)
   uint64_t o = BOTAN_ENDIAN_L2N(in);
   typecast_copy(out, o);
#else
   out[0] = get_byte<7>(in);
   out[1] = get_byte<6>(in);
   out[2] = get_byte<5>(in);
   out[3] = get_byte<4>(in);
   out[4] = get_byte<3>(in);
   out[5] = get_byte<2>(in);
   out[6] = get_byte<1>(in);
   out[7] = get_byte<0>(in);
#endif
   }

/**
* Store two little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline constexpr void store_le(uint8_t out[], T x0, T x1)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   }

/**
* Store two big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline constexpr void store_be(uint8_t out[], T x0, T x1)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   }

/**
* Store four little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline constexpr void store_le(uint8_t out[], T x0, T x1, T x2, T x3)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   }

/**
* Store four big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline constexpr void store_be(uint8_t out[], T x0, T x1, T x2, T x3)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   }

/**
* Store eight little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline constexpr void store_le(uint8_t out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   store_le(x4, out + (4 * sizeof(T)));
   store_le(x5, out + (5 * sizeof(T)));
   store_le(x6, out + (6 * sizeof(T)));
   store_le(x7, out + (7 * sizeof(T)));
   }

/**
* Store eight big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline constexpr void store_be(uint8_t out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   store_be(x4, out + (4 * sizeof(T)));
   store_be(x5, out + (5 * sizeof(T)));
   store_be(x6, out + (6 * sizeof(T)));
   store_be(x7, out + (7 * sizeof(T)));
   }

template<typename T>
void copy_out_be(uint8_t out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_be(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte_var(i % 8, in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_be(uint8_t out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_be(out, out_bytes, in.data());
   }

template<typename T>
void copy_out_le(uint8_t out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_le(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte_var(sizeof(T) - 1 - (i % 8), in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_le(uint8_t out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_le(out, out_bytes, in.data());
   }

}

namespace Botan {

/**
* MDx Hash Function Base Class
*/
class MDx_HashFunction : public HashFunction
   {
   public:
      /**
      * @param block_length is the number of bytes per block, which must
      *        be a power of 2 and at least 8.
      * @param big_byte_endian specifies if the hash uses big-endian bytes
      * @param big_bit_endian specifies if the hash uses big-endian bits
      * @param counter_size specifies the size of the counter var in bytes
      */
      MDx_HashFunction(size_t block_length,
                       bool big_byte_endian,
                       bool big_bit_endian,
                       uint8_t counter_size = 8);

      size_t hash_block_size() const override final { return m_buffer.size(); }
   protected:
      void add_data(const uint8_t input[], size_t length) override final;
      void final_result(uint8_t output[]) override final;

      /**
      * Run the hash's compression function over a set of blocks
      * @param blocks the input
      * @param block_n the number of blocks
      */
      virtual void compress_n(const uint8_t blocks[], size_t block_n) = 0;

      void clear() override;

      /**
      * Copy the output to the buffer
      * @param buffer to put the output into
      */
      virtual void copy_out(uint8_t buffer[]) = 0;
   private:
      const uint8_t m_pad_char;
      const uint8_t m_counter_size;
      const uint8_t m_block_bits;
      const bool m_count_big_endian;

      uint64_t m_count;
      secure_vector<uint8_t> m_buffer;
      size_t m_position;
   };

}

namespace Botan {

namespace OS {

/*
* This header is internal (not installed) and these functions are not
* intended to be called by applications. However they are given public
* visibility (using BOTAN_TEST_API macro) for the tests. This also probably
* allows them to be overridden by the application on ELF systems, but
* this hasn't been tested.
*/

/**
* @return process ID assigned by the operating system.
* On Unix and Windows systems, this always returns a result
* On IncludeOS it returns 0 since there is no process ID to speak of
* in a unikernel.
*/
uint32_t BOTAN_TEST_API get_process_id();

/**
* Test if we are currently running with elevated permissions
* eg setuid, setgid, or with POSIX caps set.
*/
bool running_in_privileged_state();

/**
* @return CPU processor clock, if available
*
* On Windows, calls QueryPerformanceCounter.
*
* Under GCC or Clang on supported platforms the hardware cycle counter is queried.
* Currently supported processors are x86, PPC, Alpha, SPARC, IA-64, S/390x, and HP-PA.
* If no CPU cycle counter is available on this system, returns zero.
*/
uint64_t BOTAN_TEST_API get_cpu_cycle_counter();

size_t BOTAN_TEST_API get_cpu_available();

/**
* Return the ELF auxiliary vector cooresponding to the given ID.
* This only makes sense on Unix-like systems and is currently
* only supported on Linux, Android, and FreeBSD.
*
* Returns zero if not supported on the current system or if
* the id provided is not known.
*/
unsigned long get_auxval(unsigned long id);

/*
* @return best resolution timestamp available
*
* The epoch and update rate of this clock is arbitrary and depending
* on the hardware it may not tick at a constant rate.
*
* Uses hardware cycle counter, if available.
* On POSIX platforms clock_gettime is used with a monotonic timer
* As a final fallback std::chrono::high_resolution_clock is used.
*/
uint64_t BOTAN_TEST_API get_high_resolution_clock();

/**
* @return system clock (reflecting wall clock) with best resolution
* available, normalized to nanoseconds resolution.
*/
uint64_t BOTAN_TEST_API get_system_timestamp_ns();

/**
* @return maximum amount of memory (in bytes) Botan could/should
* hyptothetically allocate for the memory poool. Reads environment
* variable "BOTAN_MLOCK_POOL_SIZE", set to "0" to disable pool.
*/
size_t get_memory_locking_limit();

/**
* Return the size of a memory page, if that can be derived on the
* current system. Otherwise returns some default value (eg 4096)
*/
size_t system_page_size();

/**
* Return the cache line size of the current processor using some
* OS specific interface, or 0 if not available on this platform.
*/
size_t get_cache_line_size();

/**
* Read the value of an environment variable, setting it to value_out if it
* exists.  Returns false and sets value_out to empty string if no such variable
* is set. If the process seems to be running in a privileged state (such as
* setuid) then always returns false and does not examine the environment.
*/
bool read_env_variable(std::string& value_out, const std::string& var_name);

/**
* Read the value of an environment variable and convert it to an
* integer. If not set or conversion fails, returns the default value.
*
* If the process seems to be running in a privileged state (such as setuid)
* then always returns nullptr, similiar to glibc's secure_getenv.
*/
size_t read_env_variable_sz(const std::string& var_name, size_t def_value = 0);

/**
* Request count pages of RAM which are locked into memory using mlock,
* VirtualLock, or some similar OS specific API. Free it with free_locked_pages.
*
* Returns an empty list on failure. This function is allowed to return fewer
* than count pages.
*
* The contents of the allocated pages are undefined.
*
* Each page is preceded by and followed by a page which is marked
* as noaccess, such that accessing it will cause a crash. This turns
* out of bound reads/writes into crash events.
*
* @param count requested number of locked pages
*/
std::vector<void*> allocate_locked_pages(size_t count);

/**
* Free memory allocated by allocate_locked_pages
* @param pages a list of pages returned by allocate_locked_pages
*/
void free_locked_pages(const std::vector<void*>& pages);

/**
* Set the MMU to prohibit access to this page
*/
void page_prohibit_access(void* page);

/**
* Set the MMU to allow R/W access to this page
*/
void page_allow_access(void* page);


/**
* Run a probe instruction to test for support for a CPU instruction.
* Runs in system-specific env that catches illegal instructions; this
* function always fails if the OS doesn't provide this.
* Returns value of probe_fn, if it could run.
* If error occurs, returns negative number.
* This allows probe_fn to indicate errors of its own, if it wants.
* For example the instruction might not only be only available on some
* CPUs, but also buggy on some subset of these - the probe function
* can test to make sure the instruction works properly before
* indicating that the instruction is available.
*
* @warning on Unix systems uses signal handling in a way that is not
* thread safe. It should only be called in a single-threaded context
* (ie, at static init time).
*
* If probe_fn throws an exception the result is undefined.
*
* Return codes:
* -1 illegal instruction detected
*/
int BOTAN_TEST_API run_cpu_instruction_probe(std::function<int ()> probe_fn);

/**
* Represents a terminal state
*/
class BOTAN_UNSTABLE_API Echo_Suppression
   {
   public:
      /**
      * Reenable echo on this terminal. Can be safely called
      * multiple times. May throw if an error occurs.
      */
      virtual void reenable_echo() = 0;

      /**
      * Implicitly calls reenable_echo, but swallows/ignored all
      * errors which would leave the terminal in an invalid state.
      */
      virtual ~Echo_Suppression() = default;
   };

/**
* Suppress echo on the terminal
* Returns null if this operation is not supported on the current system.
*/
std::unique_ptr<Echo_Suppression> BOTAN_UNSTABLE_API suppress_echo_on_terminal();

}

}

namespace Botan {

/**
* Parse a SCAN-style algorithm name
* @param scan_name the name
* @return the name components
*/
std::vector<std::string>
parse_algorithm_name(const std::string& scan_name);

/**
* Split a string
* @param str the input string
* @param delim the delimitor
* @return string split by delim
*/
BOTAN_TEST_API std::vector<std::string> split_on(
   const std::string& str, char delim);

/**
* Join a string
* @param strs strings to join
* @param delim the delimitor
* @return string joined by delim
*/
std::string string_join(const std::vector<std::string>& strs,
                        char delim);

/**
* Convert a string to a number
* @param str the string to convert
* @return number value of the string
*/
BOTAN_TEST_API uint32_t to_u32bit(const std::string& str);

/**
* Convert a string to a number
* @param str the string to convert
* @return number value of the string
*/
uint16_t to_uint16(const std::string& str);

/**
* Convert a string representation of an IPv4 address to a number
* @param ip_str the string representation
* @return integer IPv4 address
*/
uint32_t string_to_ipv4(const std::string& ip_str);

/**
* Convert an IPv4 address to a string
* @param ip_addr the IPv4 address to convert
* @return string representation of the IPv4 address
*/
std::string ipv4_to_string(uint32_t ip_addr);

std::map<std::string, std::string> read_cfg(std::istream& is);

/**
* Accepts key value pairs deliminated by commas:
*
* "" (returns empty map)
* "K=V" (returns map {'K': 'V'})
* "K1=V1,K2=V2"
* "K1=V1,K2=V2,K3=V3"
* "K1=V1,K2=V2,K3=a_value\,with\,commas_and_\=equals"
*
* Values may be empty, keys must be non-empty and unique. Duplicate
* keys cause an exception.
*
* Within both key and value, comma and equals can be escaped with
* backslash. Backslash can also be escaped.
*/
BOTAN_TEST_API
std::map<std::string, std::string> read_kv(const std::string& kv);

std::string clean_ws(const std::string& s);

std::string tolower_string(const std::string& s);

/**
* Check if the given hostname is a match for the specified wildcard
*/
BOTAN_TEST_API
bool host_wildcard_match(const std::string& wildcard,
                         const std::string& host);


}

namespace Botan {

/**
* Bit rotation left by a compile-time constant amount
* @param input the input word
* @return input rotated left by ROT bits
*/
template<size_t ROT, typename T>
inline constexpr T rotl(T input)
   {
   static_assert(ROT > 0 && ROT < 8*sizeof(T), "Invalid rotation constant");
   return static_cast<T>((input << ROT) | (input >> (8*sizeof(T) - ROT)));
   }

/**
* Bit rotation right by a compile-time constant amount
* @param input the input word
* @return input rotated right by ROT bits
*/
template<size_t ROT, typename T>
inline constexpr T rotr(T input)
   {
   static_assert(ROT > 0 && ROT < 8*sizeof(T), "Invalid rotation constant");
   return static_cast<T>((input >> ROT) | (input << (8*sizeof(T) - ROT)));
   }

/**
* Bit rotation left, variable rotation amount
* @param input the input word
* @param rot the number of bits to rotate, must be between 0 and sizeof(T)*8-1
* @return input rotated left by rot bits
*/
template<typename T>
inline constexpr T rotl_var(T input, size_t rot)
   {
   return rot ? static_cast<T>((input << rot) | (input >> (sizeof(T)*8 - rot))) : input;
   }

/**
* Bit rotation right, variable rotation amount
* @param input the input word
* @param rot the number of bits to rotate, must be between 0 and sizeof(T)*8-1
* @return input rotated right by rot bits
*/
template<typename T>
inline constexpr T rotr_var(T input, size_t rot)
   {
   return rot ? static_cast<T>((input >> rot) | (input << (sizeof(T)*8 - rot))) : input;
   }

#if defined(BOTAN_USE_GCC_INLINE_ASM)

#if defined(BOTAN_TARGET_ARCH_IS_X86_64) || defined(BOTAN_TARGET_ARCH_IS_X86_32)

template<>
inline uint32_t rotl_var(uint32_t input, size_t rot)
   {
   asm("roll %1,%0"
       : "+r" (input)
       : "c" (static_cast<uint8_t>(rot))
       : "cc");
   return input;
   }

template<>
inline uint32_t rotr_var(uint32_t input, size_t rot)
   {
   asm("rorl %1,%0"
       : "+r" (input)
       : "c" (static_cast<uint8_t>(rot))
       : "cc");
   return input;
   }

#endif

#endif

}

namespace Botan {

/**
* Round up
* @param n a non-negative integer
* @param align_to the alignment boundary
* @return n rounded up to a multiple of align_to
*/
inline size_t round_up(size_t n, size_t align_to)
   {
   BOTAN_ARG_CHECK(align_to != 0, "align_to must not be 0");

   if(n % align_to)
      n += align_to - (n % align_to);
   return n;
   }

/**
* Round down
* @param n an integer
* @param align_to the alignment boundary
* @return n rounded down to a multiple of align_to
*/
template<typename T>
inline constexpr T round_down(T n, T align_to)
   {
   return (align_to == 0) ? n : (n - (n % align_to));
   }

}

namespace Botan {

class Integer_Overflow_Detected final : public Exception
   {
   public:
      Integer_Overflow_Detected(const std::string& file, int line) :
         Exception("Integer overflow detected at " + file + ":" + std::to_string(line))
         {}

      ErrorType error_type() const noexcept override { return ErrorType::InternalError; }
   };

inline size_t checked_add(size_t x, size_t y, const char* file, int line)
   {
   // TODO: use __builtin_x_overflow on GCC and Clang
   size_t z = x + y;
   if(z < x)
      {
      throw Integer_Overflow_Detected(file, line);
      }
   return z;
   }

template<typename RT, typename AT>
RT checked_cast_to(AT i)
   {
   RT c = static_cast<RT>(i);
   if(i != static_cast<AT>(c))
      throw Internal_Error("Error during integer conversion");
   return c;
   }

#define BOTAN_CHECKED_ADD(x,y) checked_add(x,y,__FILE__,__LINE__)

}

namespace Botan {

/**
A class encapsulating a SCAN name (similar to JCE conventions)
http://www.users.zetnet.co.uk/hopwood/crypto/scan/
*/
class SCAN_Name final
   {
   public:
      /**
      * Create a SCAN_Name
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(const char* algo_spec);

      /**
      * Create a SCAN_Name
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(std::string algo_spec);

      /**
      * @return original input string
      */
      const std::string& to_string() const { return m_orig_algo_spec; }

      /**
      * @return algorithm name
      */
      const std::string& algo_name() const { return m_alg_name; }

      /**
      * @return number of arguments
      */
      size_t arg_count() const { return m_args.size(); }

      /**
      * @param lower is the lower bound
      * @param upper is the upper bound
      * @return if the number of arguments is between lower and upper
      */
      bool arg_count_between(size_t lower, size_t upper) const
         { return ((arg_count() >= lower) && (arg_count() <= upper)); }

      /**
      * @param i which argument
      * @return ith argument
      */
      std::string arg(size_t i) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument or the default value
      */
      std::string arg(size_t i, const std::string& def_value) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument as an integer, or the default value
      */
      size_t arg_as_integer(size_t i, size_t def_value) const;

      /**
      * @param i which argument
      * @return ith argument as an integer
      */
      size_t arg_as_integer(size_t i) const;

      /**
      * @return cipher mode (if any)
      */
      std::string cipher_mode() const
         { return (m_mode_info.size() >= 1) ? m_mode_info[0] : ""; }

      /**
      * @return cipher mode padding (if any)
      */
      std::string cipher_mode_pad() const
         { return (m_mode_info.size() >= 2) ? m_mode_info[1] : ""; }

   private:
      std::string m_orig_algo_spec;
      std::string m_alg_name;
      std::vector<std::string> m_args;
      std::vector<std::string> m_mode_info;
   };

// This is unrelated but it is convenient to stash it here
template<typename T>
std::vector<std::string> probe_providers_of(const std::string& algo_spec,
                                            const std::vector<std::string>& possible = { "base" })
   {
   std::vector<std::string> providers;
   for(auto&& prov : possible)
      {
      std::unique_ptr<T> o(T::create(algo_spec, prov));
      if(o)
         {
         providers.push_back(prov); // available
         }
      }
   return providers;
   }

}

namespace Botan {

/**
* SHA-224
*/
class SHA_224 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-224"; }
      size_t output_length() const override { return 28; }
      std::unique_ptr<HashFunction> new_object() const override { return std::make_unique<SHA_224>(); }
      std::unique_ptr<HashFunction> copy_state() const override;

      void clear() override;

      std::string provider() const override;

      SHA_224() : MDx_HashFunction(64, true, true), m_digest(8)
         { clear(); }
   private:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      secure_vector<uint32_t> m_digest;
   };

/**
* SHA-256
*/
class SHA_256 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-256"; }
      size_t output_length() const override { return 32; }
      std::unique_ptr<HashFunction> new_object() const override { return std::make_unique<SHA_256>(); }
      std::unique_ptr<HashFunction> copy_state() const override;

      void clear() override;

      std::string provider() const override;

      SHA_256() : MDx_HashFunction(64, true, true), m_digest(8)
         { clear(); }

      /*
      * Perform a SHA-256 compression. For internal use
      */
      static void compress_digest(secure_vector<uint32_t>& digest,
                                  const uint8_t input[],
                                  size_t blocks);

   private:

#if defined(BOTAN_HAS_SHA2_32_ARMV8)
      static void compress_digest_armv8(secure_vector<uint32_t>& digest,
                                        const uint8_t input[],
                                        size_t blocks);
#endif

#if defined(BOTAN_HAS_SHA2_32_X86_BMI2)
      static void compress_digest_x86_bmi2(secure_vector<uint32_t>& digest,
                                           const uint8_t input[],
                                           size_t blocks);
#endif

#if defined(BOTAN_HAS_SHA2_32_X86)
      static void compress_digest_x86(secure_vector<uint32_t>& digest,
                                      const uint8_t input[],
                                      size_t blocks);
#endif

      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      secure_vector<uint32_t> m_digest;
   };

}

namespace Botan {

inline std::vector<uint8_t> to_byte_vector(const std::string& s)
   {
   return std::vector<uint8_t>(s.cbegin(), s.cend());
   }

inline std::string to_string(const secure_vector<uint8_t> &bytes)
   {
   return std::string(bytes.cbegin(), bytes.cend());
   }

/**
* Return the keys of a map as a std::set
*/
template<typename K, typename V>
std::set<K> map_keys_as_set(const std::map<K, V>& kv)
   {
   std::set<K> s;
   for(auto&& i : kv)
      {
      s.insert(i.first);
      }
   return s;
   }

/*
* Searching through a std::map
* @param mapping the map to search
* @param key is what to look for
* @param null_result is the value to return if key is not in mapping
* @return mapping[key] or null_result
*/
template<typename K, typename V>
inline V search_map(const std::map<K, V>& mapping,
                    const K& key,
                    const V& null_result = V())
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return i->second;
   }

template<typename K, typename V, typename R>
inline R search_map(const std::map<K, V>& mapping, const K& key,
                    const R& null_result, const R& found_result)
   {
   auto i = mapping.find(key);
   if(i == mapping.end())
      return null_result;
   return found_result;
   }

/*
* Insert a key/value pair into a multimap
*/
template<typename K, typename V>
void multimap_insert(std::multimap<K, V>& multimap,
                     const K& key, const V& value)
   {
   multimap.insert(std::make_pair(key, value));
   }

/**
* Existence check for values
*/
template<typename T>
bool value_exists(const std::vector<T>& vec,
                  const T& val)
   {
   for(size_t i = 0; i != vec.size(); ++i)
      if(vec[i] == val)
         return true;
   return false;
   }

template<typename T, typename Pred>
void map_remove_if(Pred pred, T& assoc)
   {
   auto i = assoc.begin();
   while(i != assoc.end())
      {
      if(pred(i->first))
         assoc.erase(i++);
      else
         i++;
      }
   }

}

namespace Botan {

class BOTAN_TEST_API Timer final
   {
   public:
      Timer(const std::string& name,
            const std::string& provider,
            const std::string& doing,
            uint64_t event_mult,
            size_t buf_size,
            double clock_cycle_ratio,
            uint64_t clock_speed)
         : m_name(name + ((provider.empty() || provider == "base") ? "" : " [" + provider + "]"))
         , m_doing(doing)
         , m_buf_size(buf_size)
         , m_event_mult(event_mult)
         , m_clock_cycle_ratio(clock_cycle_ratio)
         , m_clock_speed(clock_speed)
         {}

      Timer(const std::string& name) :
         Timer(name, "", "", 1, 0, 0.0, 0)
         {}

      Timer(const std::string& name, size_t buf_size) :
         Timer(name, "", "", buf_size, buf_size, 0.0, 0)
         {}

      Timer(const Timer& other) = default;
      Timer& operator=(const Timer& other) = default;

      void start();

      void stop();

      bool under(std::chrono::milliseconds msec)
         {
         return (milliseconds() < msec.count());
         }

      class Timer_Scope final
         {
         public:
            explicit Timer_Scope(Timer& timer)
               : m_timer(timer)
               {
               m_timer.start();
               }
            ~Timer_Scope()
               {
               try
                  {
                  m_timer.stop();
                  }
               catch(...) {}
               }
         private:
            Timer& m_timer;
         };

      template<typename F>
      auto run(F f) -> decltype(f())
         {
         Timer_Scope timer(*this);
         return f();
         }

      template<typename F>
      void run_until_elapsed(std::chrono::milliseconds msec, F f)
         {
         while(this->under(msec))
            {
            run(f);
            }
         }

      uint64_t value() const
         {
         return m_time_used;
         }

      double seconds() const
         {
         return milliseconds() / 1000.0;
         }

      double milliseconds() const
         {
         return value() / 1000000.0;
         }

      double ms_per_event() const
         {
         return milliseconds() / events();
         }

      uint64_t cycles_consumed() const
         {
         if(m_clock_speed != 0)
            {
            return static_cast<uint64_t>((m_clock_speed * value()) / 1000.0);
            }
         return m_cpu_cycles_used;
         }

      uint64_t events() const
         {
         return m_event_count * m_event_mult;
         }

      const std::string& get_name() const
         {
         return m_name;
         }

      const std::string& doing() const
         {
         return m_doing;
         }

      size_t buf_size() const
         {
         return m_buf_size;
         }

      double bytes_per_second() const
         {
         return seconds() > 0.0 ? events() / seconds() : 0.0;
         }

      double events_per_second() const
         {
         return seconds() > 0.0 ? events() / seconds() : 0.0;
         }

      double seconds_per_event() const
         {
         return events() > 0 ? seconds() / events() : 0.0;
         }

      void set_custom_msg(const std::string& s)
         {
         m_custom_msg = s;
         }

      bool operator<(const Timer& other) const;

      std::string to_string() const;

   private:
      std::string result_string_bps() const;
      std::string result_string_ops() const;

      // const data
      std::string m_name, m_doing;
      size_t m_buf_size;
      uint64_t m_event_mult;
      double m_clock_cycle_ratio;
      uint64_t m_clock_speed;

      // set at runtime
      std::string m_custom_msg;
      uint64_t m_time_used = 0, m_timer_start = 0;
      uint64_t m_event_count = 0;

      uint64_t m_max_time = 0, m_min_time = 0;
      uint64_t m_cpu_cycles_start = 0, m_cpu_cycles_used = 0;
   };

}
/**
* (C) 2018,2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

static const size_t SYNC_POINTS = 4;

secure_vector<uint8_t> argon2_H0(HashFunction& blake2b,
                                 size_t output_len,
                                 const char* password, size_t password_len,
                                 const uint8_t salt[], size_t salt_len,
                                 const uint8_t key[], size_t key_len,
                                 const uint8_t ad[], size_t ad_len,
                                 size_t y, size_t p, size_t M, size_t t)
   {
   const uint8_t v = 19; // Argon2 version code

   blake2b.update_le(static_cast<uint32_t>(p));
   blake2b.update_le(static_cast<uint32_t>(output_len));
   blake2b.update_le(static_cast<uint32_t>(M));
   blake2b.update_le(static_cast<uint32_t>(t));
   blake2b.update_le(static_cast<uint32_t>(v));
   blake2b.update_le(static_cast<uint32_t>(y));

   blake2b.update_le(static_cast<uint32_t>(password_len));
   blake2b.update(cast_char_ptr_to_uint8(password), password_len);

   blake2b.update_le(static_cast<uint32_t>(salt_len));
   blake2b.update(salt, salt_len);

   blake2b.update_le(static_cast<uint32_t>(key_len));
   blake2b.update(key, key_len);

   blake2b.update_le(static_cast<uint32_t>(ad_len));
   blake2b.update(ad, ad_len);

   return blake2b.final();
   }

void Htick(secure_vector<uint8_t>& T,
           uint8_t output[],
           size_t output_len,
           HashFunction& blake2b,
           const secure_vector<uint8_t>& H0,
           size_t p0, size_t p1)
   {
   BOTAN_ASSERT_NOMSG(output_len % 64 == 0);

   blake2b.update_le(static_cast<uint32_t>(output_len));
   blake2b.update(H0);
   blake2b.update_le(static_cast<uint32_t>(p0));
   blake2b.update_le(static_cast<uint32_t>(p1));

   blake2b.final(&T[0]);

   while(output_len > 64)
      {
      copy_mem(output, &T[0], 32);
      output_len -= 32;
      output += 32;

      blake2b.update(T);
      blake2b.final(&T[0]);
      }

   if(output_len > 0)
      copy_mem(output, &T[0], output_len);
   }

void extract_key(uint8_t output[], size_t output_len,
                 const secure_vector<uint64_t>& B,
                 size_t memory, size_t threads)
   {
   const size_t lanes = memory / threads;

   secure_vector<uint64_t> sum(128);

   for(size_t lane = 0; lane != threads; ++lane)
      {
      size_t start = 128*(lane * lanes + lanes - 1);
      size_t end = 128*(lane * lanes + lanes);

      for(size_t j = start; j != end; ++j)
         {
         sum[j % 128] ^= B[j];
         }
      }

   secure_vector<uint8_t> sum8(1024);
   copy_out_le(sum8.data(), 1024, sum.data());

   if(output_len <= 64)
      {
      std::unique_ptr<HashFunction> blake2b = HashFunction::create_or_throw("BLAKE2b(" + std::to_string(output_len*8) + ")");
      blake2b->update_le(static_cast<uint32_t>(output_len));
      blake2b->update(sum8.data(), sum8.size());
      blake2b->final(output);
      }
   else
      {
      secure_vector<uint8_t> T(64);

      std::unique_ptr<HashFunction> blake2b = HashFunction::create_or_throw("BLAKE2b(512)");
      blake2b->update_le(static_cast<uint32_t>(output_len));
      blake2b->update(sum8.data(), sum8.size());
      blake2b->final(&T[0]);

      while(output_len > 64)
         {
         copy_mem(output, &T[0], 32);
         output_len -= 32;
         output += 32;

         if(output_len > 64)
            {
            blake2b->update(T);
            blake2b->final(&T[0]);
            }
         }

      if(output_len == 64)
         {
         blake2b->update(T);
         blake2b->final(output);
         }
      else
         {
         std::unique_ptr<HashFunction> blake2b_f = HashFunction::create_or_throw("BLAKE2b(" + std::to_string(output_len*8) + ")");
         blake2b_f->update(T);
         blake2b_f->final(output);
         }
      }
   }

void init_blocks(secure_vector<uint64_t>& B,
                 HashFunction& blake2b,
                 const secure_vector<uint8_t>& H0,
                 size_t memory,
                 size_t threads)
   {
   BOTAN_ASSERT_NOMSG(B.size() >= threads*256);

   secure_vector<uint8_t> H(1024);
   secure_vector<uint8_t> T(blake2b.output_length());

   for(size_t i = 0; i != threads; ++i)
      {
      const size_t B_off = i * (memory / threads);

      BOTAN_ASSERT_NOMSG(B.size() >= 128*(B_off+2));

      Htick(T, &H[0], H.size(), blake2b, H0, 0, i);

      for(size_t j = 0; j != 128; ++j)
         {
         B[128*B_off+j] = load_le<uint64_t>(H.data(), j);
         }

      Htick(T, &H[0], H.size(), blake2b, H0, 1, i);

      for(size_t j = 0; j != 128; ++j)
         {
         B[128*(B_off+1)+j] = load_le<uint64_t>(H.data(), j);
         }
      }
   }

inline void blamka_G(uint64_t& A, uint64_t& B, uint64_t& C, uint64_t& D)
   {
   A += B + (static_cast<uint64_t>(2) * static_cast<uint32_t>(A)) * static_cast<uint32_t>(B);
   D = rotr<32>(A ^ D);

   C += D + (static_cast<uint64_t>(2) * static_cast<uint32_t>(C)) * static_cast<uint32_t>(D);
   B = rotr<24>(B ^ C);

   A += B + (static_cast<uint64_t>(2) * static_cast<uint32_t>(A)) * static_cast<uint32_t>(B);
   D = rotr<16>(A ^ D);

   C += D + (static_cast<uint64_t>(2) * static_cast<uint32_t>(C)) * static_cast<uint32_t>(D);
   B = rotr<63>(B ^ C);
   }

inline void blamka(uint64_t& V0, uint64_t& V1, uint64_t& V2, uint64_t& V3,
                   uint64_t& V4, uint64_t& V5, uint64_t& V6, uint64_t& V7,
                   uint64_t& V8, uint64_t& V9, uint64_t& VA, uint64_t& VB,
                   uint64_t& VC, uint64_t& VD, uint64_t& VE, uint64_t& VF)
   {
   blamka_G(V0, V4, V8, VC);
   blamka_G(V1, V5, V9, VD);
   blamka_G(V2, V6, VA, VE);
   blamka_G(V3, V7, VB, VF);

   blamka_G(V0, V5, VA, VF);
   blamka_G(V1, V6, VB, VC);
   blamka_G(V2, V7, V8, VD);
   blamka_G(V3, V4, V9, VE);
   }

void process_block_xor(secure_vector<uint64_t>& T,
                       secure_vector<uint64_t>& B,
                       size_t offset,
                       size_t prev,
                       size_t new_offset)
   {
   for(size_t i = 0; i != 128; ++i)
      T[i] = B[128*prev+i] ^ B[128*new_offset+i];

   for(size_t i = 0; i != 128; i += 16)
      {
      blamka(T[i+ 0], T[i+ 1], T[i+ 2], T[i+ 3],
             T[i+ 4], T[i+ 5], T[i+ 6], T[i+ 7],
             T[i+ 8], T[i+ 9], T[i+10], T[i+11],
             T[i+12], T[i+13], T[i+14], T[i+15]);
      }

   for(size_t i = 0; i != 128 / 8; i += 2)
      {
      blamka(T[    i], T[    i+1], T[ 16+i], T[ 16+i+1],
             T[ 32+i], T[ 32+i+1], T[ 48+i], T[ 48+i+1],
             T[ 64+i], T[ 64+i+1], T[ 80+i], T[ 80+i+1],
             T[ 96+i], T[ 96+i+1], T[112+i], T[112+i+1]);
      }

   for(size_t i = 0; i != 128; ++i)
      B[128*offset + i] ^= T[i] ^ B[128*prev+i] ^ B[128*new_offset+i];
   }

void gen_2i_addresses(secure_vector<uint64_t>& T, secure_vector<uint64_t>& B,
                      size_t n, size_t lane, size_t slice, size_t memory,
                      size_t time, size_t mode, size_t cnt)
   {
   BOTAN_ASSERT_NOMSG(B.size() == 128);
   BOTAN_ASSERT_NOMSG(T.size() == 128);

   clear_mem(B.data(), B.size());
   B[0] = n;
   B[1] = lane;
   B[2] = slice;
   B[3] = memory;
   B[4] = time;
   B[5] = mode;
   B[6] = cnt;

   for(size_t r = 0; r != 2; ++r)
      {
      copy_mem(T.data(), B.data(), B.size());

      for(size_t i = 0; i != 128; i += 16)
         {
         blamka(T[i+ 0], T[i+ 1], T[i+ 2], T[i+ 3],
                T[i+ 4], T[i+ 5], T[i+ 6], T[i+ 7],
                T[i+ 8], T[i+ 9], T[i+10], T[i+11],
                T[i+12], T[i+13], T[i+14], T[i+15]);
         }
      for(size_t i = 0; i != 128 / 8; i += 2)
         {
         blamka(T[    i], T[    i+1], T[ 16+i], T[ 16+i+1],
                T[ 32+i], T[ 32+i+1], T[ 48+i], T[ 48+i+1],
                T[ 64+i], T[ 64+i+1], T[ 80+i], T[ 80+i+1],
                T[ 96+i], T[ 96+i+1], T[112+i], T[112+i+1]);
         }

      for(size_t i = 0; i != 128; ++i)
         B[i] ^= T[i];
      }
   }

uint32_t index_alpha(uint64_t random,
                     size_t lanes,
                     size_t segments,
                     size_t threads,
                     size_t n,
                     size_t slice,
                     size_t lane,
                     size_t index)
   {
   size_t ref_lane = static_cast<uint32_t>(random >> 32) % threads;

   if(n == 0 && slice == 0)
      ref_lane = lane;

   size_t m = 3*segments;
   size_t s = ((slice+1) % 4)*segments;

   if(lane == ref_lane)
      m += index;

   if(n == 0) {
         m = slice*segments;
         s = 0;
         if(slice == 0 || lane == ref_lane)
            m += index;
   }

   if(index == 0 || lane == ref_lane)
      m -= 1;

   uint64_t p = static_cast<uint32_t>(random);
   p = (p * p) >> 32;
   p = (p * m) >> 32;

   return static_cast<uint32_t>(ref_lane*lanes + (s + m - (p+1)) % lanes);
   }

void process_block_argon2d(secure_vector<uint64_t>& T,
                           secure_vector<uint64_t>& B,
                           size_t n, size_t slice, size_t lane,
                           size_t lanes, size_t segments, size_t threads)
   {
   size_t index = 0;
   if(n == 0 && slice == 0)
      index = 2;

   while(index < segments)
      {
      const size_t offset = lane*lanes + slice*segments + index;

      size_t prev = offset - 1;
      if(index == 0 && slice == 0)
         prev += lanes;

      const uint64_t random = B.at(128*prev);
      const size_t new_offset = index_alpha(random, lanes, segments, threads, n, slice, lane, index);

      process_block_xor(T, B, offset, prev, new_offset);

      index += 1;
      }
   }

void process_block_argon2i(secure_vector<uint64_t>& T,
                           secure_vector<uint64_t>& B,
                           size_t n, size_t slice, size_t lane,
                           size_t lanes, size_t segments, size_t threads, uint8_t mode,
                           size_t memory, size_t time)
   {
   size_t index = 0;
   if(n == 0 && slice == 0)
      index = 2;

   secure_vector<uint64_t> addresses(128);
   size_t address_counter = 1;

   gen_2i_addresses(T, addresses, n, lane, slice, memory, time, mode, address_counter);

   while(index < segments)
      {
      const size_t offset = lane*lanes + slice*segments + index;

      size_t prev = offset - 1;
      if(index == 0 && slice == 0)
         prev += lanes;

      if(index > 0 && index % 128 == 0)
         {
         address_counter += 1;
         gen_2i_addresses(T, addresses, n, lane, slice, memory, time, mode, address_counter);
         }

      const uint64_t random = addresses[index % 128];
      const size_t new_offset = index_alpha(random, lanes, segments, threads, n, slice, lane, index);

      process_block_xor(T, B, offset, prev, new_offset);

      index += 1;
      }
   }

void process_blocks(secure_vector<uint64_t>& B,
                    size_t t,
                    size_t memory,
                    size_t threads,
                    uint8_t mode)
   {
   const size_t lanes = memory / threads;
   const size_t segments = lanes / SYNC_POINTS;

   secure_vector<uint64_t> T(128);
   for(size_t n = 0; n != t; ++n)
      {
      for(size_t slice = 0; slice != SYNC_POINTS; ++slice)
         {
         // TODO can run this in Thread_Pool
         for(size_t lane = 0; lane != threads; ++lane)
            {
            if(mode == 1 || (mode == 2 && n == 0 && slice < SYNC_POINTS/2))
               process_block_argon2i(T, B, n, slice, lane, lanes, segments, threads, mode, memory, t);
            else
               process_block_argon2d(T, B, n, slice, lane, lanes, segments, threads);
            }
         }
      }

   }

}

void Argon2::argon2(uint8_t output[], size_t output_len,
                    const char* password, size_t password_len,
                    const uint8_t salt[], size_t salt_len,
                    const uint8_t key[], size_t key_len,
                    const uint8_t ad[], size_t ad_len) const
   {
   BOTAN_ARG_CHECK(output_len >= 4, "Invalid Argon2 output length");

   auto blake2 = HashFunction::create_or_throw("BLAKE2b");

   const auto H0 = argon2_H0(*blake2, output_len,
                             password, password_len,
                             salt, salt_len,
                             key, key_len,
                             ad, ad_len,
                             m_family, m_p, m_M, m_t);

   const size_t memory = (m_M / (SYNC_POINTS*m_p)) * (SYNC_POINTS*m_p);

   secure_vector<uint64_t> B(memory * 1024/8);

   init_blocks(B, *blake2, H0, memory, m_p);
   process_blocks(B, m_t, memory, m_p, m_family);

   clear_mem(output, output_len);
   extract_key(output, output_len, B, memory, m_p);
   }

}
/**
* (C) 2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <algorithm>

namespace Botan {

Argon2::Argon2(uint8_t family, size_t M, size_t t, size_t p) :
   m_family(family),
   m_M(M),
   m_t(t),
   m_p(p)
   {
   BOTAN_ARG_CHECK(m_p >= 1 && m_p <= 128, "Invalid Argon2 threads parameter");
   BOTAN_ARG_CHECK(m_M >= 8*m_p && m_M <= 8192*1024, "Invalid Argon2 M parameter");
   BOTAN_ARG_CHECK(m_t >= 1, "Invalid Argon2 t parameter");
   }

void Argon2::derive_key(uint8_t output[], size_t output_len,
                        const char* password, size_t password_len,
                        const uint8_t salt[], size_t salt_len) const
   {
   argon2(output, output_len,
          password, password_len,
          salt, salt_len,
          nullptr, 0,
          nullptr, 0);
   }

void Argon2::derive_key(uint8_t output[], size_t output_len,
                        const char* password, size_t password_len,
                        const uint8_t salt[], size_t salt_len,
                        const uint8_t ad[], size_t ad_len,
                        const uint8_t key[], size_t key_len) const
   {
   argon2(output, output_len,
          password, password_len,
          salt, salt_len,
          key, key_len,
          ad, ad_len);
   }

namespace {

std::string argon2_family_name(uint8_t f)
   {
   switch(f)
      {
      case 0:
         return "Argon2d";
      case 1:
         return "Argon2i";
      case 2:
         return "Argon2id";
      default:
         throw Invalid_Argument("Unknown Argon2 parameter");
      }
   }

}

std::string Argon2::to_string() const
   {
   return argon2_family_name(m_family) + "(" +
      std::to_string(m_M) + "," +
      std::to_string(m_t) + "," +
      std::to_string(m_p) + ")";
  }

Argon2_Family::Argon2_Family(uint8_t family) : m_family(family)
   {
   if(m_family != 0 && m_family != 1 && m_family != 2)
      throw Invalid_Argument("Unknown Argon2 family identifier");
   }

std::string Argon2_Family::name() const
   {
   return argon2_family_name(m_family);
   }

std::unique_ptr<PasswordHash> Argon2_Family::tune(size_t /*output_length*/,
                                                  std::chrono::milliseconds msec,
                                                  size_t max_memory) const
   {
   const size_t max_kib = (max_memory == 0) ? 256*1024 : max_memory*1024;

   // Tune with a large memory otherwise we measure cache vs RAM speeds and underestimate
   // costs for larger params. Default is 36 MiB, or use 128 for long times.
   const size_t tune_M = (msec >= std::chrono::milliseconds(500) ? 128 : 36) * 1024;
   const size_t p = 1;
   size_t t = 1;

   Timer timer("Argon2");
   const auto tune_time = BOTAN_PBKDF_TUNING_TIME;

   auto pwhash = this->from_params(tune_M, t, p);

   timer.run_until_elapsed(tune_time, [&]() {
      uint8_t output[64] = { 0 };
      pwhash->derive_key(output, sizeof(output),
                         "test", 4,
                         nullptr, 0);
      });

   if(timer.events() == 0 || timer.value() == 0)
      return default_params();

   size_t M = 4*1024;

   const uint64_t measured_time = timer.value() / (timer.events() * (tune_M / M));

   const uint64_t target_nsec = msec.count() * static_cast<uint64_t>(1000000);

   /*
   * Argon2 scaling rules:
   * k*M, k*t, k*p all increase cost by about k
   *
   * Since we don't even take advantage of p > 1, we prefer increasing
   * t or M instead.
   *
   * If possible to increase M, prefer that.
   */

   uint64_t est_nsec = measured_time;

   if(est_nsec < target_nsec && M < max_kib)
      {
      const uint64_t desired_cost_increase = (target_nsec + est_nsec - 1) / est_nsec;
      const uint64_t mem_headroom = max_kib / M;

      const uint64_t M_mult = std::min(desired_cost_increase, mem_headroom);
      M *= static_cast<size_t>(M_mult);
      est_nsec *= M_mult;
      }

   if(est_nsec < target_nsec)
      {
      const uint64_t desired_cost_increase = (target_nsec + est_nsec - 1) / est_nsec;
      t *= static_cast<size_t>(desired_cost_increase);
      }

   return this->from_params(M, t, p);
   }

std::unique_ptr<PasswordHash> Argon2_Family::default_params() const
   {
   return this->from_params(128*1024, 1, 1);
   }

std::unique_ptr<PasswordHash> Argon2_Family::from_iterations(size_t iter) const
   {
   /*
   These choices are arbitrary, but should not change in future
   releases since they will break applications expecting deterministic
   mapping from iteration count to params
   */
   const size_t M = iter;
   const size_t t = 1;
   const size_t p = 1;
   return this->from_params(M, t, p);
   }

std::unique_ptr<PasswordHash> Argon2_Family::from_params(size_t M, size_t t, size_t p) const
   {
   return std::make_unique<Argon2>(m_family, M, t, p);
   }

}
/**
* (C) 2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <sstream>

namespace Botan {

namespace {

std::string strip_padding(std::string s)
   {
   while(s.size() > 0 && s[s.size()-1] == '=')
      s.resize(s.size() - 1);
   return s;
   }

std::string argon2_family(uint8_t y)
   {
   if(y == 0)
      return "Argon2d";
   else if(y == 1)
      return "Argon2i";
   else if(y == 2)
      return "Argon2id";
   else
      throw Not_Implemented("Unknown Argon2 family type");
   }

}

std::string argon2_generate_pwhash(const char* password, size_t password_len,
                                   RandomNumberGenerator& rng,
                                   size_t p, size_t M, size_t t,
                                   uint8_t y, size_t salt_len, size_t output_len)
   {
   std::vector<uint8_t> salt(salt_len);
   rng.randomize(salt.data(), salt.size());

   std::vector<uint8_t> output(output_len);

   auto pwdhash_fam = PasswordHashFamily::create_or_throw(argon2_family(y));
   auto pwdhash = pwdhash_fam->from_params(M, t, p);

   pwdhash->derive_key(output.data(), output.size(),
                       password, password_len,
                       salt.data(), salt.size());

   std::ostringstream oss;

   if(y == 0)
      oss << "$argon2d$";
   else if(y == 1)
      oss << "$argon2i$";
   else
      oss << "$argon2id$";

   oss << "v=19$m=" << std::to_string(M)
       << ",t=" << std::to_string(t)
       << ",p=" << std::to_string(p) << "$";
   oss << strip_padding(base64_encode(salt)) << "$" << strip_padding(base64_encode(output));

   return oss.str();
   }

bool argon2_check_pwhash(const char* password, size_t password_len,
                         const std::string& input_hash)
   {
   const std::vector<std::string> parts = split_on(input_hash, '$');

   if(parts.size() != 5)
      return false;

   uint8_t family = 0;

   if(parts[0] == "argon2d")
      family = 0;
   else if(parts[0] == "argon2i")
      family = 1;
   else if(parts[0] == "argon2id")
      family = 2;
   else
      return false;

   if(parts[1] != "v=19")
      return false;

   const std::vector<std::string> params = split_on(parts[2], ',');

   if(params.size() != 3)
      return false;

   size_t M = 0, t = 0, p = 0;

   for(auto param_str : params)
      {
      const std::vector<std::string> param = split_on(param_str, '=');

      if(param.size() != 2)
         return false;

      const std::string key = param[0];
      const size_t val = to_u32bit(param[1]);
      if(key == "m")
         M = val;
      else if(key == "t")
         t = val;
      else if(key == "p")
         p = val;
      else
         return false;
      }

   std::vector<uint8_t> salt(base64_decode_max_output(parts[3].size()));
   salt.resize(base64_decode(salt.data(), parts[3], false));

   std::vector<uint8_t> hash(base64_decode_max_output(parts[4].size()));
   hash.resize(base64_decode(hash.data(), parts[4], false));

   if(hash.size() < 4)
      return false;

   std::vector<uint8_t> generated(hash.size());
   auto pwdhash_fam = PasswordHashFamily::create_or_throw(argon2_family(family));
   auto pwdhash = pwdhash_fam->from_params(M, t, p);

   pwdhash->derive_key(generated.data(), generated.size(),
                       password, password_len,
                       salt.data(), salt.size());

   return constant_time_compare(generated.data(), hash.data(), generated.size());
   }

}
/*
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_SYSTEM_RNG)
#endif

#if !defined(BOTAN_AUTO_RNG_HMAC)
#error "No hash function defined for AutoSeeded_RNG in build.h (try enabling sha2_32)"
#endif

namespace Botan {

AutoSeeded_RNG::~AutoSeeded_RNG()
   {
   // for unique_ptr
   }

AutoSeeded_RNG::AutoSeeded_RNG(RandomNumberGenerator& underlying_rng,
                               size_t reseed_interval)
   {
   m_rng.reset(new HMAC_DRBG(MessageAuthenticationCode::create_or_throw(BOTAN_AUTO_RNG_HMAC),
                             underlying_rng,
                             reseed_interval));
   force_reseed();
   }

AutoSeeded_RNG::AutoSeeded_RNG(Entropy_Sources& entropy_sources,
                               size_t reseed_interval)
   {
   m_rng.reset(new HMAC_DRBG(MessageAuthenticationCode::create_or_throw(BOTAN_AUTO_RNG_HMAC),
                             entropy_sources,
                             reseed_interval));
   force_reseed();
   }

AutoSeeded_RNG::AutoSeeded_RNG(RandomNumberGenerator& underlying_rng,
                               Entropy_Sources& entropy_sources,
                               size_t reseed_interval)
   {
   m_rng.reset(new HMAC_DRBG(
                  MessageAuthenticationCode::create_or_throw(BOTAN_AUTO_RNG_HMAC),
                  underlying_rng, entropy_sources, reseed_interval));
   force_reseed();
   }

AutoSeeded_RNG::AutoSeeded_RNG(size_t reseed_interval) :
#if defined(BOTAN_HAS_SYSTEM_RNG)
   AutoSeeded_RNG(system_rng(), reseed_interval)
#else
   AutoSeeded_RNG(Entropy_Sources::global_sources(), reseed_interval)
#endif
   {
   }

void AutoSeeded_RNG::force_reseed()
   {
   m_rng->force_reseed();
   m_rng->next_byte();

   if(!m_rng->is_seeded())
      {
      throw Internal_Error("AutoSeeded_RNG reseeding failed");
      }
   }

bool AutoSeeded_RNG::is_seeded() const
   {
   return m_rng->is_seeded();
   }

void AutoSeeded_RNG::clear()
   {
   m_rng->clear();
   }

std::string AutoSeeded_RNG::name() const
   {
   return m_rng->name();
   }

void AutoSeeded_RNG::add_entropy(const uint8_t in[], size_t len)
   {
   m_rng->add_entropy(in, len);
   }

size_t AutoSeeded_RNG::reseed(Entropy_Sources& srcs,
                              size_t poll_bits,
                              std::chrono::milliseconds poll_timeout)
   {
   return m_rng->reseed(srcs, poll_bits, poll_timeout);
   }

void AutoSeeded_RNG::randomize(uint8_t output[], size_t output_len)
   {
   m_rng->randomize_with_ts_input(output, output_len);
   }

void AutoSeeded_RNG::randomize_with_input(uint8_t output[], size_t output_len,
                                          const uint8_t ad[], size_t ad_len)
   {
   m_rng->randomize_with_input(output, output_len, ad, ad_len);
   }

}
/*
* (C) 2019 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void Buffered_Computation::update_be(uint16_t val)
   {
   uint8_t inb[sizeof(val)];
   store_be(val, inb);
   add_data(inb, sizeof(inb));
   }

void Buffered_Computation::update_be(uint32_t val)
   {
   uint8_t inb[sizeof(val)];
   store_be(val, inb);
   add_data(inb, sizeof(inb));
   }

void Buffered_Computation::update_be(uint64_t val)
   {
   uint8_t inb[sizeof(val)];
   store_be(val, inb);
   add_data(inb, sizeof(inb));
   }

void Buffered_Computation::update_le(uint16_t val)
   {
   uint8_t inb[sizeof(val)];
   store_le(val, inb);
   add_data(inb, sizeof(inb));
   }

void Buffered_Computation::update_le(uint32_t val)
   {
   uint8_t inb[sizeof(val)];
   store_le(val, inb);
   add_data(inb, sizeof(inb));
   }

void Buffered_Computation::update_le(uint64_t val)
   {
   uint8_t inb[sizeof(val)];
   store_le(val, inb);
   add_data(inb, sizeof(inb));
   }

}
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void SymmetricAlgorithm::throw_key_not_set_error() const
   {
   throw Key_Not_Set(name());
   }

void SymmetricAlgorithm::set_key(const uint8_t key[], size_t length)
   {
   if(!valid_keylength(length))
      throw Invalid_Key_Length(name(), length);
   key_schedule(key, length);
   }

}
/*
* OctetString
* (C) 1999-2007 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Create an OctetString from RNG output
*/
OctetString::OctetString(RandomNumberGenerator& rng,
                         size_t len)
   {
   rng.random_vec(m_data, len);
   }

/*
* Create an OctetString from a hex string
*/
OctetString::OctetString(const std::string& hex_string)
   {
   if(!hex_string.empty())
      {
      m_data.resize(1 + hex_string.length() / 2);
      m_data.resize(hex_decode(m_data.data(), hex_string));
      }
   }

/*
* Create an OctetString from a byte string
*/
OctetString::OctetString(const uint8_t in[], size_t n)
   {
   m_data.assign(in, in + n);
   }

namespace {

uint8_t odd_parity_of(uint8_t x)
   {
   uint8_t f = x | 0x01;
   f ^= (f >> 4);
   f ^= (f >> 2);
   f ^= (f >> 1);

   return (x & 0xFE) ^ (f & 0x01);
   }

}

/*
* Set the parity of each key byte to odd
*/
void OctetString::set_odd_parity()
   {
   for(size_t j = 0; j != m_data.size(); ++j)
      m_data[j] = odd_parity_of(m_data[j]);
   }

/*
* Hex encode an OctetString
*/
std::string OctetString::to_string() const
   {
   return hex_encode(m_data.data(), m_data.size());
   }

/*
* XOR Operation for OctetStrings
*/
OctetString& OctetString::operator^=(const OctetString& k)
   {
   if(&k == this) { zeroise(m_data); return (*this); }
   xor_buf(m_data.data(), k.begin(), std::min(length(), k.length()));
   return (*this);
   }

/*
* Equality Operation for OctetStrings
*/
bool operator==(const OctetString& s1, const OctetString& s2)
   {
   return (s1.bits_of() == s2.bits_of());
   }

/*
* Unequality Operation for OctetStrings
*/
bool operator!=(const OctetString& s1, const OctetString& s2)
   {
   return !(s1 == s2);
   }

/*
* Append Operation for OctetStrings
*/
OctetString operator+(const OctetString& k1, const OctetString& k2)
   {
   secure_vector<uint8_t> out;
   out += k1.bits_of();
   out += k2.bits_of();
   return OctetString(out);
   }

/*
* XOR Operation for OctetStrings
*/
OctetString operator^(const OctetString& k1, const OctetString& k2)
   {
   secure_vector<uint8_t> out(std::max(k1.length(), k2.length()));

   copy_mem(out.data(), k1.begin(), k1.length());
   xor_buf(out.data(), k2.begin(), k2.length());
   return OctetString(out);
   }

}
/*
* Base64 Encoding and Decoding
* (C) 2010,2015,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

class Base64 final
   {
   public:
      static inline std::string name() noexcept
         {
         return "base64";
         }

      static inline size_t encoding_bytes_in() noexcept
         {
         return m_encoding_bytes_in;
         }
      static inline size_t encoding_bytes_out() noexcept
         {
         return m_encoding_bytes_out;
         }

      static inline size_t decoding_bytes_in() noexcept
         {
         return m_encoding_bytes_out;
         }
      static inline size_t decoding_bytes_out() noexcept
         {
         return m_encoding_bytes_in;
         }

      static inline size_t bits_consumed() noexcept
         {
         return m_encoding_bits;
         }
      static inline size_t remaining_bits_before_padding() noexcept
         {
         return m_remaining_bits_before_padding;
         }

      static inline size_t encode_max_output(size_t input_length)
         {
         return (round_up(input_length, m_encoding_bytes_in) / m_encoding_bytes_in) * m_encoding_bytes_out;
         }
      static inline size_t decode_max_output(size_t input_length)
         {
         return (round_up(input_length, m_encoding_bytes_out) * m_encoding_bytes_in) / m_encoding_bytes_out;
         }

      static void encode(char out[8], const uint8_t in[5]) noexcept;

      static uint8_t lookup_binary_value(char input) noexcept;

      static bool check_bad_char(uint8_t bin, char input, bool ignore_ws);

      static void decode(uint8_t* out_ptr, const uint8_t decode_buf[4])
         {
         out_ptr[0] = (decode_buf[0] << 2) | (decode_buf[1] >> 4);
         out_ptr[1] = (decode_buf[1] << 4) | (decode_buf[2] >> 2);
         out_ptr[2] = (decode_buf[2] << 6) | decode_buf[3];
         }

      static inline size_t bytes_to_remove(size_t final_truncate)
         {
         return final_truncate;
         }

   private:
      static const size_t m_encoding_bits = 6;
      static const size_t m_remaining_bits_before_padding = 8;

      static const size_t m_encoding_bytes_in = 3;
      static const size_t m_encoding_bytes_out = 4;
   };

char lookup_base64_char(uint8_t x)
   {
   BOTAN_DEBUG_ASSERT(x < 64);

   const auto in_az = CT::Mask<uint8_t>::is_within_range(x, 26, 51);
   const auto in_09 = CT::Mask<uint8_t>::is_within_range(x, 52, 61);
   const auto eq_plus = CT::Mask<uint8_t>::is_equal(x, 62);
   const auto eq_slash = CT::Mask<uint8_t>::is_equal(x, 63);

   const char c_AZ = 'A' + x;
   const char c_az = 'a' + (x - 26);
   const char c_09 = '0' + (x - 2*26);
   const char c_plus = '+';
   const char c_slash = '/';

   char ret = c_AZ;
   ret = in_az.select(c_az, ret);
   ret = in_09.select(c_09, ret);
   ret = eq_plus.select(c_plus, ret);
   ret = eq_slash.select(c_slash, ret);

   return ret;
   }

//static
void Base64::encode(char out[8], const uint8_t in[5]) noexcept
   {
   const uint8_t b0 = (in[0] & 0xFC) >> 2;
   const uint8_t b1 = ((in[0] & 0x03) << 4) | (in[1] >> 4);
   const uint8_t b2 = ((in[1] & 0x0F) << 2) | (in[2] >> 6);
   const uint8_t b3 = in[2] & 0x3F;
   out[0] = lookup_base64_char(b0);
   out[1] = lookup_base64_char(b1);
   out[2] = lookup_base64_char(b2);
   out[3] = lookup_base64_char(b3);
   }

//static
uint8_t Base64::lookup_binary_value(char input) noexcept
   {
   const uint8_t c = static_cast<uint8_t>(input);

   const auto is_alpha_upper = CT::Mask<uint8_t>::is_within_range(c, uint8_t('A'), uint8_t('Z'));
   const auto is_alpha_lower = CT::Mask<uint8_t>::is_within_range(c, uint8_t('a'), uint8_t('z'));
   const auto is_decimal     = CT::Mask<uint8_t>::is_within_range(c, uint8_t('0'), uint8_t('9'));

   const auto is_plus        = CT::Mask<uint8_t>::is_equal(c, uint8_t('+'));
   const auto is_slash       = CT::Mask<uint8_t>::is_equal(c, uint8_t('/'));
   const auto is_equal       = CT::Mask<uint8_t>::is_equal(c, uint8_t('='));

   const auto is_whitespace  = CT::Mask<uint8_t>::is_any_of(c, {
         uint8_t(' '), uint8_t('\t'), uint8_t('\n'), uint8_t('\r')
      });

   const uint8_t c_upper = c - uint8_t('A');
   const uint8_t c_lower = c - uint8_t('a') + 26;
   const uint8_t c_decim = c - uint8_t('0') + 2*26;

   uint8_t ret = 0xFF; // default value

   ret = is_alpha_upper.select(c_upper, ret);
   ret = is_alpha_lower.select(c_lower, ret);
   ret = is_decimal.select(c_decim, ret);
   ret = is_plus.select(62, ret);
   ret = is_slash.select(63, ret);
   ret = is_equal.select(0x81, ret);
   ret = is_whitespace.select(0x80, ret);

   return ret;
   }

//static
bool Base64::check_bad_char(uint8_t bin, char input, bool ignore_ws)
   {
   if(bin <= 0x3F)
      {
      return true;
      }
   else if(!(bin == 0x81 || (bin == 0x80 && ignore_ws)))
      {
      std::string bad_char(1, input);
      if(bad_char == "\t")
         { bad_char = "\\t"; }
      else if(bad_char == "\n")
         { bad_char = "\\n"; }
      else if(bad_char == "\r")
         { bad_char = "\\r"; }

      throw Invalid_Argument(
         std::string("base64_decode: invalid base64 character '") +
         bad_char + "'");
      }
   return false;
   }

}

size_t base64_encode(char out[],
                     const uint8_t in[],
                     size_t input_length,
                     size_t& input_consumed,
                     bool final_inputs)
   {
   return base_encode(Base64(), out, in, input_length, input_consumed, final_inputs);
   }

std::string base64_encode(const uint8_t input[],
                          size_t input_length)
   {
   return base_encode_to_string(Base64(), input, input_length);
   }

size_t base64_decode(uint8_t out[],
                     const char in[],
                     size_t input_length,
                     size_t& input_consumed,
                     bool final_inputs,
                     bool ignore_ws)
   {
   return base_decode(Base64(), out, in, input_length, input_consumed, final_inputs, ignore_ws);
   }

size_t base64_decode(uint8_t output[],
                     const char input[],
                     size_t input_length,
                     bool ignore_ws)
   {
   return base_decode_full(Base64(), output, input, input_length, ignore_ws);
   }

size_t base64_decode(uint8_t output[],
                     const std::string& input,
                     bool ignore_ws)
   {
   return base64_decode(output, input.data(), input.length(), ignore_ws);
   }

secure_vector<uint8_t> base64_decode(const char input[],
                                     size_t input_length,
                                     bool ignore_ws)
   {
   return base_decode_to_vec<secure_vector<uint8_t>>(Base64(), input, input_length, ignore_ws);
   }

secure_vector<uint8_t> base64_decode(const std::string& input,
                                     bool ignore_ws)
   {
   return base64_decode(input.data(), input.size(), ignore_ws);
   }

size_t base64_encode_max_output(size_t input_length)
   {
   return Base64::encode_max_output(input_length);
   }

size_t base64_decode_max_output(size_t input_length)
   {
   return Base64::decode_max_output(input_length);
   }

}
/*
* BLAKE2b
* (C) 2016 cynecx
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

enum blake2b_constant {
  BLAKE2B_BLOCKBYTES = 128,
  BLAKE2B_IVU64COUNT = 8
};

const uint64_t blake2b_IV[BLAKE2B_IVU64COUNT] = {
   0x6a09e667f3bcc908, 0xbb67ae8584caa73b,
   0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
   0x510e527fade682d1, 0x9b05688c2b3e6c1f,
   0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};

}

BLAKE2b::BLAKE2b(size_t output_bits) :
   m_output_bits(output_bits),
   m_buffer(BLAKE2B_BLOCKBYTES),
   m_bufpos(0),
   m_H(BLAKE2B_IVU64COUNT),
   m_key_size(0)
   {
   if(output_bits == 0 || output_bits > 512 || output_bits % 8 != 0)
      {
      throw Invalid_Argument("Bad output bits size for BLAKE2b");
      }

   state_init();
   }

void BLAKE2b::state_init()
   {
   copy_mem(m_H.data(), blake2b_IV, BLAKE2B_IVU64COUNT);
   m_H[0] ^= (0x01010000 | (static_cast<uint8_t>(m_key_size) << 8) | static_cast<uint8_t>(output_length()));
   m_T[0] = m_T[1] = 0;
   m_F[0] = m_F[1] = 0;

   if(m_key_size == 0)
      {
      m_bufpos = 0;
      }
   else
      {
      BOTAN_ASSERT_NOMSG(m_padded_key_buffer.size() == m_buffer.size());
      copy_mem(m_buffer.data(), m_padded_key_buffer.data(), m_padded_key_buffer.size());
      m_bufpos = m_padded_key_buffer.size();
      }
   }

namespace {

BOTAN_FORCE_INLINE void G(uint64_t& a, uint64_t& b, uint64_t& c, uint64_t& d,
                          uint64_t M0, uint64_t M1)
   {
   a = a + b + M0;
   d = rotr<32>(d ^ a);
   c = c + d;
   b = rotr<24>(b ^ c);
   a = a + b + M1;
   d = rotr<16>(d ^ a);
   c = c + d;
   b = rotr<63>(b ^ c);
   }

template<size_t i0, size_t i1, size_t i2, size_t i3, size_t i4, size_t i5, size_t i6, size_t i7,
         size_t i8, size_t i9, size_t iA, size_t iB, size_t iC, size_t iD, size_t iE, size_t iF>
BOTAN_FORCE_INLINE void ROUND(uint64_t* v, const uint64_t* M)
   {
   G(v[ 0], v[ 4], v[ 8], v[12], M[i0], M[i1]);
   G(v[ 1], v[ 5], v[ 9], v[13], M[i2], M[i3]);
   G(v[ 2], v[ 6], v[10], v[14], M[i4], M[i5]);
   G(v[ 3], v[ 7], v[11], v[15], M[i6], M[i7]);
   G(v[ 0], v[ 5], v[10], v[15], M[i8], M[i9]);
   G(v[ 1], v[ 6], v[11], v[12], M[iA], M[iB]);
   G(v[ 2], v[ 7], v[ 8], v[13], M[iC], M[iD]);
   G(v[ 3], v[ 4], v[ 9], v[14], M[iE], M[iF]);
   }


}

void BLAKE2b::compress(const uint8_t* input, size_t blocks, uint64_t increment)
   {
   for(size_t b = 0; b != blocks; ++b)
      {
      m_T[0] += increment;
      if(m_T[0] < increment)
         {
         m_T[1]++;
         }

      uint64_t M[16];
      uint64_t v[16];
      load_le(M, input, 16);

      input += BLAKE2B_BLOCKBYTES;

      for(size_t i = 0; i < 8; i++)
         v[i] = m_H[i];
      for(size_t i = 0; i != 8; ++i)
         v[i + 8] = blake2b_IV[i];

      v[12] ^= m_T[0];
      v[13] ^= m_T[1];
      v[14] ^= m_F[0];
      v[15] ^= m_F[1];

      ROUND< 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15>(v, M);
      ROUND<14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3>(v, M);
      ROUND<11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4>(v, M);
      ROUND< 7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8>(v, M);
      ROUND< 9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13>(v, M);
      ROUND< 2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9>(v, M);
      ROUND<12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11>(v, M);
      ROUND<13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10>(v, M);
      ROUND< 6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5>(v, M);
      ROUND<10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13,  0>(v, M);
      ROUND< 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15>(v, M);
      ROUND<14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3>(v, M);

      for(size_t i = 0; i < 8; i++)
         {
         m_H[i] ^= v[i] ^ v[i + 8];
         }
      }
   }

void BLAKE2b::add_data(const uint8_t input[], size_t length)
   {
   if(length == 0)
      return;

   if(m_bufpos > 0)
      {
      if(m_bufpos < BLAKE2B_BLOCKBYTES)
         {
         const size_t take = std::min(BLAKE2B_BLOCKBYTES - m_bufpos, length);
         copy_mem(&m_buffer[m_bufpos], input, take);
         m_bufpos += take;
         length -= take;
         input += take;
         }

      if(m_bufpos == m_buffer.size() && length > 0)
         {
         compress(m_buffer.data(), 1, BLAKE2B_BLOCKBYTES);
         m_bufpos = 0;
         }
      }

   if(length > BLAKE2B_BLOCKBYTES)
      {
      const size_t full_blocks = ((length-1) / BLAKE2B_BLOCKBYTES);
      compress(input, full_blocks, BLAKE2B_BLOCKBYTES);

      input += full_blocks * BLAKE2B_BLOCKBYTES;
      length -= full_blocks * BLAKE2B_BLOCKBYTES;
      }

   if(length > 0)
      {
      copy_mem(&m_buffer[m_bufpos], input, length);
      m_bufpos += length;
      }
   }

void BLAKE2b::final_result(uint8_t output[])
   {
   if(m_bufpos != BLAKE2B_BLOCKBYTES)
      clear_mem(&m_buffer[m_bufpos], BLAKE2B_BLOCKBYTES - m_bufpos);
   m_F[0] = 0xFFFFFFFFFFFFFFFF;
   compress(m_buffer.data(), 1, m_bufpos);
   copy_out_vec_le(output, output_length(), m_H);
   state_init();
   }

Key_Length_Specification BLAKE2b::key_spec() const
   {
   return Key_Length_Specification(0, 64);
   }

std::string BLAKE2b::name() const
   {
   return "BLAKE2b(" + std::to_string(m_output_bits) + ")";
   }

std::unique_ptr<HashFunction> BLAKE2b::new_object() const
   {
   return std::make_unique<BLAKE2b>(m_output_bits);
   }

std::unique_ptr<HashFunction> BLAKE2b::copy_state() const
   {
   return std::make_unique<BLAKE2b>(*this);
   }

void BLAKE2b::key_schedule(const uint8_t key[], size_t length)
   {
   BOTAN_ASSERT_NOMSG(length <= m_buffer.size());

   m_key_size = length;
   m_padded_key_buffer.resize(m_buffer.size());

   if(m_padded_key_buffer.size() > length)
      {
      size_t padding = m_padded_key_buffer.size() - length;
      clear_mem(m_padded_key_buffer.data() + length, padding);
      }

   copy_mem(m_padded_key_buffer.data(), key, length);
   state_init();
   }

void BLAKE2b::clear()
   {
   zeroise(m_H);
   zeroise(m_buffer);
   zeroise(m_padded_key_buffer);
   m_bufpos = 0;
   m_key_size = 0;
   state_init();
   }

}
/*
* Runtime CPU detection
* (C) 2009,2010,2013,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <ostream>

namespace Botan {

bool CPUID::has_simd_32()
   {
#if defined(BOTAN_TARGET_SUPPORTS_SSE2)
   return CPUID::has_sse2();
#elif defined(BOTAN_TARGET_SUPPORTS_ALTIVEC)
   return CPUID::has_altivec();
#elif defined(BOTAN_TARGET_SUPPORTS_NEON)
   return CPUID::has_neon();
#else
   return true;
#endif
   }

//static
std::string CPUID::to_string()
   {
   std::vector<std::string> flags;

#define CPUID_PRINT(flag) do { if(has_##flag()) { flags.push_back(#flag); } } while(0)

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
   CPUID_PRINT(sse2);
   CPUID_PRINT(ssse3);
   CPUID_PRINT(sse41);
   CPUID_PRINT(sse42);
   CPUID_PRINT(avx2);
   CPUID_PRINT(avx512f);
   CPUID_PRINT(avx512dq);
   CPUID_PRINT(avx512bw);
   CPUID_PRINT(avx512_icelake);

   CPUID_PRINT(rdtsc);
   CPUID_PRINT(bmi1);
   CPUID_PRINT(bmi2);
   CPUID_PRINT(adx);

   CPUID_PRINT(aes_ni);
   CPUID_PRINT(clmul);
   CPUID_PRINT(rdrand);
   CPUID_PRINT(rdseed);
   CPUID_PRINT(intel_sha);
   CPUID_PRINT(avx512_aes);
   CPUID_PRINT(avx512_clmul);
#endif

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
   CPUID_PRINT(altivec);
   CPUID_PRINT(power_crypto);
   CPUID_PRINT(darn_rng);
#endif

#if defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
   CPUID_PRINT(neon);
   CPUID_PRINT(arm_sve);

   CPUID_PRINT(arm_sha1);
   CPUID_PRINT(arm_sha2);
   CPUID_PRINT(arm_aes);
   CPUID_PRINT(arm_pmull);
   CPUID_PRINT(arm_sha2_512);
   CPUID_PRINT(arm_sha3);
   CPUID_PRINT(arm_sm3);
   CPUID_PRINT(arm_sm4);
#endif

#undef CPUID_PRINT

   return string_join(flags, ' ');
   }

//static
void CPUID::initialize()
   {
   state() = CPUID_Data();
   }

CPUID::CPUID_Data::CPUID_Data()
   {
   m_cache_line_size = 0;
   m_processor_features = 0;

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY) || \
    defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY) || \
    defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

   m_processor_features = detect_cpu_features(&m_cache_line_size);

#endif

   m_processor_features |= CPUID::CPUID_INITIALIZED_BIT;

   if(m_cache_line_size == 0)
      {
      m_cache_line_size = OS::get_cache_line_size();
      if(m_cache_line_size == 0)
         m_cache_line_size = BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE;
      }

   m_endian_status = runtime_check_endian();
   }

//static
CPUID::Endian_Status CPUID::CPUID_Data::runtime_check_endian()
   {
   // Check runtime endian
   const uint32_t endian32 = 0x01234567;
   const uint8_t* e8 = reinterpret_cast<const uint8_t*>(&endian32);

   CPUID::Endian_Status endian = CPUID::Endian_Status::Unknown;

   if(e8[0] == 0x01 && e8[1] == 0x23 && e8[2] == 0x45 && e8[3] == 0x67)
      {
      endian = CPUID::Endian_Status::Big;
      }
   else if(e8[0] == 0x67 && e8[1] == 0x45 && e8[2] == 0x23 && e8[3] == 0x01)
      {
      endian = CPUID::Endian_Status::Little;
      }
   else
      {
      throw Internal_Error("Unexpected endian at runtime, neither big nor little");
      }

   // If we were compiled with a known endian, verify it matches at runtime
#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
   BOTAN_ASSERT(endian == CPUID::Endian_Status::Little, "Build and runtime endian match");
#elif defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
   BOTAN_ASSERT(endian == CPUID::Endian_Status::Big, "Build and runtime endian match");
#endif

   return endian;
   }

std::vector<Botan::CPUID::CPUID_bits>
CPUID::bit_from_string(const std::string& tok)
   {
#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
   if(tok == "sse2" || tok == "simd")
      return {Botan::CPUID::CPUID_SSE2_BIT};
   if(tok == "ssse3")
      return {Botan::CPUID::CPUID_SSSE3_BIT};
   if(tok == "sse41")
      return {Botan::CPUID::CPUID_SSE41_BIT};
   if(tok == "sse42")
      return {Botan::CPUID::CPUID_SSE42_BIT};
   // aes_ni is the string printed on the console when running "botan cpuid"
   if(tok == "aesni" || tok == "aes_ni")
      return {Botan::CPUID::CPUID_AESNI_BIT};
   if(tok == "clmul")
      return {Botan::CPUID::CPUID_CLMUL_BIT};
   if(tok == "avx2")
      return {Botan::CPUID::CPUID_AVX2_BIT};
   if(tok == "avx512f")
      return {Botan::CPUID::CPUID_AVX512F_BIT};
   if(tok == "avx512_icelake")
      return {Botan::CPUID::CPUID_AVX512_ICL_BIT};
   // there were two if statements testing "sha" and "intel_sha" separately; combined
   if(tok == "sha" || tok=="intel_sha")
      return {Botan::CPUID::CPUID_SHA_BIT};
   if(tok == "rdtsc")
      return {Botan::CPUID::CPUID_RDTSC_BIT};
   if(tok == "bmi1")
      return {Botan::CPUID::CPUID_BMI1_BIT};
   if(tok == "bmi2")
      return {Botan::CPUID::CPUID_BMI2_BIT};
   if(tok == "adx")
      return {Botan::CPUID::CPUID_ADX_BIT};
   if(tok == "rdrand")
      return {Botan::CPUID::CPUID_RDRAND_BIT};
   if(tok == "rdseed")
      return {Botan::CPUID::CPUID_RDSEED_BIT};
   if(tok == "avx512_aes")
      return {Botan::CPUID::CPUID_AVX512_AES_BIT};
   if(tok == "avx512_clmul")
      return {Botan::CPUID::CPUID_AVX512_CLMUL_BIT};

#elif defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
   if(tok == "altivec" || tok == "simd")
      return {Botan::CPUID::CPUID_ALTIVEC_BIT};
   if(tok == "power_crypto")
      return {Botan::CPUID::CPUID_POWER_CRYPTO_BIT};
   if(tok == "darn_rng")
      return {Botan::CPUID::CPUID_DARN_BIT};

#elif defined(BOTAN_TARGET_CPU_IS_ARM_FAMILY)
   if(tok == "neon" || tok == "simd")
      return {Botan::CPUID::CPUID_ARM_NEON_BIT};
   if(tok == "arm_sve")
      return {Botan::CPUID::CPUID_ARM_SVE_BIT};
   if(tok == "armv8sha1" || tok == "arm_sha1")
      return {Botan::CPUID::CPUID_ARM_SHA1_BIT};
   if(tok == "armv8sha2" || tok == "arm_sha2")
      return {Botan::CPUID::CPUID_ARM_SHA2_BIT};
   if(tok == "armv8aes" || tok == "arm_aes")
      return {Botan::CPUID::CPUID_ARM_AES_BIT};
   if(tok == "armv8pmull" || tok == "arm_pmull")
      return {Botan::CPUID::CPUID_ARM_PMULL_BIT};
   if(tok == "armv8sha3" || tok == "arm_sha3")
      return {Botan::CPUID::CPUID_ARM_SHA3_BIT};
   if(tok == "armv8sha2_512" || tok == "arm_sha2_512")
      return {Botan::CPUID::CPUID_ARM_SHA2_512_BIT};
   if(tok == "armv8sm3" || tok == "arm_sm3")
      return {Botan::CPUID::CPUID_ARM_SM3_BIT};
   if(tok == "armv8sm4" || tok == "arm_sm4")
      return {Botan::CPUID::CPUID_ARM_SM4_BIT};

#else
   BOTAN_UNUSED(tok);
#endif

   return {};
   }

}
/*
* Runtime CPU detection for Aarch64
* (C) 2009,2010,2013,2017,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_ARCH_IS_ARM64)


#if defined(BOTAN_TARGET_OS_IS_IOS) || defined(BOTAN_TARGET_OS_IS_MACOS)
  #include <sys/types.h>
  #include <sys/sysctl.h>
#endif

namespace Botan {

#if defined(BOTAN_TARGET_OS_IS_MACOS)
namespace {

bool sysctlbyname_has_feature(const char* feature_name)
   {
   unsigned int feature;
   size_t size = sizeof(feature);
   ::sysctlbyname(feature_name, &feature, &size, nullptr, 0);
   return (feature == 1);
   }

}
#endif

uint64_t CPUID::CPUID_Data::detect_cpu_features(size_t* cache_line_size)
   {
   BOTAN_UNUSED(cache_line_size);

   uint64_t detected_features = 0;

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL) || defined(BOTAN_TARGET_OS_HAS_ELF_AUX_INFO)
   /*
   * On systems with getauxval these bits should normally be defined
   * in bits/auxv.h but some buggy? glibc installs seem to miss them.
   * These following values are all fixed, for the Linux ELF format,
   * so we just hardcode them in ARM_hwcap_bit enum.
   */

   enum ARM_hwcap_bit {
      NEON_bit  = (1 << 1),
      AES_bit   = (1 << 3),
      PMULL_bit = (1 << 4),
      SHA1_bit  = (1 << 5),
      SHA2_bit  = (1 << 6),
      SHA3_bit  = (1 << 17),
      SM3_bit  = (1 << 18),
      SM4_bit  = (1 << 19),
      SHA2_512_bit = (1 << 21),
      SVE_bit = (1 << 22),

      ARCH_hwcap = 16, // AT_HWCAP
   };

   const unsigned long hwcap = OS::get_auxval(ARM_hwcap_bit::ARCH_hwcap);
   if(hwcap & ARM_hwcap_bit::NEON_bit)
      {
      detected_features |= CPUID::CPUID_ARM_NEON_BIT;
      if(hwcap & ARM_hwcap_bit::AES_bit)
         detected_features |= CPUID::CPUID_ARM_AES_BIT;
      if(hwcap & ARM_hwcap_bit::PMULL_bit)
         detected_features |= CPUID::CPUID_ARM_PMULL_BIT;
      if(hwcap & ARM_hwcap_bit::SHA1_bit)
         detected_features |= CPUID::CPUID_ARM_SHA1_BIT;
      if(hwcap & ARM_hwcap_bit::SHA2_bit)
         detected_features |= CPUID::CPUID_ARM_SHA2_BIT;
      if(hwcap & ARM_hwcap_bit::SHA3_bit)
         detected_features |= CPUID::CPUID_ARM_SHA3_BIT;
      if(hwcap & ARM_hwcap_bit::SM3_bit)
         detected_features |= CPUID::CPUID_ARM_SM3_BIT;
      if(hwcap & ARM_hwcap_bit::SM4_bit)
         detected_features |= CPUID::CPUID_ARM_SM4_BIT;
      if(hwcap & ARM_hwcap_bit::SHA2_512_bit)
         detected_features |= CPUID::CPUID_ARM_SHA2_512_BIT;
      if(hwcap & ARM_hwcap_bit::SVE_bit)
         detected_features |= CPUID::CPUID_ARM_SVE_BIT;
      }

#elif defined(BOTAN_TARGET_OS_IS_IOS) || defined(BOTAN_TARGET_OS_IS_MACOS)

   // All 64-bit Apple ARM chips have NEON, AES, and SHA support
   detected_features |= CPUID::CPUID_ARM_NEON_BIT;
   detected_features |= CPUID::CPUID_ARM_AES_BIT;
   detected_features |= CPUID::CPUID_ARM_PMULL_BIT;
   detected_features |= CPUID::CPUID_ARM_SHA1_BIT;
   detected_features |= CPUID::CPUID_ARM_SHA2_BIT;

#if defined(BOTAN_TARGET_OS_IS_MACOS)
   if(sysctlbyname_has_feature("hw.optional.armv8_2_sha3"))
     detected_features |= CPUID::CPUID_ARM_SHA3_BIT;
   if(sysctlbyname_has_feature("hw.optional.armv8_2_sha512"))
      detected_features |= CPUID::CPUID_ARM_SHA2_512_BIT;
#endif

#elif defined(BOTAN_USE_GCC_INLINE_ASM)

   /*
   No getauxval API available, fall back on probe functions. We only
   bother with Aarch64 here to simplify the code and because going to
   extreme contortions to detect NEON on devices that probably don't
   support it doesn't seem worthwhile.

   NEON registers v0-v7 are caller saved in Aarch64
   */

   auto neon_probe  = []() noexcept -> int { asm("and v0.16b, v0.16b, v0.16b"); return 1; };
   auto aes_probe   = []() noexcept -> int { asm(".word 0x4e284800"); return 1; };
   auto pmull_probe = []() noexcept -> int { asm(".word 0x0ee0e000"); return 1; };
   auto sha1_probe  = []() noexcept -> int { asm(".word 0x5e280800"); return 1; };
   auto sha2_probe  = []() noexcept -> int { asm(".word 0x5e282800"); return 1; };
   auto sha512_probe = []() noexcept -> int { asm(".long 0xcec08000"); return 1; };

   // Only bother running the crypto detection if we found NEON

   if(OS::run_cpu_instruction_probe(neon_probe) == 1)
      {
      detected_features |= CPUID::CPUID_ARM_NEON_BIT;

      if(OS::run_cpu_instruction_probe(aes_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_AES_BIT;
      if(OS::run_cpu_instruction_probe(pmull_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_PMULL_BIT;
      if(OS::run_cpu_instruction_probe(sha1_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_SHA1_BIT;
      if(OS::run_cpu_instruction_probe(sha2_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_SHA2_BIT;
      if(OS::run_cpu_instruction_probe(sha512_probe) == 1)
         detected_features |= CPUID::CPUID_ARM_SHA2_512_BIT;
      }

#endif

   return detected_features;
   }

}

#endif
/*
* Runtime CPU detection for 32-bit ARM
* (C) 2009,2010,2013,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_ARCH_IS_ARM32)


namespace Botan {

uint64_t CPUID::CPUID_Data::detect_cpu_features(size_t* cache_line_size)
   {
   BOTAN_UNUSED(cache_line_size);

   uint64_t detected_features = 0;

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL) || defined(BOTAN_TARGET_OS_HAS_ELF_AUX_INFO)
   /*
   * On systems with getauxval these bits should normally be defined
   * in bits/auxv.h but some buggy? glibc installs seem to miss them.
   * These following values are all fixed, for the Linux ELF format,
   * so we just hardcode them in ARM_hwcap_bit enum.
   */

   enum ARM_hwcap_bit {
      NEON_bit  = (1 << 12),
      AES_bit   = (1 << 0),
      PMULL_bit = (1 << 1),
      SHA1_bit  = (1 << 2),
      SHA2_bit  = (1 << 3),

      ARCH_hwcap_neon   = 16, // AT_HWCAP
      ARCH_hwcap_crypto = 26, // AT_HWCAP2
   };

   const unsigned long hwcap_neon = OS::get_auxval(ARM_hwcap_bit::ARCH_hwcap_neon);
   if(hwcap_neon & ARM_hwcap_bit::NEON_bit)
      {
      detected_features |= CPUID::CPUID_ARM_NEON_BIT;

      const unsigned long hwcap_crypto = OS::get_auxval(ARM_hwcap_bit::ARCH_hwcap_crypto);
      if(hwcap_crypto & ARM_hwcap_bit::AES_bit)
         detected_features |= CPUID::CPUID_ARM_AES_BIT;
      if(hwcap_crypto & ARM_hwcap_bit::PMULL_bit)
         detected_features |= CPUID::CPUID_ARM_PMULL_BIT;
      if(hwcap_crypto & ARM_hwcap_bit::SHA1_bit)
         detected_features |= CPUID::CPUID_ARM_SHA1_BIT;
      if(hwcap_crypto & ARM_hwcap_bit::SHA2_bit)
         detected_features |= CPUID::CPUID_ARM_SHA2_BIT;
      }
#endif

   return detected_features;
   }

}

#endif
/*
* Runtime CPU detection for POWER/PowerPC
* (C) 2009,2010,2013,2017,2021 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)

namespace Botan {

uint64_t CPUID::CPUID_Data::detect_cpu_features(size_t* cache_line_size)
   {
   BOTAN_UNUSED(cache_line_size);

   uint64_t detected_features = 0;

#if (defined(BOTAN_TARGET_OS_HAS_GETAUXVAL) || defined(BOTAN_TARGET_HAS_ELF_AUX_INFO)) && defined(BOTAN_TARGET_ARCH_IS_PPC64)

   enum PPC_hwcap_bit {
      ALTIVEC_bit  = (1 << 28),
      CRYPTO_bit   = (1 << 25),
      DARN_bit     = (1 << 21),

      ARCH_hwcap_altivec = 16, // AT_HWCAP
      ARCH_hwcap_crypto  = 26, // AT_HWCAP2
   };

   const unsigned long hwcap_altivec = OS::get_auxval(PPC_hwcap_bit::ARCH_hwcap_altivec);
   if(hwcap_altivec & PPC_hwcap_bit::ALTIVEC_bit)
      {
      detected_features |= CPUID::CPUID_ALTIVEC_BIT;

      const unsigned long hwcap_crypto = OS::get_auxval(PPC_hwcap_bit::ARCH_hwcap_crypto);
      if(hwcap_crypto & PPC_hwcap_bit::CRYPTO_bit)
         detected_features |= CPUID::CPUID_POWER_CRYPTO_BIT;
      if(hwcap_crypto & PPC_hwcap_bit::DARN_bit)
         detected_features |= CPUID::CPUID_DARN_BIT;
      }

#else

   auto vmx_probe = []() noexcept -> int { asm("vor 0, 0, 0"); return 1; };

   if(OS::run_cpu_instruction_probe(vmx_probe) == 1)
      {
      detected_features |= CPUID::CPUID_ALTIVEC_BIT;

      #if defined(BOTAN_TARGET_ARCH_IS_PPC64)
      auto vcipher_probe = []() noexcept -> int { asm("vcipher 0, 0, 0"); return 1; };

      if(OS::run_cpu_instruction_probe(vcipher_probe) == 1)
         detected_features |= CPUID::CPUID_POWER_CRYPTO_BIT;

      auto darn_probe = []() noexcept -> int {
         uint64_t output = 0;
         asm volatile("darn %0, 1" : "=r" (output));
         return (~output) != 0;
      };

      if(OS::run_cpu_instruction_probe(darn_probe) == 1)
         detected_features |= CPUID::CPUID_DARN_BIT;
      #endif
      }

#endif

   return detected_features;
   }

}

#endif
/*
* Runtime CPU detection for x86
* (C) 2009,2010,2013,2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

#if defined(BOTAN_BUILD_COMPILER_IS_MSVC)
  #include <intrin.h>
#elif defined(BOTAN_BUILD_COMPILER_IS_INTEL)
  #include <ia32intrin.h>
#elif defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)
  #include <cpuid.h>
#endif

#endif

namespace Botan {

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

namespace {

void invoke_cpuid(uint32_t type, uint32_t out[4])
   {
#if defined(BOTAN_BUILD_COMPILER_IS_MSVC) || defined(BOTAN_BUILD_COMPILER_IS_INTEL)
   __cpuid((int*)out, type);

#elif defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)
   __get_cpuid(type, out, out+1, out+2, out+3);

#elif defined(BOTAN_USE_GCC_INLINE_ASM)
   asm("cpuid\n\t"
       : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3])
       : "0" (type));

#else
   #warning "No way of calling x86 cpuid instruction for this compiler"
   clear_mem(out, 4);
#endif
   }

void invoke_cpuid_sublevel(uint32_t type, uint32_t level, uint32_t out[4])
   {
#if defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   __cpuidex((int*)out, type, level);

#elif defined(BOTAN_BUILD_COMPILER_IS_GCC) || defined(BOTAN_BUILD_COMPILER_IS_CLANG)
   __cpuid_count(type, level, out[0], out[1], out[2], out[3]);

#elif defined(BOTAN_USE_GCC_INLINE_ASM)
   asm("cpuid\n\t"
       : "=a" (out[0]), "=b" (out[1]), "=c" (out[2]), "=d" (out[3])     \
       : "0" (type), "2" (level));

#else
   #warning "No way of calling x86 cpuid instruction for this compiler"
   clear_mem(out, 4);
#endif
   }

}

uint64_t CPUID::CPUID_Data::detect_cpu_features(size_t* cache_line_size)
   {
   uint64_t features_detected = 0;
   uint32_t cpuid[4] = { 0 };

   // CPUID 0: vendor identification, max sublevel
   invoke_cpuid(0, cpuid);

   const uint32_t max_supported_sublevel = cpuid[0];

   const uint32_t INTEL_CPUID[3] = { 0x756E6547, 0x6C65746E, 0x49656E69 };
   const uint32_t AMD_CPUID[3] = { 0x68747541, 0x444D4163, 0x69746E65 };
   const bool is_intel = same_mem(cpuid + 1, INTEL_CPUID, 3);
   const bool is_amd = same_mem(cpuid + 1, AMD_CPUID, 3);

   if(max_supported_sublevel >= 1)
      {
      // CPUID 1: feature bits
      invoke_cpuid(1, cpuid);
      const uint64_t flags0 = (static_cast<uint64_t>(cpuid[2]) << 32) | cpuid[3];

      enum x86_CPUID_1_bits : uint64_t {
         RDTSC = (1ULL << 4),
         SSE2 = (1ULL << 26),
         CLMUL = (1ULL << 33),
         SSSE3 = (1ULL << 41),
         SSE41 = (1ULL << 51),
         SSE42 = (1ULL << 52),
         AESNI = (1ULL << 57),
         RDRAND = (1ULL << 62)
      };

      if(flags0 & x86_CPUID_1_bits::RDTSC)
         features_detected |= CPUID::CPUID_RDTSC_BIT;
      if(flags0 & x86_CPUID_1_bits::SSE2)
         features_detected |= CPUID::CPUID_SSE2_BIT;
      if(flags0 & x86_CPUID_1_bits::CLMUL)
         features_detected |= CPUID::CPUID_CLMUL_BIT;
      if(flags0 & x86_CPUID_1_bits::SSSE3)
         features_detected |= CPUID::CPUID_SSSE3_BIT;
      if(flags0 & x86_CPUID_1_bits::SSE41)
         features_detected |= CPUID::CPUID_SSE41_BIT;
      if(flags0 & x86_CPUID_1_bits::SSE42)
         features_detected |= CPUID::CPUID_SSE42_BIT;
      if(flags0 & x86_CPUID_1_bits::AESNI)
         features_detected |= CPUID::CPUID_AESNI_BIT;
      if(flags0 & x86_CPUID_1_bits::RDRAND)
         features_detected |= CPUID::CPUID_RDRAND_BIT;
      }

   if(is_intel)
      {
      // Intel cache line size is in cpuid(1) output
      *cache_line_size = 8 * get_byte<2>(cpuid[1]);
      }
   else if(is_amd)
      {
      // AMD puts it in vendor zone
      invoke_cpuid(0x80000005, cpuid);
      *cache_line_size = get_byte<3>(cpuid[2]);
      }

   if(max_supported_sublevel >= 7)
      {
      clear_mem(cpuid, 4);
      invoke_cpuid_sublevel(7, 0, cpuid);

      enum x86_CPUID_7_bits : uint64_t {
         BMI1 = (1ULL << 3),
         AVX2 = (1ULL << 5),
         BMI2 = (1ULL << 8),
         AVX512_F = (1ULL << 16),
         AVX512_DQ = (1ULL << 17),
         RDSEED = (1ULL << 18),
         ADX = (1ULL << 19),
         AVX512_IFMA = (1ULL << 21),
         SHA = (1ULL << 29),
         AVX512_BW = (1ULL << 30),
         AVX512_VL = (1ULL << 31),
         AVX512_VBMI = (1ULL << 33),
         AVX512_VBMI2 = (1ULL << 38),
         AVX512_VAES = (1ULL << 41),
         AVX512_VCLMUL = (1ULL << 42),
         AVX512_VBITALG = (1ULL << 44),
      };

      const uint64_t flags7 = (static_cast<uint64_t>(cpuid[2]) << 32) | cpuid[1];

      if(flags7 & x86_CPUID_7_bits::AVX2)
         features_detected |= CPUID::CPUID_AVX2_BIT;
      if(flags7 & x86_CPUID_7_bits::BMI1)
         {
         features_detected |= CPUID::CPUID_BMI1_BIT;
         /*
         We only set the BMI2 bit if BMI1 is also supported, so BMI2
         code can safely use both extensions. No known processor
         implements BMI2 but not BMI1.
         */
         if(flags7 & x86_CPUID_7_bits::BMI2)
            {
            features_detected |= CPUID::CPUID_BMI2_BIT;

            /*
            Up until Zen3, AMD CPUs with BMI2 support had microcoded
            pdep/pext, which works but is very slow.

            TODO: check for Zen3 here
            */
            if(is_intel)
               {
               features_detected |= CPUID::CPUID_FAST_PDEP_BIT;
               }
            }
         }

      if(flags7 & x86_CPUID_7_bits::AVX512_F)
         {
         features_detected |= CPUID::CPUID_AVX512F_BIT;

         if(flags7 & x86_CPUID_7_bits::AVX512_DQ)
            features_detected |= CPUID::CPUID_AVX512DQ_BIT;
         if(flags7 & x86_CPUID_7_bits::AVX512_BW)
            features_detected |= CPUID::CPUID_AVX512BW_BIT;

         const uint64_t ICELAKE_FLAGS =
            x86_CPUID_7_bits::AVX512_F |
            x86_CPUID_7_bits::AVX512_DQ |
            x86_CPUID_7_bits::AVX512_IFMA |
            x86_CPUID_7_bits::AVX512_BW |
            x86_CPUID_7_bits::AVX512_VL |
            x86_CPUID_7_bits::AVX512_VBMI |
            x86_CPUID_7_bits::AVX512_VBMI2 |
            x86_CPUID_7_bits::AVX512_VBITALG;

         if((flags7 & ICELAKE_FLAGS) == ICELAKE_FLAGS)
            features_detected |= CPUID::CPUID_AVX512_ICL_BIT;

         if(flags7 & x86_CPUID_7_bits::AVX512_VAES)
            features_detected |= CPUID::CPUID_AVX512_AES_BIT;
         if(flags7 & x86_CPUID_7_bits::AVX512_VCLMUL)
            features_detected |= CPUID::CPUID_AVX512_CLMUL_BIT;
         }

      if(flags7 & x86_CPUID_7_bits::RDSEED)
         features_detected |= CPUID::CPUID_RDSEED_BIT;
      if(flags7 & x86_CPUID_7_bits::ADX)
         features_detected |= CPUID::CPUID_ADX_BIT;
      if(flags7 & x86_CPUID_7_bits::SHA)
         features_detected |= CPUID::CPUID_SHA_BIT;
      }

   /*
   * If we don't have access to CPUID, we can still safely assume that
   * any x86-64 processor has SSE2 and RDTSC
   */
#if defined(BOTAN_TARGET_ARCH_IS_X86_64)
   if(features_detected == 0)
      {
      features_detected |= CPUID::CPUID_SSE2_BIT;
      features_detected |= CPUID::CPUID_RDTSC_BIT;
      }
#endif

   return features_detected;
   }

#endif

}
/*
* Dynamically Loaded Object
* (C) 2010 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
  #include <dlfcn.h>
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
  #define NOMINMAX 1
  #define _WINSOCKAPI_ // stop windows.h including winsock.h
  #include <windows.h>
#endif

namespace Botan {

namespace {

void raise_runtime_loader_exception(const std::string& lib_name,
                                    const char* msg)
   {
   const std::string ex_msg =
      "Failed to load " + lib_name + ": " +
      (msg ? msg : "Unknown error");

   throw System_Error(ex_msg, 0);
   }

}

Dynamically_Loaded_Library::Dynamically_Loaded_Library(
   const std::string& library) :
   m_lib_name(library), m_lib(nullptr)
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   m_lib = ::dlopen(m_lib_name.c_str(), RTLD_LAZY);

   if(!m_lib)
      raise_runtime_loader_exception(m_lib_name, ::dlerror());

#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   m_lib = ::LoadLibraryA(m_lib_name.c_str());

   if(!m_lib)
      raise_runtime_loader_exception(m_lib_name, "LoadLibrary failed");
#endif

   if(!m_lib)
      raise_runtime_loader_exception(m_lib_name, "Dynamic load not supported");
   }

Dynamically_Loaded_Library::~Dynamically_Loaded_Library()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   ::dlclose(m_lib);
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   ::FreeLibrary(reinterpret_cast<HMODULE>(m_lib));
#endif
   }

void* Dynamically_Loaded_Library::resolve_symbol(const std::string& symbol)
   {
   void* addr = nullptr;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   addr = ::dlsym(m_lib, symbol.c_str());
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   addr = reinterpret_cast<void*>(::GetProcAddress(reinterpret_cast<HMODULE>(m_lib), symbol.c_str()));
#endif

   if(!addr)
      throw Invalid_Argument("Failed to resolve symbol " + symbol +
                             " in " + m_lib_name);

   return addr;
   }

}
/*
* Entropy Source Polling
* (C) 2008-2010,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_SYSTEM_RNG)
#endif

#if defined(BOTAN_HAS_PROCESSOR_RNG)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDSEED)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_WIN32)
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_GETENTROPY)
#endif

namespace Botan {

namespace {

#if defined(BOTAN_HAS_SYSTEM_RNG)

class System_RNG_EntropySource final : public Entropy_Source
   {
   public:
      size_t poll(RandomNumberGenerator& rng) override
         {
         const size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS;
         rng.reseed_from_rng(system_rng(), poll_bits);
         return poll_bits;
         }

      std::string name() const override { return "system_rng"; }
   };

#endif

#if defined(BOTAN_HAS_PROCESSOR_RNG)

class Processor_RNG_EntropySource final : public Entropy_Source
   {
   public:
      size_t poll(RandomNumberGenerator& rng) override
         {
         /*
         * Intel's documentation for RDRAND at
         * https://software.intel.com/en-us/articles/intel-digital-random-number-generator-drng-software-implementation-guide
         * claims that software can guarantee a reseed event by polling enough data:
         * "There is an upper bound of 511 samples per seed in the implementation
         * where samples are 128 bits in size and can provide two 64-bit random
         * numbers each."
         *
         * By requesting 65536 bits we are asking for 512 samples and thus are assured
         * that at some point in producing the output, at least one reseed of the
         * internal state will occur.
         *
         * The reseeding conditions of the POWER and ARM processor RNGs are not known
         * but probably work in a somewhat similar manner. The exact amount requested
         * may be tweaked if and when such conditions become publically known.
         */
         const size_t poll_bits = 65536;
         rng.reseed_from_rng(m_hwrng, poll_bits);
         // Avoid trusting a black box, don't count this as contributing entropy:
         return 0;
         }

      std::string name() const override { return m_hwrng.name(); }
   private:
      Processor_RNG m_hwrng;
   };

#endif

}

std::unique_ptr<Entropy_Source> Entropy_Source::create(const std::string& name)
   {
#if defined(BOTAN_HAS_SYSTEM_RNG)
   if(name == "system_rng")
      {
      return std::make_unique<System_RNG_EntropySource>();
      }
#endif

#if defined(BOTAN_HAS_PROCESSOR_RNG)
   if(name == "hwrng")
      {
      if(Processor_RNG::available())
         {
         return std::make_unique<Processor_RNG_EntropySource>();
         }
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_RDSEED)
   if(name == "rdseed")
      {
      return std::make_unique<Intel_Rdseed>();
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_GETENTROPY)
   if(name == "getentropy")
      {
      return std::make_unique<Getentropy>();
      }
#endif

#if defined(BOTAN_HAS_ENTROPY_SRC_WIN32)
   if(name == "system_stats")
      {
      return std::make_unique<Win32_EntropySource>();
      }
#endif

   BOTAN_UNUSED(name);
   return nullptr;
   }

void Entropy_Sources::add_source(std::unique_ptr<Entropy_Source> src)
   {
   if(src.get())
      {
      m_srcs.push_back(std::move(src));
      }
   }

std::vector<std::string> Entropy_Sources::enabled_sources() const
   {
   std::vector<std::string> sources;
   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      sources.push_back(m_srcs[i]->name());
      }
   return sources;
   }

size_t Entropy_Sources::poll(RandomNumberGenerator& rng,
                             size_t poll_bits,
                             std::chrono::milliseconds timeout)
   {
   typedef std::chrono::system_clock clock;

   auto deadline = clock::now() + timeout;

   size_t bits_collected = 0;

   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      bits_collected += m_srcs[i]->poll(rng);

      if (bits_collected >= poll_bits || clock::now() > deadline)
         break;
      }

   return bits_collected;
   }

size_t Entropy_Sources::poll_just(RandomNumberGenerator& rng, const std::string& the_src)
   {
   for(size_t i = 0; i != m_srcs.size(); ++i)
      {
      if(m_srcs[i]->name() == the_src)
         {
         return m_srcs[i]->poll(rng);
         }
      }

   return 0;
   }

Entropy_Sources::Entropy_Sources(const std::vector<std::string>& sources)
   {
   for(auto&& src_name : sources)
      {
      add_source(Entropy_Source::create(src_name));
      }
   }

Entropy_Sources& Entropy_Sources::global_sources()
   {
   static Entropy_Sources global_entropy_sources(BOTAN_ENTROPY_DEFAULT_SOURCES);

   return global_entropy_sources;
   }

}
/*
* Hash Functions
* (C) 2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_ADLER32)
#endif

#if defined(BOTAN_HAS_CRC24)
#endif

#if defined(BOTAN_HAS_CRC32)
#endif

#if defined(BOTAN_HAS_GOST_34_11)
#endif

#if defined(BOTAN_HAS_KECCAK)
#endif

#if defined(BOTAN_HAS_MD4)
#endif

#if defined(BOTAN_HAS_MD5)
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
#endif

#if defined(BOTAN_HAS_SHA1)
#endif

#if defined(BOTAN_HAS_SHA2_32)
#endif

#if defined(BOTAN_HAS_SHA2_64)
#endif

#if defined(BOTAN_HAS_SHA3)
#endif

#if defined(BOTAN_HAS_SHAKE)
#endif

#if defined(BOTAN_HAS_SKEIN_512)
#endif

#if defined(BOTAN_HAS_STREEBOG)
#endif

#if defined(BOTAN_HAS_SM3)
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
#endif

#if defined(BOTAN_HAS_COMB4P)
#endif

#if defined(BOTAN_HAS_BLAKE2B)
#endif

#if defined(BOTAN_HAS_COMMONCRYPTO)
#endif

namespace Botan {

std::unique_ptr<HashFunction> HashFunction::create(const std::string& algo_spec,
                                                   const std::string& provider)
   {

#if defined(BOTAN_HAS_COMMONCRYPTO)
   if(provider.empty() || provider == "commoncrypto")
      {
      if(auto hash = make_commoncrypto_hash(algo_spec))
         return hash;

      if(!provider.empty())
         return nullptr;
      }
#endif

   if(provider.empty() == false && provider != "base")
      return nullptr; // unknown provider

#if defined(BOTAN_HAS_SHA1)
   if(algo_spec == "SHA-160" ||
      algo_spec == "SHA-1" ||
      algo_spec == "SHA1")
      {
      return std::make_unique<SHA_160>();
      }
#endif

#if defined(BOTAN_HAS_SHA2_32)
   if(algo_spec == "SHA-224")
      {
      return std::make_unique<SHA_224>();
      }

   if(algo_spec == "SHA-256")
      {
      return std::make_unique<SHA_256>();
      }
#endif

#if defined(BOTAN_HAS_SHA2_64)
   if(algo_spec == "SHA-384")
      {
      return std::make_unique<SHA_384>();
      }

   if(algo_spec == "SHA-512")
      {
      return std::make_unique<SHA_512>();
      }

   if(algo_spec == "SHA-512-256")
      {
      return std::make_unique<SHA_512_256>();
      }
#endif

#if defined(BOTAN_HAS_RIPEMD_160)
   if(algo_spec == "RIPEMD-160")
      {
      return std::make_unique<RIPEMD_160>();
      }
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
   if(algo_spec == "Whirlpool")
      {
      return std::make_unique<Whirlpool>();
      }
#endif

#if defined(BOTAN_HAS_MD5)
   if(algo_spec == "MD5")
      {
      return std::make_unique<MD5>();
      }
#endif

#if defined(BOTAN_HAS_MD4)
   if(algo_spec == "MD4")
      {
      return std::make_unique<MD4>();
      }
#endif

#if defined(BOTAN_HAS_GOST_34_11)
   if(algo_spec == "GOST-R-34.11-94" || algo_spec == "GOST-34.11")
      {
      return std::make_unique<GOST_34_11>();
      }
#endif

#if defined(BOTAN_HAS_ADLER32)
   if(algo_spec == "Adler32")
      {
      return std::make_unique<Adler32>();
      }
#endif

#if defined(BOTAN_HAS_CRC24)
   if(algo_spec == "CRC24")
      {
      return std::make_unique<CRC24>();
      }
#endif

#if defined(BOTAN_HAS_CRC32)
   if(algo_spec == "CRC32")
      {
      return std::make_unique<CRC32>();
      }
#endif

   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_SKEIN_512)
   if(req.algo_name() == "Skein-512")
      {
      return std::make_unique<Skein_512>(req.arg_as_integer(0, 512), req.arg(1, ""));
      }
#endif

#if defined(BOTAN_HAS_BLAKE2B)
   if(req.algo_name() == "Blake2b" || req.algo_name() == "BLAKE2b")
      {
      return std::make_unique<Blake2b>(req.arg_as_integer(0, 512));
   }
#endif

#if defined(BOTAN_HAS_KECCAK)
   if(req.algo_name() == "Keccak-1600")
      {
      return std::make_unique<Keccak_1600>(req.arg_as_integer(0, 512));
      }
#endif

#if defined(BOTAN_HAS_SHA3)
   if(req.algo_name() == "SHA-3")
      {
      return std::make_unique<SHA_3>(req.arg_as_integer(0, 512));
      }
#endif

#if defined(BOTAN_HAS_SHAKE)
   if(req.algo_name() == "SHAKE-128" && req.arg_count() == 1)
      {
      return std::make_unique<SHAKE_128>(req.arg_as_integer(0));
      }
   if(req.algo_name() == "SHAKE-256" && req.arg_count() == 1)
      {
      return std::make_unique<SHAKE_256>(req.arg_as_integer(0));
      }
#endif

#if defined(BOTAN_HAS_STREEBOG)
   if(algo_spec == "Streebog-256")
      {
      return std::make_unique<Streebog_256>();
      }
   if(algo_spec == "Streebog-512")
      {
      return std::make_unique<Streebog_512>();
      }
#endif

#if defined(BOTAN_HAS_SM3)
   if(algo_spec == "SM3")
      {
      return std::make_unique<SM3>();
      }
#endif

#if defined(BOTAN_HAS_WHIRLPOOL)
   if(req.algo_name() == "Whirlpool")
      {
      return std::make_unique<Whirlpool>();
      }
#endif

#if defined(BOTAN_HAS_PARALLEL_HASH)
   if(req.algo_name() == "Parallel")
      {
      std::vector<std::unique_ptr<HashFunction>> hashes;

      for(size_t i = 0; i != req.arg_count(); ++i)
         {
         auto h = HashFunction::create(req.arg(i));
         if(!h)
            {
            return nullptr;
            }
         hashes.push_back(std::move(h));
         }

      return std::make_unique<Parallel>(hashes);
      }
#endif

#if defined(BOTAN_HAS_COMB4P)
   if(req.algo_name() == "Comb4P" && req.arg_count() == 2)
      {
      std::unique_ptr<HashFunction> h1 = HashFunction::create(req.arg(0));
      std::unique_ptr<HashFunction> h2 = HashFunction::create(req.arg(1));

      if(h1 && h2)
         return std::make_unique<Comb4P>(std::move(h1), std::move(h2));
      }
#endif


   return nullptr;
   }

//static
std::unique_ptr<HashFunction>
HashFunction::create_or_throw(const std::string& algo,
                              const std::string& provider)
   {
   if(auto hash = HashFunction::create(algo, provider))
      {
      return hash;
      }
   throw Lookup_Error("Hash", algo, provider);
   }

std::vector<std::string> HashFunction::providers(const std::string& algo_spec)
   {
   return probe_providers_of<HashFunction>(algo_spec, {"base", "commoncrypto"});
   }

}

/*
* Hex Encoding and Decoding
* (C) 2010,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

char hex_encode_nibble(uint8_t n, bool uppercase)
   {
   BOTAN_DEBUG_ASSERT(n <= 15);

   const auto in_09 = CT::Mask<uint8_t>::is_lt(n, 10);

   const char c_09 = n + '0';
   const char c_af = n + (uppercase ? 'A' : 'a') - 10;

   return in_09.select(c_09, c_af);
   }

}

void hex_encode(char output[],
                const uint8_t input[],
                size_t input_length,
                bool uppercase)
   {
   for(size_t i = 0; i != input_length; ++i)
      {
      const uint8_t n0 = (input[i] >> 4) & 0xF;
      const uint8_t n1 = (input[i]     ) & 0xF;

      output[2*i  ] = hex_encode_nibble(n0, uppercase);
      output[2*i+1] = hex_encode_nibble(n1, uppercase);
      }
   }

std::string hex_encode(const uint8_t input[],
                       size_t input_length,
                       bool uppercase)
   {
   std::string output(2 * input_length, 0);

   if(input_length)
      hex_encode(&output.front(), input, input_length, uppercase);

   return output;
   }

namespace {

uint8_t hex_char_to_bin(char input)
   {
   const uint8_t c = static_cast<uint8_t>(input);

   const auto is_alpha_upper = CT::Mask<uint8_t>::is_within_range(c, uint8_t('A'), uint8_t('F'));
   const auto is_alpha_lower = CT::Mask<uint8_t>::is_within_range(c, uint8_t('a'), uint8_t('f'));
   const auto is_decimal     = CT::Mask<uint8_t>::is_within_range(c, uint8_t('0'), uint8_t('9'));

   const auto is_whitespace  = CT::Mask<uint8_t>::is_any_of(c, {
         uint8_t(' '), uint8_t('\t'), uint8_t('\n'), uint8_t('\r')
      });

   const uint8_t c_upper = c - uint8_t('A') + 10;
   const uint8_t c_lower = c - uint8_t('a') + 10;
   const uint8_t c_decim = c - uint8_t('0');

   uint8_t ret = 0xFF; // default value

   ret = is_alpha_upper.select(c_upper, ret);
   ret = is_alpha_lower.select(c_lower, ret);
   ret = is_decimal.select(c_decim, ret);
   ret = is_whitespace.select(0x80, ret);

   return ret;
   }

}


size_t hex_decode(uint8_t output[],
                  const char input[],
                  size_t input_length,
                  size_t& input_consumed,
                  bool ignore_ws)
   {
   uint8_t* out_ptr = output;
   bool top_nibble = true;

   clear_mem(output, input_length / 2);

   for(size_t i = 0; i != input_length; ++i)
      {
      const uint8_t bin = hex_char_to_bin(input[i]);

      if(bin >= 0x10)
         {
         if(bin == 0x80 && ignore_ws)
            continue;

         std::string bad_char(1, input[i]);
         if(bad_char == "\t")
           bad_char = "\\t";
         else if(bad_char == "\n")
           bad_char = "\\n";

         throw Invalid_Argument(
           std::string("hex_decode: invalid hex character '") +
           bad_char + "'");
         }

      if(top_nibble)
         *out_ptr |= bin << 4;
      else
         *out_ptr |= bin;

      top_nibble = !top_nibble;
      if(top_nibble)
         ++out_ptr;
      }

   input_consumed = input_length;
   size_t written = (out_ptr - output);

   /*
   * We only got half of a uint8_t at the end; zap the half-written
   * output and mark it as unread
   */
   if(!top_nibble)
      {
      *out_ptr = 0;
      input_consumed -= 1;
      }

   return written;
   }

size_t hex_decode(uint8_t output[],
                  const char input[],
                  size_t input_length,
                  bool ignore_ws)
   {
   size_t consumed = 0;
   size_t written = hex_decode(output, input, input_length,
                               consumed, ignore_ws);

   if(consumed != input_length)
      throw Invalid_Argument("hex_decode: input did not have full bytes");

   return written;
   }

size_t hex_decode(uint8_t output[],
                  const std::string& input,
                  bool ignore_ws)
   {
   return hex_decode(output, input.data(), input.length(), ignore_ws);
   }

secure_vector<uint8_t> hex_decode_locked(const char input[],
                                      size_t input_length,
                                      bool ignore_ws)
   {
   secure_vector<uint8_t> bin(1 + input_length / 2);

   size_t written = hex_decode(bin.data(),
                               input,
                               input_length,
                               ignore_ws);

   bin.resize(written);
   return bin;
   }

secure_vector<uint8_t> hex_decode_locked(const std::string& input,
                                      bool ignore_ws)
   {
   return hex_decode_locked(input.data(), input.size(), ignore_ws);
   }

std::vector<uint8_t> hex_decode(const char input[],
                             size_t input_length,
                             bool ignore_ws)
   {
   std::vector<uint8_t> bin(1 + input_length / 2);

   size_t written = hex_decode(bin.data(),
                               input,
                               input_length,
                               ignore_ws);

   bin.resize(written);
   return bin;
   }

std::vector<uint8_t> hex_decode(const std::string& input,
                             bool ignore_ws)
   {
   return hex_decode(input.data(), input.size(), ignore_ws);
   }

}
/*
* HMAC
* (C) 1999-2007,2014,2020 Jack Lloyd
*     2007 Yves Jerschow
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* Update a HMAC Calculation
*/
void HMAC::add_data(const uint8_t input[], size_t length)
   {
   verify_key_set(m_ikey.empty() == false);
   m_hash->update(input, length);
   }

/*
* Finalize a HMAC Calculation
*/
void HMAC::final_result(uint8_t mac[])
   {
   verify_key_set(m_okey.empty() == false);
   m_hash->final(mac);
   m_hash->update(m_okey);
   m_hash->update(mac, m_hash_output_length);
   m_hash->final(mac);
   m_hash->update(m_ikey);
   }

Key_Length_Specification HMAC::key_spec() const
   {
   // Support very long lengths for things like PBKDF2 and the TLS PRF
   return Key_Length_Specification(0, 4096);
   }

size_t HMAC::output_length() const
   {
   return m_hash_output_length;
   }

/*
* HMAC Key Schedule
*/
void HMAC::key_schedule(const uint8_t key[], size_t length)
   {
   const uint8_t ipad = 0x36;
   const uint8_t opad = 0x5C;

   m_hash->clear();

   m_ikey.resize(m_hash_block_size);
   m_okey.resize(m_hash_block_size);

   clear_mem(m_ikey.data(), m_ikey.size());
   clear_mem(m_okey.data(), m_okey.size());

   /*
   * Sometimes the HMAC key length itself is sensitive, as with PBKDF2 where it
   * reveals the length of the passphrase. Make some attempt to hide this to
   * side channels. Clearly if the secret is longer than the block size then the
   * branch to hash first reveals that. In addition, counting the number of
   * compression functions executed reveals the size at the granularity of the
   * hash function's block size.
   *
   * The greater concern is for smaller keys; being able to detect when a
   * passphrase is say 4 bytes may assist choosing weaker targets. Even though
   * the loop bounds are constant, we can only actually read key[0..length] so
   * it doesn't seem possible to make this computation truly constant time.
   *
   * We don't mind leaking if the length is exactly zero since that's
   * trivial to simply check.
   */

   if(length > m_hash_block_size)
      {
      m_hash->update(key, length);
      m_hash->final(m_ikey.data());
      }
   else if(length > 0)
      {
      for(size_t i = 0, i_mod_length = 0; i != m_hash_block_size; ++i)
         {
         /*
         access key[i % length] but avoiding division due to variable
         time computation on some processors.
         */
         auto needs_reduction = CT::Mask<size_t>::is_lte(length, i_mod_length);
         i_mod_length = needs_reduction.select(0, i_mod_length);
         const uint8_t kb = key[i_mod_length];

         auto in_range = CT::Mask<size_t>::is_lt(i, length);
         m_ikey[i] = static_cast<uint8_t>(in_range.if_set_return(kb));
         i_mod_length += 1;
         }
      }

   for(size_t i = 0; i != m_hash_block_size; ++i)
      {
      m_ikey[i] ^= ipad;
      m_okey[i] = m_ikey[i] ^ ipad ^ opad;
      }

   m_hash->update(m_ikey);
   }

/*
* Clear memory of sensitive data
*/
void HMAC::clear()
   {
   m_hash->clear();
   zap(m_ikey);
   zap(m_okey);
   }

/*
* Return the name of this type
*/
std::string HMAC::name() const
   {
   return "HMAC(" + m_hash->name() + ")";
   }

/*
* Return a new_object of this object
*/
std::unique_ptr<MessageAuthenticationCode> HMAC::new_object() const
   {
   return std::make_unique<HMAC>(m_hash->new_object());
   }

/*
* HMAC Constructor
*/
HMAC::HMAC(std::unique_ptr<HashFunction> hash) :
   m_hash(std::move(hash)),
   m_hash_output_length(m_hash->output_length()),
   m_hash_block_size(m_hash->hash_block_size())
   {
   BOTAN_ARG_CHECK(m_hash_block_size >= m_hash_output_length,
                   "HMAC is not compatible with this hash function");
   }

}
/*
* HMAC_DRBG
* (C) 2014,2015,2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

size_t hmac_drbg_security_level(size_t mac_output_length)
   {
   // security strength of the hash function
   // for pre-image resistance (see NIST SP 800-57)
   // SHA-160: 128 bits
   // SHA-224, SHA-512/224: 192 bits,
   // SHA-256, SHA-512/256, SHA-384, SHA-512: >= 256 bits
   // NIST SP 800-90A only supports up to 256 bits though

   if(mac_output_length < 32)
      {
      return (mac_output_length - 4) * 8;
      }
   else
      {
      return 32 * 8;
      }
   }

void check_limits(size_t reseed_interval,
                  size_t max_number_of_bytes_per_request)
   {
   // SP800-90A permits up to 2^48, but it is not usable on 32 bit
   // platforms, so we only allow up to 2^24, which is still reasonably high
   if(reseed_interval == 0 || reseed_interval > static_cast<size_t>(1) << 24)
      {
      throw Invalid_Argument("Invalid value for reseed_interval");
      }

   if(max_number_of_bytes_per_request == 0 || max_number_of_bytes_per_request > 64 * 1024)
      {
      throw Invalid_Argument("Invalid value for max_number_of_bytes_per_request");
      }
   }

}

HMAC_DRBG::HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                     RandomNumberGenerator& underlying_rng,
                     size_t reseed_interval,
                     size_t max_number_of_bytes_per_request) :
   Stateful_RNG(underlying_rng, reseed_interval),
   m_mac(std::move(prf)),
   m_max_number_of_bytes_per_request(max_number_of_bytes_per_request),
   m_security_level(hmac_drbg_security_level(m_mac->output_length()))
   {
   BOTAN_ASSERT_NONNULL(m_mac);

   check_limits(reseed_interval, max_number_of_bytes_per_request);

   clear();
   }

HMAC_DRBG::HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                     RandomNumberGenerator& underlying_rng,
                     Entropy_Sources& entropy_sources,
                     size_t reseed_interval,
                     size_t max_number_of_bytes_per_request) :
   Stateful_RNG(underlying_rng, entropy_sources, reseed_interval),
   m_mac(std::move(prf)),
   m_max_number_of_bytes_per_request(max_number_of_bytes_per_request),
   m_security_level(hmac_drbg_security_level(m_mac->output_length()))
   {
   BOTAN_ASSERT_NONNULL(m_mac);

   check_limits(reseed_interval, max_number_of_bytes_per_request);

   clear();
   }

HMAC_DRBG::HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                     Entropy_Sources& entropy_sources,
                     size_t reseed_interval,
                     size_t max_number_of_bytes_per_request) :
   Stateful_RNG(entropy_sources, reseed_interval),
   m_mac(std::move(prf)),
   m_max_number_of_bytes_per_request(max_number_of_bytes_per_request),
   m_security_level(hmac_drbg_security_level(m_mac->output_length()))
   {
   BOTAN_ASSERT_NONNULL(m_mac);

   check_limits(reseed_interval, max_number_of_bytes_per_request);

   clear();
   }

HMAC_DRBG::HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf) :
   Stateful_RNG(),
   m_mac(std::move(prf)),
   m_max_number_of_bytes_per_request(64*1024),
   m_security_level(hmac_drbg_security_level(m_mac->output_length()))
   {
   BOTAN_ASSERT_NONNULL(m_mac);
   clear();
   }

HMAC_DRBG::HMAC_DRBG(const std::string& hmac_hash) :
   Stateful_RNG(),
   m_mac(MessageAuthenticationCode::create_or_throw("HMAC(" + hmac_hash + ")")),
   m_max_number_of_bytes_per_request(64 * 1024),
   m_security_level(hmac_drbg_security_level(m_mac->output_length()))
   {
   clear();
   }

void HMAC_DRBG::clear_state()
   {
   if(m_V.size() == 0)
      {
      const size_t output_length = m_mac->output_length();
      m_V.resize(output_length);
      }

   for(size_t i = 0; i != m_V.size(); ++i)
      m_V[i] = 0x01;
   m_mac->set_key(std::vector<uint8_t>(m_V.size(), 0x00));
   }

std::string HMAC_DRBG::name() const
   {
   return "HMAC_DRBG(" + m_mac->name() + ")";
   }

/*
* HMAC_DRBG generation
* See NIST SP800-90A section 10.1.2.5
*/
void HMAC_DRBG::generate_output(uint8_t output[], size_t output_len,
                                const uint8_t input[], size_t input_len)
   {
   if(input_len > 0)
      {
      update(input, input_len);
      }

   while(output_len > 0)
      {
      const size_t to_copy = std::min(output_len, m_V.size());
      m_mac->update(m_V.data(), m_V.size());
      m_mac->final(m_V.data());
      copy_mem(output, m_V.data(), to_copy);

      output += to_copy;
      output_len -= to_copy;
      }

   update(input, input_len);
   }

/*
* Reset V and the mac key with new values
* See NIST SP800-90A section 10.1.2.2
*/
void HMAC_DRBG::update(const uint8_t input[], size_t input_len)
   {
   secure_vector<uint8_t> T(m_V.size());
   m_mac->update(m_V);
   m_mac->update(0x00);
   m_mac->update(input, input_len);
   m_mac->final(T.data());
   m_mac->set_key(T);

   m_mac->update(m_V.data(), m_V.size());
   m_mac->final(m_V.data());

   if(input_len > 0)
      {
      m_mac->update(m_V);
      m_mac->update(0x01);
      m_mac->update(input, input_len);
      m_mac->final(T.data());
      m_mac->set_key(T);

      m_mac->update(m_V.data(), m_V.size());
      m_mac->final(m_V.data());
      }
   }

size_t HMAC_DRBG::security_level() const
   {
   return m_security_level;
   }
}
/*
* Message Authentication Code base class
* (C) 1999-2008 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_CMAC)
#endif

#if defined(BOTAN_HAS_GMAC)
#endif

#if defined(BOTAN_HAS_HMAC)
#endif

#if defined(BOTAN_HAS_POLY1305)
#endif

#if defined(BOTAN_HAS_SIPHASH)
#endif

#if defined(BOTAN_HAS_ANSI_X919_MAC)
#endif

#if defined(BOTAN_HAS_BLAKE2BMAC)
#endif

namespace Botan {

std::unique_ptr<MessageAuthenticationCode>
MessageAuthenticationCode::create(const std::string& algo_spec,
                                  const std::string& provider)
   {
   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_BLAKE2BMAC)
   if(req.algo_name() == "Blake2b" || req.algo_name() == "BLAKE2b")
      {
      return std::make_unique<BLAKE2bMAC>(req.arg_as_integer(0, 512));
      }
#endif

#if defined(BOTAN_HAS_GMAC)
   if(req.algo_name() == "GMAC" && req.arg_count() == 1)
      {
      if(provider.empty() || provider == "base")
         {
         if(auto bc = BlockCipher::create(req.arg(0)))
            return std::make_unique<GMAC>(std::move(bc));
         }
      }
#endif

#if defined(BOTAN_HAS_HMAC)
   if(req.algo_name() == "HMAC" && req.arg_count() == 1)
      {
      if(provider.empty() || provider == "base")
         {
         if(auto hash = HashFunction::create(req.arg(0)))
            return std::make_unique<HMAC>(std::move(hash));
         }
      }
#endif

#if defined(BOTAN_HAS_POLY1305)
   if(req.algo_name() == "Poly1305" && req.arg_count() == 0)
      {
      if(provider.empty() || provider == "base")
         return std::make_unique<Poly1305>();
      }
#endif

#if defined(BOTAN_HAS_SIPHASH)
   if(req.algo_name() == "SipHash")
      {
      if(provider.empty() || provider == "base")
         {
         return std::make_unique<SipHash>(req.arg_as_integer(0, 2), req.arg_as_integer(1, 4));
         }
      }
#endif

#if defined(BOTAN_HAS_CMAC)
   if((req.algo_name() == "CMAC" || req.algo_name() == "OMAC") && req.arg_count() == 1)
      {
      if(provider.empty() || provider == "base")
         {
         if(auto bc = BlockCipher::create(req.arg(0)))
            return std::make_unique<CMAC>(std::move(bc));
         }
      }
#endif


#if defined(BOTAN_HAS_ANSI_X919_MAC)
   if(req.algo_name() == "X9.19-MAC")
      {
      if(provider.empty() || provider == "base")
         {
         return std::make_unique<ANSI_X919_MAC>();
         }
      }
#endif

   BOTAN_UNUSED(req);
   BOTAN_UNUSED(provider);

   return nullptr;
   }

std::vector<std::string>
MessageAuthenticationCode::providers(const std::string& algo_spec)
   {
   return probe_providers_of<MessageAuthenticationCode>(algo_spec);
   }

//static
std::unique_ptr<MessageAuthenticationCode>
MessageAuthenticationCode::create_or_throw(const std::string& algo,
                                           const std::string& provider)
   {
   if(auto mac = MessageAuthenticationCode::create(algo, provider))
      {
      return mac;
      }
   throw Lookup_Error("MAC", algo, provider);
   }

void MessageAuthenticationCode::start_msg(const uint8_t nonce[], size_t nonce_len)
   {
   BOTAN_UNUSED(nonce);
   if(nonce_len > 0)
      throw Invalid_IV_Length(name(), nonce_len);
   }

/*
* Default (deterministic) MAC verification operation
*/
bool MessageAuthenticationCode::verify_mac(const uint8_t mac[], size_t length)
   {
   secure_vector<uint8_t> our_mac = final();

   if(our_mac.size() != length)
      return false;

   return constant_time_compare(our_mac.data(), mac, length);
   }

}
/*
* Merkle-Damgard Hash Function
* (C) 1999-2008,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
* MDx_HashFunction Constructor
*/
MDx_HashFunction::MDx_HashFunction(size_t block_len,
                                   bool byte_big_endian,
                                   bool bit_big_endian,
                                   uint8_t cnt_size) :
   m_pad_char(bit_big_endian == true ? 0x80 : 0x01),
   m_counter_size(cnt_size),
   m_block_bits(ceil_log2(block_len)),
   m_count_big_endian(byte_big_endian),
   m_count(0),
   m_buffer(block_len),
   m_position(0)
   {
   if(!is_power_of_2(block_len))
      throw Invalid_Argument("MDx_HashFunction block length must be a power of 2");
   if(m_block_bits < 3 || m_block_bits > 16)
      throw Invalid_Argument("MDx_HashFunction block size too large or too small");
   if(m_counter_size < 8 || m_counter_size > block_len)
      throw Invalid_State("MDx_HashFunction invalid counter length");
   }

/*
* Clear memory of sensitive data
*/
void MDx_HashFunction::clear()
   {
   zeroise(m_buffer);
   m_count = m_position = 0;
   }

/*
* Update the hash
*/
void MDx_HashFunction::add_data(const uint8_t input[], size_t length)
   {
   const size_t block_len = static_cast<size_t>(1) << m_block_bits;

   m_count += length;

   if(m_position)
      {
      buffer_insert(m_buffer, m_position, input, length);

      if(m_position + length >= block_len)
         {
         compress_n(m_buffer.data(), 1);
         input += (block_len - m_position);
         length -= (block_len - m_position);
         m_position = 0;
         }
      }

   // Just in case the compiler can't figure out block_len is a power of 2
   const size_t full_blocks = length >> m_block_bits;
   const size_t remaining   = length & (block_len - 1);

   if(full_blocks > 0)
      {
      compress_n(input, full_blocks);
      }

   buffer_insert(m_buffer, m_position, input + full_blocks * block_len, remaining);
   m_position += remaining;
   }

/*
* Finalize a hash
*/
void MDx_HashFunction::final_result(uint8_t output[])
   {
   const size_t block_len = static_cast<size_t>(1) << m_block_bits;

   clear_mem(&m_buffer[m_position], block_len - m_position);
   m_buffer[m_position] = m_pad_char;

   if(m_position >= block_len - m_counter_size)
      {
      compress_n(m_buffer.data(), 1);
      zeroise(m_buffer);
      }

   BOTAN_ASSERT_NOMSG(m_counter_size <= output_length());
   BOTAN_ASSERT_NOMSG(m_counter_size >= 8);

   const uint64_t bit_count = m_count * 8;

   if(m_count_big_endian)
      store_be(bit_count, &m_buffer[block_len - 8]);
   else
      store_le(bit_count, &m_buffer[block_len - 8]);

   compress_n(m_buffer.data(), 1);
   copy_out(output);
   clear();
   }

}
/*
* PBKDF
* (C) 2012 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_PBKDF2)
#endif

#if defined(BOTAN_HAS_PGP_S2K)
#endif

namespace Botan {

std::unique_ptr<PBKDF> PBKDF::create(const std::string& algo_spec,
                                     const std::string& provider)
   {
   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_PBKDF2)
   if(req.algo_name() == "PBKDF2")
      {
      // TODO OpenSSL

      if(provider.empty() || provider == "base")
         {
         if(auto mac = MessageAuthenticationCode::create("HMAC(" + req.arg(0) + ")"))
            return std::make_unique<PKCS5_PBKDF2>(mac.release());

         if(auto mac = MessageAuthenticationCode::create(req.arg(0)))
            return std::make_unique<PKCS5_PBKDF2>(mac.release());
         }

      return nullptr;
      }
#endif

#if defined(BOTAN_HAS_PGP_S2K)
   if(req.algo_name() == "OpenPGP-S2K" && req.arg_count() == 1)
      {
      if(auto hash = HashFunction::create(req.arg(0)))
         return std::make_unique<OpenPGP_S2K>(hash.release());
      }
#endif

   BOTAN_UNUSED(req);
   BOTAN_UNUSED(provider);

   return nullptr;
   }

//static
std::unique_ptr<PBKDF>
PBKDF::create_or_throw(const std::string& algo,
                             const std::string& provider)
   {
   if(auto pbkdf = PBKDF::create(algo, provider))
      {
      return pbkdf;
      }
   throw Lookup_Error("PBKDF", algo, provider);
   }

std::vector<std::string> PBKDF::providers(const std::string& algo_spec)
   {
   return probe_providers_of<PBKDF>(algo_spec);
   }

void PBKDF::pbkdf_timed(uint8_t out[], size_t out_len,
                        const std::string& passphrase,
                        const uint8_t salt[], size_t salt_len,
                        std::chrono::milliseconds msec,
                        size_t& iterations) const
   {
   iterations = pbkdf(out, out_len, passphrase, salt, salt_len, 0, msec);
   }

void PBKDF::pbkdf_iterations(uint8_t out[], size_t out_len,
                             const std::string& passphrase,
                             const uint8_t salt[], size_t salt_len,
                             size_t iterations) const
   {
   if(iterations == 0)
      throw Invalid_Argument(name() + ": Invalid iteration count");

   const size_t iterations_run = pbkdf(out, out_len, passphrase,
                                       salt, salt_len, iterations,
                                       std::chrono::milliseconds(0));
   BOTAN_ASSERT_EQUAL(iterations, iterations_run, "Expected PBKDF iterations");
   }

secure_vector<uint8_t> PBKDF::pbkdf_iterations(size_t out_len,
                                            const std::string& passphrase,
                                            const uint8_t salt[], size_t salt_len,
                                            size_t iterations) const
   {
   secure_vector<uint8_t> out(out_len);
   pbkdf_iterations(out.data(), out_len, passphrase, salt, salt_len, iterations);
   return out;
   }

secure_vector<uint8_t> PBKDF::pbkdf_timed(size_t out_len,
                                       const std::string& passphrase,
                                       const uint8_t salt[], size_t salt_len,
                                       std::chrono::milliseconds msec,
                                       size_t& iterations) const
   {
   secure_vector<uint8_t> out(out_len);
   pbkdf_timed(out.data(), out_len, passphrase, salt, salt_len, msec, iterations);
   return out;
   }

}
/*
* (C) 2018 Ribose Inc
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_PBKDF2)
#endif

#if defined(BOTAN_HAS_PGP_S2K)
#endif

#if defined(BOTAN_HAS_SCRYPT)
#endif

#if defined(BOTAN_HAS_ARGON2)
#endif

#if defined(BOTAN_HAS_PBKDF_BCRYPT)
#endif

namespace Botan {

void PasswordHash::derive_key(uint8_t out[], size_t out_len,
                              const char* password, size_t password_len,
                              const uint8_t salt[], size_t salt_len,
                              const uint8_t ad[], size_t ad_len,
                              const uint8_t key[], size_t key_len) const
   {
   BOTAN_UNUSED(ad, key);

   if(ad_len == 0 && key_len == 0)
      return this->derive_key(out, out_len,
                              password, password_len,
                              salt, salt_len);
   else
      throw Not_Implemented("PasswordHash " + this->to_string() + " does not support AD or key");
   }

std::unique_ptr<PasswordHashFamily> PasswordHashFamily::create(const std::string& algo_spec,
                                     const std::string& provider)
   {
   const SCAN_Name req(algo_spec);

#if defined(BOTAN_HAS_PBKDF2)
   if(req.algo_name() == "PBKDF2")
      {
      if(provider.empty() || provider == "base")
         {
         if(auto mac = MessageAuthenticationCode::create("HMAC(" + req.arg(0) + ")"))
            return std::make_unique<PBKDF2_Family>(mac.release());

         if(auto mac = MessageAuthenticationCode::create(req.arg(0)))
            return std::make_unique<PBKDF2_Family>(mac.release());
         }

      return nullptr;
      }
#endif

#if defined(BOTAN_HAS_SCRYPT)
   if(req.algo_name() == "Scrypt")
      {
      return std::make_unique<Scrypt_Family>();
      }
#endif

#if defined(BOTAN_HAS_ARGON2)
   if(req.algo_name() == "Argon2d")
      {
      return std::make_unique<Argon2_Family>(static_cast<uint8_t>(0));
      }
   else if(req.algo_name() == "Argon2i")
      {
      return std::make_unique<Argon2_Family>(static_cast<uint8_t>(1));
      }
   else if(req.algo_name() == "Argon2id")
      {
      return std::make_unique<Argon2_Family>(static_cast<uint8_t>(2));
      }
#endif

#if defined(BOTAN_HAS_PBKDF_BCRYPT)
   if(req.algo_name() == "Bcrypt-PBKDF")
      {
      return std::make_unique<Bcrypt_PBKDF_Family>();
      }
#endif

#if defined(BOTAN_HAS_PGP_S2K)
   if(req.algo_name() == "OpenPGP-S2K" && req.arg_count() == 1)
      {
      if(auto hash = HashFunction::create(req.arg(0)))
         {
         return std::make_unique<RFC4880_S2K_Family>(hash.release());
         }
      }
#endif

   BOTAN_UNUSED(req);
   BOTAN_UNUSED(provider);

   return nullptr;
   }

//static
std::unique_ptr<PasswordHashFamily>
PasswordHashFamily::create_or_throw(const std::string& algo,
                             const std::string& provider)
   {
   if(auto pbkdf = PasswordHashFamily::create(algo, provider))
      {
      return pbkdf;
      }
   throw Lookup_Error("PasswordHashFamily", algo, provider);
   }

std::vector<std::string> PasswordHashFamily::providers(const std::string& algo_spec)
   {
   return probe_providers_of<PasswordHashFamily>(algo_spec);
   }

}
/*
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
#endif

namespace Botan {

void RandomNumberGenerator::randomize_with_ts_input(uint8_t output[], size_t output_len)
   {
   if(this->accepts_input())
      {
      /*
      Form additional input which is provided to the PRNG implementation
      to paramaterize the KDF output.
      */
      uint8_t additional_input[16] = { 0 };
      store_le(OS::get_system_timestamp_ns(), additional_input);
      store_le(OS::get_high_resolution_clock(), additional_input + 8);

      this->randomize_with_input(output, output_len, additional_input, sizeof(additional_input));
      }
   else
      {
      this->randomize(output, output_len);
      }
   }

void RandomNumberGenerator::randomize_with_input(uint8_t output[], size_t output_len,
                                                 const uint8_t input[], size_t input_len)
   {
   this->add_entropy(input, input_len);
   this->randomize(output, output_len);
   }

size_t RandomNumberGenerator::reseed(Entropy_Sources& srcs,
                                     size_t poll_bits,
                                     std::chrono::milliseconds poll_timeout)
   {
   if(this->accepts_input())
      {
      return srcs.poll(*this, poll_bits, poll_timeout);
      }
   else
      {
      return 0;
      }
   }

void RandomNumberGenerator::reseed_from_rng(RandomNumberGenerator& rng, size_t poll_bits)
   {
   if(this->accepts_input())
      {
      secure_vector<uint8_t> buf(poll_bits / 8);
      rng.randomize(buf.data(), buf.size());
      this->add_entropy(buf.data(), buf.size());
      }
   }

}
/*
* SHA-{224,256}
* (C) 1999-2010,2017 Jack Lloyd
*     2007 FlexSecure GmbH
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

std::string sha256_provider()
   {
#if defined(BOTAN_HAS_SHA2_32_X86)
   if(CPUID::has_intel_sha())
      {
      return "shani";
      }
#endif

#if defined(BOTAN_HAS_SHA2_32_X86_BMI2)
   if(CPUID::has_bmi2())
      {
      return "bmi2";
      }
#endif

#if defined(BOTAN_HAS_SHA2_32_ARMV8)
   if(CPUID::has_arm_sha2())
      {
      return "armv8";
      }
#endif

   return "base";
   }

}

std::unique_ptr<HashFunction> SHA_224::copy_state() const
   {
   return std::make_unique<SHA_224>(*this);
   }

std::unique_ptr<HashFunction> SHA_256::copy_state() const
   {
   return std::make_unique<SHA_256>(*this);
   }

/*
* SHA-256 F1 Function
*
* Use a macro as many compilers won't inline a function this big,
* even though it is much faster if inlined.
*/
#define SHA2_32_F(A, B, C, D, E, F, G, H, M1, M2, M3, M4, magic) do {   \
   uint32_t A_rho = rotr<2>(A) ^ rotr<13>(A) ^ rotr<22>(A);             \
   uint32_t E_rho = rotr<6>(E) ^ rotr<11>(E) ^ rotr<25>(E);             \
   uint32_t M2_sigma = rotr<17>(M2) ^ rotr<19>(M2) ^ (M2 >> 10);        \
   uint32_t M4_sigma = rotr<7>(M4) ^ rotr<18>(M4) ^ (M4 >> 3);          \
   H += magic + E_rho + choose(E, F, G) + M1;                           \
   D += H;                                                              \
   H += A_rho + majority(A, B, C);                                      \
   M1 += M2_sigma + M3 + M4_sigma;                                      \
   } while(0);

/*
* SHA-224 / SHA-256 compression function
*/
void SHA_256::compress_digest(secure_vector<uint32_t>& digest,
                              const uint8_t input[], size_t blocks)
   {
#if defined(BOTAN_HAS_SHA2_32_X86)
   if(CPUID::has_intel_sha())
      {
      return SHA_256::compress_digest_x86(digest, input, blocks);
      }
#endif

#if defined(BOTAN_HAS_SHA2_32_X86_BMI2)
   if(CPUID::has_bmi2())
      {
      return SHA_256::compress_digest_x86_bmi2(digest, input, blocks);
      }
#endif

#if defined(BOTAN_HAS_SHA2_32_ARMV8)
   if(CPUID::has_arm_sha2())
      {
      return SHA_256::compress_digest_armv8(digest, input, blocks);
      }
#endif

   uint32_t A = digest[0], B = digest[1], C = digest[2],
            D = digest[3], E = digest[4], F = digest[5],
            G = digest[6], H = digest[7];

   for(size_t i = 0; i != blocks; ++i)
      {
      uint32_t W00 = load_be<uint32_t>(input,  0);
      uint32_t W01 = load_be<uint32_t>(input,  1);
      uint32_t W02 = load_be<uint32_t>(input,  2);
      uint32_t W03 = load_be<uint32_t>(input,  3);
      uint32_t W04 = load_be<uint32_t>(input,  4);
      uint32_t W05 = load_be<uint32_t>(input,  5);
      uint32_t W06 = load_be<uint32_t>(input,  6);
      uint32_t W07 = load_be<uint32_t>(input,  7);
      uint32_t W08 = load_be<uint32_t>(input,  8);
      uint32_t W09 = load_be<uint32_t>(input,  9);
      uint32_t W10 = load_be<uint32_t>(input, 10);
      uint32_t W11 = load_be<uint32_t>(input, 11);
      uint32_t W12 = load_be<uint32_t>(input, 12);
      uint32_t W13 = load_be<uint32_t>(input, 13);
      uint32_t W14 = load_be<uint32_t>(input, 14);
      uint32_t W15 = load_be<uint32_t>(input, 15);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x428A2F98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x71374491);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0xB5C0FBCF);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0xE9B5DBA5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x3956C25B);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x59F111F1);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x923F82A4);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0xAB1C5ED5);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xD807AA98);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x12835B01);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x243185BE);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x550C7DC3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x72BE5D74);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0x80DEB1FE);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x9BDC06A7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC19BF174);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0xE49B69C1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0xEFBE4786);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x0FC19DC6);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x240CA1CC);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x2DE92C6F);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4A7484AA);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5CB0A9DC);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x76F988DA);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x983E5152);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA831C66D);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xB00327C8);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xBF597FC7);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xC6E00BF3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD5A79147);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0x06CA6351);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x14292967);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x27B70A85);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x2E1B2138);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x4D2C6DFC);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x53380D13);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x650A7354);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x766A0ABB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x81C2C92E);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x92722C85);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0xA2BFE8A1);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0xA81A664B);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0xC24B8B70);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0xC76C51A3);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0xD192E819);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xD6990624);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xF40E3585);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0x106AA070);

      SHA2_32_F(A, B, C, D, E, F, G, H, W00, W14, W09, W01, 0x19A4C116);
      SHA2_32_F(H, A, B, C, D, E, F, G, W01, W15, W10, W02, 0x1E376C08);
      SHA2_32_F(G, H, A, B, C, D, E, F, W02, W00, W11, W03, 0x2748774C);
      SHA2_32_F(F, G, H, A, B, C, D, E, W03, W01, W12, W04, 0x34B0BCB5);
      SHA2_32_F(E, F, G, H, A, B, C, D, W04, W02, W13, W05, 0x391C0CB3);
      SHA2_32_F(D, E, F, G, H, A, B, C, W05, W03, W14, W06, 0x4ED8AA4A);
      SHA2_32_F(C, D, E, F, G, H, A, B, W06, W04, W15, W07, 0x5B9CCA4F);
      SHA2_32_F(B, C, D, E, F, G, H, A, W07, W05, W00, W08, 0x682E6FF3);
      SHA2_32_F(A, B, C, D, E, F, G, H, W08, W06, W01, W09, 0x748F82EE);
      SHA2_32_F(H, A, B, C, D, E, F, G, W09, W07, W02, W10, 0x78A5636F);
      SHA2_32_F(G, H, A, B, C, D, E, F, W10, W08, W03, W11, 0x84C87814);
      SHA2_32_F(F, G, H, A, B, C, D, E, W11, W09, W04, W12, 0x8CC70208);
      SHA2_32_F(E, F, G, H, A, B, C, D, W12, W10, W05, W13, 0x90BEFFFA);
      SHA2_32_F(D, E, F, G, H, A, B, C, W13, W11, W06, W14, 0xA4506CEB);
      SHA2_32_F(C, D, E, F, G, H, A, B, W14, W12, W07, W15, 0xBEF9A3F7);
      SHA2_32_F(B, C, D, E, F, G, H, A, W15, W13, W08, W00, 0xC67178F2);

      A = (digest[0] += A);
      B = (digest[1] += B);
      C = (digest[2] += C);
      D = (digest[3] += D);
      E = (digest[4] += E);
      F = (digest[5] += F);
      G = (digest[6] += G);
      H = (digest[7] += H);

      input += 64;
      }
   }

std::string SHA_224::provider() const
   {
   return sha256_provider();
   }

std::string SHA_256::provider() const
   {
   return sha256_provider();
   }

/*
* SHA-224 compression function
*/
void SHA_224::compress_n(const uint8_t input[], size_t blocks)
   {
   SHA_256::compress_digest(m_digest, input, blocks);
   }

/*
* Copy out the digest
*/
void SHA_224::copy_out(uint8_t output[])
   {
   copy_out_vec_be(output, output_length(), m_digest);
   }

/*
* Clear memory of sensitive data
*/
void SHA_224::clear()
   {
   MDx_HashFunction::clear();
   m_digest[0] = 0xC1059ED8;
   m_digest[1] = 0x367CD507;
   m_digest[2] = 0x3070DD17;
   m_digest[3] = 0xF70E5939;
   m_digest[4] = 0xFFC00B31;
   m_digest[5] = 0x68581511;
   m_digest[6] = 0x64F98FA7;
   m_digest[7] = 0xBEFA4FA4;
   }

/*
* SHA-256 compression function
*/
void SHA_256::compress_n(const uint8_t input[], size_t blocks)
   {
   SHA_256::compress_digest(m_digest, input, blocks);
   }

/*
* Copy out the digest
*/
void SHA_256::copy_out(uint8_t output[])
   {
   copy_out_vec_be(output, output_length(), m_digest);
   }

/*
* Clear memory of sensitive data
*/
void SHA_256::clear()
   {
   MDx_HashFunction::clear();
   m_digest[0] = 0x6A09E667;
   m_digest[1] = 0xBB67AE85;
   m_digest[2] = 0x3C6EF372;
   m_digest[3] = 0xA54FF53A;
   m_digest[4] = 0x510E527F;
   m_digest[5] = 0x9B05688C;
   m_digest[6] = 0x1F83D9AB;
   m_digest[7] = 0x5BE0CD19;
   }

}
/*
* (C) 2016,2020 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_HAS_SYSTEM_RNG)
#endif

namespace Botan {

void Stateful_RNG::clear()
   {
   lock_guard_type<recursive_mutex_type> lock(m_mutex);
   m_reseed_counter = 0;
   m_last_pid = 0;
   clear_state();
   }

void Stateful_RNG::force_reseed()
   {
   lock_guard_type<recursive_mutex_type> lock(m_mutex);
   m_reseed_counter = 0;
   }

bool Stateful_RNG::is_seeded() const
   {
   lock_guard_type<recursive_mutex_type> lock(m_mutex);
   return m_reseed_counter > 0;
   }

void Stateful_RNG::add_entropy(const uint8_t input[], size_t input_len)
   {
   lock_guard_type<recursive_mutex_type> lock(m_mutex);

   update(input, input_len);

   if(8*input_len >= security_level())
      {
      reset_reseed_counter();
      }
   }

void Stateful_RNG::initialize_with(const uint8_t input[], size_t len)
   {
   lock_guard_type<recursive_mutex_type> lock(m_mutex);

   clear();
   add_entropy(input, len);
   }

void Stateful_RNG::randomize(uint8_t output[], size_t output_len)
   {
   randomize_with_input(output, output_len, nullptr, 0);
   }

void Stateful_RNG::randomize_with_ts_input(uint8_t output[], size_t output_len)
   {
   uint8_t additional_input[20] = { 0 };

   store_le(OS::get_high_resolution_clock(), additional_input);

#if defined(BOTAN_HAS_SYSTEM_RNG)
   System_RNG system_rng;
   system_rng.randomize(additional_input + 8, sizeof(additional_input) - 8);
#else
   store_le(OS::get_system_timestamp_ns(), additional_input + 8);
   store_le(OS::get_process_id(), additional_input + 16);
#endif

   randomize_with_input(output, output_len, additional_input, sizeof(additional_input));
   }

void Stateful_RNG::randomize_with_input(uint8_t output[], size_t output_len,
                                        const uint8_t input[], size_t input_len)
   {
   if(output_len == 0)
      return;

   lock_guard_type<recursive_mutex_type> lock(m_mutex);

   const size_t max_per_request = max_number_of_bytes_per_request();

   if(max_per_request == 0) // no limit
      {
      reseed_check();
      this->generate_output(output, output_len, input, input_len);
      }
   else
      {
      while(output_len > 0)
         {
         const size_t this_req = std::min(max_per_request, output_len);

         /*
         * We split the request into several requests to the underlying DRBG but
         * pass the input to each invocation. It might be more sensible to only
         * provide it for the first invocation, however between 2.0 and 2.15
         * HMAC_DRBG always provided it for all requests so retain that here.
         */

         reseed_check();
         this->generate_output(output, this_req, input, input_len);

         output += this_req;
         output_len -= this_req;
         }
      }
   }

size_t Stateful_RNG::reseed(Entropy_Sources& srcs,
                            size_t poll_bits,
                            std::chrono::milliseconds poll_timeout)
   {
   lock_guard_type<recursive_mutex_type> lock(m_mutex);

   const size_t bits_collected = RandomNumberGenerator::reseed(srcs, poll_bits, poll_timeout);

   if(bits_collected >= security_level())
      {
      reset_reseed_counter();
      }

   return bits_collected;
   }

void Stateful_RNG::reseed_from_rng(RandomNumberGenerator& rng, size_t poll_bits)
   {
   lock_guard_type<recursive_mutex_type> lock(m_mutex);

   RandomNumberGenerator::reseed_from_rng(rng, poll_bits);

   if(poll_bits >= security_level())
      {
      reset_reseed_counter();
      }
   }

void Stateful_RNG::reset_reseed_counter()
   {
   // Lock is held whenever this function is called
   m_reseed_counter = 1;
   }

void Stateful_RNG::reseed_check()
   {
   // Lock is held whenever this function is called

   const uint32_t cur_pid = OS::get_process_id();

   const bool fork_detected = (m_last_pid > 0) && (cur_pid != m_last_pid);

   if(is_seeded() == false ||
      fork_detected ||
      (m_reseed_interval > 0 && m_reseed_counter >= m_reseed_interval))
      {
      m_reseed_counter = 0;
      m_last_pid = cur_pid;

      if(m_underlying_rng)
         {
         reseed_from_rng(*m_underlying_rng, security_level());
         }

      if(m_entropy_sources)
         {
         reseed(*m_entropy_sources, security_level());
         }

      if(!is_seeded())
         {
         if(fork_detected)
            throw Invalid_State("Detected use of fork but cannot reseed DRBG");
         else
            throw PRNG_Unseeded(name());
         }
      }
   else
      {
      BOTAN_ASSERT(m_reseed_counter != 0, "RNG is seeded");
      m_reseed_counter += 1;
      }
   }

}
/*
* System RNG
* (C) 2014,2015,2017,2018 Jack Lloyd
* (C) 2021 Tom Crowley
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#if defined(BOTAN_TARGET_OS_HAS_RTLGENRANDOM)
  #define NOMINMAX 1
  #define _WINSOCKAPI_ // stop windows.h including winsock.h
  #include <windows.h>

#elif defined(BOTAN_TARGET_OS_HAS_CRYPTO_NG)
  #include <bcrypt.h>

#elif defined(BOTAN_TARGET_OS_HAS_CCRANDOM)
  #include <CommonCrypto/CommonRandom.h>

#elif defined(BOTAN_TARGET_OS_HAS_ARC4RANDOM)
  #include <stdlib.h>

#elif defined(BOTAN_TARGET_OS_HAS_GETRANDOM)
  #include <sys/random.h>
  #include <errno.h>

#elif defined(BOTAN_TARGET_OS_HAS_DEV_RANDOM)
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_OS_HAS_RTLGENRANDOM)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      System_RNG_Impl() : m_advapi("advapi32.dll")
         {
         // This throws if the function is not found
         m_rtlgenrandom = m_advapi.resolve<RtlGenRandom_fptr>("SystemFunction036");
         }

      void randomize(uint8_t buf[], size_t len) override
         {
         const size_t limit = std::numeric_limits<ULONG>::max();

         uint8_t* pData = buf;
         size_t bytesLeft = len;
         while (bytesLeft > 0)
            {
            const ULONG blockSize = static_cast<ULONG>(std::min(bytesLeft, limit));

            const bool success = m_rtlgenrandom(pData, blockSize) == TRUE;
            if (!success)
               {
               throw System_Error("RtlGenRandom failed");
               }

            BOTAN_ASSERT(bytesLeft >= blockSize, "Block is oversized");
            bytesLeft -= blockSize;
            pData += blockSize;
            }
         }

      void add_entropy(const uint8_t[], size_t) override { /* ignored */ }
      bool is_seeded() const override { return true; }
      bool accepts_input() const override { return false; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "RtlGenRandom"; }
   private:
      using RtlGenRandom_fptr = BOOLEAN (NTAPI *)(PVOID, ULONG);

      Dynamically_Loaded_Library m_advapi;
      RtlGenRandom_fptr m_rtlgenrandom;
   };

#elif defined(BOTAN_TARGET_OS_HAS_CRYPTO_NG)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      System_RNG_Impl()
         {
         NTSTATUS ret = ::BCryptOpenAlgorithmProvider(&m_prov,
                                                      BCRYPT_RNG_ALGORITHM,
                                                      MS_PRIMITIVE_PROVIDER, 0);
         if(ret != STATUS_SUCCESS)
            throw System_Error("System_RNG failed to acquire crypto provider", ret);
         }

      ~System_RNG_Impl()
         {
         ::BCryptCloseAlgorithmProvider(m_prov, 0);
         }

      void randomize(uint8_t buf[], size_t len) override
         {
         const size_t limit = std::numeric_limits<ULONG>::max();

         uint8_t* pData = buf;
         size_t bytesLeft = len;
         while (bytesLeft > 0)
            {
            const ULONG blockSize = static_cast<ULONG>(std::min(bytesLeft, limit));

            const NTSTATUS ret = BCryptGenRandom(m_prov, static_cast<PUCHAR>(pData), blockSize, 0);
            if (ret != STATUS_SUCCESS)
               {
               throw System_Error("System_RNG call to BCryptGenRandom failed", ret);
               }

            BOTAN_ASSERT(bytesLeft >= blockSize, "Block is oversized");
            bytesLeft -= blockSize;
            pData += blockSize;
            }
         }

      void add_entropy(const uint8_t in[], size_t length) override
         {
         /*
         There is a flag BCRYPT_RNG_USE_ENTROPY_IN_BUFFER to provide
         entropy inputs, but it is ignored in Windows 8 and later.
         */
         }

      bool is_seeded() const override { return true; }
      bool accepts_input() const override { return false; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "crypto_ng"; }
   private:
      BCRYPT_ALG_HANDLE m_prov;
   };

#elif defined(BOTAN_TARGET_OS_HAS_CCRANDOM)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      void randomize(uint8_t buf[], size_t len) override
         {
         if (::CCRandomGenerateBytes(buf, len) != kCCSuccess)
            {
            throw System_Error("System_RNG CCRandomGenerateBytes failed", errno);
            }
         }
      bool accepts_input() const override { return false; }
      void add_entropy(const uint8_t[], size_t) override { /* ignored */ }
      bool is_seeded() const override { return true; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "CCRandomGenerateBytes"; }
   };

#elif defined(BOTAN_TARGET_OS_HAS_ARC4RANDOM)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      // No constructor or destructor needed as no userland state maintained

      void randomize(uint8_t buf[], size_t len) override
         {
         // macOS 10.15 arc4random crashes if called with buf == nullptr && len == 0
	 // however it uses ccrng_generate internally which returns a status, ignored
	 // to respect arc4random "no-fail" interface contract
         if(len > 0)
            {
            ::arc4random_buf(buf, len);
            }
         }

      bool accepts_input() const override { return false; }
      void add_entropy(const uint8_t[], size_t) override { /* ignored */ }
      bool is_seeded() const override { return true; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "arc4random"; }
   };

#elif defined(BOTAN_TARGET_OS_HAS_GETRANDOM)

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      // No constructor or destructor needed as no userland state maintained

      void randomize(uint8_t buf[], size_t len) override
         {
         const unsigned int flags = 0;

         while(len > 0)
            {
            const ssize_t got = ::getrandom(buf, len, flags);

            if(got < 0)
               {
               if(errno == EINTR)
                  continue;
               throw System_Error("System_RNG getrandom failed", errno);
               }

            buf += got;
            len -= got;
            }
         }

      bool accepts_input() const override { return false; }
      void add_entropy(const uint8_t[], size_t) override { /* ignored */ }
      bool is_seeded() const override { return true; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "getrandom"; }
   };


#elif defined(BOTAN_TARGET_OS_HAS_DEV_RANDOM)

// Read a random device

class System_RNG_Impl final : public RandomNumberGenerator
   {
   public:
      System_RNG_Impl()
         {
#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

         /*
         * First open /dev/random and read one byte. On old Linux kernels
         * this blocks the RNG until we have been actually seeded.
         */
         m_fd = ::open("/dev/random", O_RDONLY | O_NOCTTY);
         if(m_fd < 0)
            throw System_Error("System_RNG failed to open RNG device", errno);

         uint8_t b;
         const size_t got = ::read(m_fd, &b, 1);
         ::close(m_fd);

         if(got != 1)
            throw System_Error("System_RNG failed to read blocking RNG device");

         m_fd = ::open("/dev/urandom", O_RDWR | O_NOCTTY);

         if(m_fd >= 0)
            {
            m_writable = true;
            }
         else
            {
            /*
            Cannot open in read-write mode. Fall back to read-only,
            calls to add_entropy will fail, but randomize will work
            */
            m_fd = ::open("/dev/urandom", O_RDONLY | O_NOCTTY);
            m_writable = false;
            }

         if(m_fd < 0)
            throw System_Error("System_RNG failed to open RNG device", errno);
         }

      ~System_RNG_Impl()
         {
         ::close(m_fd);
         m_fd = -1;
         }

      void randomize(uint8_t buf[], size_t len) override;
      void add_entropy(const uint8_t in[], size_t length) override;
      bool is_seeded() const override { return true; }
      bool accepts_input() const override { return m_writable; }
      void clear() override { /* not possible */ }
      std::string name() const override { return "urandom"; }
   private:
      int m_fd;
      bool m_writable;
   };

void System_RNG_Impl::randomize(uint8_t buf[], size_t len)
   {
   while(len)
      {
      ssize_t got = ::read(m_fd, buf, len);

      if(got < 0)
         {
         if(errno == EINTR)
            continue;
         throw System_Error("System_RNG read failed", errno);
         }
      if(got == 0)
         throw System_Error("System_RNG EOF on device"); // ?!?

      buf += got;
      len -= got;
      }
   }

void System_RNG_Impl::add_entropy(const uint8_t input[], size_t len)
   {
   if(!m_writable)
      return;

   while(len)
      {
      ssize_t got = ::write(m_fd, input, len);

      if(got < 0)
         {
         if(errno == EINTR)
            continue;

         /*
         * This is seen on OS X CI, despite the fact that the man page
         * for macOS urandom explicitly states that writing to it is
         * supported, and write(2) does not document EPERM at all.
         * But in any case EPERM seems indicative of a policy decision
         * by the OS or sysadmin that additional entropy is not wanted
         * in the system pool, so we accept that and return here,
         * since there is no corrective action possible.
         *
         * In Linux EBADF or EPERM is returned if m_fd is not opened for
         * writing.
         */
         if(errno == EPERM || errno == EBADF)
            return;

         // maybe just ignore any failure here and return?
         throw System_Error("System_RNG write failed", errno);
         }

      input += got;
      len -= got;
      }
   }

#endif

}

RandomNumberGenerator& system_rng()
   {
   static System_RNG_Impl g_system_rng;
   return g_system_rng;
   }

}
/*
* Runtime assertion checking
* (C) 2010,2012,2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void throw_invalid_argument(const char* message,
                            const char* func,
                            const char* file)
   {
   std::ostringstream format;
   format << message << " in " << func << ":" << file;
   throw Invalid_Argument(format.str());
   }

void throw_invalid_state(const char* expr,
                         const char* func,
                         const char* file)
   {
   std::ostringstream format;
   format << "Invalid state: " << expr << " was false in " << func << ":" << file;
   throw Invalid_State(format.str());
   }

void assertion_failure(const char* expr_str,
                       const char* assertion_made,
                       const char* func,
                       const char* file,
                       int line)
   {
   std::ostringstream format;

   format << "False assertion ";

   if(assertion_made && assertion_made[0] != 0)
      format << "'" << assertion_made << "' (expression " << expr_str << ") ";
   else
      format << expr_str << " ";

   if(func)
      format << "in " << func << " ";

   format << "@" << file << ":" << line;

   throw Internal_Error(format.str());
   }

}
/*
* Calendar Functions
* (C) 1999-2010,2017 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <ctime>
#include <iomanip>
#include <stdlib.h>

namespace Botan {

namespace {

std::tm do_gmtime(std::time_t time_val)
   {
   std::tm tm;

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   ::gmtime_s(&tm, &time_val); // Windows
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   ::gmtime_r(&time_val, &tm); // Unix/SUSv2
#else
   std::tm* tm_p = std::gmtime(&time_val);
   if (tm_p == nullptr)
      throw Encoding_Error("time_t_to_tm could not convert");
   tm = *tm_p;
#endif

   return tm;
   }

/*
Portable replacement for timegm, _mkgmtime, etc

Algorithm due to Howard Hinnant

See https://howardhinnant.github.io/date_algorithms.html#days_from_civil
for details and explaination. The code is slightly simplified by our assumption
that the date is at least 1970, which is sufficient for our purposes.
*/
size_t days_since_epoch(uint32_t year, uint32_t month, uint32_t day)
   {
   if(month <= 2)
      year -= 1;
   const uint32_t era = year / 400;
   const uint32_t yoe = year - era * 400;      // [0, 399]
   const uint32_t doy = (153*(month + (month > 2 ? -3 : 9)) + 2)/5 + day-1;  // [0, 365]
   const uint32_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
   return era * 146097 + doe - 719468;
   }

}

std::chrono::system_clock::time_point calendar_point::to_std_timepoint() const
   {
   if(year() < 1970)
      throw Invalid_Argument("calendar_point::to_std_timepoint() does not support years before 1970");

   // 32 bit time_t ends at January 19, 2038
   // https://msdn.microsoft.com/en-us/library/2093ets1.aspx
   // Throw after 2037 if 32 bit time_t is used

   if constexpr(sizeof(std::time_t) == 4)
      {
      if(year() > 2037)
         {
         throw Invalid_Argument("calendar_point::to_std_timepoint() does not support years after 2037 on this system");
         }
      }

   // This upper bound is completely arbitrary
   if(year() >= 2400)
      {
      throw Invalid_Argument("calendar_point::to_std_timepoint() does not support years after 2400");
      }

   const uint64_t seconds_64 = (days_since_epoch(year(), month(), day()) * 86400) +
                                (hour() * 60 * 60) + (minutes() * 60) + seconds();

   const time_t seconds_time_t = static_cast<time_t>(seconds_64);

   if(seconds_64 - seconds_time_t != 0)
      {
      throw Invalid_Argument("calendar_point::to_std_timepoint time_t overflow");
      }

   return std::chrono::system_clock::from_time_t(seconds_time_t);
   }

std::string calendar_point::to_string() const
   {
   // desired format: <YYYY>-<MM>-<dd>T<HH>:<mm>:<ss>
   std::stringstream output;
   output << std::setfill('0')
          << std::setw(4) << year() << "-"
          << std::setw(2) << month() << "-"
          << std::setw(2) << day() << "T"
          << std::setw(2) << hour() << ":"
          << std::setw(2) << minutes() << ":"
          << std::setw(2) << seconds();
   return output.str();
   }

calendar_point::calendar_point(const std::chrono::system_clock::time_point& time_point)
   {
   std::tm tm = do_gmtime(std::chrono::system_clock::to_time_t(time_point));

   m_year = tm.tm_year + 1900;
   m_month = tm.tm_mon + 1;
   m_day = tm.tm_mday;
   m_hour = tm.tm_hour;
   m_minutes = tm.tm_min;
   m_seconds = tm.tm_sec;
   }

}
/*
* Character Set Handling
* (C) 1999-2007,2021 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

void append_utf8_for(std::string& s, uint32_t c)
   {
   if(c >= 0xD800 && c < 0xE000)
      throw Decoding_Error("Invalid Unicode character");

   if(c <= 0x7F)
      {
      const uint8_t b0 = static_cast<uint8_t>(c);
      s.push_back(static_cast<char>(b0));
      }
   else if(c <= 0x7FF)
      {
      const uint8_t b0 = 0xC0 | static_cast<uint8_t>(c >> 6);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
      }
   else if(c <= 0xFFFF)
      {
      const uint8_t b0 = 0xE0 | static_cast<uint8_t>(c >> 12);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>((c >> 6) & 0x3F);
      const uint8_t b2 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
      s.push_back(static_cast<char>(b2));
      }
   else if(c <= 0x10FFFF)
      {
      const uint8_t b0 = 0xF0 | static_cast<uint8_t>(c >> 18);
      const uint8_t b1 = 0x80 | static_cast<uint8_t>((c >> 12) & 0x3F);
      const uint8_t b2 = 0x80 | static_cast<uint8_t>((c >> 6) & 0x3F);
      const uint8_t b3 = 0x80 | static_cast<uint8_t>(c & 0x3F);
      s.push_back(static_cast<char>(b0));
      s.push_back(static_cast<char>(b1));
      s.push_back(static_cast<char>(b2));
      s.push_back(static_cast<char>(b3));
      }
   else
      throw Decoding_Error("Invalid Unicode character");

   }

}

std::string ucs2_to_utf8(const uint8_t ucs2[], size_t len)
   {
   if(len % 2 != 0)
      throw Decoding_Error("Invalid length for UCS-2 string");

   const size_t chars = len / 2;

   std::string s;
   for(size_t i = 0; i != chars; ++i)
      {
      const uint32_t c = load_be<uint16_t>(ucs2, i);
      append_utf8_for(s, c);
      }

   return s;
   }

std::string ucs4_to_utf8(const uint8_t ucs4[], size_t len)
   {
   if(len % 4 != 0)
      throw Decoding_Error("Invalid length for UCS-4 string");

   const size_t chars = len / 4;

   std::string s;
   for(size_t i = 0; i != chars; ++i)
      {
      const uint32_t c = load_be<uint32_t>(ucs4, i);
      append_utf8_for(s, c);
      }

   return s;
   }

/*
* Convert from ISO 8859-1 to UTF-8
*/
std::string latin1_to_utf8(const uint8_t chars[], size_t len)
   {
   std::string s;
   for(size_t i = 0; i != len; ++i)
      {
      const uint32_t c = static_cast<uint8_t>(chars[i]);
      append_utf8_for(s, c);
      }
   return s;
   }

}

/*
* (C) 2018,2021 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace CT {

secure_vector<uint8_t> copy_output(CT::Mask<uint8_t> bad_input_u8,
                                   const uint8_t input[],
                                   size_t input_length,
                                   size_t offset)
   {
   /*
   * We do not poison the input here because if we did we would have
   * to unpoison it at exit. We assume instead that callers have
   * already poisoned the input and will unpoison it at their own
   * time.
   */
   CT::poison(&offset, sizeof(size_t));

   secure_vector<uint8_t> output(input_length);

   auto bad_input = CT::Mask<size_t>::expand(bad_input_u8);

   /*
   * If the offset is greater than input_length then the arguments are
   * invalid. Ideally we would through an exception but that leaks
   * information about the offset. Instead treat it as if the input
   * was invalid.
   */
   bad_input |= CT::Mask<size_t>::is_gt(offset, input_length);

   /*
   * If the input is invalid, then set offset == input_length as a result
   * at the end we will set output_bytes == 0 causing the final result to
   * be an empty vector.
   */
   offset = bad_input.select(input_length, offset);

   /*
   Move the desired output bytes to the front using a slow (O^n)
   but constant time loop that does not leak the value of the offset
   */
   for(size_t i = 0; i != input_length; ++i)
      {
      /*
      * If bad_input was set then we modified offset to equal the input_length.
      * In that case, this_loop will be greater than input_length, and so is_eq
      * mask will always be false. As a result none of the input values will be
      * written to output.
      *
      * This is ignoring the possibility of integer overflow of offset + i. But
      * for this to happen the input would have to consume nearly the entire
      * address space, and we just allocated an output buffer of equal size.
      */
      const size_t this_loop = offset + i;

      /*
      start index from i rather than 0 since we know j must be >= i + offset
      to have any effect, and starting from i does not reveal information
      */
      for(size_t j = i; j != input_length; ++j)
         {
         const uint8_t b = input[j];
         const auto is_eq = CT::Mask<size_t>::is_equal(j, this_loop);
         output[i] |= is_eq.if_set_return(b);
         }
      }

   const size_t output_bytes = input_length - offset;

   CT::unpoison(output.data(), output.size());
   CT::unpoison(output_bytes);

   /*
   This is potentially not const time, depending on how std::vector is
   implemented. But since we are always reducing length, it should
   just amount to setting the member var holding the length.
   */
   output.resize(output_bytes);
   return output;
   }

secure_vector<uint8_t> strip_leading_zeros(const uint8_t in[], size_t length)
   {
   size_t leading_zeros = 0;

   auto only_zeros = Mask<uint8_t>::set();

   for(size_t i = 0; i != length; ++i)
      {
      only_zeros &= CT::Mask<uint8_t>::is_zero(in[i]);
      leading_zeros += only_zeros.if_set_return(1);
      }

   return copy_output(CT::Mask<uint8_t>::cleared(), in, length, leading_zeros);
   }

}

}
/*
* DataSource
* (C) 1999-2007 Jack Lloyd
*     2005 Matthew Gregan
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <istream>

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
  #include <fstream>
#endif

namespace Botan {

/*
* Read a single byte from the DataSource
*/
size_t DataSource::read_byte(uint8_t& out)
   {
   return read(&out, 1);
   }

/*
* Peek a single byte from the DataSource
*/
size_t DataSource::peek_byte(uint8_t& out) const
   {
   return peek(&out, 1, 0);
   }

/*
* Discard the next N bytes of the data
*/
size_t DataSource::discard_next(size_t n)
   {
   uint8_t buf[64] = { 0 };
   size_t discarded = 0;

   while(n)
      {
      const size_t got = this->read(buf, std::min(n, sizeof(buf)));
      discarded += got;
      n -= got;

      if(got == 0)
         break;
      }

   return discarded;
   }

/*
* Read from a memory buffer
*/
size_t DataSource_Memory::read(uint8_t out[], size_t length)
   {
   const size_t got = std::min<size_t>(m_source.size() - m_offset, length);
   copy_mem(out, m_source.data() + m_offset, got);
   m_offset += got;
   return got;
   }

bool DataSource_Memory::check_available(size_t n)
   {
   return (n <= (m_source.size() - m_offset));
   }

/*
* Peek into a memory buffer
*/
size_t DataSource_Memory::peek(uint8_t out[], size_t length,
                               size_t peek_offset) const
   {
   const size_t bytes_left = m_source.size() - m_offset;
   if(peek_offset >= bytes_left) return 0;

   const size_t got = std::min(bytes_left - peek_offset, length);
   copy_mem(out, &m_source[m_offset + peek_offset], got);
   return got;
   }

/*
* Check if the memory buffer is empty
*/
bool DataSource_Memory::end_of_data() const
   {
   return (m_offset == m_source.size());
   }

/*
* DataSource_Memory Constructor
*/
DataSource_Memory::DataSource_Memory(const std::string& in) :
   m_source(cast_char_ptr_to_uint8(in.data()),
            cast_char_ptr_to_uint8(in.data()) + in.length()),
   m_offset(0)
   {
   }

/*
* Read from a stream
*/
size_t DataSource_Stream::read(uint8_t out[], size_t length)
   {
   m_source.read(cast_uint8_ptr_to_char(out), length);
   if(m_source.bad())
      throw Stream_IO_Error("DataSource_Stream::read: Source failure");

   const size_t got = static_cast<size_t>(m_source.gcount());
   m_total_read += got;
   return got;
   }

bool DataSource_Stream::check_available(size_t n)
   {
   const std::streampos orig_pos = m_source.tellg();
   m_source.seekg(0, std::ios::end);
   const size_t avail = static_cast<size_t>(m_source.tellg() - orig_pos);
   m_source.seekg(orig_pos);
   return (avail >= n);
   }

/*
* Peek into a stream
*/
size_t DataSource_Stream::peek(uint8_t out[], size_t length, size_t offset) const
   {
   if(end_of_data())
      throw Invalid_State("DataSource_Stream: Cannot peek when out of data");

   size_t got = 0;

   if(offset)
      {
      secure_vector<uint8_t> buf(offset);
      m_source.read(cast_uint8_ptr_to_char(buf.data()), buf.size());
      if(m_source.bad())
         throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
      got = static_cast<size_t>(m_source.gcount());
      }

   if(got == offset)
      {
      m_source.read(cast_uint8_ptr_to_char(out), length);
      if(m_source.bad())
         throw Stream_IO_Error("DataSource_Stream::peek: Source failure");
      got = static_cast<size_t>(m_source.gcount());
      }

   if(m_source.eof())
      m_source.clear();
   m_source.seekg(m_total_read, std::ios::beg);

   return got;
   }

/*
* Check if the stream is empty or in error
*/
bool DataSource_Stream::end_of_data() const
   {
   return (!m_source.good());
   }

/*
* Return a human-readable ID for this stream
*/
std::string DataSource_Stream::id() const
   {
   return m_identifier;
   }

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(const std::string& path,
                                     bool use_binary) :
   m_identifier(path),
   m_source_memory(new std::ifstream(path, use_binary ? std::ios::binary : std::ios::in)),
   m_source(*m_source_memory),
   m_total_read(0)
   {
   if(!m_source.good())
      {
      throw Stream_IO_Error("DataSource: Failure opening file " + path);
      }
   }

#endif

/*
* DataSource_Stream Constructor
*/
DataSource_Stream::DataSource_Stream(std::istream& in,
                                     const std::string& name) :
   m_identifier(name),
   m_source(in),
   m_total_read(0)
   {
   }

DataSource_Stream::~DataSource_Stream()
   {
   // for ~unique_ptr
   }

}
/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

std::string to_string(ErrorType type)
   {
   switch(type)
      {
      case ErrorType::Unknown:
         return "Unknown";
      case ErrorType::SystemError:
         return "SystemError";
      case ErrorType::NotImplemented:
         return "NotImplemented";
      case ErrorType::OutOfMemory:
         return "OutOfMemory";
      case ErrorType::InternalError:
         return "InternalError";
      case ErrorType::IoError:
         return "IoError";
      case ErrorType::InvalidObjectState :
         return "InvalidObjectState";
      case ErrorType::KeyNotSet:
         return "KeyNotSet";
      case ErrorType::InvalidArgument:
         return "InvalidArgument";
      case ErrorType::InvalidKeyLength:
         return "InvalidKeyLength";
      case ErrorType::InvalidNonceLength:
         return "InvalidNonceLength";
      case ErrorType::LookupError:
         return "LookupError";
      case ErrorType::EncodingFailure:
         return "EncodingFailure";
      case ErrorType::DecodingFailure:
         return "DecodingFailure";
      case ErrorType::TLSError:
         return "TLSError";
      case ErrorType::HttpError:
         return "HttpError";
      case ErrorType::InvalidTag:
         return "InvalidTag";
      case ErrorType::RoughtimeError:
         return "RoughtimeError";
      case ErrorType::CommonCryptoError:
         return "CommonCryptoError";
      case ErrorType::Pkcs11Error:
         return "Pkcs11Error";
      case ErrorType::TPMError:
         return "TPMError";
      case ErrorType::DatabaseError:
         return "DatabaseError";
      case ErrorType::ZlibError :
         return "ZlibError";
      case ErrorType::Bzip2Error:
         return "Bzip2Error" ;
      case ErrorType::LzmaError:
         return "LzmaError";
      }

   // No default case in above switch so compiler warns
   return "Unrecognized Botan error";
   }

Exception::Exception(const std::string& msg) : m_msg(msg)
   {}

Exception::Exception(const std::string& msg, const std::exception& e) :
   m_msg(msg + " failed with " + std::string(e.what()))
   {}

Exception::Exception(const char* prefix, const std::string& msg) :
   m_msg(std::string(prefix) + " " + msg)
   {}

Invalid_Argument::Invalid_Argument(const std::string& msg) :
   Exception(msg)
   {}

Invalid_Argument::Invalid_Argument(const std::string& msg, const std::string& where) :
   Exception(msg + " in " + where)
   {}

Invalid_Argument::Invalid_Argument(const std::string& msg, const std::exception& e) :
   Exception(msg, e) {}

Lookup_Error::Lookup_Error(const std::string& type,
                           const std::string& algo,
                           const std::string& provider) :
   Exception("Unavailable " + type + " " + algo +
             (provider.empty() ? std::string("") : (" for provider " + provider)))
   {}

Internal_Error::Internal_Error(const std::string& err) :
   Exception("Internal error: " + err)
   {}

Invalid_Key_Length::Invalid_Key_Length(const std::string& name, size_t length) :
   Invalid_Argument(name + " cannot accept a key of length " +
                    std::to_string(length))
   {}

Invalid_IV_Length::Invalid_IV_Length(const std::string& mode, size_t bad_len) :
   Invalid_Argument("IV length " + std::to_string(bad_len) +
                    " is invalid for " + mode)
   {}

Key_Not_Set::Key_Not_Set(const std::string& algo) :
   Invalid_State("Key not set in " + algo)
   {}

PRNG_Unseeded::PRNG_Unseeded(const std::string& algo) :
   Invalid_State("PRNG not seeded: " + algo)
   {}

Algorithm_Not_Found::Algorithm_Not_Found(const std::string& name) :
   Lookup_Error("Could not find any algorithm named \"" + name + "\"")
   {}

Provider_Not_Found::Provider_Not_Found(const std::string& algo, const std::string& provider) :
   Lookup_Error("Could not find provider '" + provider + "' for " + algo)
   {}

Invalid_Algorithm_Name::Invalid_Algorithm_Name(const std::string& name):
   Invalid_Argument("Invalid algorithm name: " + name)
   {}

Encoding_Error::Encoding_Error(const std::string& name) :
   Exception("Encoding error: " + name)
   {}

Decoding_Error::Decoding_Error(const std::string& name) :
   Exception(name)
   {}

Decoding_Error::Decoding_Error(const std::string& msg, const std::exception& e) :
   Exception(msg, e)
   {}

Decoding_Error::Decoding_Error(const std::string& name, const char* exception_message) :
   Exception(name + " failed with exception " + exception_message) {}

Invalid_Authentication_Tag::Invalid_Authentication_Tag(const std::string& msg) :
   Exception("Invalid authentication tag: " + msg)
   {}

Stream_IO_Error::Stream_IO_Error(const std::string& err) :
   Exception("I/O error: " + err)
   {}

System_Error::System_Error(const std::string& msg, int err_code) :
   Exception(msg + " error code " + std::to_string(err_code)),
   m_error_code(err_code)
   {}

Not_Implemented::Not_Implemented(const std::string& err) :
   Exception("Not implemented", err)
   {}

}
/*
* (C) 2015,2017,2019 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <deque>
#include <memory>

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <dirent.h>
  #include <functional>
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
  #define NOMINMAX 1
  #define _WINSOCKAPI_ // stop windows.h including winsock.h
  #include <windows.h>
#endif

namespace Botan {

namespace {

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)

std::vector<std::string> impl_readdir(const std::string& dir_path)
   {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(dir_path);

   while(!dir_list.empty())
      {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      std::unique_ptr<DIR, std::function<int (DIR*)>> dir(::opendir(cur_path.c_str()), ::closedir);

      if(dir)
         {
         while(struct dirent* dirent = ::readdir(dir.get()))
            {
            const std::string filename = dirent->d_name;
            if(filename == "." || filename == "..")
               continue;
            const std::string full_path = cur_path + "/" + filename;

            struct stat stat_buf;

            if(::stat(full_path.c_str(), &stat_buf) == -1)
               continue;

            if(S_ISDIR(stat_buf.st_mode))
               dir_list.push_back(full_path);
            else if(S_ISREG(stat_buf.st_mode))
               out.push_back(full_path);
            }
         }
      }

   return out;
   }

#elif defined(BOTAN_TARGET_OS_HAS_WIN32)

std::vector<std::string> impl_win32(const std::string& dir_path)
   {
   std::vector<std::string> out;
   std::deque<std::string> dir_list;
   dir_list.push_back(dir_path);

   while(!dir_list.empty())
      {
      const std::string cur_path = dir_list[0];
      dir_list.pop_front();

      WIN32_FIND_DATAA find_data;
      HANDLE dir = ::FindFirstFileA((cur_path + "/*").c_str(), &find_data);

      if(dir != INVALID_HANDLE_VALUE)
         {
         do
            {
            const std::string filename = find_data.cFileName;
            if(filename == "." || filename == "..")
               continue;
            const std::string full_path = cur_path + "/" + filename;

            if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
               {
               dir_list.push_back(full_path);
               }
            else
               {
               out.push_back(full_path);
               }
            }
         while(::FindNextFileA(dir, &find_data));
         }

      ::FindClose(dir);
      }

   return out;
}
#endif

}

bool has_filesystem_impl()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return true;
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   return true;
#else
   return false;
#endif
   }

std::vector<std::string> get_files_recursive(const std::string& dir)
   {
   std::vector<std::string> files;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   files = impl_readdir(dir);
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   files = impl_win32(dir);
#else
   BOTAN_UNUSED(dir);
   throw No_Filesystem_Access();
#endif

   std::sort(files.begin(), files.end());

   return files;
   }

}
/*
* (C) 2017 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <cstdlib>
#include <new>

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
#endif

namespace Botan {

BOTAN_MALLOC_FN void* allocate_memory(size_t elems, size_t elem_size)
   {
   if(elems == 0 || elem_size == 0)
      return nullptr;

   // Some calloc implementations do not check for overflow (?!?)
   const size_t total_size = elems * elem_size;

   if(total_size < elems || total_size < elem_size)
      throw std::bad_alloc();

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   if(void* p = mlock_allocator::instance().allocate(elems, elem_size))
      return p;
#endif

#if defined(BOTAN_TARGET_OS_HAS_ALLOC_CONCEAL)
   void *ptr = ::calloc_conceal(elems, elem_size);
#else
   void* ptr = std::calloc(elems, elem_size);
#endif
   if(!ptr)
      throw std::bad_alloc();
   return ptr;
   }

void deallocate_memory(void* p, size_t elems, size_t elem_size)
   {
   if(p == nullptr)
      return;

   secure_scrub_memory(p, elems * elem_size);

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   if(mlock_allocator::instance().deallocate(p, elems, elem_size))
      return;
#endif
   std::free(p);
   }

void initialize_allocator()
   {
#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
   mlock_allocator::instance();
#endif
   }

uint8_t ct_compare_u8(const uint8_t x[],
                      const uint8_t y[],
                      size_t len)
   {
   volatile uint8_t difference = 0;

   for(size_t i = 0; i != len; ++i)
      difference |= (x[i] ^ y[i]);

   return CT::Mask<uint8_t>::is_zero(difference).value();
   }

}
/*
* OS and machine specific utility functions
* (C) 2015,2016,2017,2018 Jack Lloyd
* (C) 2016 Daniel Neus
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


#include <chrono>

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
  #include <thread>
#endif

#if defined(BOTAN_TARGET_OS_HAS_EXPLICIT_BZERO)
  #include <string.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
  #include <sys/types.h>
  #include <sys/resource.h>
  #include <sys/mman.h>
  #include <signal.h>
  #include <setjmp.h>
  #include <unistd.h>
  #include <errno.h>
  #include <termios.h>
  #undef B0
#endif

#if defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
  #include <emscripten/emscripten.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL) || defined(BOTAN_TARGET_OS_IS_ANDROID) || \
  defined(BOTAN_TARGET_OS_HAS_ELF_AUX_INFO)
  #include <sys/auxv.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_AUXINFO)
  #include <dlfcn.h>
  #include <elf.h>
#endif

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
  #define NOMINMAX 1
  #define _WINSOCKAPI_ // stop windows.h including winsock.h
  #include <windows.h>
#endif

#if defined(BOTAN_TARGET_OS_IS_ANDROID)
  #include <elf.h>
  extern "C" char **environ;
#endif

#if defined(BOTAN_TARGET_OS_IS_IOS) || defined(BOTAN_TARGET_OS_IS_MACOS)
  #include <sys/types.h>
  #include <sys/sysctl.h>
  #include <mach/vm_statistics.h>
#endif

namespace Botan {

// Not defined in OS namespace for historical reasons
void secure_scrub_memory(void* ptr, size_t n)
   {
#if defined(BOTAN_TARGET_OS_HAS_RTLSECUREZEROMEMORY)
   ::RtlSecureZeroMemory(ptr, n);

#elif defined(BOTAN_TARGET_OS_HAS_EXPLICIT_BZERO)
   ::explicit_bzero(ptr, n);

#elif defined(BOTAN_TARGET_OS_HAS_EXPLICIT_MEMSET)
   (void)::explicit_memset(ptr, 0, n);

#elif defined(BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO) && (BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO == 1)
   /*
   Call memset through a static volatile pointer, which the compiler
   should not elide. This construct should be safe in conforming
   compilers, but who knows. I did confirm that on x86-64 GCC 6.1 and
   Clang 3.8 both create code that saves the memset address in the
   data segment and unconditionally loads and jumps to that address.
   */
   static void* (*const volatile memset_ptr)(void*, int, size_t) = std::memset;
   (memset_ptr)(ptr, 0, n);
#else

   volatile uint8_t* p = reinterpret_cast<volatile uint8_t*>(ptr);

   for(size_t i = 0; i != n; ++i)
      p[i] = 0;
#endif
   }

uint32_t OS::get_process_id()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return ::getpid();
#elif defined(BOTAN_TARGET_OS_HAS_WIN32)
   return ::GetCurrentProcessId();
#elif defined(BOTAN_TARGET_OS_IS_INCLUDEOS) || defined(BOTAN_TARGET_OS_IS_LLVM) || defined(BOTAN_TARGET_OS_IS_NONE)
   return 0; // truly no meaningful value
#else
   #error "Missing get_process_id"
#endif
   }

size_t OS::get_cache_line_size()
   {
#if defined(BOTAN_TARGET_OS_IS_IOS) || defined(BOTAN_TARGET_OS_IS_MACOS)
   unsigned long cache_line_size_vl;
   size_t size = sizeof(cache_line_size_vl);
   if(::sysctlbyname("hw.cachelinesize", &cache_line_size_vl, &size, nullptr, 0) == 0)
      return static_cast<size_t>(cache_line_size_vl);
#endif

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(_SC_LEVEL1_DCACHE_LINESIZE)
   const long res = ::sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
   if(res > 0)
      return static_cast<size_t>(res);
#endif

#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL)

   #if defined(AT_L1D_CACHEGEOMETRY)
   // Cache line size is bottom 16 bits
   const unsigned long dcache_info = OS::get_auxval(AT_L1D_CACHEGEOMETRY);
   if(dcache_info != 0)
      return static_cast<size_t>(dcache_info & 0xFFFF);
   #endif

   #if defined(AT_DCACHEBSIZE)
   const unsigned long dcache_bsize = OS::get_auxval(AT_DCACHEBSIZE);
   if(dcache_bsize != 0)
      return static_cast<size_t>(dcache_bsize);
   #endif

#endif

   // TODO: on Windows this is returned by GetLogicalProcessorInformation

   // not available on this platform
   return 0;
   }

unsigned long OS::get_auxval(unsigned long id)
   {
#if defined(BOTAN_TARGET_OS_HAS_GETAUXVAL)
   return ::getauxval(id);
#elif defined(BOTAN_TARGET_OS_IS_ANDROID) && defined(BOTAN_TARGET_ARCH_IS_ARM32)

   if(id == 0)
      return 0;

   char **p = environ;

   while(*p++ != nullptr)
      ;

   Elf32_auxv_t *e = reinterpret_cast<Elf32_auxv_t*>(p);

   while(e != nullptr)
      {
      if(e->a_type == id)
         return e->a_un.a_val;
      e++;
      }

   return 0;
#elif defined(BOTAN_TARGET_OS_HAS_ELF_AUX_INFO)
   unsigned long auxinfo = 0;
   ::elf_aux_info(static_cast<int>(id), &auxinfo, sizeof(auxinfo));
   return auxinfo;
#elif defined(BOTAN_TARGET_OS_HAS_AUXINFO)
   for (const AuxInfo *auxinfo = static_cast<AuxInfo *>(::_dlauxinfo()); auxinfo != AT_NULL; ++auxinfo)
      {
      if (id == auxinfo->a_type)
          return auxinfo->a_v;
      }

   return 0;
#else
   BOTAN_UNUSED(id);
   return 0;
#endif
   }

bool OS::running_in_privileged_state()
   {
#if defined(AT_SECURE)
   return OS::get_auxval(AT_SECURE) != 0;
#elif defined(BOTAN_TARGET_OS_HAS_POSIX1)
   return (::getuid() != ::geteuid()) || (::getgid() != ::getegid());
#else
   return false;
#endif
   }

uint64_t OS::get_cpu_cycle_counter()
   {
   uint64_t rtc = 0;

#if defined(BOTAN_TARGET_OS_HAS_WIN32)
   LARGE_INTEGER tv;
   ::QueryPerformanceCounter(&tv);
   rtc = tv.QuadPart;

#elif defined(BOTAN_USE_GCC_INLINE_ASM)

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

   if(CPUID::has_rdtsc())
      {
      uint32_t rtc_low = 0, rtc_high = 0;
      asm volatile("rdtsc" : "=d" (rtc_high), "=a" (rtc_low));
      rtc = (static_cast<uint64_t>(rtc_high) << 32) | rtc_low;
      }

#elif defined(BOTAN_TARGET_ARCH_IS_PPC64)

   for(;;)
      {
      uint32_t rtc_low = 0, rtc_high = 0, rtc_high2 = 0;
      asm volatile("mftbu %0" : "=r" (rtc_high));
      asm volatile("mftb %0" : "=r" (rtc_low));
      asm volatile("mftbu %0" : "=r" (rtc_high2));

      if(rtc_high == rtc_high2)
         {
         rtc = (static_cast<uint64_t>(rtc_high) << 32) | rtc_low;
         break;
         }
      }

#elif defined(BOTAN_TARGET_ARCH_IS_ALPHA)
   asm volatile("rpcc %0" : "=r" (rtc));

   // OpenBSD does not trap access to the %tick register
#elif defined(BOTAN_TARGET_ARCH_IS_SPARC64) && !defined(BOTAN_TARGET_OS_IS_OPENBSD)
   asm volatile("rd %%tick, %0" : "=r" (rtc));

#elif defined(BOTAN_TARGET_ARCH_IS_IA64)
   asm volatile("mov %0=ar.itc" : "=r" (rtc));

#elif defined(BOTAN_TARGET_ARCH_IS_S390X)
   asm volatile("stck 0(%0)" : : "a" (&rtc) : "memory", "cc");

#elif defined(BOTAN_TARGET_ARCH_IS_HPPA)
   asm volatile("mfctl 16,%0" : "=r" (rtc)); // 64-bit only?

#else
   //#warning "OS::get_cpu_cycle_counter not implemented"
#endif

#endif

   return rtc;
   }

size_t OS::get_cpu_available()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)

#if defined(_SC_NPROCESSORS_ONLN)
   const long cpu_online = ::sysconf(_SC_NPROCESSORS_ONLN);
   if(cpu_online > 0)
      return static_cast<size_t>(cpu_online);
#endif

#if defined(_SC_NPROCESSORS_CONF)
   const long cpu_conf = ::sysconf(_SC_NPROCESSORS_CONF);
   if(cpu_conf > 0)
      return static_cast<size_t>(cpu_conf);
#endif

#endif

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
   // hardware_concurrency is allowed to return 0 if the value is not
   // well defined or not computable.
   const size_t hw_concur = std::thread::hardware_concurrency();

   if(hw_concur > 0)
      return hw_concur;
#endif

   return 1;
   }

uint64_t OS::get_high_resolution_clock()
   {
   if(uint64_t cpu_clock = OS::get_cpu_cycle_counter())
      return cpu_clock;

#if defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
   return emscripten_get_now();
#endif

   /*
   If we got here either we either don't have an asm instruction
   above, or (for x86) RDTSC is not available at runtime. Try some
   clock_gettimes and return the first one that works, or otherwise
   fall back to std::chrono.
   */

#if defined(BOTAN_TARGET_OS_HAS_CLOCK_GETTIME)

   // The ordering here is somewhat arbitrary...
   const clockid_t clock_types[] = {
#if defined(CLOCK_MONOTONIC_HR)
      CLOCK_MONOTONIC_HR,
#endif
#if defined(CLOCK_MONOTONIC_RAW)
      CLOCK_MONOTONIC_RAW,
#endif
#if defined(CLOCK_MONOTONIC)
      CLOCK_MONOTONIC,
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
      CLOCK_PROCESS_CPUTIME_ID,
#endif
#if defined(CLOCK_THREAD_CPUTIME_ID)
      CLOCK_THREAD_CPUTIME_ID,
#endif
   };

   for(clockid_t clock : clock_types)
      {
      struct timespec ts;
      if(::clock_gettime(clock, &ts) == 0)
         {
         return (static_cast<uint64_t>(ts.tv_sec) * 1000000000) + static_cast<uint64_t>(ts.tv_nsec);
         }
      }
#endif

   // Plain C++11 fallback
   auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
   return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
   }

uint64_t OS::get_system_timestamp_ns()
   {
#if defined(BOTAN_TARGET_OS_HAS_CLOCK_GETTIME)
   struct timespec ts;
   if(::clock_gettime(CLOCK_REALTIME, &ts) == 0)
      {
      return (static_cast<uint64_t>(ts.tv_sec) * 1000000000) + static_cast<uint64_t>(ts.tv_nsec);
      }
#endif

   auto now = std::chrono::system_clock::now().time_since_epoch();
   return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
   }

size_t OS::system_page_size()
   {
   const size_t default_page_size = 4096;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   long p = ::sysconf(_SC_PAGESIZE);
   if(p > 1)
      return static_cast<size_t>(p);
   else
      return default_page_size;
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   BOTAN_UNUSED(default_page_size);
   SYSTEM_INFO sys_info;
   ::GetSystemInfo(&sys_info);
   return sys_info.dwPageSize;
#else
   return default_page_size;
#endif
   }

size_t OS::get_memory_locking_limit()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK) && defined(RLIMIT_MEMLOCK)
   /*
   * If RLIMIT_MEMLOCK is not defined, likely the OS does not support
   * unprivileged mlock calls.
   *
   * Linux defaults to only 64 KiB of mlockable memory per process
   * (too small) but BSDs offer a small fraction of total RAM (more
   * than we need). Bound the total mlock size to 512 KiB which is
   * enough to run the entire test suite without spilling to non-mlock
   * memory (and thus presumably also enough for many useful
   * programs), but small enough that we should not cause problems
   * even if many processes are mlocking on the same machine.
   */
   const size_t user_req = read_env_variable_sz("BOTAN_MLOCK_POOL_SIZE", BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB);

   const size_t mlock_requested = std::min<size_t>(user_req, BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB);

   if(mlock_requested > 0)
      {
      struct ::rlimit limits;

      ::getrlimit(RLIMIT_MEMLOCK, &limits);

      if(limits.rlim_cur < limits.rlim_max)
         {
         limits.rlim_cur = limits.rlim_max;
         ::setrlimit(RLIMIT_MEMLOCK, &limits);
         ::getrlimit(RLIMIT_MEMLOCK, &limits);
         }

      return std::min<size_t>(limits.rlim_cur, mlock_requested * 1024);
      }

#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   SIZE_T working_min = 0, working_max = 0;
   if(!::GetProcessWorkingSetSize(::GetCurrentProcess(), &working_min, &working_max))
      {
      return 0;
      }

   // According to Microsoft MSDN:
   // The maximum number of pages that a process can lock is equal to the number of pages in its minimum working set minus a small overhead
   // In the book "Windows Internals Part 2": the maximum lockable pages are minimum working set size - 8 pages
   // But the information in the book seems to be inaccurate/outdated
   // I've tested this on Windows 8.1 x64, Windows 10 x64 and Windows 7 x86
   // On all three OS the value is 11 instead of 8
   const size_t overhead = OS::system_page_size() * 11;
   if(working_min > overhead)
      {
      const size_t lockable_bytes = working_min - overhead;
      return std::min<size_t>(lockable_bytes, BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB * 1024);
      }
#endif

   // Not supported on this platform
   return 0;
   }

bool OS::read_env_variable(std::string& value_out, const std::string& name)
   {
   value_out = "";

   if(running_in_privileged_state())
      return false;

#if defined(BOTAN_TARGET_OS_HAS_WIN32) && defined(BOTAN_BUILD_COMPILER_IS_MSVC)
   char val[128] = { 0 };
   size_t req_size = 0;
   if(getenv_s(&req_size, val, sizeof(val), name.c_str()) == 0)
      {
      value_out = std::string(val, req_size);
      return true;
      }
#else
   if(const char* val = std::getenv(name.c_str()))
      {
      value_out = val;
      return true;
      }
#endif

   return false;
   }

size_t OS::read_env_variable_sz(const std::string& name, size_t def)
   {
   std::string value;
   if(read_env_variable(value, name))
      {
      try
         {
         const size_t val = std::stoul(value, nullptr);
         return val;
         }
      catch(std::exception&) { /* ignore it */ }
      }

   return def;
   }

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)

namespace {

int get_locked_fd()
   {
#if defined(BOTAN_TARGET_OS_IS_IOS) || defined(BOTAN_TARGET_OS_IS_MACOS)
   // On Darwin, tagging anonymous pages allows vmmap to track these.
   // Allowed from 240 to 255 for userland applications
   static constexpr int default_locked_fd = 255;
   int locked_fd = default_locked_fd;

   if(size_t locked_fdl = OS::read_env_variable_sz("BOTAN_LOCKED_FD", default_locked_fd))
      {
      if(locked_fdl < 240 || locked_fdl > 255)
         {
         locked_fdl = default_locked_fd;
         }
      locked_fd = static_cast<int>(locked_fdl);
      }
   return VM_MAKE_TAG(locked_fd);
#else
   return -1;
#endif
   }

}

#endif

std::vector<void*> OS::allocate_locked_pages(size_t count)
   {
   std::vector<void*> result;

#if (defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)) || defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)

   result.reserve(count);

   const size_t page_size = OS::system_page_size();

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
   static const int locked_fd = get_locked_fd();
#endif

   for(size_t i = 0; i != count; ++i)
      {
      void* ptr = nullptr;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)

#if !defined(MAP_ANONYMOUS)
   #define MAP_ANONYMOUS MAP_ANON
#endif

#if !defined(MAP_NOCORE)
#if defined(MAP_CONCEAL)
   #define MAP_NOCORE MAP_CONCEAL
#else
   #define MAP_NOCORE 0
#endif
#endif

#if !defined(PROT_MAX)
   #define PROT_MAX(p) 0
#endif
      const int pflags = PROT_READ | PROT_WRITE;

      ptr = ::mmap(nullptr, 3*page_size,
                   pflags | PROT_MAX(pflags),
                   MAP_ANONYMOUS | MAP_PRIVATE | MAP_NOCORE,
                   /*fd=*/locked_fd, /*offset=*/0);

      if(ptr == MAP_FAILED)
         {
         continue;
         }

      // lock the data page
      if(::mlock(static_cast<uint8_t*>(ptr) + page_size, page_size) != 0)
         {
         ::munmap(ptr, 3*page_size);
         continue;
         }

#if defined(MADV_DONTDUMP)
      // we ignore errors here, as DONTDUMP is just a bonus
      ::madvise(static_cast<uint8_t*>(ptr) + page_size, page_size, MADV_DONTDUMP);
#endif

#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
      ptr = ::VirtualAlloc(nullptr, 3*page_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

      if(ptr == nullptr)
         continue;

      if(::VirtualLock(static_cast<uint8_t*>(ptr) + page_size, page_size) == 0)
         {
         ::VirtualFree(ptr, 0, MEM_RELEASE);
         continue;
         }
#endif

      std::memset(ptr, 0, 3*page_size); // zero data page and both guard pages

      // Make guard page preceeding the data page
      page_prohibit_access(static_cast<uint8_t*>(ptr));
      // Make guard page following the data page
      page_prohibit_access(static_cast<uint8_t*>(ptr) + 2*page_size);

      result.push_back(static_cast<uint8_t*>(ptr) + page_size);
      }
#else
   BOTAN_UNUSED(count);
#endif

   return result;
   }

void OS::page_allow_access(void* page)
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   const size_t page_size = OS::system_page_size();
   ::mprotect(page, page_size, PROT_READ | PROT_WRITE);
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   const size_t page_size = OS::system_page_size();
   DWORD old_perms = 0;
   ::VirtualProtect(page, page_size, PAGE_READWRITE, &old_perms);
   BOTAN_UNUSED(old_perms);
#else
   BOTAN_UNUSED(page);
#endif
   }

void OS::page_prohibit_access(void* page)
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   const size_t page_size = OS::system_page_size();
   ::mprotect(page, page_size, PROT_NONE);
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
   const size_t page_size = OS::system_page_size();
   DWORD old_perms = 0;
   ::VirtualProtect(page, page_size, PAGE_NOACCESS, &old_perms);
   BOTAN_UNUSED(old_perms);
#else
   BOTAN_UNUSED(page);
#endif
   }

void OS::free_locked_pages(const std::vector<void*>& pages)
   {
   const size_t page_size = OS::system_page_size();

   for(size_t i = 0; i != pages.size(); ++i)
      {
      void* ptr = pages[i];

      secure_scrub_memory(ptr, page_size);

      // ptr points to the data page, guard pages are before and after
      page_allow_access(static_cast<uint8_t*>(ptr) - page_size);
      page_allow_access(static_cast<uint8_t*>(ptr) + page_size);

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && defined(BOTAN_TARGET_OS_HAS_POSIX_MLOCK)
      ::munlock(ptr, page_size);
      ::munmap(static_cast<uint8_t*>(ptr) - page_size, 3*page_size);
#elif defined(BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK)
      ::VirtualUnlock(ptr, page_size);
      ::VirtualFree(static_cast<uint8_t*>(ptr) - page_size, 0, MEM_RELEASE);
#endif
      }
   }

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && !defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)

namespace {

static ::sigjmp_buf g_sigill_jmp_buf;

void botan_sigill_handler(int)
   {
   siglongjmp(g_sigill_jmp_buf, /*non-zero return value*/1);
   }

}

#endif

int OS::run_cpu_instruction_probe(std::function<int ()> probe_fn)
   {
   volatile int probe_result = -3;

#if defined(BOTAN_TARGET_OS_HAS_POSIX1) && !defined(BOTAN_TARGET_OS_IS_EMSCRIPTEN)
   struct sigaction old_sigaction;
   struct sigaction sigaction;

   sigaction.sa_handler = botan_sigill_handler;
   sigemptyset(&sigaction.sa_mask);
   sigaction.sa_flags = 0;

   int rc = ::sigaction(SIGILL, &sigaction, &old_sigaction);

   if(rc != 0)
      throw System_Error("run_cpu_instruction_probe sigaction failed", errno);

   rc = sigsetjmp(g_sigill_jmp_buf, /*save sigs*/1);

   if(rc == 0)
      {
      // first call to sigsetjmp
      probe_result = probe_fn();
      }
   else if(rc == 1)
      {
      // non-local return from siglongjmp in signal handler: return error
      probe_result = -1;
      }

   // Restore old SIGILL handler, if any
   rc = ::sigaction(SIGILL, &old_sigaction, nullptr);
   if(rc != 0)
      throw System_Error("run_cpu_instruction_probe sigaction restore failed", errno);

#else
   BOTAN_UNUSED(probe_fn);
#endif

   return probe_result;
   }

std::unique_ptr<OS::Echo_Suppression> OS::suppress_echo_on_terminal()
   {
#if defined(BOTAN_TARGET_OS_HAS_POSIX1)
   class POSIX_Echo_Suppression : public Echo_Suppression
      {
      public:
         POSIX_Echo_Suppression()
            {
            m_stdin_fd = fileno(stdin);
            if(::tcgetattr(m_stdin_fd, &m_old_termios) != 0)
               throw System_Error("Getting terminal status failed", errno);

            struct termios noecho_flags = m_old_termios;
            noecho_flags.c_lflag &= ~ECHO;
            noecho_flags.c_lflag |= ECHONL;

            if(::tcsetattr(m_stdin_fd, TCSANOW, &noecho_flags) != 0)
               throw System_Error("Clearing terminal echo bit failed", errno);
            }

         void reenable_echo() override
            {
            if(m_stdin_fd > 0)
               {
               if(::tcsetattr(m_stdin_fd, TCSANOW, &m_old_termios) != 0)
                  throw System_Error("Restoring terminal echo bit failed", errno);
               m_stdin_fd = -1;
               }
            }

         ~POSIX_Echo_Suppression()
            {
            try
               {
               reenable_echo();
               }
            catch(...)
               {
               }
            }

      private:
         int m_stdin_fd;
         struct termios m_old_termios;
      };

   return std::make_unique<POSIX_Echo_Suppression>();

#elif defined(BOTAN_TARGET_OS_HAS_WIN32)

   class Win32_Echo_Suppression : public Echo_Suppression
      {
      public:
         Win32_Echo_Suppression()
            {
            m_input_handle = ::GetStdHandle(STD_INPUT_HANDLE);
            if(::GetConsoleMode(m_input_handle, &m_console_state) == 0)
               throw System_Error("Getting console mode failed", ::GetLastError());

            DWORD new_mode = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT;
            if(::SetConsoleMode(m_input_handle, new_mode) == 0)
               throw System_Error("Setting console mode failed", ::GetLastError());
            }

         void reenable_echo() override
            {
            if(m_input_handle != INVALID_HANDLE_VALUE)
               {
               if(::SetConsoleMode(m_input_handle, m_console_state) == 0)
                  throw System_Error("Setting console mode failed", ::GetLastError());
               m_input_handle = INVALID_HANDLE_VALUE;
               }
            }

         ~Win32_Echo_Suppression()
            {
            try
               {
               reenable_echo();
               }
            catch(...)
               {
               }
            }

      private:
         HANDLE m_input_handle;
         DWORD m_console_state;
      };

   return std::make_unique<Win32_Echo_Suppression>();

#else

   // Not supported on this platform, return null
   return nullptr;
#endif
   }

}
/*
* Various string utils and parsing functions
* (C) 1999-2007,2013,2014,2015,2018 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
* (C) 2017 René Korthaus, Rohde & Schwarz Cybersecurity
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <cctype>
#include <limits>

namespace Botan {

uint16_t to_uint16(const std::string& str)
   {
   const uint32_t x = to_u32bit(str);

   if(x >> 16)
      throw Invalid_Argument("Integer value exceeds 16 bit range");

   return static_cast<uint16_t>(x);
   }

uint32_t to_u32bit(const std::string& str)
   {
   // std::stoul is not strict enough. Ensure that str is digit only [0-9]*
   for(const char chr : str)
      {
      if(chr < '0' || chr > '9')
         {
         std::string chrAsString(1, chr);
         throw Invalid_Argument("String contains non-digit char: " + chrAsString);
         }
      }

   const unsigned long int x = std::stoul(str);

   if constexpr(sizeof(unsigned long int) > 4)
      {
      // x might be uint64
      if(x > std::numeric_limits<uint32_t>::max())
         {
         throw Invalid_Argument("Integer value of " + str + " exceeds 32 bit range");
         }
      }

   return static_cast<uint32_t>(x);
   }

/*
* Parse a SCAN-style algorithm name
*/
std::vector<std::string> parse_algorithm_name(const std::string& namex)
   {
   if(namex.find('(') == std::string::npos &&
      namex.find(')') == std::string::npos)
      return std::vector<std::string>(1, namex);

   std::string name = namex, substring;
   std::vector<std::string> elems;
   size_t level = 0;

   elems.push_back(name.substr(0, name.find('(')));
   name = name.substr(name.find('('));

   for(auto i = name.begin(); i != name.end(); ++i)
      {
      char c = *i;

      if(c == '(')
         ++level;
      if(c == ')')
         {
         if(level == 1 && i == name.end() - 1)
            {
            if(elems.size() == 1)
               elems.push_back(substring.substr(1));
            else
               elems.push_back(substring);
            return elems;
            }

         if(level == 0 || (level == 1 && i != name.end() - 1))
            throw Invalid_Algorithm_Name(namex);
         --level;
         }

      if(c == ',' && level == 1)
         {
         if(elems.size() == 1)
            elems.push_back(substring.substr(1));
         else
            elems.push_back(substring);
         substring.clear();
         }
      else
         substring += c;
      }

   if(!substring.empty())
      throw Invalid_Algorithm_Name(namex);

   return elems;
   }

std::vector<std::string> split_on(const std::string& str, char delim)
   {
   std::vector<std::string> elems;
   if(str.empty()) return elems;

   std::string substr;
   for(auto i = str.begin(); i != str.end(); ++i)
      {
      if(*i == delim)
         {
         if(!substr.empty())
            elems.push_back(substr);
         substr.clear();
         }
      else
         substr += *i;
      }

   if(substr.empty())
      throw Invalid_Argument("Unable to split string: " + str);
   elems.push_back(substr);

   return elems;
   }

/*
* Join a string
*/
std::string string_join(const std::vector<std::string>& strs, char delim)
   {
   std::string out = "";

   for(size_t i = 0; i != strs.size(); ++i)
      {
      if(i != 0)
         out += delim;
      out += strs[i];
      }

   return out;
   }

/*
* Convert a decimal-dotted string to binary IP
*/
uint32_t string_to_ipv4(const std::string& str)
   {
   std::vector<std::string> parts = split_on(str, '.');

   if(parts.size() != 4)
      throw Decoding_Error("Invalid IP string " + str);

   uint32_t ip = 0;

   for(auto part = parts.begin(); part != parts.end(); ++part)
      {
      uint32_t octet = to_u32bit(*part);

      if(octet > 255)
         throw Decoding_Error("Invalid IP string " + str);

      ip = (ip << 8) | (octet & 0xFF);
      }

   return ip;
   }

/*
* Convert an IP address to decimal-dotted string
*/
std::string ipv4_to_string(uint32_t ip)
   {
   std::string str;
   uint8_t bits[4];
   store_be(ip, bits);

   for(size_t i = 0; i != 4; ++i)
      {
      if(i > 0)
         {
         str += ".";
         }
      str += std::to_string(bits[i]);
      }

   return str;
   }

std::string tolower_string(const std::string& in)
   {
   std::string s = in;
   for(size_t i = 0; i != s.size(); ++i)
      {
      const int cu = static_cast<unsigned char>(s[i]);
      if(std::isalpha(cu))
         s[i] = static_cast<char>(std::tolower(cu));
      }
   return s;
   }

bool host_wildcard_match(const std::string& issued_, const std::string& host_)
   {
   const std::string issued = tolower_string(issued_);
   const std::string host = tolower_string(host_);

   if(host.empty() || issued.empty())
      return false;

   /*
   If there are embedded nulls in your issued name
   Well I feel bad for you son
   */
   if(std::count(issued.begin(), issued.end(), char(0)) > 0)
      return false;

   // If more than one wildcard, then issued name is invalid
   const size_t stars = std::count(issued.begin(), issued.end(), '*');
   if(stars > 1)
      return false;

   // '*' is not a valid character in DNS names so should not appear on the host side
   if(std::count(host.begin(), host.end(), '*') != 0)
      return false;

   // Similarly a DNS name can't end in .
   if(host[host.size() - 1] == '.')
      return false;

   // And a host can't have an empty name component, so reject that
   if(host.find("..") != std::string::npos)
      return false;

   // Exact match: accept
   if(issued == host)
      {
      return true;
      }

   /*
   Otherwise it might be a wildcard

   If the issued size is strictly longer than the hostname size it
   couldn't possibly be a match, even if the issued value is a
   wildcard. The only exception is when the wildcard ends up empty
   (eg www.example.com matches www*.example.com)
   */
   if(issued.size() > host.size() + 1)
      {
      return false;
      }

   // If no * at all then not a wildcard, and so not a match
   if(stars != 1)
      {
      return false;
      }

   /*
   Now walk through the issued string, making sure every character
   matches. When we come to the (singular) '*', jump forward in the
   hostname by the corresponding amount. We know exactly how much
   space the wildcard takes because it must be exactly `len(host) -
   len(issued) + 1 chars`.

   We also verify that the '*' comes in the leftmost component, and
   doesn't skip over any '.' in the hostname.
   */
   size_t dots_seen = 0;
   size_t host_idx = 0;

   for(size_t i = 0; i != issued.size(); ++i)
      {
      dots_seen += (issued[i] == '.');

      if(issued[i] == '*')
         {
         // Fail: wildcard can only come in leftmost component
         if(dots_seen > 0)
            {
            return false;
            }

         /*
         Since there is only one * we know the tail of the issued and
         hostname must be an exact match. In this case advance host_idx
         to match.
         */
         const size_t advance = (host.size() - issued.size() + 1);

         if(host_idx + advance > host.size()) // shouldn't happen
            return false;

         // Can't be any intervening .s that we would have skipped
         if(std::count(host.begin() + host_idx,
                       host.begin() + host_idx + advance, '.') != 0)
            return false;

         host_idx += advance;
         }
      else
         {
         if(issued[i] != host[host_idx])
            {
            return false;
            }

         host_idx += 1;
         }
      }

   // Wildcard issued name must have at least 3 components
   if(dots_seen < 2)
      {
      return false;
      }

   return true;
   }

}
/*
* Simple config/test file reader
* (C) 2013,2014,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

std::string clean_ws(const std::string& s)
   {
   const char* ws = " \t\n";
   auto start = s.find_first_not_of(ws);
   auto end = s.find_last_not_of(ws);

   if(start == std::string::npos)
      return "";

   if(end == std::string::npos)
      return s.substr(start, end);
   else
      return s.substr(start, start + end + 1);
   }

std::map<std::string, std::string> read_cfg(std::istream& is)
   {
   std::map<std::string, std::string> kv;
   size_t line = 0;

   while(is.good())
      {
      std::string s;

      std::getline(is, s);

      ++line;

      if(s.empty() || s[0] == '#')
         continue;

      s = clean_ws(s.substr(0, s.find('#')));

      if(s.empty())
         continue;

      auto eq = s.find("=");

      if(eq == std::string::npos || eq == 0 || eq == s.size() - 1)
         throw Decoding_Error("Bad read_cfg input '" + s + "' on line " + std::to_string(line));

      const std::string key = clean_ws(s.substr(0, eq));
      const std::string val = clean_ws(s.substr(eq + 1, std::string::npos));

      kv[key] = val;
      }

   return kv;
   }

}
/*
* (C) 2018 Ribose Inc
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

std::map<std::string, std::string> read_kv(const std::string& kv)
   {
   std::map<std::string, std::string> m;
   if(kv == "")
      return m;

   std::vector<std::string> parts;

   try
      {
      parts = split_on(kv, ',');
      }
   catch(std::exception&)
      {
      throw Invalid_Argument("Bad KV spec");
      }

   bool escaped = false;
   bool reading_key = true;
   std::string cur_key;
   std::string cur_val;

   for(char c : kv)
      {
      if(c == '\\' && !escaped)
         {
         escaped = true;
         }
      else if(c == ',' && !escaped)
         {
         if(cur_key.empty())
            throw Invalid_Argument("Bad KV spec empty key");

         if(m.find(cur_key) != m.end())
            throw Invalid_Argument("Bad KV spec duplicated key");
         m[cur_key] = cur_val;
         cur_key = "";
         cur_val = "";
         reading_key = true;
         }
      else if(c == '=' && !escaped)
         {
         if(reading_key == false)
            throw Invalid_Argument("Bad KV spec unexpected equals sign");
         reading_key = false;
         }
      else
         {
         if(reading_key)
            cur_key += c;
         else
            cur_val += c;

         if(escaped)
            escaped = false;
         }
      }

   if(!cur_key.empty())
      {
      if(reading_key == false)
         {
         if(m.find(cur_key) != m.end())
            throw Invalid_Argument("Bad KV spec duplicated key");
         m[cur_key] = cur_val;
         }
      else
         throw Invalid_Argument("Bad KV spec incomplete string");
      }

   return m;
   }

}
/*
* SCAN Name Abstraction
* (C) 2008-2009,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

namespace {

std::string make_arg(const std::vector<std::pair<size_t, std::string>>& name, size_t start)
   {
   std::string output = name[start].second;
   size_t level = name[start].first;

   size_t paren_depth = 0;

   for(size_t i = start + 1; i != name.size(); ++i)
      {
      if(name[i].first <= name[start].first)
         break;

      if(name[i].first > level)
         {
         output += "(" + name[i].second;
         ++paren_depth;
         }
      else if(name[i].first < level)
         {
         for (size_t j = name[i].first; j < level; j++) {
            output += ")";
            --paren_depth;
         }
         output += "," + name[i].second;
         }
      else
         {
         if(output[output.size() - 1] != '(')
            output += ",";
         output += name[i].second;
         }

      level = name[i].first;
      }

   for(size_t i = 0; i != paren_depth; ++i)
      output += ")";

   return output;
   }

}

SCAN_Name::SCAN_Name(const char* algo_spec) : SCAN_Name(std::string(algo_spec))
   {
   }

SCAN_Name::SCAN_Name(std::string algo_spec) : m_orig_algo_spec(algo_spec), m_alg_name(), m_args(), m_mode_info()
   { 
   if(algo_spec.size() == 0)
      throw Invalid_Argument("Expected algorithm name, got empty string");

   std::vector<std::pair<size_t, std::string>> name;
   size_t level = 0;
   std::pair<size_t, std::string> accum = std::make_pair(level, "");

   const std::string decoding_error = "Bad SCAN name '" + algo_spec + "': ";

   for(size_t i = 0; i != algo_spec.size(); ++i)
      {
      char c = algo_spec[i];

      if(c == '/' || c == ',' || c == '(' || c == ')')
         {
         if(c == '(')
            ++level;
         else if(c == ')')
            {
            if(level == 0)
               throw Decoding_Error(decoding_error + "Mismatched parens");
            --level;
            }

         if(c == '/' && level > 0)
            accum.second.push_back(c);
         else
            {
            if(accum.second != "")
               name.push_back(accum);
            accum = std::make_pair(level, "");
            }
         }
      else
         accum.second.push_back(c);
      }

   if(accum.second != "")
      name.push_back(accum);

   if(level != 0)
      throw Decoding_Error(decoding_error + "Missing close paren");

   if(name.size() == 0)
      throw Decoding_Error(decoding_error + "Empty name");

   m_alg_name = name[0].second;

   bool in_modes = false;

   for(size_t i = 1; i != name.size(); ++i)
      {
      if(name[i].first == 0)
         {
         m_mode_info.push_back(make_arg(name, i));
         in_modes = true;
         }
      else if(name[i].first == 1 && !in_modes)
         m_args.push_back(make_arg(name, i));
      }
   }

std::string SCAN_Name::arg(size_t i) const
   {
   if(i >= arg_count())
      throw Invalid_Argument("SCAN_Name::arg " + std::to_string(i) +
                             " out of range for '" + to_string() + "'");
   return m_args[i];
   }

std::string SCAN_Name::arg(size_t i, const std::string& def_value) const
   {
   if(i >= arg_count())
      return def_value;
   return m_args[i];
   }

size_t SCAN_Name::arg_as_integer(size_t i, size_t def_value) const
   {
   if(i >= arg_count())
      return def_value;
   return to_u32bit(m_args[i]);
   }

size_t SCAN_Name::arg_as_integer(size_t i) const
   {
   return to_u32bit(arg(i));
   }

}
/*
* (C) 2018 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

void Timer::start()
   {
   stop();
   m_timer_start = OS::get_system_timestamp_ns();
   m_cpu_cycles_start = OS::get_cpu_cycle_counter();
   }

void Timer::stop()
   {
   if(m_timer_start)
      {
      if(m_cpu_cycles_start != 0)
         {
         const uint64_t cycles_taken = OS::get_cpu_cycle_counter() - m_cpu_cycles_start;
         if(cycles_taken > 0)
            {
            m_cpu_cycles_used += static_cast<size_t>(cycles_taken * m_clock_cycle_ratio);
            }
         }

      const uint64_t now = OS::get_system_timestamp_ns();

      if(now > m_timer_start)
         {
         const uint64_t dur = now - m_timer_start;

         m_time_used += dur;

         if(m_event_count == 0)
            {
            m_min_time = m_max_time = dur;
            }
         else
            {
            m_max_time = std::max(m_max_time, dur);
            m_min_time = std::min(m_min_time, dur);
            }
         }

      m_timer_start = 0;
      ++m_event_count;
      }
   }

bool Timer::operator<(const Timer& other) const
   {
   if(this->doing() != other.doing())
      return (this->doing() < other.doing());

   return (this->get_name() < other.get_name());
   }

std::string Timer::to_string() const
   {
   if(m_custom_msg.size() > 0)
      {
      return m_custom_msg;
      }
   else if(this->buf_size() == 0)
      {
      return result_string_ops();
      }
   else
      {
      return result_string_bps();
      }
   }

std::string Timer::result_string_bps() const
   {
   const size_t MiB = 1024 * 1024;

   const double MiB_total = static_cast<double>(events()) / MiB;
   const double MiB_per_sec = MiB_total / seconds();

   std::ostringstream oss;
   oss << get_name();

   if(!doing().empty())
      {
      oss << " " << doing();
      }

   if(buf_size() > 0)
      {
      oss << " buffer size " << buf_size() << " bytes:";
      }

   if(events() == 0)
      oss << " " << "N/A";
   else
      oss << " " << std::fixed << std::setprecision(3) << MiB_per_sec << " MiB/sec";

   if(cycles_consumed() != 0)
      {
      const double cycles_per_byte = static_cast<double>(cycles_consumed()) / events();
      oss << " " << std::fixed << std::setprecision(2) << cycles_per_byte << " cycles/byte";
      }

   oss << " (" << MiB_total << " MiB in " << milliseconds() << " ms)\n";

   return oss.str();
   }

std::string Timer::result_string_ops() const
   {
   std::ostringstream oss;

   oss << get_name() << " ";

   if(events() == 0)
      {
      oss << "no events\n";
      }
   else
      {
      oss << static_cast<uint64_t>(events_per_second())
          << ' ' << doing() << "/sec; "
          << std::setprecision(2) << std::fixed
          << ms_per_event() << " ms/op";

      if(cycles_consumed() != 0)
         {
         const double cycles_per_op = static_cast<double>(cycles_consumed()) / events();
         const int precision = (cycles_per_op < 10000) ? 2 : 0;
         oss << " " << std::fixed << std::setprecision(precision) << cycles_per_op << " cycles/op";
         }

      oss << " (" << events() << " " << (events() == 1 ? "op" : "ops")
          << " in " << milliseconds() << " ms)\n";
      }

   return oss.str();
   }

}
/*
* Version Information
* (C) 1999-2013,2015 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/


namespace Botan {

/*
  These are intentionally compiled rather than inlined, so an
  application running against a shared library can test the true
  version they are running against.
*/

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

const char* short_version_cstr()
   {
   return STR(BOTAN_VERSION_MAJOR) "."
          STR(BOTAN_VERSION_MINOR) "."
          STR(BOTAN_VERSION_PATCH)
#if defined(BOTAN_VERSION_SUFFIX)
          STR(BOTAN_VERSION_SUFFIX)
#endif
      ;
   }

const char* version_cstr()
   {

   /*
   It is intentional that this string is a compile-time constant;
   it makes it much easier to find in binaries.
   */

   return "Botan " STR(BOTAN_VERSION_MAJOR) "."
                   STR(BOTAN_VERSION_MINOR) "."
                   STR(BOTAN_VERSION_PATCH)
#if defined(BOTAN_VERSION_SUFFIX)
                   STR(BOTAN_VERSION_SUFFIX)
#endif
                   " ("
#if defined(BOTAN_UNSAFE_FUZZER_MODE)
                   "UNSAFE FUZZER MODE BUILD "
#endif
                   BOTAN_VERSION_RELEASE_TYPE
#if (BOTAN_VERSION_DATESTAMP != 0)
                   ", dated " STR(BOTAN_VERSION_DATESTAMP)
#endif
                   ", revision " BOTAN_VERSION_VC_REVISION
                   ", distribution " BOTAN_DISTRIBUTION_INFO ")";
   }

#undef STR
#undef QUOTE

/*
* Return the version as a string
*/
std::string version_string()
   {
   return std::string(version_cstr());
   }

std::string short_version_string()
   {
   return std::string(short_version_cstr());
   }

uint32_t version_datestamp() { return BOTAN_VERSION_DATESTAMP; }

/*
* Return parts of the version as integers
*/
uint32_t version_major() { return BOTAN_VERSION_MAJOR; }
uint32_t version_minor() { return BOTAN_VERSION_MINOR; }
uint32_t version_patch() { return BOTAN_VERSION_PATCH; }

std::string runtime_version_check(uint32_t major,
                                  uint32_t minor,
                                  uint32_t patch)
   {
   if(major != version_major() || minor != version_minor() || patch != version_patch())
      {
      std::ostringstream oss;
      oss << "Warning: linked version (" << short_version_string() << ")"
          << " does not match version built against "
          << "(" << std::to_string(major)
          << '.' << std::to_string(minor)
          << '.' << std::to_string(patch) << ")\n";
      return oss.str();
      }

   return "";
   }

}
