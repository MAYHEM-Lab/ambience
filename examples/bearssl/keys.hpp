//
// Created by fatih on 3/10/19.
//

#pragma once



#include <bearssl.h>
static const unsigned char CERT0[] = {
        0x30, 0x82, 0x05, 0xD0, 0x30, 0x82, 0x03, 0xB8, 0xA0, 0x03, 0x02, 0x01,
        0x02, 0x02, 0x09, 0x00, 0xE1, 0xED, 0xDE, 0x35, 0x88, 0x94, 0x7B, 0xE5,
        0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01,
        0x0B, 0x05, 0x00, 0x30, 0x7D, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55,
        0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03,
        0x55, 0x04, 0x08, 0x0C, 0x02, 0x43, 0x41, 0x31, 0x0F, 0x30, 0x0D, 0x06,
        0x03, 0x55, 0x04, 0x07, 0x0C, 0x06, 0x47, 0x6F, 0x6C, 0x65, 0x74, 0x61,
        0x31, 0x0C, 0x30, 0x0A, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x03, 0x54,
        0x6F, 0x73, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x0C,
        0x04, 0x4E, 0x6F, 0x6E, 0x65, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55,
        0x04, 0x03, 0x0C, 0x04, 0x54, 0x4F, 0x53, 0x53, 0x31, 0x24, 0x30, 0x22,
        0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x09, 0x01, 0x16,
        0x15, 0x6D, 0x66, 0x61, 0x74, 0x69, 0x68, 0x62, 0x61, 0x6B, 0x69, 0x72,
        0x40, 0x67, 0x6D, 0x61, 0x69, 0x6C, 0x2E, 0x63, 0x6F, 0x6D, 0x30, 0x1E,
        0x17, 0x0D, 0x31, 0x39, 0x30, 0x33, 0x31, 0x31, 0x30, 0x35, 0x30, 0x35,
        0x31, 0x32, 0x5A, 0x17, 0x0D, 0x32, 0x30, 0x30, 0x33, 0x31, 0x30, 0x30,
        0x35, 0x30, 0x35, 0x31, 0x32, 0x5A, 0x30, 0x7D, 0x31, 0x0B, 0x30, 0x09,
        0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0B, 0x30,
        0x09, 0x06, 0x03, 0x55, 0x04, 0x08, 0x0C, 0x02, 0x43, 0x41, 0x31, 0x0F,
        0x30, 0x0D, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0C, 0x06, 0x47, 0x6F, 0x6C,
        0x65, 0x74, 0x61, 0x31, 0x0C, 0x30, 0x0A, 0x06, 0x03, 0x55, 0x04, 0x0A,
        0x0C, 0x03, 0x54, 0x6F, 0x73, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55,
        0x04, 0x0B, 0x0C, 0x04, 0x4E, 0x6F, 0x6E, 0x65, 0x31, 0x0D, 0x30, 0x0B,
        0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x04, 0x54, 0x4F, 0x53, 0x53, 0x31,
        0x24, 0x30, 0x22, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01,
        0x09, 0x01, 0x16, 0x15, 0x6D, 0x66, 0x61, 0x74, 0x69, 0x68, 0x62, 0x61,
        0x6B, 0x69, 0x72, 0x40, 0x67, 0x6D, 0x61, 0x69, 0x6C, 0x2E, 0x63, 0x6F,
        0x6D, 0x30, 0x82, 0x02, 0x22, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48,
        0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x02, 0x0F,
        0x00, 0x30, 0x82, 0x02, 0x0A, 0x02, 0x82, 0x02, 0x01, 0x00, 0xD1, 0x80,
        0xB6, 0xF1, 0xE7, 0xAF, 0x95, 0xD1, 0xA9, 0x8E, 0xA0, 0xCD, 0x42, 0xC9,
        0x55, 0x1A, 0xEC, 0x30, 0xB1, 0xE4, 0x75, 0x04, 0x80, 0x65, 0xF1, 0xCB,
        0xED, 0xCB, 0x9B, 0xDB, 0x81, 0x71, 0x52, 0xD2, 0x51, 0x8B, 0xD5, 0xBA,
        0x7C, 0x30, 0xDB, 0x89, 0x40, 0x1C, 0xDF, 0x0D, 0x32, 0x24, 0xCB, 0xA5,
        0xEB, 0xD8, 0x29, 0xF9, 0x09, 0xCD, 0xBE, 0xF0, 0xD7, 0x86, 0x2C, 0x1B,
        0x08, 0xA5, 0x75, 0xD5, 0xF8, 0x8D, 0x7D, 0x05, 0xCE, 0x4B, 0x02, 0xC5,
        0x59, 0x9E, 0xEC, 0xD0, 0x57, 0x25, 0x86, 0xCF, 0x94, 0x0D, 0xE6, 0xA2,
        0x7C, 0x32, 0x37, 0x14, 0xA7, 0xCB, 0xF1, 0xDC, 0x0E, 0x81, 0xA0, 0x1B,
        0x85, 0x0B, 0x0A, 0x71, 0xC8, 0x99, 0xFB, 0xB6, 0x2C, 0xE3, 0xE3, 0x55,
        0x59, 0x7A, 0x1F, 0x06, 0x86, 0x76, 0x7E, 0xE0, 0xCC, 0x16, 0x8B, 0x48,
        0x04, 0x8F, 0xDE, 0x55, 0xE8, 0xEF, 0xE2, 0x68, 0xA9, 0x66, 0x10, 0x0D,
        0xB9, 0x69, 0x29, 0x2C, 0x1F, 0xF2, 0x99, 0x65, 0xC8, 0x1B, 0xF2, 0xC0,
        0x61, 0x26, 0x7F, 0xB2, 0x9E, 0xF8, 0x70, 0xAB, 0x65, 0x65, 0x42, 0xF6,
        0xCC, 0x7A, 0x9C, 0xF9, 0xC5, 0x11, 0xF3, 0x6A, 0x84, 0x13, 0x54, 0x86,
        0x1E, 0xC6, 0x40, 0x9E, 0xE1, 0x60, 0xB9, 0x42, 0xE9, 0xA5, 0xDB, 0xA3,
        0x5F, 0xCC, 0xB7, 0xED, 0x94, 0xA2, 0x65, 0xD5, 0xB5, 0xD7, 0x4B, 0x16,
        0xD2, 0xDA, 0x2B, 0x9C, 0xAD, 0x14, 0xE5, 0x82, 0x8F, 0xD6, 0x6D, 0x5E,
        0xBA, 0x11, 0x11, 0xDB, 0x24, 0x17, 0x86, 0x55, 0x9F, 0x88, 0x94, 0x12,
        0xA8, 0x81, 0x28, 0xF6, 0x70, 0x42, 0x79, 0x9C, 0x51, 0xB8, 0x45, 0xA8,
        0xE0, 0x3F, 0x0D, 0xB6, 0xDD, 0xE3, 0xCE, 0xED, 0x6D, 0xA6, 0xE2, 0xBF,
        0x79, 0x3A, 0x26, 0xBA, 0xAC, 0x77, 0xA3, 0x9A, 0xDD, 0x75, 0xB7, 0x2E,
        0x98, 0x9D, 0x0E, 0x30, 0x36, 0x28, 0x69, 0x55, 0x76, 0x73, 0x2F, 0x45,
        0x7E, 0x63, 0x94, 0x25, 0xF3, 0x09, 0x2A, 0x77, 0xF2, 0x1E, 0x32, 0x2D,
        0xC7, 0x9C, 0x4F, 0x70, 0x05, 0x78, 0xFC, 0x0D, 0x91, 0x8A, 0x32, 0x95,
        0x34, 0x69, 0x9E, 0xE7, 0x3D, 0x51, 0xA9, 0xC4, 0xCF, 0x36, 0x33, 0xD2,
        0xB6, 0x1F, 0x7C, 0x2E, 0x07, 0xCC, 0x0D, 0xDC, 0x15, 0x8D, 0x2C, 0x6A,
        0x10, 0xBC, 0xE8, 0x03, 0x5B, 0x09, 0xC2, 0x5A, 0x15, 0x88, 0x8B, 0xE3,
        0x11, 0xD4, 0xEF, 0x23, 0xE9, 0x02, 0x69, 0xB3, 0xEE, 0xD3, 0xB5, 0x62,
        0x72, 0x21, 0xB6, 0x9E, 0x90, 0x90, 0xF3, 0x47, 0xE1, 0x2C, 0x54, 0xA9,
        0x85, 0x7A, 0x51, 0xF8, 0x6A, 0xA3, 0x3C, 0x2A, 0x3B, 0x00, 0x67, 0x1F,
        0x2A, 0x7E, 0x90, 0x6F, 0xAD, 0x63, 0x9B, 0x06, 0xF0, 0x1F, 0x01, 0xD1,
        0x1D, 0x64, 0x19, 0x24, 0xDE, 0xFE, 0xD4, 0x29, 0x16, 0xB6, 0xCA, 0x6E,
        0x31, 0x84, 0x5F, 0x83, 0xFB, 0x77, 0x7A, 0xF4, 0xB9, 0xD1, 0xC5, 0xAB,
        0x0C, 0xEE, 0x05, 0x62, 0x77, 0xB2, 0xFC, 0xD2, 0x43, 0x14, 0x78, 0x76,
        0x7C, 0xFA, 0xFA, 0x30, 0x85, 0x85, 0x94, 0xE8, 0xF9, 0x05, 0x57, 0xB9,
        0xAE, 0xE0, 0x15, 0xC9, 0xD6, 0x0B, 0x70, 0xBA, 0x91, 0xDB, 0xB3, 0x8F,
        0x36, 0x91, 0x4F, 0x29, 0xD1, 0x15, 0x41, 0xE0, 0x2C, 0xFA, 0xEF, 0x87,
        0xCD, 0x7A, 0x74, 0x85, 0xD8, 0x38, 0xFA, 0x05, 0xF7, 0x21, 0x67, 0xB4,
        0xF5, 0x80, 0xA0, 0xCA, 0xFA, 0x45, 0x2C, 0xBB, 0xC5, 0x0D, 0x49, 0x85,
        0x42, 0xD8, 0x65, 0xDE, 0x19, 0xE7, 0xAB, 0x96, 0xC5, 0x8F, 0xEB, 0x45,
        0xB3, 0x32, 0xB7, 0x75, 0xD0, 0xF6, 0xFE, 0x04, 0x14, 0x05, 0x79, 0xE1,
        0x33, 0x35, 0xCC, 0x49, 0xF6, 0x68, 0x21, 0x29, 0x00, 0xE5, 0xB3, 0xE5,
        0x85, 0x93, 0x8B, 0xF0, 0xFA, 0xEB, 0x02, 0x03, 0x01, 0x00, 0x01, 0xA3,
        0x53, 0x30, 0x51, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16,
        0x04, 0x14, 0x01, 0xBC, 0x07, 0x0F, 0x23, 0xF7, 0x3E, 0x21, 0x12, 0xED,
        0xDD, 0xC8, 0x12, 0x02, 0x0E, 0xBB, 0x31, 0x50, 0x80, 0x19, 0x30, 0x1F,
        0x06, 0x03, 0x55, 0x1D, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0x01,
        0xBC, 0x07, 0x0F, 0x23, 0xF7, 0x3E, 0x21, 0x12, 0xED, 0xDD, 0xC8, 0x12,
        0x02, 0x0E, 0xBB, 0x31, 0x50, 0x80, 0x19, 0x30, 0x0F, 0x06, 0x03, 0x55,
        0x1D, 0x13, 0x01, 0x01, 0xFF, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF,
        0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01,
        0x0B, 0x05, 0x00, 0x03, 0x82, 0x02, 0x01, 0x00, 0x93, 0x9B, 0xF4, 0xA3,
        0xBB, 0x36, 0x94, 0xC4, 0x47, 0x28, 0x6F, 0x6A, 0xE8, 0xA1, 0x2D, 0xEA,
        0xF5, 0xE9, 0xF3, 0xCA, 0x11, 0x96, 0x4C, 0xFD, 0x04, 0xB9, 0xC6, 0x97,
        0x02, 0x46, 0x6A, 0x0F, 0x22, 0xE6, 0xB0, 0x8B, 0xA1, 0xFC, 0x77, 0x54,
        0xA4, 0xB9, 0x7C, 0xF8, 0xA8, 0xE3, 0x55, 0x37, 0x4C, 0x9E, 0xDD, 0x51,
        0x32, 0xC3, 0xDB, 0x5A, 0x5B, 0x04, 0xCB, 0x58, 0xF5, 0x6C, 0x9F, 0x9D,
        0x0A, 0xC1, 0x5A, 0x48, 0x04, 0x59, 0xD5, 0x36, 0x5E, 0xA1, 0x5B, 0x6B,
        0xC8, 0xA9, 0x24, 0xA7, 0x5D, 0x5F, 0xDC, 0x9C, 0x12, 0xB4, 0x83, 0xA2,
        0xD9, 0x5C, 0xFC, 0x9C, 0xF5, 0xCB, 0x19, 0x70, 0x7A, 0x98, 0x01, 0xE1,
        0x3E, 0x26, 0xE2, 0xB6, 0x21, 0xA1, 0xA0, 0x9E, 0xFB, 0xBD, 0x2D, 0x3F,
        0xEF, 0x2B, 0xC2, 0x31, 0xBF, 0x55, 0xC4, 0x6F, 0x2F, 0x73, 0xE0, 0x9F,
        0xE9, 0xE5, 0xA9, 0xE7, 0x92, 0xA0, 0x47, 0x82, 0x6B, 0x0A, 0x3C, 0xDB,
        0x25, 0x76, 0x01, 0x18, 0xD9, 0x31, 0x91, 0xF4, 0x64, 0xBD, 0x9A, 0x27,
        0x09, 0x49, 0x25, 0x6A, 0x76, 0xBB, 0xAA, 0xE6, 0x92, 0xB6, 0x5A, 0x91,
        0xE8, 0x44, 0x58, 0xE7, 0xB6, 0xBB, 0xD3, 0xAE, 0x0E, 0xC5, 0xF0, 0xD7,
        0xDB, 0xD3, 0x5F, 0x14, 0x37, 0xCE, 0x36, 0x66, 0x22, 0x2D, 0x93, 0x2F,
        0x57, 0x5F, 0x05, 0xFB, 0x5E, 0x99, 0x9D, 0x65, 0x7E, 0x43, 0xB6, 0xF4,
        0xAD, 0x10, 0x25, 0x6A, 0x4D, 0x00, 0x97, 0x7C, 0xB2, 0x4D, 0x78, 0x2D,
        0x78, 0x52, 0x99, 0xAC, 0xD8, 0xB4, 0x8A, 0x4C, 0x97, 0xAB, 0xC5, 0x3F,
        0xEF, 0x06, 0x5E, 0x0E, 0xD8, 0xD7, 0x3E, 0x25, 0x0B, 0xE7, 0x7E, 0xD4,
        0x7D, 0xE5, 0x35, 0x23, 0xA3, 0x55, 0xD8, 0x2F, 0x6A, 0xA7, 0x82, 0x32,
        0x3D, 0x76, 0x9F, 0xF1, 0xA0, 0x6E, 0xEF, 0x8F, 0x3F, 0x75, 0x01, 0x2E,
        0xF2, 0xEA, 0x5F, 0xF6, 0x74, 0x35, 0xF6, 0xAB, 0x23, 0xD8, 0xED, 0x06,
        0x29, 0x83, 0xC5, 0x4F, 0xC6, 0x7D, 0x11, 0x7C, 0x1C, 0x13, 0x1A, 0xA9,
        0x4A, 0xF3, 0x37, 0x7E, 0xA4, 0x72, 0x69, 0x97, 0x1F, 0x54, 0x19, 0x55,
        0x39, 0x61, 0x14, 0x72, 0x45, 0x60, 0xC1, 0xC7, 0x49, 0xCC, 0x5E, 0x84,
        0xCB, 0x11, 0x9B, 0xAB, 0x5E, 0x8C, 0x92, 0x5A, 0x9C, 0x08, 0x82, 0x6F,
        0xD1, 0x95, 0x62, 0xC0, 0xCF, 0x10, 0xE1, 0x9E, 0x0B, 0x9F, 0xF3, 0x9C,
        0x4D, 0x76, 0x9D, 0x2D, 0xC2, 0x83, 0xD6, 0x3E, 0xF5, 0x04, 0xA1, 0x56,
        0x76, 0x10, 0xD4, 0x24, 0x9B, 0xF2, 0x98, 0xFF, 0x92, 0xBD, 0x8E, 0xDD,
        0x35, 0x90, 0xC8, 0x08, 0xB8, 0x18, 0x8F, 0x6D, 0x49, 0x0F, 0x9B, 0xF3,
        0x26, 0xD6, 0xD1, 0xE5, 0x8E, 0xF6, 0x41, 0x8A, 0x82, 0x20, 0xCF, 0x6C,
        0x5B, 0xA8, 0x45, 0x40, 0xF8, 0x78, 0x2F, 0xA2, 0xB8, 0xED, 0x66, 0xD0,
        0xED, 0xC2, 0xDF, 0x8C, 0xA8, 0xDD, 0x91, 0xD9, 0x71, 0x3E, 0x5C, 0xB8,
        0xBC, 0x37, 0xD2, 0x8D, 0x78, 0x97, 0x3A, 0xF3, 0xF2, 0xC9, 0x66, 0xAB,
        0x0E, 0x48, 0x44, 0x74, 0x72, 0x92, 0xEB, 0x3C, 0xC1, 0xFE, 0x80, 0x20,
        0xDB, 0x9D, 0x9A, 0x52, 0x7E, 0xE1, 0x8C, 0x13, 0x1E, 0xC5, 0x1A, 0xC1,
        0x00, 0x46, 0x85, 0xA4, 0x63, 0x43, 0xBE, 0x44, 0x0A, 0xB8, 0xB4, 0x1D,
        0x7D, 0xDA, 0x9C, 0x6D, 0x14, 0xF4, 0xEE, 0x9A, 0xFF, 0xF8, 0x5B, 0xB1,
        0x92, 0x52, 0xA3, 0xA1, 0xE8, 0x64, 0xD9, 0xD5, 0x66, 0x17, 0xB4, 0x6F,
        0x4D, 0x47, 0x7F, 0x2E, 0x8D, 0x9D, 0x2E, 0xC0, 0x06, 0x6B, 0xD2, 0x7E,
        0x9F, 0xA0, 0x22, 0x8A, 0x9C, 0x52, 0x88, 0xC3, 0xC7, 0x6B, 0x8D, 0xFF,
        0x93, 0x58, 0x2C, 0xB7, 0xAB, 0x8B, 0x6E, 0x26, 0xF9, 0xF3, 0xCE, 0xF8,
        0x52, 0xDF, 0x5A, 0x49
};

