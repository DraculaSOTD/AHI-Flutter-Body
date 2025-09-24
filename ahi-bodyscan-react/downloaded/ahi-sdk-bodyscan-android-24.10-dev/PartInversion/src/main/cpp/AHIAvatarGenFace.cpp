//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AHIAvatarGenFace.hpp"

namespace avatar_gen {
    AHIAvatarGenFace::AHIAvatarGenFace(int a, int b, int c) : bFree(true) {
        mFaces = new int[3];
        mFaces[0] = a;
        mFaces[1] = b;
        mFaces[2] = c;
    }

    AHIAvatarGenFace::AHIAvatarGenFace(int *src) : bFree(false) {
        mFaces = src;
    }

    AHIAvatarGenFace::~AHIAvatarGenFace() {
        if (bFree && nullptr != mFaces) {
            delete mFaces;
            mFaces = nullptr;
        }
    }

    int AHIAvatarGenFace::operator[](size_t i) {
        return mFaces[i];
    }
}