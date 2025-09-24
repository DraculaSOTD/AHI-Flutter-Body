//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AHIMULTISCAN_KEYSUTIL_HPP
#define AHIMULTISCAN_KEYSUTIL_HPP


#include <string>

class KeysUtil {
public:
    static KeysUtil *getInstance();

    static void updateKeys(std::string encodedKeys, std::string delim);

    static std::string getAHIVerifyKey();

    static std::string getAHIPrivKey();


private:
    static KeysUtil *inst_;   // The one, single instance
    std::string encodedKeyVerify;
    std::string encodedKeyPriv;
};


#endif //AHIMULTISCAN_KEYSUTIL_HPP
