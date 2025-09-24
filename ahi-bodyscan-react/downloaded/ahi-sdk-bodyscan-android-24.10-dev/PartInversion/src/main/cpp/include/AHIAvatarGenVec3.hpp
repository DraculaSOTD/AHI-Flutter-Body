//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AHIAvatarGenVec3_hpp
#define AHIAvatarGenVec3_hpp

#include <cstddef>

namespace avatar_gen {
    class AHIAvatarGenVec3 {
    public:
        //TODO: shared pointer?? pmVec = std::make_shared(new float[3], [](float* p) {delete[] p; });
        float *mVec;
        bool bFree;

        AHIAvatarGenVec3(float *src);

        AHIAvatarGenVec3();

        AHIAvatarGenVec3(float x, float y, float z);

        AHIAvatarGenVec3(const AHIAvatarGenVec3 &v3d);

        virtual ~AHIAvatarGenVec3();

        void set(float x, float y, float z);

        void set(AHIAvatarGenVec3 &v3d);

        float operator[](size_t i);

        /**
         * Computes the cross-product of two vectors q and r and places
         * the result in out.
         */
        void cross(AHIAvatarGenVec3 &q, AHIAvatarGenVec3 &r);

        void add(AHIAvatarGenVec3 &q);

        void add(AHIAvatarGenVec3 &q, AHIAvatarGenVec3 &r);

        void subtract(AHIAvatarGenVec3 &q, AHIAvatarGenVec3 &r);

        void scale(float v);

        /**
         * Returns the length of a vector, given as three floats.
         */
        float length();

        float lengthSquared();

        /**
         * Normalizes the given vector.
         * Vectors with length zero are unaffected.
         */
        void normalize();

        void min(float x, float y, float z);

        void max(float x, float y, float z);

        void put(float *dest, size_t offset);

        void get(float *dest, size_t offset);

        float max();

        float absmax();
    };
}

#endif /* AHIAvatarGenVec3_hpp */