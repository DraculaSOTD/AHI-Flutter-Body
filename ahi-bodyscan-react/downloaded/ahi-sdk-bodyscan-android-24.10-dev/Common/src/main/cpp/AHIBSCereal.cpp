//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AHIBSCereal.hpp"

struct membuf : std::streambuf {
    membuf(char *begin, char *end) {
        this->setg(begin, begin, end);
    }
};

// Decoder of OpenCV model resource.
AHIModelCV ahiDecodeCvFromBytes(char *bytes, size_t len) {
    membuf sbuf(bytes, bytes + len);
    std::istream in(&sbuf);
    cereal::PortableBinaryInputArchive archive(in);
    AHIModelCV cv;
    archive(cv);
    return cv;
}

// Decoder of OpenCV model resource.
AHIModelCV ahiDecodeCv(const std::string &cv_file_path) {
    // De-Serialize
    std::ifstream is(cv_file_path);
    cereal::PortableBinaryInputArchive archive(is);
    // Read into object
    AHIModelCV cv;
    archive(cv);
    // Return
    return cv;
}

// Decoder of SVR model resource.
AHIModelSVR ahiDecodeSvrFromBytes(char *bytes, size_t len) {
    membuf sbuf(bytes, bytes + len);
    std::istream in(&sbuf);
    cereal::PortableBinaryInputArchive archive(in);
    // Read into object
    AHIModelSVR svr;
    archive(svr);
    // Return
    return svr;
}

// Decoder of SVR model resource.
AHIModelSVR ahiDecodeSvr(const std::string &svr_file_path) {
    // De-Serialize
    std::ifstream is(svr_file_path);
    cereal::PortableBinaryInputArchive archive(is);
    // Read into object
    AHIModelSVR svr;
    archive(svr);
    // Return
    return svr;
}

