// Copyright © 2017 by Donald King <chronos@chronos-tachyon.net>
// Available under the MIT License. See LICENSE for details.

#include <string>
#include <vector>

#include "base/result_testing.h"
#include "crypto/cipher/aes.h"
#include "crypto/cipher/cbc.h"
#include "crypto/cipher/ctr.h"
#include "crypto/cipher/gcm.h"
#include "crypto/crypto.h"
#include "encoding/hex.h"
#include "gtest/gtest.h"

static const char* const LOREM_IPSUM =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nunc sagittis "
    "sed arcu non rhoncus. Nulla facilisi. Phasellus quis dolor vel nibh "
    "condimentum venenatis. Nam neque purus, malesuada dignissim sodales et, "
    "varius sit amet sem. Nulla orci velit, ultricies a diam sed, viverra "
    "finibus libero. Sed consectetur, ex sed malesuada mattis, urna orci "
    "varius tellus, in pretium nulla magna non ipsum. In at vestibulum "
    "justo. Sed et posuere metus. Donec auctor sapien ut nisl pretium "
    "tincidunt. Phasellus est nulla, ornare vel ullamcorper eu, facilisis "
    "vitae orci. Praesent lobortis dolor in mi volutpat, ut dignissim metus "
    "dapibus. Proin eget tellus eget orci fermentum sollicitudin. "
    "Suspendisse a dui eget quam feugiat elementum sodales ac ante.";

static std::string hex(base::Bytes bytes) {
  return encode(encoding::HEX, bytes);
}

struct RawTestRow {
  const char* name;
  std::vector<uint8_t> key;
  std::vector<uint8_t> plaintext;
  std::vector<uint8_t> ciphertext;
};

