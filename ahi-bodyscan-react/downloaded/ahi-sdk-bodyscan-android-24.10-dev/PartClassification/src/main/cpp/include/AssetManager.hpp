//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#pragma once

#include "Types.hpp"
#include <cstdint>

/**
 * Abstraction for filesystem read from file/android asset.
 */
class Asset
{
    void *mHandle;
    bool mIsAsset;

public:

    Asset(void* handle, bool isAsset = false) : mHandle(handle), mIsAsset(isAsset)
    {

    }

    bool isAsset()
    {
        return mIsAsset;
    }

    void* handle()
    {
        return mHandle;
    }
};

class AssetManager
{
public:
    virtual Asset* open(const char* filename) = 0;
    virtual Asset* openIfReady(const char* filename) = 0;
    virtual void close(Asset* handle) = 0;
    virtual int64_t getLength(Asset* handle) = 0;
    virtual size_t read(Asset *handle, void *buf, size_t count) = 0;
    virtual int64_t seek(Asset *handle, int64_t offset, int whence) = 0;
    // Load the whole file in to memory.
    virtual void* load(Asset *handle) = 0;
};