static const br_x509_certificate CHAIN[] = {
        { (unsigned char *)CERT0, sizeof CERT0 }
};

#define CHAIN_LEN   1

static const unsigned char RSA_P[] = {
        0xEA, 0xAC, 0x6B, 0xDF, 0x32, 0xAB, 0x89, 0xF4, 0x2B, 0x69, 0x91, 0xE3,
        0x64, 0x8B, 0xEE, 0x00, 0xDD, 0xB9, 0x0A, 0x62, 0xA1, 0x62, 0xCE, 0xDA,
        0xC5, 0xF4, 0x09, 0x67, 0xE7, 0x42, 0x80, 0xE6, 0x24, 0x81, 0x7C, 0x51,
        0xF1, 0xE0, 0x48, 0xEF, 0xB6, 0xEA, 0x1E, 0x89, 0x0A, 0xCE, 0xC9, 0xA6,
        0x99, 0x0D, 0x64, 0x07, 0xA9, 0xB9, 0xD1, 0x0B, 0xDB, 0x3F, 0xF6, 0xD8,
        0xD8, 0x63, 0xBD, 0xAE, 0x79, 0x7C, 0x07, 0x0F, 0x92, 0x9D, 0xFB, 0x54,
        0x8B, 0x61, 0xFA, 0xF2, 0x7E, 0x9F, 0x40, 0x91, 0x76, 0x75, 0x9E, 0x19,
        0x33, 0xE4, 0xBD, 0x13, 0xA1, 0x6C, 0x7F, 0x52, 0xBB, 0xEB, 0x93, 0x85,
        0x52, 0x4E, 0x50, 0xDB, 0x25, 0x2A, 0xD4, 0x58, 0x20, 0xE6, 0x65, 0x76,
        0x14, 0x15, 0x4B, 0x38, 0x7C, 0x98, 0x4A, 0x48, 0x88, 0x4F, 0x27, 0x52,
        0xF3, 0x83, 0x4A, 0x10, 0x69, 0x0D, 0xDE, 0x7E, 0xD2, 0x14, 0x59, 0xC2,
        0xE7, 0x71, 0x6B, 0xA4, 0xCA, 0x1D, 0xFC, 0xC4, 0x0E, 0x9C, 0x95, 0xF1,
        0x0B, 0x65, 0xB1, 0xBA, 0x14, 0x13, 0x9C, 0xBD, 0xA9, 0x8F, 0x0D, 0x80,
        0x9B, 0x5D, 0x3E, 0xBE, 0x6D, 0xEE, 0x19, 0xD5, 0xA4, 0x2A, 0xD8, 0xDC,
        0xAE, 0x66, 0x4E, 0x59, 0xBD, 0x1E, 0xF9, 0x53, 0x54, 0x0D, 0xBB, 0xE4,
        0xCB, 0x04, 0xCF, 0x94, 0x59, 0xC7, 0x4E, 0xD4, 0x6E, 0x6C, 0x96, 0x61,
        0x64, 0x6B, 0x55, 0x9A, 0x43, 0xAD, 0x8B, 0x8C, 0x16, 0x1B, 0x83, 0x22,
        0x1A, 0x74, 0x5A, 0xC2, 0x4E, 0x19, 0x38, 0xD7, 0x26, 0x13, 0x56, 0x00,
        0x28, 0xEF, 0xD6, 0x0E, 0x4C, 0xAC, 0x1D, 0xAE, 0xB3, 0x56, 0x1E, 0xAE,
        0xFD, 0x1B, 0x00, 0x2E, 0xA9, 0xF3, 0xC3, 0x5A, 0x0D, 0x85, 0x65, 0xC5,
        0x9D, 0xB8, 0xF7, 0x36, 0x87, 0xB9, 0x08, 0xED, 0x65, 0x97, 0xAD, 0xD8,
        0x9D, 0xC0, 0x1C, 0x07
};

