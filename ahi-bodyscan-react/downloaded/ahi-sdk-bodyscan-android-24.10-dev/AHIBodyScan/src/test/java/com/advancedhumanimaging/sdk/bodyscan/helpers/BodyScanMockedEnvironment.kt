//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.helpers

import com.advancedhumanimaging.sdk.common.models.AHILogging
import io.mockk.every
import io.mockk.mockkObject
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.TestDispatcher
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.setMain
import org.junit.jupiter.api.extension.AfterEachCallback
import org.junit.jupiter.api.extension.BeforeEachCallback
import org.junit.jupiter.api.extension.ExtensionContext


class BodyScanMockedEnvironment (private val dispatcher: TestDispatcher = StandardTestDispatcher()) :
    AfterEachCallback, BeforeEachCallback {

    override fun beforeEach(context: ExtensionContext?) {
        mockLogging()
        Dispatchers.setMain(dispatcher)
    }

    private fun mockLogging() {
        mockkObject(AHILogging)
        every { AHILogging.setLogLevel(any()) } returns Unit
        every { AHILogging.log(any(), any()) } returns Unit
    }
    override fun afterEach(context: ExtensionContext?) {
        Dispatchers.resetMain()
    }
}