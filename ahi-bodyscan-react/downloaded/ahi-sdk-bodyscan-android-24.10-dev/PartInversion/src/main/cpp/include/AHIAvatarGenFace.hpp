//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef AHIAvatarGenFace_hpp
#define AHIAvatarGenFace_hpp

#include <cstddef>

namespace avatar_gen {
    class AHIAvatarGenFace {
    public:
        int *mFaces;
        bool bFree;

        AHIAvatarGenFace(int a, int b, int c);

        AHIAvatarGenFace(int *src);

        virtual ~AHIAvatarGenFace();

        int operator[](size_t i);
    };
}
#endif /* AHIAvatarGenFace_hpp */