static const unsigned char RSA_Q[] = {
        0xE4, 0x8A, 0xB5, 0x01, 0x28, 0xAF, 0xFC, 0x10, 0x5F, 0x91, 0x40, 0x55,
        0xEE, 0x4A, 0xFD, 0xA3, 0xBA, 0xC1, 0x14, 0x8D, 0xD1, 0xA2, 0x9B, 0x96,
        0x7A, 0x88, 0xE9, 0x3B, 0x40, 0x33, 0x40, 0x3B, 0xBC, 0x6D, 0xCA, 0xD7,
        0xF7, 0x0E, 0x19, 0x26, 0x8B, 0xFA, 0x7C, 0x4E, 0x30, 0x03, 0x3C, 0xC8,
        0xE2, 0xDE, 0x19, 0x09, 0xDA, 0x04, 0x6F, 0x49, 0xA4, 0xBF, 0x31, 0xC7,
        0xE8, 0x4A, 0x2A, 0x41, 0xBD, 0x71, 0x00, 0xE9, 0x3B, 0xA7, 0xA1, 0x7D,
        0x2E, 0xF4, 0xEF, 0x67, 0x87, 0x4B, 0x92, 0x01, 0xA4, 0x77, 0x6A, 0x23,
        0xD4, 0x9C, 0x4E, 0xAC, 0x43, 0x82, 0xB0, 0x65, 0x3D, 0xB7, 0x7D, 0x07,
        0x5B, 0x26, 0xA9, 0x8B, 0x40, 0x3E, 0x95, 0xC2, 0x07, 0x6D, 0x0F, 0x3F,
        0x45, 0x25, 0x5C, 0xEC, 0x81, 0xD4, 0x6C, 0xBF, 0x8B, 0x17, 0x3F, 0x37,
        0x25, 0x3C, 0x0D, 0x2A, 0x4B, 0xE6, 0x90, 0xB2, 0xC7, 0x99, 0x8A, 0x56,
        0x5C, 0x2E, 0x2E, 0x01, 0xB4, 0x07, 0x7C, 0x5B, 0x0F, 0x0E, 0xB2, 0x42,
        0xC8, 0xF9, 0x69, 0xF7, 0x56, 0x87, 0x96, 0xC1, 0x38, 0x54, 0x8A, 0x5B,
        0x5C, 0x1A, 0x15, 0x87, 0x37, 0x03, 0xF5, 0xB4, 0xD2, 0x82, 0xC7, 0xA0,
        0xFB, 0x11, 0x1B, 0xA5, 0xB3, 0xBC, 0x9A, 0xA7, 0x21, 0xDB, 0x13, 0xB4,
        0x7F, 0x8E, 0xF1, 0xED, 0xEB, 0xAE, 0x1F, 0xC8, 0xE3, 0x95, 0xE4, 0x0D,
        0x17, 0x93, 0x67, 0x60, 0xE9, 0x48, 0x90, 0xAF, 0x9E, 0xC3, 0xE9, 0x94,
        0xE8, 0x88, 0x95, 0x82, 0xEF, 0xDA, 0x85, 0xF0, 0x81, 0x6C, 0xF0, 0x07,
        0x80, 0xF2, 0xDA, 0x05, 0x74, 0x59, 0x92, 0x36, 0x1E, 0xDD, 0xF4, 0xE6,
        0x53, 0x31, 0x83, 0x75, 0x25, 0xC1, 0x29, 0x06, 0x68, 0x1C, 0x9F, 0x33,
        0xEC, 0x93, 0x1C, 0xCB, 0xBA, 0x82, 0xEE, 0x9C, 0x44, 0x88, 0x74, 0xAF,
        0xAF, 0xFE, 0x78, 0xFD
};

