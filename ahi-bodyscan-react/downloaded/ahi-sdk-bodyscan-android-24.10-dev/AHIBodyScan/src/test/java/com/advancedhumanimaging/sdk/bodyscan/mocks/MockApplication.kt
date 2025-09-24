//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.mocks

import android.app.Application
import android.content.Context
import io.mockk.mockk

class MockApplication: Application() {
    override fun getApplicationContext(): Context {
        return mockk<Context>()
    }
}