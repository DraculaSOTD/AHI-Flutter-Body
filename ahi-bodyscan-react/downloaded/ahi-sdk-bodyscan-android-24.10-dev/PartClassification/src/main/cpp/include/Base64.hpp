//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#pragma once

#include "Types.hpp"

class Base64
{
public:
    static const char* BASE64_DEFAULT;
    static const char* BASE64_URLSAFE;

    static Base64* getInstance();

    std::string base64_encode(const unsigned char*, size_t len, bool bUrlSafe = true, bool bPad = false);
    std::string base64_encode(const char*, size_t len, bool bUrlSafe = true, bool bPad = false);

    bool base64_decode_isUrlSafe(const std::string &in);
    size_t base64_decode_size(const std::string &in);
    size_t base64_decode(const std::string &in, char* dest);
    std::string base64_decode_toStr(const std::string &in);
    std::string base64_decode_toHex(const std::string &in);

private:
    static Base64* pThis;

    std::vector<int> DECODE_DEFAULT;
    std::vector<int> DECODE_URLSAFE;

    Base64();
};