static const unsigned char RSA_DP[] = {
        0x7D, 0x67, 0x07, 0xF9, 0xEC, 0xAC, 0xC7, 0xFA, 0x67, 0x9C, 0x71, 0x6E,
        0x2F, 0x13, 0xAB, 0x6A, 0x61, 0x11, 0x79, 0xA0, 0x3D, 0x17, 0x7C, 0xEC,
        0x61, 0x6D, 0xC7, 0xFC, 0xF3, 0x12, 0x91, 0xA8, 0xEA, 0x07, 0x1B, 0xB4,
        0xFA, 0x68, 0xDC, 0xF0, 0xE4, 0x30, 0xF5, 0x82, 0x4B, 0xF9, 0x36, 0xBE,
        0xC4, 0xF8, 0xE7, 0xE4, 0x44, 0x69, 0x5C, 0x71, 0x56, 0x87, 0x36, 0xD4,
        0xA7, 0xC5, 0x9C, 0xDD, 0xF8, 0x63, 0x33, 0xAE, 0xB9, 0xAD, 0x9C, 0x59,
        0x90, 0x66, 0x1A, 0x60, 0xAD, 0x13, 0x52, 0xC0, 0xA8, 0xA9, 0xAF, 0x83,
        0x4E, 0x3C, 0xC4, 0x61, 0x12, 0x18, 0xD5, 0x6D, 0x13, 0xA8, 0x8E, 0x5B,
        0x67, 0x21, 0x1C, 0x8A, 0x0E, 0xB1, 0x75, 0x69, 0x66, 0xA3, 0x57, 0xC2,
        0xA2, 0x76, 0xC2, 0x8C, 0x7B, 0xE0, 0x31, 0x89, 0xD2, 0xF5, 0x61, 0x51,
        0x2D, 0x79, 0x92, 0x01, 0x74, 0xF2, 0x09, 0x5C, 0x81, 0xCC, 0x88, 0x2E,
        0xE3, 0xD8, 0xB3, 0x2A, 0x0B, 0x2B, 0xBD, 0x1F, 0xD3, 0xA3, 0x0F, 0x69,
        0x10, 0xAB, 0x16, 0x2F, 0x16, 0x7D, 0x68, 0x7B, 0xDD, 0x78, 0xE9, 0x41,
        0x39, 0x44, 0xB9, 0xBB, 0xAF, 0x28, 0xD6, 0x3B, 0x59, 0x0F, 0xE0, 0x75,
        0x04, 0xE8, 0xFC, 0xA3, 0x6D, 0xB1, 0x78, 0x90, 0x73, 0xD1, 0x26, 0xFF,
        0xDB, 0x11, 0x46, 0x0F, 0xC3, 0x6E, 0x03, 0x0C, 0xF7, 0xB9, 0x25, 0x1F,
        0x04, 0x37, 0x6B, 0x36, 0x15, 0x46, 0x4E, 0xD4, 0xDC, 0x17, 0x75, 0x9A,
        0xAA, 0xC2, 0x54, 0xA1, 0x92, 0x74, 0x5D, 0xF0, 0x72, 0x93, 0x50, 0x98,
        0xA3, 0x47, 0x8E, 0x7E, 0x73, 0xB0, 0x61, 0xE8, 0x32, 0x43, 0x2A, 0x00,
        0x5E, 0x96, 0xA8, 0x21, 0x47, 0x86, 0x50, 0x74, 0xF5, 0xD6, 0xBC, 0x0C,
        0xBE, 0xFC, 0xD1, 0x76, 0x1D, 0x4E, 0xDE, 0x9B, 0x65, 0x77, 0x89, 0x59,
        0xC0, 0xEA, 0xA0, 0xCF
};

