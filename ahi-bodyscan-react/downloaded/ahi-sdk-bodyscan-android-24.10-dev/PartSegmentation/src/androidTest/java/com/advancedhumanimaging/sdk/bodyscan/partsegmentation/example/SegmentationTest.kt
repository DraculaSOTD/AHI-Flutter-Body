package com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PointF
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.Segmentation
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.runTest
import org.junit.Assert
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see [Testing documentation](http://d.android.com/tools/testing)
 */
@RunWith(AndroidJUnit4::class)
@OptIn(ExperimentalCoroutinesApi::class)
class SegmentationTest {
    // Context of the app under test.
    private val appContext: android.content.Context = InstrumentationRegistry.getInstrumentation().targetContext

    private val joints = mapOf(
        "CentroidHeadTop" to PointF(350.479F, 78.79068F),
        "CentroidNeck" to PointF(350.479F, 225.30754F),
        "CentroidRightAnkle" to PointF(268.8666F, 1122.775F),
        "CentroidLeftAnkle" to PointF(453.10526F, 1125.771F),
        "CentroidRightKnee" to PointF(283.39487F, 889.2501F),
        "CentroidLeftKnee" to PointF(433.1378F, 886.6291F),
        "CentroidRightHip" to PointF(292.84134F, 628.79407F),
        "CentroidLeftHip" to PointF(417.66504F, 629.3525F),
        "CentroidRightHand" to PointF(98.42963F, 652.48303F),
        "CentroidLeftHand" to PointF(621.12915F, 651.6245F),
        "CentroidRightElbow" to PointF(177.2645F, 471.7944F),
        "CentroidLeftElbow" to PointF(548.3079F, 468.18817F),
        "CentroidRightShoulder" to PointF(249.55823F, 309.0467F),
        "CentroidLeftShoulder" to PointF(476.70828F, 309.1195F),
        "CentroidNose" to PointF(348.65778F, 160.92563F)
    )
    private val captureStream = appContext.assets.open("front.png")
    private val captureBmp: Bitmap =
        Bitmap.createScaledBitmap(BitmapFactory.decodeStream(captureStream), 720, 1280, true)
    private val contourStream = appContext.assets.open("contourMaskFront.png")
    private val contourBmp: Bitmap =
        Bitmap.createScaledBitmap(BitmapFactory.decodeStream(contourStream), 720, 1280, true)
    private val k2Stream = InstrumentationRegistry.getInstrumentation().context.assets.open("k2.jpeg")
    private val k2Bmp: Bitmap =
        Bitmap.createScaledBitmap(BitmapFactory.decodeStream(k2Stream), 600, 450, true)

    @Test
    fun givenCorrectParameters_whenSegment_thenReturnSuccessNotNull(): Unit =
        runTest {
            val result = Segmentation.segment(
                captureBmp,
                contourBmp,
                Profile.front,
                joints,
                appContext,
                MockResources()
            )
            Assert.assertTrue(result.isSuccess)
            Assert.assertNotNull(result.getOrNull())
        }

    @Test
    fun givenEmptyJoints_whenSegment_thenReturnIncorrectNumberOfJointsError(): Unit =
        runTest {
            val result = Segmentation.segment(
                captureBmp,
                contourBmp,
                Profile.front,
                emptyMap(),
                appContext,
                MockResources()
            )
            Assert.assertTrue(result.isFailure)
            Assert.assertEquals(
                result.error(),
                BodyScanError.BODY_SCAN_SEGMENTATION_MISSING_JOINTS
            )
        }

    @Test
    fun givenTooManyJoints_whenSegment_thenReturnIncorrectNumberOfJointsError(): Unit =
        runTest {
            val tooManyJoints = mapOf(
                "CentroidRightAnkle" to PointF(268.8666F, 1122.775F),
                "CentroidLeftAnkle" to PointF(453.10526F, 1125.771F),
                "CentroidRightKnee" to PointF(283.39487F, 889.2501F),
                "CentroidLeftKnee" to PointF(433.1378F, 886.6291F),
                "CentroidRightHip" to PointF(292.84134F, 628.79407F),
                "CentroidLeftHip" to PointF(417.66504F, 629.3525F),
                "CentroidRightHand" to PointF(98.42963F, 652.48303F),
                "CentroidLeftHand" to PointF(621.12915F, 651.6245F),
                "CentroidRightElbow" to PointF(177.2645F, 471.7944F),
                "CentroidLeftElbow" to PointF(548.3079F, 468.18817F),
                "CentroidRightShoulder" to PointF(249.55823F, 309.0467F),
                "CentroidLeftShoulder" to PointF(476.70828F, 309.1195F),
                "CentroidNose" to PointF(348.65778F, 160.92563F),
                "ExtraJoint" to PointF(348.65778F, 160.92563F),
            )
            val result = Segmentation.segment(
                captureBmp,
                contourBmp,
                Profile.front,
                tooManyJoints,
                appContext,
                MockResources()
            )
            Assert.assertTrue(result.isFailure)
            Assert.assertEquals(
                result.error(),
                BodyScanError.BODY_SCAN_SEGMENTATION_MISSING_JOINTS
            )
        }

    @Test
    fun givenTooFewJoints_whenSegment_thenReturnIncorrectNumberOfJointsError(): Unit =
        runTest {
            val tooManyJoints = mapOf(
                "CentroidRightAnkle" to PointF(268.8666F, 1122.775F),
                "CentroidLeftAnkle" to PointF(453.10526F, 1125.771F),
                "CentroidRightKnee" to PointF(283.39487F, 889.2501F),
                "CentroidLeftKnee" to PointF(433.1378F, 886.6291F),
                "CentroidRightHip" to PointF(292.84134F, 628.79407F),
                "CentroidLeftHip" to PointF(417.66504F, 629.3525F),
                "CentroidLeftHand" to PointF(621.12915F, 651.6245F),
                "CentroidRightElbow" to PointF(177.2645F, 471.7944F),
                "CentroidLeftElbow" to PointF(548.3079F, 468.18817F),
                "CentroidRightShoulder" to PointF(249.55823F, 309.0467F),
                "CentroidLeftShoulder" to PointF(476.70828F, 309.1195F),
                "CentroidNose" to PointF(348.65778F, 160.92563F),
            )
            val result = Segmentation.segment(
                captureBmp,
                contourBmp,
                Profile.front,
                tooManyJoints,
                appContext,
                MockResources()
            )
            Assert.assertTrue(result.isFailure)
            Assert.assertEquals(
                result.error(),
                BodyScanError.BODY_SCAN_SEGMENTATION_MISSING_JOINTS
            )
        }

    @Test
    fun givenIncorrectResolutionCaptureImage_whenSegment_thenReturnIncorrectCaptureResolutionError(): Unit =
        runTest {
            val result = Segmentation.segment(
                k2Bmp,
                contourBmp,
                Profile.front,
                joints,
                appContext,
                MockResources()
            )
            Assert.assertTrue(result.isFailure)
            Assert.assertEquals(
                result.error(),
                BodyScanError.BODY_SCAN_SEGMENTATION_INCORRECT_CAPTURE_RESOLUTION
            )
        }

    @Test
    fun givenIncorrectResolutionContourImage_whenSegment_thenReturnIncorrectContourResolutionError(): Unit =
        runTest {
            val result = Segmentation.segment(
                captureBmp,
                k2Bmp,
                Profile.front,
                joints,
                appContext,
                MockResources()
            )
            Assert.assertTrue(result.isFailure)
            Assert.assertEquals(
                result.error(),
                BodyScanError.BODY_SCAN_SEGMENTATION_INCORRECT_CONTOUR_RESOLUTION
            )
        }
}