static const std::vector<RawTestRow>& raw_testdata() {
  static auto& vec = *new std::vector<RawTestRow>{
      {
          "AES-128 Test A",
          {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
           0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c},
          {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
           0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34},
          {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb,
           0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32},
      },
      {
          "AES-128 Test B",
          {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
           0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f},
          {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
           0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
          {0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
           0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a},
      },
      {
          "AES-192 Test A",
          {0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52,
           0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5,
           0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b},
          {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
           0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34},
          {0x58, 0x5e, 0x9f, 0xb6, 0xc2, 0x72, 0x2b, 0x9a,
           0xf4, 0xf4, 0x92, 0xc1, 0x2b, 0xb0, 0x24, 0xc1},
      },
      {
          "AES-192 Test B",
          {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
           0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
           0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17},
          {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
           0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
          {0xdd, 0xa9, 0x7c, 0xa4, 0x86, 0x4c, 0xdf, 0xe0,
           0x6e, 0xaf, 0x70, 0xa0, 0xec, 0x0d, 0x71, 0x91},
      },
      {
          "AES-256 Test A",
          {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
           0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
           0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
           0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4},
          {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d,
           0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34},
          {0x30, 0x21, 0x61, 0x3a, 0x97, 0x3e, 0x58, 0x2f,
           0x4a, 0x29, 0x23, 0x41, 0x37, 0xae, 0xc4, 0x94},
      },
      {
          "AES-256 Test B",
          {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
           0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
           0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
           0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f},
          {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
           0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff},
          {0x8e, 0xa2, 0xb7, 0xca, 0x51, 0x67, 0x45, 0xbf,
           0xea, 0xfc, 0x49, 0x90, 0x4b, 0x49, 0x60, 0x89},
      },
  };
  return vec;
}

struct GCMTestRow {
  const char* name;
  std::vector<uint8_t> key;
  std::vector<uint8_t> nonce;
  std::vector<uint8_t> additional;
  std::vector<uint8_t> plaintext;
  std::vector<uint8_t> ciphertext;
  std::vector<uint8_t> tag;
};

static const std::vector<GCMTestRow>& gcm_testdata() {
  // Test cases come from:
  //
  //   http://csrc.nist.gov/groups/ST/toolkit/BCM/documents/proposedmodes/gcm/gcm-spec.pdf
  //   Appendix B
  //
  static auto& vec = *new std::vector<GCMTestRow>{
    {
      "Test Case 1",
      // K
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // IV
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // A
      {},
      // P
      {},
      // C
      {},
      // T
      {0x58, 0xe2, 0xfc, 0xce, 0xfa, 0x7e, 0x30, 0x61,
       0x36, 0x7f, 0x1d, 0x57, 0xa4, 0xe7, 0x45, 0x5a},
    },
    {
      "Test Case 2",
      // K
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // IV
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // A
      {},
      // P
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // C
      {0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92,
       0xf3, 0x28, 0xc2, 0xb9, 0x71, 0xb2, 0xfe, 0x78},
      // T
      {0xab, 0x6e, 0x47, 0xd4, 0x2c, 0xec, 0x13, 0xbd,
       0xf5, 0x3a, 0x67, 0xb2, 0x12, 0x57, 0xbd, 0xdf},
    },
    {
      "Test Case 3",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
       0xde, 0xca, 0xf8, 0x88},
      // A
      {},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55},
      // C
      {0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24,
       0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c,
       0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
       0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e,
       0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c,
       0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
       0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97,
       0x3d, 0x58, 0xe0, 0x91, 0x47, 0x3f, 0x59, 0x85},
      // T
      {0x4d, 0x5c, 0x2a, 0xf3, 0x27, 0xcd, 0x64, 0xa6,
       0x2c, 0xf3, 0x5a, 0xbd, 0x2b, 0xa6, 0xfa, 0xb4},
    },
    {
      "Test Case 4",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
       0xde, 0xca, 0xf8, 0x88},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0x42, 0x83, 0x1e, 0xc2, 0x21, 0x77, 0x74, 0x24,
       0x4b, 0x72, 0x21, 0xb7, 0x84, 0xd0, 0xd4, 0x9c,
       0xe3, 0xaa, 0x21, 0x2f, 0x2c, 0x02, 0xa4, 0xe0,
       0x35, 0xc1, 0x7e, 0x23, 0x29, 0xac, 0xa1, 0x2e,
       0x21, 0xd5, 0x14, 0xb2, 0x54, 0x66, 0x93, 0x1c,
       0x7d, 0x8f, 0x6a, 0x5a, 0xac, 0x84, 0xaa, 0x05,
       0x1b, 0xa3, 0x0b, 0x39, 0x6a, 0x0a, 0xac, 0x97,
       0x3d, 0x58, 0xe0, 0x91},
      // T
      {0x5b, 0xc9, 0x4f, 0xbc, 0x32, 0x21, 0xa5, 0xdb,
       0x94, 0xfa, 0xe9, 0x5a, 0xe7, 0x12, 0x1a, 0x47},
    },
    {
      "Test Case 5",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0x61, 0x35, 0x3b, 0x4c, 0x28, 0x06, 0x93, 0x4a,
       0x77, 0x7f, 0xf5, 0x1f, 0xa2, 0x2a, 0x47, 0x55,
       0x69, 0x9b, 0x2a, 0x71, 0x4f, 0xcd, 0xc6, 0xf8,
       0x37, 0x66, 0xe5, 0xf9, 0x7b, 0x6c, 0x74, 0x23,
       0x73, 0x80, 0x69, 0x00, 0xe4, 0x9f, 0x24, 0xb2,
       0x2b, 0x09, 0x75, 0x44, 0xd4, 0x89, 0x6b, 0x42,
       0x49, 0x89, 0xb5, 0xe1, 0xeb, 0xac, 0x0f, 0x07,
       0xc2, 0x3f, 0x45, 0x98},
      // T
      {0x36, 0x12, 0xd2, 0xe7, 0x9e, 0x3b, 0x07, 0x85,
       0x56, 0x1b, 0xe1, 0x4a, 0xac, 0xa2, 0xfc, 0xcb},
    },
    {
      "Test Case 6",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
      // IV
      {0x93, 0x13, 0x22, 0x5d, 0xf8, 0x84, 0x06, 0xe5,
       0x55, 0x90, 0x9c, 0x5a, 0xff, 0x52, 0x69, 0xaa,
       0x6a, 0x7a, 0x95, 0x38, 0x53, 0x4f, 0x7d, 0xa1,
       0xe4, 0xc3, 0x03, 0xd2, 0xa3, 0x18, 0xa7, 0x28,
       0xc3, 0xc0, 0xc9, 0x51, 0x56, 0x80, 0x95, 0x39,
       0xfc, 0xf0, 0xe2, 0x42, 0x9a, 0x6b, 0x52, 0x54,
       0x16, 0xae, 0xdb, 0xf5, 0xa0, 0xde, 0x6a, 0x57,
       0xa6, 0x37, 0xb3, 0x9b},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0x8c, 0xe2, 0x49, 0x98, 0x62, 0x56, 0x15, 0xb6,
       0x03, 0xa0, 0x33, 0xac, 0xa1, 0x3f, 0xb8, 0x94,
       0xbe, 0x91, 0x12, 0xa5, 0xc3, 0xa2, 0x11, 0xa8,
       0xba, 0x26, 0x2a, 0x3c, 0xca, 0x7e, 0x2c, 0xa7,
       0x01, 0xe4, 0xa9, 0xa4, 0xfb, 0xa4, 0x3c, 0x90,
       0xcc, 0xdc, 0xb2, 0x81, 0xd4, 0x8c, 0x7c, 0x6f,
       0xd6, 0x28, 0x75, 0xd2, 0xac, 0xa4, 0x17, 0x03,
       0x4c, 0x34, 0xae, 0xe5},
      // T
      {0x61, 0x9c, 0xc5, 0xae, 0xff, 0xfe, 0x0b, 0xfa,
       0x46, 0x2a, 0xf4, 0x3c, 0x16, 0x99, 0xd0, 0x50},
    },
    {
      "Test Case 7",
      // K
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // IV
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00},
      // A
      {},
      // P
      {},
      // C
      {},
      // T
      {0xcd, 0x33, 0xb2, 0x8a, 0xc7, 0x73, 0xf7, 0x4b,
       0xa0, 0x0e, 0xd1, 0xf3, 0x12, 0x57, 0x24, 0x35},
    },
    {
      "Test Case 8",
      // K
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // IV
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00},
      // A
      {},
      // P
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // C
      {0x98, 0xe7, 0x24, 0x7c, 0x07, 0xf0, 0xfe, 0x41,
       0x1c, 0x26, 0x7e, 0x43, 0x84, 0xb0, 0xf6, 0x00},
      // T
      {0x2f, 0xf5, 0x8d, 0x80, 0x03, 0x39, 0x27, 0xab,
       0x8e, 0xf4, 0xd4, 0x58, 0x75, 0x14, 0xf0, 0xfb},
    },
    {
      "Test Case 9",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
       0xde, 0xca, 0xf8, 0x88},
      // A
      {},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55},
      // C
      {0x39, 0x80, 0xca, 0x0b, 0x3c, 0x00, 0xe8, 0x41,
       0xeb, 0x06, 0xfa, 0xc4, 0x87, 0x2a, 0x27, 0x57,
       0x85, 0x9e, 0x1c, 0xea, 0xa6, 0xef, 0xd9, 0x84,
       0x62, 0x85, 0x93, 0xb4, 0x0c, 0xa1, 0xe1, 0x9c,
       0x7d, 0x77, 0x3d, 0x00, 0xc1, 0x44, 0xc5, 0x25,
       0xac, 0x61, 0x9d, 0x18, 0xc8, 0x4a, 0x3f, 0x47,
       0x18, 0xe2, 0x44, 0x8b, 0x2f, 0xe3, 0x24, 0xd9,
       0xcc, 0xda, 0x27, 0x10, 0xac, 0xad, 0xe2, 0x56},
      // T
      {0x99, 0x24, 0xa7, 0xc8, 0x58, 0x73, 0x36, 0xbf,
       0xb1, 0x18, 0x02, 0x4d, 0xb8, 0x67, 0x4a, 0x14},
    },
    {
      "Test Case 10",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
       0xde, 0xca, 0xf8, 0x88},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0x39, 0x80, 0xca, 0x0b, 0x3c, 0x00, 0xe8, 0x41,
       0xeb, 0x06, 0xfa, 0xc4, 0x87, 0x2a, 0x27, 0x57,
       0x85, 0x9e, 0x1c, 0xea, 0xa6, 0xef, 0xd9, 0x84,
       0x62, 0x85, 0x93, 0xb4, 0x0c, 0xa1, 0xe1, 0x9c,
       0x7d, 0x77, 0x3d, 0x00, 0xc1, 0x44, 0xc5, 0x25,
       0xac, 0x61, 0x9d, 0x18, 0xc8, 0x4a, 0x3f, 0x47,
       0x18, 0xe2, 0x44, 0x8b, 0x2f, 0xe3, 0x24, 0xd9,
       0xcc, 0xda, 0x27, 0x10},
      // T
      {0x25, 0x19, 0x49, 0x8e, 0x80, 0xf1, 0x47, 0x8f,
       0x37, 0xba, 0x55, 0xbd, 0x6d, 0x27, 0x61, 0x8c},
    },
    {
      "Test Case 11",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0x0f, 0x10, 0xf5, 0x99, 0xae, 0x14, 0xa1, 0x54,
       0xed, 0x24, 0xb3, 0x6e, 0x25, 0x32, 0x4d, 0xb8,
       0xc5, 0x66, 0x63, 0x2e, 0xf2, 0xbb, 0xb3, 0x4f,
       0x83, 0x47, 0x28, 0x0f, 0xc4, 0x50, 0x70, 0x57,
       0xfd, 0xdc, 0x29, 0xdf, 0x9a, 0x47, 0x1f, 0x75,
       0xc6, 0x65, 0x41, 0xd4, 0xd4, 0xda, 0xd1, 0xc9,
       0xe9, 0x3a, 0x19, 0xa5, 0x8e, 0x8b, 0x47, 0x3f,
       0xa0, 0xf0, 0x62, 0xf7},
      // T
      {0x65, 0xdc, 0xc5, 0x7f, 0xcf, 0x62, 0x3a, 0x24,
       0x09, 0x4f, 0xcc, 0xa4, 0x0d, 0x35, 0x33, 0xf8},
    },
    {
      "Test Case 12",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c},
      // IV
      {0x93, 0x13, 0x22, 0x5d, 0xf8, 0x84, 0x06, 0xe5,
       0x55, 0x90, 0x9c, 0x5a, 0xff, 0x52, 0x69, 0xaa,
       0x6a, 0x7a, 0x95, 0x38, 0x53, 0x4f, 0x7d, 0xa1,
       0xe4, 0xc3, 0x03, 0xd2, 0xa3, 0x18, 0xa7, 0x28,
       0xc3, 0xc0, 0xc9, 0x51, 0x56, 0x80, 0x95, 0x39,
       0xfc, 0xf0, 0xe2, 0x42, 0x9a, 0x6b, 0x52, 0x54,
       0x16, 0xae, 0xdb, 0xf5, 0xa0, 0xde, 0x6a, 0x57,
       0xa6, 0x37, 0xb3, 0x9b},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0xd2, 0x7e, 0x88, 0x68, 0x1c, 0xe3, 0x24, 0x3c,
       0x48, 0x30, 0x16, 0x5a, 0x8f, 0xdc, 0xf9, 0xff,
       0x1d, 0xe9, 0xa1, 0xd8, 0xe6, 0xb4, 0x47, 0xef,
       0x6e, 0xf7, 0xb7, 0x98, 0x28, 0x66, 0x6e, 0x45,
       0x81, 0xe7, 0x90, 0x12, 0xaf, 0x34, 0xdd, 0xd9,
       0xe2, 0xf0, 0x37, 0x58, 0x9b, 0x29, 0x2d, 0xb3,
       0xe6, 0x7c, 0x03, 0x67, 0x45, 0xfa, 0x22, 0xe7,
       0xe9, 0xb7, 0x37, 0x3b},
      // T
      {0xdc, 0xf5, 0x66, 0xff, 0x29, 0x1c, 0x25, 0xbb,
       0xb8, 0x56, 0x8f, 0xc3, 0xd3, 0x76, 0xa6, 0xd9},
    },
    {
      "Test Case 13",
      // K
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // IV
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00},
      // A
      {},
      // P
      {},
      // C
      {},
      // T
      {0x53, 0x0f, 0x8a, 0xfb, 0xc7, 0x45, 0x36, 0xb9,
       0xa9, 0x63, 0xb4, 0xf1, 0xc4, 0xcb, 0x73, 0x8b},
    },
    {
      "Test Case 14",
      // K
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // IV
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00},
      // A
      {},
      // P
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
      // C
      {0xce, 0xa7, 0x40, 0x3d, 0x4d, 0x60, 0x6b, 0x6e,
       0x07, 0x4e, 0xc5, 0xd3, 0xba, 0xf3, 0x9d, 0x18},
      // T
      {0xd0, 0xd1, 0xc8, 0xa7, 0x99, 0x99, 0x6b, 0xf0,
       0x26, 0x5b, 0x98, 0xb5, 0xd4, 0x8a, 0xb9, 0x19},
    },
    {
      "Test Case 15",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
       0xde, 0xca, 0xf8, 0x88},
      // A
      {},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39, 0x1a, 0xaf, 0xd2, 0x55},
      // C
      {0x52, 0x2d, 0xc1, 0xf0, 0x99, 0x56, 0x7d, 0x07,
       0xf4, 0x7f, 0x37, 0xa3, 0x2a, 0x84, 0x42, 0x7d,
       0x64, 0x3a, 0x8c, 0xdc, 0xbf, 0xe5, 0xc0, 0xc9,
       0x75, 0x98, 0xa2, 0xbd, 0x25, 0x55, 0xd1, 0xaa,
       0x8c, 0xb0, 0x8e, 0x48, 0x59, 0x0d, 0xbb, 0x3d,
       0xa7, 0xb0, 0x8b, 0x10, 0x56, 0x82, 0x88, 0x38,
       0xc5, 0xf6, 0x1e, 0x63, 0x93, 0xba, 0x7a, 0x0a,
       0xbc, 0xc9, 0xf6, 0x62, 0x89, 0x80, 0x15, 0xad},
      // T
      {0xb0, 0x94, 0xda, 0xc5, 0xd9, 0x34, 0x71, 0xbd,
       0xec, 0x1a, 0x50, 0x22, 0x70, 0xe3, 0xcc, 0x6c},
    },
    {
      "Test Case 16",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad,
       0xde, 0xca, 0xf8, 0x88},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0x52, 0x2d, 0xc1, 0xf0, 0x99, 0x56, 0x7d, 0x07,
       0xf4, 0x7f, 0x37, 0xa3, 0x2a, 0x84, 0x42, 0x7d,
       0x64, 0x3a, 0x8c, 0xdc, 0xbf, 0xe5, 0xc0, 0xc9,
       0x75, 0x98, 0xa2, 0xbd, 0x25, 0x55, 0xd1, 0xaa,
       0x8c, 0xb0, 0x8e, 0x48, 0x59, 0x0d, 0xbb, 0x3d,
       0xa7, 0xb0, 0x8b, 0x10, 0x56, 0x82, 0x88, 0x38,
       0xc5, 0xf6, 0x1e, 0x63, 0x93, 0xba, 0x7a, 0x0a,
       0xbc, 0xc9, 0xf6, 0x62},
      // T
      {0x76, 0xfc, 0x6e, 0xce, 0x0f, 0x4e, 0x17, 0x68,
       0xcd, 0xdf, 0x88, 0x53, 0xbb, 0x2d, 0x55, 0x1b},
    },
    {
      "Test Case 17",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
      // IV
      {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce, 0xdb, 0xad},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0xc3, 0x76, 0x2d, 0xf1, 0xca, 0x78, 0x7d, 0x32,
       0xae, 0x47, 0xc1, 0x3b, 0xf1, 0x98, 0x44, 0xcb,
       0xaf, 0x1a, 0xe1, 0x4d, 0x0b, 0x97, 0x6a, 0xfa,
       0xc5, 0x2f, 0xf7, 0xd7, 0x9b, 0xba, 0x9d, 0xe0,
       0xfe, 0xb5, 0x82, 0xd3, 0x39, 0x34, 0xa4, 0xf0,
       0x95, 0x4c, 0xc2, 0x36, 0x3b, 0xc7, 0x3f, 0x78,
       0x62, 0xac, 0x43, 0x0e, 0x64, 0xab, 0xe4, 0x99,
       0xf4, 0x7c, 0x9b, 0x1f},
      // T
      {0x3a, 0x33, 0x7d, 0xbf, 0x46, 0xa7, 0x92, 0xc4,
       0x5e, 0x45, 0x49, 0x13, 0xfe, 0x2e, 0xa8, 0xf2},
    },
    {
      "Test Case 18",
      // K
      {0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08,
       0xfe, 0xff, 0xe9, 0x92, 0x86, 0x65, 0x73, 0x1c,
       0x6d, 0x6a, 0x8f, 0x94, 0x67, 0x30, 0x83, 0x08},
      // IV
      {0x93, 0x13, 0x22, 0x5d, 0xf8, 0x84, 0x06, 0xe5,
       0x55, 0x90, 0x9c, 0x5a, 0xff, 0x52, 0x69, 0xaa,
       0x6a, 0x7a, 0x95, 0x38, 0x53, 0x4f, 0x7d, 0xa1,
       0xe4, 0xc3, 0x03, 0xd2, 0xa3, 0x18, 0xa7, 0x28,
       0xc3, 0xc0, 0xc9, 0x51, 0x56, 0x80, 0x95, 0x39,
       0xfc, 0xf0, 0xe2, 0x42, 0x9a, 0x6b, 0x52, 0x54,
       0x16, 0xae, 0xdb, 0xf5, 0xa0, 0xde, 0x6a, 0x57,
       0xa6, 0x37, 0xb3, 0x9b},
      // A
      {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef,
       0xab, 0xad, 0xda, 0xd2},
      // P
      {0xd9, 0x31, 0x32, 0x25, 0xf8, 0x84, 0x06, 0xe5,
       0xa5, 0x59, 0x09, 0xc5, 0xaf, 0xf5, 0x26, 0x9a,
       0x86, 0xa7, 0xa9, 0x53, 0x15, 0x34, 0xf7, 0xda,
       0x2e, 0x4c, 0x30, 0x3d, 0x8a, 0x31, 0x8a, 0x72,
       0x1c, 0x3c, 0x0c, 0x95, 0x95, 0x68, 0x09, 0x53,
       0x2f, 0xcf, 0x0e, 0x24, 0x49, 0xa6, 0xb5, 0x25,
       0xb1, 0x6a, 0xed, 0xf5, 0xaa, 0x0d, 0xe6, 0x57,
       0xba, 0x63, 0x7b, 0x39},
      // C
      {0x5a, 0x8d, 0xef, 0x2f, 0x0c, 0x9e, 0x53, 0xf1,
       0xf7, 0x5d, 0x78, 0x53, 0x65, 0x9e, 0x2a, 0x20,
       0xee, 0xb2, 0xb2, 0x2a, 0xaf, 0xde, 0x64, 0x19,
       0xa0, 0x58, 0xab, 0x4f, 0x6f, 0x74, 0x6b, 0xf4,
       0x0f, 0xc0, 0xc3, 0xb7, 0x80, 0xf2, 0x44, 0x45,
       0x2d, 0xa3, 0xeb, 0xf1, 0xc5, 0xd8, 0x2c, 0xde,
       0xa2, 0x41, 0x89, 0x97, 0x20, 0x0e, 0xf8, 0x2e,
       0x44, 0xae, 0x7e, 0x3f},
      // T
      {0xa4, 0x4a, 0x82, 0x66, 0xee, 0x1c, 0x8e, 0xb0,
       0xc8, 0xb5, 0xd4, 0xcf, 0x5a, 0xe9, 0xf1, 0x9a},
    },
  };
  return vec;
}

