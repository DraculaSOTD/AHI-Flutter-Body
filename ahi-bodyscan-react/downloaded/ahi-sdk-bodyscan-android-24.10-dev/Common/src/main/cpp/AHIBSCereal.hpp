//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/portable_binary.hpp>

#ifndef ahi_cereal_hpp
#define ahi_cereal_hpp


// Declaration of OpenCV model structure
struct AHIModelCV {
    std::string name;
    int type; // 1: vi, 2: vd, 3: vvd, 4: vf, 5: vvf
    std::vector<int> vi;
    std::vector<double> vd;
    std::vector<std::vector<double> > vvd;
    std::vector<float> vf;
    std::vector<std::vector<float> > vvf;

    template<class Archive>
    void serialize(Archive &ar) {
        ar(name, type, vi, vd, vvd, vf, vvf);
    }
};

// Declaration of SVR model structure
struct AHIModelSVR {
    std::string name;
    std::vector<std::vector<double> > vectors;
    std::vector<double> coefficients;
    std::vector<double> intercepts;

    template<class Archive>
    void serialize(Archive &ar) {
        ar(name, vectors, coefficients, intercepts);
    }
};

// Decoder of OpenCV model resource.
AHIModelCV ahiDecodeCv(const std::string &cv_file_path);

AHIModelCV ahiDecodeCvFromBytes(char *bytes, size_t len);

// Decoder of SVR model resource.
AHIModelSVR ahiDecodeSvr(const std::string &svr_file_path);

AHIModelSVR ahiDecodeSvrFromBytes(char *bytes, size_t len);


#endif /* ahi_cereal_hpp */
