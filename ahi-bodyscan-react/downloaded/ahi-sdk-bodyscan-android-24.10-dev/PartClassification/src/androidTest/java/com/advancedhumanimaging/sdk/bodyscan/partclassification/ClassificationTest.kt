package com.advancedhumanimaging.sdk.bodyscan.partclassification

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PointF
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.common.Capture
import com.advancedhumanimaging.sdk.bodyscan.common.CaptureGrouping
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.runTest
import org.junit.Assert

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see [Testing documentation](http://d.android.com/tools/testing)
 */
class ClassificationTest {
    private val classification = Classification()
    private val resources = Resources()
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext
    private val frontJoints = mapOf(
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
    private val sideJoints = mapOf(
        "CentroidHeadTop" to PointF(368.21106F, 68.45384F),
        "CentroidNeck" to PointF(368.21106F, 228.95241F),
        "CentroidRightAnkle" to PointF(345.386F, 1149.2936F),
        "CentroidLeftAnkle" to PointF(337.06787F, 1120.261F),
        "CentroidRightKnee" to PointF(359.0062F, 887.20996F),
        "CentroidLeftKnee" to PointF(355.96588F, 872.59674F),
        "CentroidRightHip" to PointF(367.0083F, 620.60876F),
        "CentroidLeftHip" to PointF(371.98386F, 623.1391F),
        "CentroidRightHand" to PointF(395.06506F, 677.1813F),
        "CentroidLeftHand" to PointF(388.1402F, 662.5207F),
        "CentroidRightElbow" to PointF(343.24966F, 463.985F),
        "CentroidLeftElbow" to PointF(356.23596F, 460.41724F),
        "CentroidRightShoulder" to PointF(328.8516F, 273.66187F),
        "CentroidLeftShoulder" to PointF(344.94974F, 281.6684F),
        "CentroidNose" to PointF(382.90192F, 161.08696F)
    )
    private val frontSilhouette: Bitmap =
        Bitmap.createScaledBitmap(BitmapFactory.decodeStream(appContext.assets.open("front_silhouette.JPEG")), 720, 1280, true)
    private val sideSilhouette: Bitmap =
        Bitmap.createScaledBitmap(BitmapFactory.decodeStream(appContext.assets.open("side_silhouette.JPEG")), 720, 1280, true)
    private val frontCapture = Capture(
        image = frontSilhouette,
        meta = mapOf("joints" to frontJoints)
    )
    private val sideCapture = Capture(
        sideSilhouette,
        meta = mapOf("joints" to sideJoints)
    )

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenValidInputs_whenClassified_thenValidResults(): Unit =
        runTest {
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                59.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture,
                        sideCapture
                    )
                ),
                useAverage = true
            )
            val result = results.isSuccess && results.getOrNull()?.isNotEmpty() ?: false
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenNoCapture_whenClassified_thenError(): Unit =
        runTest {
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                59.0,
                captures = arrayOf(),
                useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_INVALID_CAPTURE
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenInvalidHeight_whenClassified_thenError(): Unit =
        runTest {
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                0.0,
                59.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture,
                        sideCapture
                    )
                ),
                useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_INVALID_HEIGHT_WEIGHT
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenInvalidWeight_whenClassified_thenError(): Unit =
        runTest {
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                500.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture,
                        sideCapture
                    )
                ),
                useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_INVALID_HEIGHT_WEIGHT
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenNoFrontJoints_whenClassified_thenError(): Unit =
        runTest {
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                59.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture.copy(meta = mapOf()),
                        sideCapture
                    )
                )
                ,useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_MISSING_JOINTS
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenNoSideJoints_whenClassified_thenError(): Unit =
        runTest {
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                59.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture,
                        sideCapture.copy(meta = mapOf())
                    )
                ),
                useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_MISSING_JOINTS
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenIncompleteFrontJoints_whenClassified_thenError(): Unit =
        runTest {
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                59.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture.copy(meta = mapOf("joints" to frontJoints.filterNot { it.key == "CentroidHeadTop" || it.key == "CentroidRightAnkle" || it.key == "CentroidLeftHand" })),
                        sideCapture
                    )
                ),
                useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_MISSING_JOINTS
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenIncompleteSideJoints_whenClassified_thenError(): Unit =
        runTest {
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                59.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture,
                        sideCapture.copy(meta = mapOf("joints" to sideJoints.filterNot { it.key == "CentroidHeadTop" || it.key == "CentroidRightAnkle" || it.key == "CentroidLeftHand" }))
                    )
                ),
                useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_MISSING_JOINTS
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenFrontSilhouetteWithIncorrectDimensions_whenClassified_thenError(): Unit =
        runTest {
            val invalidSilhouette = Bitmap.createScaledBitmap(frontSilhouette, 920, 1280, true)
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                59.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture.copy(image = invalidSilhouette),
                        sideCapture
                    )
                ),
                useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_INVALID_CAPTURE_IMAGE_DIMENSIONS
            Assert.assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @org.junit.Test
    fun givenSideSilhouetteWithIncorrectDimensions_whenClassified_thenError(): Unit =
        runTest {
            val invalidSilhouette = Bitmap.createScaledBitmap(frontSilhouette, 720, 1080, true)
            val results = classification.classify(
                context = appContext,
                resources,
                sex = SexType.male,
                166.0,
                59.0,
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture,
                        sideCapture.copy(image = invalidSilhouette)
                    )
                ),
                useAverage = true
            )
            val result = results.isFailure && results.error() == BodyScanError.BODY_SCAN_CLASSIFICATION_INVALID_CAPTURE_IMAGE_DIMENSIONS
            Assert.assertEquals(true, result)
        }
}