TEST(AES, Encrypt) {
  for (const auto& row : raw_testdata()) {
    SCOPED_TRACE(row.name);
    auto crypter = crypto::cipher::new_aes(row.key);
    std::vector<uint8_t> tmp;
    tmp.resize(row.plaintext.size());
    crypter->block_encrypt(tmp, row.plaintext);
    EXPECT_EQ(hex(row.ciphertext), hex(tmp));
  }
}

TEST(AES, Decrypt) {
  for (const auto& row : raw_testdata()) {
    SCOPED_TRACE(row.name);
    auto crypter = crypto::cipher::new_aes(row.key);
    std::vector<uint8_t> tmp;
    tmp.resize(row.ciphertext.size());
    crypter->block_decrypt(tmp, row.ciphertext);
    EXPECT_EQ(hex(row.plaintext), hex(tmp));
  }
}

TEST(AES, CBC) {
  std::vector<uint8_t> key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                              0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                              0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                              0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
  std::vector<uint8_t> iv = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                             0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f};

  std::string tmp(LOREM_IPSUM);
  std::size_t overhang = (tmp.size() % crypto::cipher::AES_BLOCKSIZE);
  std::size_t padding = crypto::cipher::AES_BLOCKSIZE - overhang;

  std::vector<uint8_t> original;
  original.resize(tmp.size() + padding);
  ::memcpy(original.data(), tmp.data(), tmp.size());
  original[tmp.size()] = 0x80;

  std::vector<uint8_t> ciphertext0;
  ciphertext0.resize(original.size());
  auto crypter0 = crypto::cipher::new_aes_cbc(key, iv);
  crypter0->encrypt(ciphertext0, original);

  std::vector<uint8_t> ciphertext1;
  ciphertext1.resize(original.size());
  auto crypter1 = crypto::cipher::new_cbc(crypto::cipher::new_aes(key), iv);
  crypter1->encrypt(ciphertext1, original);

  EXPECT_EQ(hex(ciphertext0), hex(ciphertext1));

  std::vector<uint8_t> plaintext0;
  plaintext0.resize(original.size());
  crypter0 = crypto::cipher::new_aes_cbc(key, iv);
  crypter0->decrypt(plaintext0, ciphertext0);

  std::vector<uint8_t> plaintext1;
  plaintext1.resize(original.size());
  crypter1 = crypto::cipher::new_cbc(crypto::cipher::new_aes(key), iv);
  crypter1->decrypt(plaintext1, ciphertext1);

  EXPECT_EQ(hex(plaintext0), hex(plaintext1));
  EXPECT_EQ(hex(plaintext0), hex(original));
}

