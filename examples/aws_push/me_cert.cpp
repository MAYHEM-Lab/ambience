#include <stdint.h>
#include "resources.hpp"

unsigned char me_cert_der[] = {
  0x30, 0x82, 0x03, 0x59, 0x30, 0x82, 0x02, 0x41, 0xa0, 0x03, 0x02, 0x01,
  0x02, 0x02, 0x14, 0x4f, 0x1f, 0x98, 0xed, 0x79, 0xc0, 0xe2, 0x3d, 0xbd,
  0xa7, 0xd2, 0xd8, 0xd1, 0x43, 0xca, 0xfc, 0x22, 0xb7, 0x9a, 0x54, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x30, 0x4d, 0x31, 0x4b, 0x30, 0x49, 0x06, 0x03, 0x55, 0x04,
  0x0b, 0x0c, 0x42, 0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x20, 0x57, 0x65,
  0x62, 0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x73, 0x20, 0x4f,
  0x3d, 0x41, 0x6d, 0x61, 0x7a, 0x6f, 0x6e, 0x2e, 0x63, 0x6f, 0x6d, 0x20,
  0x49, 0x6e, 0x63, 0x2e, 0x20, 0x4c, 0x3d, 0x53, 0x65, 0x61, 0x74, 0x74,
  0x6c, 0x65, 0x20, 0x53, 0x54, 0x3d, 0x57, 0x61, 0x73, 0x68, 0x69, 0x6e,
  0x67, 0x74, 0x6f, 0x6e, 0x20, 0x43, 0x3d, 0x55, 0x53, 0x30, 0x1e, 0x17,
  0x0d, 0x31, 0x38, 0x30, 0x39, 0x32, 0x31, 0x30, 0x31, 0x32, 0x32, 0x33,
  0x31, 0x5a, 0x17, 0x0d, 0x34, 0x39, 0x31, 0x32, 0x33, 0x31, 0x32, 0x33,
  0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x1e, 0x31, 0x1c, 0x30, 0x1a, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x0c, 0x13, 0x41, 0x57, 0x53, 0x20, 0x49, 0x6f,
  0x54, 0x20, 0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74,
  0x65, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f,
  0x00, 0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xb6, 0x54,
  0x99, 0xbe, 0xf4, 0xe6, 0xab, 0x2f, 0x61, 0x7e, 0x64, 0x6b, 0x24, 0xa8,
  0x6d, 0x9b, 0x30, 0xd2, 0x0e, 0x76, 0x76, 0x72, 0x54, 0x23, 0x59, 0x1a,
  0xc3, 0x60, 0x9d, 0xc8, 0xa0, 0x2b, 0x5f, 0x92, 0x59, 0xc1, 0x40, 0x95,
  0x02, 0xec, 0x55, 0x45, 0x5f, 0xc4, 0xa5, 0xbf, 0x45, 0x2f, 0xe0, 0x00,
  0x77, 0xbe, 0xe5, 0x02, 0x97, 0x5a, 0x5f, 0x60, 0x41, 0xd6, 0x41, 0xf9,
  0x68, 0x1c, 0x4d, 0x99, 0x95, 0x61, 0xcb, 0x67, 0xf5, 0xb2, 0xb8, 0x23,
  0x32, 0xc2, 0xb4, 0x38, 0x3c, 0xb5, 0xb4, 0x9a, 0x2f, 0x58, 0xae, 0x50,
  0x2f, 0x91, 0xf1, 0xac, 0xc5, 0x66, 0x45, 0x8c, 0xfb, 0x07, 0x0f, 0x2a,
  0x73, 0x2e, 0x11, 0xd5, 0xe9, 0x82, 0xb4, 0xbe, 0x5d, 0x1c, 0x40, 0x6e,
  0x77, 0xce, 0x8e, 0xad, 0xb1, 0x47, 0x68, 0xd1, 0xf8, 0x75, 0xfc, 0x30,
  0xb6, 0xd8, 0x92, 0x8a, 0x10, 0xef, 0xe2, 0x25, 0x02, 0xae, 0xf8, 0x18,
  0x96, 0xa7, 0x90, 0x13, 0xa5, 0x3c, 0xc4, 0x57, 0x70, 0xb3, 0x00, 0x5e,
  0x4a, 0x89, 0xb6, 0xda, 0xe8, 0xcf, 0x8c, 0xcf, 0x58, 0xc5, 0xfa, 0x4d,
  0xd0, 0x37, 0x34, 0x81, 0x00, 0xfb, 0x1e, 0xb6, 0x36, 0x46, 0x20, 0x9b,
  0x7e, 0x20, 0x5c, 0xc9, 0x15, 0x9e, 0xae, 0x42, 0x08, 0xb9, 0xf7, 0xad,
  0x48, 0x92, 0x45, 0x89, 0x4b, 0x08, 0xbf, 0xe2, 0x16, 0xeb, 0xb7, 0xad,
  0xf0, 0xc8, 0x6d, 0x73, 0x62, 0x6e, 0xdf, 0x9a, 0xde, 0x9c, 0xe5, 0x22,
  0x62, 0xdb, 0x7d, 0x49, 0x3b, 0x1e, 0x9e, 0x46, 0xf4, 0x83, 0x5e, 0x52,
  0x7f, 0x09, 0x33, 0x6e, 0xcc, 0x5c, 0xee, 0x49, 0xcc, 0xac, 0xbb, 0xe5,
  0x5d, 0x5b, 0x49, 0xbc, 0x5a, 0xbd, 0x84, 0xe1, 0xf3, 0xe9, 0xbf, 0xea,
  0x07, 0xbd, 0x44, 0xda, 0x82, 0x09, 0x3e, 0x51, 0xc9, 0xb1, 0x36, 0xa8,
  0x58, 0x7f, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x60, 0x30, 0x5e, 0x30,
  0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14,
  0xea, 0x4b, 0x23, 0x36, 0x42, 0x3f, 0x1c, 0xa9, 0xe1, 0x45, 0x38, 0xc2,
  0x58, 0xe7, 0x10, 0x95, 0x61, 0x9e, 0x46, 0x30, 0x30, 0x1d, 0x06, 0x03,
  0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x77, 0xab, 0x17, 0x97, 0x56,
  0xa1, 0x67, 0x07, 0xf4, 0x72, 0xed, 0xdf, 0xe2, 0x32, 0x4b, 0x45, 0x97,
  0xf7, 0x46, 0x49, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01, 0x01,
  0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f,
  0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x07, 0x80, 0x30, 0x0d, 0x06,
  0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00,
  0x03, 0x82, 0x01, 0x01, 0x00, 0x69, 0x53, 0x0b, 0x3d, 0x64, 0x7f, 0x09,
  0x44, 0xd7, 0xab, 0xfd, 0xd1, 0x8d, 0xbe, 0x4c, 0xdc, 0x2b, 0x8c, 0xed,
  0xa9, 0xfa, 0x6e, 0x94, 0xf4, 0x1c, 0xa4, 0xd9, 0xd0, 0xe2, 0xda, 0xb1,
  0x00, 0xe0, 0xdd, 0x5d, 0x6f, 0x7e, 0xf9, 0x59, 0x7c, 0x88, 0xef, 0x8f,
  0x20, 0x49, 0xa8, 0x8f, 0x9d, 0xed, 0xea, 0x40, 0x69, 0x21, 0xb7, 0x37,
  0x22, 0x2f, 0x9a, 0x59, 0x6e, 0xef, 0x61, 0x9c, 0xd0, 0xd2, 0x46, 0xa4,
  0x17, 0x06, 0x52, 0xc3, 0x79, 0x97, 0xc0, 0xf2, 0x6f, 0x7e, 0x7a, 0x03,
  0x0f, 0x10, 0x9e, 0x80, 0x98, 0xa3, 0x98, 0x01, 0xc5, 0x94, 0x57, 0xfd,
  0xc7, 0xbc, 0x85, 0x14, 0x62, 0x69, 0xa8, 0x40, 0x1a, 0xc3, 0x7c, 0x74,
  0x7e, 0x51, 0x87, 0xeb, 0xbf, 0xc8, 0xe8, 0xe7, 0xdd, 0x50, 0x8f, 0x67,
  0x3e, 0x00, 0xf1, 0xb4, 0x83, 0x41, 0x49, 0x9b, 0x8f, 0xc4, 0x0c, 0xcd,
  0xeb, 0x71, 0x5e, 0xbe, 0x7b, 0x78, 0xce, 0xbf, 0x5e, 0xa0, 0x20, 0x6e,
  0x49, 0xe2, 0x32, 0x95, 0xa3, 0x07, 0x73, 0x1d, 0x39, 0xd7, 0x36, 0x6b,
  0xe8, 0xeb, 0xbc, 0x3f, 0x92, 0xaf, 0xf3, 0xdb, 0x5d, 0x24, 0x6d, 0xbc,
  0x18, 0x84, 0x7f, 0x96, 0x50, 0xe3, 0xa8, 0x50, 0xc4, 0x6c, 0xb1, 0xa4,
  0x26, 0x0a, 0x2c, 0xf5, 0x70, 0x08, 0x3f, 0xd1, 0x01, 0xd6, 0x9d, 0x06,
  0x92, 0x9f, 0x13, 0xda, 0xa7, 0x4e, 0xf6, 0x1a, 0x64, 0x79, 0x72, 0x4c,
  0xfc, 0xdd, 0x3b, 0xd6, 0x29, 0xc9, 0x79, 0xf6, 0x49, 0x0c, 0x5b, 0x97,
  0x2b, 0xa1, 0xfd, 0x8e, 0xf3, 0xeb, 0xe3, 0x19, 0xd1, 0x69, 0xe2, 0xb9,
  0x46, 0x8d, 0xab, 0xc5, 0xaf, 0x83, 0x2e, 0x26, 0x93, 0x74, 0xe8, 0x7f,
  0xe4, 0x0a, 0x1b, 0x5b, 0x45, 0x96, 0x61, 0x5c, 0xb2, 0x42, 0x78, 0x99,
  0xe3, 0xa7, 0x25, 0x67, 0xe1, 0xef, 0x19, 0x49, 0xef
};
unsigned int me_cert_der_len = 861;


tos::span<const uint8_t> me_cert { (const uint8_t*)me_cert_der, me_cert_der_len };