static const unsigned char RSA_DQ[] = {
        0x74, 0x81, 0x25, 0x4E, 0xA6, 0xF0, 0x37, 0xBE, 0x1D, 0x09, 0xCD, 0xD3,
        0x40, 0x7B, 0xE8, 0x1B, 0x0C, 0x3C, 0x1B, 0x7F, 0x44, 0x6B, 0xF1, 0x86,
        0xDF, 0x86, 0x65, 0xE7, 0x37, 0x8E, 0x28, 0xE3, 0x8A, 0xE6, 0x29, 0x6E,
        0xB8, 0xD9, 0xEE, 0x06, 0x51, 0x7B, 0x6A, 0xDC, 0xEC, 0xEA, 0xE9, 0x94,
        0xBA, 0xDA, 0x5F, 0xC7, 0x3E, 0xE5, 0xDD, 0xD9, 0x9B, 0xEB, 0x7F, 0xB4,
        0x19, 0xFD, 0x9C, 0xD7, 0x10, 0x7C, 0xC7, 0xEB, 0x1D, 0xE7, 0x11, 0x92,
        0xE0, 0x5A, 0x2A, 0xA5, 0x0B, 0x3A, 0x81, 0xFE, 0xE6, 0x59, 0x9D, 0xB9,
        0x10, 0x4C, 0x72, 0x6A, 0xAC, 0xAB, 0xB5, 0xC2, 0x96, 0x98, 0xB0, 0x5B,
        0x0C, 0xA3, 0x83, 0xF0, 0xA1, 0xDA, 0x85, 0xBB, 0x78, 0xCF, 0xA6, 0xE3,
        0x29, 0x1B, 0xA7, 0xE8, 0x45, 0x33, 0x06, 0x11, 0x0B, 0x15, 0xD9, 0xBB,
        0x01, 0xF7, 0x3F, 0xFA, 0xDC, 0xE0, 0x77, 0xC9, 0x3B, 0xA9, 0x50, 0xF5,
        0x3F, 0x5E, 0xA7, 0x43, 0x65, 0x04, 0x1F, 0xE7, 0xC7, 0xA0, 0x45, 0x4D,
        0x78, 0x68, 0x75, 0x15, 0x8A, 0x7D, 0xEA, 0x63, 0x2B, 0x95, 0x25, 0x46,
        0xF7, 0x87, 0xB0, 0x8B, 0xD2, 0x86, 0xE9, 0x3D, 0xB9, 0xC6, 0xC3, 0x7F,
        0x94, 0x1D, 0x9F, 0x8B, 0x74, 0x39, 0xE4, 0x58, 0xE9, 0x10, 0x66, 0x56,
        0x30, 0x64, 0xAD, 0x7E, 0x0D, 0x14, 0x21, 0xCD, 0xE5, 0xFE, 0xE6, 0x57,
        0xAA, 0x3A, 0x11, 0x8C, 0x3C, 0xC0, 0x2A, 0x49, 0xEC, 0xD4, 0x90, 0x81,
        0xC1, 0x9C, 0xC7, 0x23, 0x0D, 0xA4, 0xC7, 0x7C, 0xD8, 0x6D, 0xB1, 0x5B,
        0x11, 0xE8, 0x3B, 0x2E, 0x66, 0xA6, 0xB6, 0xBB, 0x72, 0x36, 0xD1, 0xFE,
        0x8C, 0x50, 0x84, 0x64, 0x36, 0xCF, 0xE6, 0x5B, 0x06, 0xBD, 0xF8, 0x86,
        0x9E, 0xC6, 0x4B, 0xFA, 0xAB, 0x64, 0xCD, 0xC0, 0xC9, 0xCB, 0x0A, 0x38,
        0xF9, 0xAD, 0xEE, 0xFD
};