TEST(AES, CTR) {
  std::vector<uint8_t> key = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                              0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                              0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                              0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
  std::vector<uint8_t> iv = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                             0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f};

  std::string tmp(LOREM_IPSUM);

  std::vector<uint8_t> original;
  original.resize(tmp.size());
  ::memcpy(original.data(), tmp.data(), tmp.size());

  std::vector<uint8_t> ciphertext0;
  ciphertext0.resize(original.size());
  auto crypter0 = crypto::cipher::new_aes_ctr(key, iv);
  crypter0->encrypt(ciphertext0, original);

  std::vector<uint8_t> ciphertext1;
  ciphertext1.resize(original.size());
  auto crypter1 = crypto::cipher::new_ctr(crypto::cipher::new_aes(key), iv);
  crypter1->encrypt(ciphertext1, original);

  EXPECT_EQ(hex(ciphertext0), hex(ciphertext1));

  std::vector<uint8_t> plaintext0;
  plaintext0.resize(original.size());
  EXPECT_OK(crypter0->seek(0, SEEK_SET));
  crypter0->decrypt(plaintext0, ciphertext0);

  std::vector<uint8_t> plaintext1;
  plaintext1.resize(original.size());
  EXPECT_OK(crypter1->seek(0, SEEK_SET));
  crypter1->decrypt(plaintext1, ciphertext1);

  EXPECT_EQ(hex(plaintext0), hex(plaintext1));
  EXPECT_EQ(hex(plaintext0), hex(original));
}

