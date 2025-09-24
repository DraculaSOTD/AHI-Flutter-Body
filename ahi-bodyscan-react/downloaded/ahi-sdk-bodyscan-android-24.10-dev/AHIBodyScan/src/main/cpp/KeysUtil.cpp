//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include <vector>
#include "include/KeysUtil.hpp"
#include "AHICrypto.hpp"
#include "AHILogging.hpp"
#include "AHICrypto.hpp"

using namespace std;

KeysUtil *KeysUtil::inst_ = NULL;

KeysUtil *KeysUtil::getInstance() {
    if (inst_ == NULL) {
        inst_ = new KeysUtil();
    }
    return (inst_);
}


void KeysUtil::updateKeys(std::string encodedKeys, std::string delim) {
    try {
        auto keysUtil = KeysUtil::getInstance();
        if (delim == "BODYSCANKEY:") {
            auto bodyScanKeys = AHIUtilities::split(encodedKeys, delim);
            keysUtil->encodedKeyVerify = bodyScanKeys[0];
            keysUtil->encodedKeyPriv = bodyScanKeys[1];
        } else if (delim == "AHIKEY:") {
            auto licensekeys = AHIUtilities::split(encodedKeys, delim);
            keysUtil->encodedKeyVerify = licensekeys[2];
            keysUtil->encodedKeyPriv = licensekeys[3];
        }
    }
    catch (exception) {
//        do nothing
    }
}

std::string KeysUtil::getAHIVerifyKey() {
    std::string decodedTokenVerify = AHIUtilities::decode91(
            KeysUtil::getInstance()->encodedKeyVerify);
    return decodedTokenVerify;
}

std::string KeysUtil::getAHIPrivKey() {
    std::string decodedTokenPriv = AHIUtilities::decode91(
            KeysUtil::getInstance()->encodedKeyPriv);
    return decodedTokenPriv;
}
