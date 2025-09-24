package com.advancedhumanimaging.sdk.bodyscan.partinversion.example

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partinversion.Inversion
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.runTest
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see [Testing documentation](http://d.android.com/tools/testing)
 */

@OptIn(ExperimentalCoroutinesApi::class)
@RunWith(AndroidJUnit4::class)
class InversionTest {
    // Context of the app under test.
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext
    private val resources = Resources()
    var sex: SexType = SexType.male
    var height = 190.0
    var weight = 90.0
    var chest = 97.0
    var waist = 82.0
    var hip = 96.0
    var inseam = 77.0
    var fitness = 0.9

    @Test
    fun givenCorrectParameters_whenInvert_thenReturnSuccessNotNull() = runTest {
        val result = Inversion().invert(
            "test",
            sex,
            height,
            weight,
            chest,
            waist,
            hip,
            inseam,
            fitness,
            appContext,
            resources
        )
        org.junit.Assert.assertTrue(result.isSuccess)
        org.junit.Assert.assertNotNull(result.getOrNull())
        org.junit.Assert.assertEquals(
            result.getOrNull()?.path,
            "/data/user/0/com.advancedhumanimaging.sdk.bodyscan.partinversion.test/files/test.obj"
        )
    }

    @Test
    fun givenEmptyName_whenInvert_thenReturnFailureNameMissing() = runTest {
        val result = Inversion().invert(
            "",
            sex,
            height,
            weight,
            chest,
            waist,
            hip,
            inseam,
            fitness,
            appContext,
            resources
        )
        org.junit.Assert.assertTrue(result.isFailure)
        org.junit.Assert.assertEquals(
            result.error(),
            BodyScanError.BODY_SCAN_INVERSION_NAME_MISSING
        )
    }

    @Test
    fun givenZeroHeight_whenInvert_thenReturnFailureInvalidHeightOrWeight() = runTest {
        val result = Inversion().invert(
            "test",
            sex,
            0.0,
            weight,
            chest,
            waist,
            hip,
            inseam,
            fitness,
            appContext,
            resources
        )
        org.junit.Assert.assertTrue(result.isFailure)
        org.junit.Assert.assertEquals(
            result.error(),
            BodyScanError.BODY_SCAN_INVERSION_INVALID_HEIGHT_OR_WEIGHT
        )
    }

    @Test
    fun givenNegativeHeight_whenInvert_thenReturnFailureInvalidHeightOrWeight() = runTest {
        val result = Inversion().invert(
            "test",
            sex,
            -1.0,
            weight,
            chest,
            waist,
            hip,
            inseam,
            fitness,
            appContext,
            resources
        )
        org.junit.Assert.assertTrue(result.isFailure)
        org.junit.Assert.assertEquals(
            result.error(),
            BodyScanError.BODY_SCAN_INVERSION_INVALID_HEIGHT_OR_WEIGHT
        )
    }

    @Test
    fun givenZeroWeight_whenInvert_thenReturnFailureInvalidHeightOrWeight() = runTest {
        val result = Inversion().invert(
            "test",
            sex,
            height,
            0.0,
            chest,
            waist,
            hip,
            inseam,
            fitness,
            appContext,
            resources
        )
        org.junit.Assert.assertTrue(result.isFailure)
        org.junit.Assert.assertEquals(
            result.error(),
            BodyScanError.BODY_SCAN_INVERSION_INVALID_HEIGHT_OR_WEIGHT
        )
    }

    @Test
    fun givenNegativeWeight_whenInvert_thenReturnFailureInvalidHeightOrWeight() = runTest {
        val result = Inversion().invert(
            "test",
            sex,
            height,
            -1.0,
            chest,
            waist,
            hip,
            inseam,
            fitness,
            appContext,
            resources
        )
        org.junit.Assert.assertTrue(result.isFailure)
        org.junit.Assert.assertEquals(
            result.error(),
            BodyScanError.BODY_SCAN_INVERSION_INVALID_HEIGHT_OR_WEIGHT
        )
    }

    @Test
    fun givenAllZeroParametersBarHeightAndWeight_whenInvert_thenReturnSuccessNotNull() = runTest {
        val result = Inversion().invert(
            "test",
            sex,
            height,
            weight,
            0.0,
            0.0,
            0.0,
            0.0,
            0.0,
            appContext,
            resources
        )
        org.junit.Assert.assertTrue(result.isSuccess)
        org.junit.Assert.assertNotNull(result.getOrNull())
        org.junit.Assert.assertEquals(
            result.getOrNull()?.path,
            "/data/user/0/com.advancedhumanimaging.sdk.bodyscan.partinversion.test/files/test.obj"
        )
    }
}