TEST(AES, GCM) {
  for (const auto& row : gcm_testdata()) {
    SCOPED_TRACE(row.name);
    ASSERT_EQ(row.plaintext.size(), row.ciphertext.size());

    crypto::Tag tag0, tag1;

    std::vector<uint8_t> ciphertext0, plaintext0;
    std::vector<uint8_t> ciphertext1, plaintext1;
    ciphertext0.resize(row.plaintext.size());
    ciphertext1.resize(row.plaintext.size());
    plaintext0.resize(row.plaintext.size());
    plaintext1.resize(row.plaintext.size());

    auto sealer0 = crypto::cipher::new_aes_gcm(row.key);
    auto sealer1 = crypto::cipher::new_gcm(crypto::cipher::new_aes(row.key));

    sealer0->seal(&tag0, ciphertext0, row.plaintext, row.additional, row.nonce);
    sealer1->seal(&tag1, ciphertext1, row.plaintext, row.additional, row.nonce);
    EXPECT_EQ(hex(row.ciphertext), hex(ciphertext0));
    EXPECT_EQ(hex(row.ciphertext), hex(ciphertext1));
    EXPECT_EQ(hex(row.tag), hex(tag0.bytes()));
    EXPECT_EQ(hex(row.tag), hex(tag1.bytes()));

    crypto::Tag tag;
    tag.set_size(row.tag.size());
    ::memcpy(tag.mutable_data(), row.tag.data(), row.tag.size());

    EXPECT_TRUE(sealer0->unseal(tag, plaintext0, row.ciphertext, row.additional, row.nonce));
    EXPECT_TRUE(sealer1->unseal(tag, plaintext1, row.ciphertext, row.additional, row.nonce));
    EXPECT_EQ(hex(row.plaintext), hex(plaintext0));
    EXPECT_EQ(hex(row.plaintext), hex(plaintext1));
  }
}
