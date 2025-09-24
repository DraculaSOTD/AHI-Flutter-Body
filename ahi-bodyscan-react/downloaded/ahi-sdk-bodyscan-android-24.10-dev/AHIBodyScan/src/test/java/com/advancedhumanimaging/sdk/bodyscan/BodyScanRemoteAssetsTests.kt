//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan

import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.helpers.BodyScanMockedEnvironment
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.runTest
import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Test
import org.junit.jupiter.api.extension.ExtendWith

@ExperimentalCoroutinesApi
@ExtendWith(BodyScanMockedEnvironment::class)
class BodyScanRemoteAssetsTests {

    @Test
    fun givenAreResourcesDownloaded_whenSetupNotDone_thenFalse() = runTest {
        val result = BodyScan().areResourcesDownloaded()
        Assertions.assertEquals(false, result)
    }

    @Test
    fun givenDownloadResourcesInForegroundCalled_whenSetupNotDone_thenError() = runTest {
        val result = BodyScan().downloadResourcesInForeground(1)
        Assertions.assertEquals(BodyScanError.BODY_SCAN_REMOTE_ASSET_NOT_INIT, result.error())
    }

    @Test
    fun givenDownloadResourcesInBackground_whenSetupNotDone_thenError() = runTest {
        val result = BodyScan().downloadResourcesInBackground(1)
        Assertions.assertEquals(BodyScanError.BODY_SCAN_REMOTE_ASSET_BACKGROUND_NOT_INIT, result.error())
    }

    @Test
    fun givenTotalRemoteAssetsSizeInBytes_whenSetupNotDone_thenZero() = runTest {
        val result = BodyScan().totalRemoteAssetsSizeInBytes()
        Assertions.assertEquals(0, result)
    }

    @Test
    fun givenCurrentDownloadedSizeInBytes_whenSetupNotDone_thenZero() = runTest {
        val result = BodyScan().currentDownloadedSizeInBytes()
        Assertions.assertEquals(0, result)
    }


}