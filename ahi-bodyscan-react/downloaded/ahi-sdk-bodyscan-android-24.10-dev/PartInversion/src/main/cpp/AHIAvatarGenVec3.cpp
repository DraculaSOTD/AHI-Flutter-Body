//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#include "AHIAvatarGenVec3.hpp"
#include <algorithm>

namespace avatar_gen {
    AHIAvatarGenVec3::AHIAvatarGenVec3(float *src) : bFree(false) {
        mVec = src;
    }

    AHIAvatarGenVec3::AHIAvatarGenVec3() : bFree(true) {
        mVec = new float[3];
    }

    AHIAvatarGenVec3::AHIAvatarGenVec3(float x, float y, float z) : bFree(true) {
        mVec = new float[3];
        mVec[0] = x;
        mVec[1] = y;
        mVec[2] = z;
    }

    AHIAvatarGenVec3::AHIAvatarGenVec3(const AHIAvatarGenVec3 &v3d) : bFree(v3d.bFree) {
        mVec = v3d.mVec;
    }

    AHIAvatarGenVec3::~AHIAvatarGenVec3() {
        if (bFree && nullptr != mVec) {
            delete mVec;
            mVec = nullptr;
        }
    }

    void AHIAvatarGenVec3::set(float x, float y, float z) {
        mVec[0] = x;
        mVec[1] = y;
        mVec[2] = z;
    }

    void AHIAvatarGenVec3::set(AHIAvatarGenVec3 &v3d) {
        mVec[0] = v3d.mVec[0];
        mVec[1] = v3d.mVec[1];
        mVec[2] = v3d.mVec[2];
    }

    float AHIAvatarGenVec3::operator[](size_t i) {
        return mVec[i];
    }

/*
 * Computes the cross-product of two vectors q and r and places
 * the result in out.
 */
    void AHIAvatarGenVec3::cross(AHIAvatarGenVec3 &q, AHIAvatarGenVec3 &r) {
        mVec[0] = r.mVec[1] * q.mVec[2] - r.mVec[2] * q.mVec[1];
        mVec[1] = r.mVec[2] * q.mVec[0] - r.mVec[0] * q.mVec[2];
        mVec[2] = r.mVec[0] * q.mVec[1] - r.mVec[1] * q.mVec[0];
    }

    void AHIAvatarGenVec3::add(AHIAvatarGenVec3 &q) {
        mVec[0] += q.mVec[0];
        mVec[1] += q.mVec[1];
        mVec[2] += q.mVec[2];
    }

    void AHIAvatarGenVec3::add(AHIAvatarGenVec3 &q, AHIAvatarGenVec3 &r) {
        mVec[0] = q.mVec[0] + r.mVec[0];
        mVec[1] = q.mVec[1] + r.mVec[1];
        mVec[2] = q.mVec[2] + r.mVec[2];
    }

    void AHIAvatarGenVec3::subtract(AHIAvatarGenVec3 &q, AHIAvatarGenVec3 &r) {
        mVec[0] = q.mVec[0] - r.mVec[0];
        mVec[1] = q.mVec[1] - r.mVec[1];
        mVec[2] = q.mVec[2] - r.mVec[2];
    }

    void AHIAvatarGenVec3::scale(float v) {
        mVec[0] *= v;
        mVec[1] *= v;
        mVec[2] *= v;
    }

/**
 * Returns the length of a vector, given as three floats.
 */
    float AHIAvatarGenVec3::length() {
        return (float) sqrt(mVec[0] * mVec[0] + mVec[1] * mVec[1] + mVec[2] * mVec[2]);
    }

    float AHIAvatarGenVec3::lengthSquared() {
        return (float) (mVec[0] * mVec[0] + mVec[1] * mVec[1] + mVec[2] * mVec[2]);
    }

/**
 * Normalizes the given vector.
 * Vectors with length zero are unaffected.
 */
    void AHIAvatarGenVec3::normalize() {
        float len = length();
        if (len != 0.0f) {
            float norm = 1.0f / len;
            mVec[0] *= norm;
            mVec[1] *= norm;
            mVec[2] *= norm;
        }
    }

    void AHIAvatarGenVec3::min(float x, float y, float z) {
        mVec[0] = std::min(mVec[0], x);
        mVec[1] = std::min(mVec[1], y);
        mVec[2] = std::min(mVec[2], z);
    }

    void AHIAvatarGenVec3::max(float x, float y, float z) {
        mVec[0] = std::max(mVec[0], x);
        mVec[1] = std::max(mVec[1], y);
        mVec[2] = std::max(mVec[2], z);
    }

    void AHIAvatarGenVec3::put(float *dest, size_t offset) {
        dest[offset] = mVec[0];
        dest[offset + 1] = mVec[1];
        dest[offset + 2] = mVec[2];
    }

    void AHIAvatarGenVec3::get(float *dest, size_t offset) {
        mVec[0] = dest[offset];
        mVec[1] = dest[offset + 1];
        mVec[2] = dest[offset + 2];
    }

    float AHIAvatarGenVec3::max() {
        return std::max(mVec[2], std::max(mVec[0], mVec[1]));
    }

    float AHIAvatarGenVec3::absmax() {
        return std::max(abs(mVec[2]), std::max(abs(mVec[0]), abs(mVec[1])));
    }
}