static const unsigned char RSA_IQ[] = {
        0xAB, 0xAE, 0xF3, 0xD9, 0xED, 0xC2, 0xFC, 0x25, 0xBD, 0xAB, 0x05, 0x3B,
        0x79, 0xF4, 0x09, 0xF0, 0x91, 0xF1, 0x4A, 0x43, 0xF9, 0xD4, 0x66, 0x82,
        0x35, 0xD4, 0xB4, 0x37, 0x70, 0x5F, 0xD0, 0x4D, 0x5C, 0xE2, 0x87, 0x78,
        0x53, 0x63, 0x46, 0xBC, 0x04, 0xA0, 0x0A, 0x48, 0xA8, 0xB0, 0xCB, 0x7C,
        0xCB, 0xAC, 0x91, 0x9D, 0xF6, 0xEB, 0x70, 0x5B, 0xF0, 0xD1, 0x8F, 0x28,
        0xF4, 0xA0, 0xDD, 0x39, 0xC0, 0x50, 0x7B, 0x63, 0xDD, 0xBA, 0x60, 0xF5,
        0x74, 0xDE, 0xAC, 0x27, 0xB5, 0xE4, 0x84, 0xC7, 0x11, 0xF5, 0x95, 0x6D,
        0x20, 0x6F, 0x60, 0x31, 0xF7, 0xA8, 0x34, 0x01, 0xAD, 0xA5, 0x6E, 0x6B,
        0x1D, 0x83, 0xE0, 0xF9, 0xFE, 0x23, 0xB8, 0x40, 0x32, 0x6E, 0xCD, 0x7E,
        0xD2, 0x49, 0x52, 0x17, 0xEB, 0x8A, 0x49, 0x2C, 0xA2, 0x6A, 0xF3, 0x0E,
        0x9F, 0x72, 0x56, 0xE5, 0x43, 0x79, 0x04, 0x19, 0x69, 0x10, 0xA9, 0x53,
        0x26, 0x94, 0xAD, 0xEE, 0x01, 0xFF, 0xBC, 0x51, 0x9B, 0x32, 0xD8, 0x1E,
        0x50, 0xF4, 0xB8, 0x8D, 0xF1, 0x77, 0xFE, 0xD9, 0x67, 0xF0, 0xAE, 0xED,
        0x8B, 0xF6, 0x78, 0xFE, 0x02, 0x9F, 0x26, 0x2B, 0x3D, 0xBD, 0xEE, 0x0B,
        0xD1, 0xFB, 0x96, 0x7C, 0x09, 0x41, 0xC5, 0x34, 0x56, 0xC7, 0xCA, 0x08,
        0xE0, 0xC0, 0xA0, 0x74, 0x0E, 0x7F, 0x7A, 0x97, 0x40, 0x92, 0x77, 0x78,
        0x51, 0x96, 0x9B, 0x43, 0x06, 0x75, 0x4C, 0xA8, 0x63, 0x44, 0xF5, 0x5B,
        0xA8, 0xDA, 0x4D, 0x79, 0x27, 0x5A, 0xD8, 0x69, 0x33, 0x0D, 0x31, 0x48,
        0x8C, 0xBA, 0x76, 0x6A, 0xF0, 0x0C, 0x68, 0xE0, 0xB7, 0x72, 0x87, 0xD1,
        0xEE, 0x7C, 0xE1, 0x09, 0xF7, 0xD3, 0xF4, 0x5B, 0x02, 0x86, 0x46, 0xF8,
        0x0A, 0x41, 0xCA, 0x7F, 0x67, 0x40, 0xAC, 0x6B, 0xEC, 0x7E, 0x49, 0x05,
        0xF3, 0x5B, 0xAD, 0xDC
};

static const br_rsa_private_key RSA = {
        4096,
        (unsigned char *)RSA_P, sizeof RSA_P,
        (unsigned char *)RSA_Q, sizeof RSA_Q,
        (unsigned char *)RSA_DP, sizeof RSA_DP,
        (unsigned char *)RSA_DQ, sizeof RSA_DQ,
        (unsigned char *)RSA_IQ, sizeof RSA_IQ
};
