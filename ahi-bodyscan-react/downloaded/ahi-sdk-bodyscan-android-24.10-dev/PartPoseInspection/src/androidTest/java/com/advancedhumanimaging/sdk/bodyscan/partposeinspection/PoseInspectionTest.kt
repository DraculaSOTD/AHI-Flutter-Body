package com.advancedhumanimaging.sdk.bodyscan.partposeinspection

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PointF
import android.graphics.RectF
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.InspectionResult
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see [Testing documentation](http://d.android.com/tools/testing)
 */
@RunWith(AndroidJUnit4::class)
class PoseInspectionTest {
    // Context of the app under test.
    private val appContext: android.content.Context = InstrumentationRegistry.getInstrumentation().targetContext
    private val poseInspection = PoseInspection()

    private val poseJointsFront = mapOf(
        "face" to listOf(
            mapOf(
                "CentroidHeadTop" to PointF(350.479F, 78.79068F),
                "CentroidNeck" to PointF(350.479F, 225.30754F)
            )
        ),
        "body" to listOf(
            mapOf(
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
        )
    )
    private val poseJointsSide = mapOf(
        "face" to listOf(
            mapOf(
                "CentroidHeadTop" to PointF(368.21106F, 68.45384F),
                "CentroidNeck" to PointF(368.21106F, 228.95241F)
            )
        ),
        "body" to listOf(
            mapOf(
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
        )
    )
    private val invalidPoseJointsFront = mapOf(
        "face" to listOf(
            mapOf(
                "CentroidHeadTop" to PointF(350.479F, 651.6245F),
                "CentroidNeck" to PointF(350.479F, 225.30754F)
            )
        ),
        "body" to listOf(
            mapOf(
                "CentroidRightAnkle" to PointF(268.8666F, 0F),
                "CentroidLeftAnkle" to PointF(453.10526F, 1125.771F),
                "CentroidRightKnee" to PointF(283.39487F, 889.2501F),
                "CentroidLeftKnee" to PointF(433.1378F, 886.6291F),
                "CentroidRightHip" to PointF(292.84134F, 628.79407F),
                "CentroidLeftHip" to PointF(417.66504F, 629.3525F),
                "CentroidRightHand" to PointF(621.12915F, 651.6245F),
                "CentroidRightElbow" to PointF(177.2645F, 471.7944F),
                "CentroidLeftElbow" to PointF(548.3079F, 468.18817F),
                "CentroidRightShoulder" to PointF(249.55823F, 309.0467F),
                "CentroidLeftShoulder" to PointF(476.70828F, 309.1195F),
                "CentroidNose" to PointF(348.65778F, 160.92563F)
            )
        )
    )
    private val invalidPoseJointsSide = mapOf(
        "face" to listOf(
            mapOf(
                "CentroidHeadTop" to PointF(368.21106F, 68.45384F),
                "CentroidNeck" to PointF(368.21106F, 228.95241F)
            ),
            mapOf(
                "CentroidHeadTop" to PointF(368.21106F, 68.45384F),
                "CentroidNeck" to PointF(368.21106F, 228.95241F)
            )
        ),
        "body" to listOf(
            mapOf(
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
        )
    )
    private val optimalZonesFront = mapOf(
        "zoneHead" to RectF(0.0F, 25.0F, 720.0F, 126.0F),
        "zoneHeadTarget" to RectF(0.0F, 27.525002F, 720.0F, 123.475F),
        "zoneAnkles" to RectF(0.0F, 1090.0F, 720.0F, 1220.0F),
        "zoneAnklesTarget" to RectF(0.0F, 1093.25F, 720.0F, 1216.75F)
    )
    private val optimalZonesSide = mapOf(
        "zoneHead" to RectF(0.0F, 25.0F, 720.0F, 127.0F),
        "zoneHeadTarget" to RectF(0.0F, 27.55F, 720.0F, 124.45F),
        "zoneAnkles" to RectF(0.0F, 1079.0F, 720.0F, 1220.0F),
        "zoneAnklesTarget" to RectF(0.0F, 1082.525F, 720.0F, 1216.475F)
    )
    private val contourMaskFront: Bitmap =
        Bitmap.createScaledBitmap(BitmapFactory.decodeStream(appContext.assets.open("contourMaskFront.jpg")), 720, 1280, true)
    private val contourMaskSide: Bitmap =
        Bitmap.createScaledBitmap(BitmapFactory.decodeStream(appContext.assets.open("contourMaskSide.jpg")), 720, 1280, true)

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenFrontPoseJointsAndFrontContourMask_whenInspected_thenAllInspectionResultsAreTrueInContour() =
        runTest {
            val results = poseInspection.inspect(
                contourMaskFront,
                contourMaskFront,
                optimalZonesFront,
                Profile.front,
                poseJointsFront
            )
            val result = results.isSuccess && results.getOrNull()?.values?.all { it == InspectionResult.trueInContour } ?: false
            assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenSidePoseJointsAndSideContourMask_whenInspected_thenAllInspectionResultsAreTrueInContour() =
        runTest {
            val results = poseInspection.inspect(
                contourMaskSide,
                contourMaskSide,
                optimalZonesSide,
                Profile.side,
                poseJointsSide
            )
            val result = results.isSuccess && results.getOrNull()?.values?.all { it == InspectionResult.trueInContour } ?: false
            assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenFrontPoseJointsAndSideContourMask_whenInspected_thenNotAllInspectionResultsAreTrueInContour() =
        runTest {
            val results = poseInspection.inspect(
                contourMaskSide,
                contourMaskSide,
                optimalZonesSide,
                Profile.side,
                poseJointsFront
            )
            val result = results.isSuccess && results.getOrNull()?.values?.all { it == InspectionResult.trueInContour } ?: true
            assertEquals(false, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenSidePoseJointsAndFrontContourMask_whenInspected_thenNotAllInspectionResultsAreTrueInContour() =
        runTest {
            val results = poseInspection.inspect(
                contourMaskSide,
                contourMaskSide,
                optimalZonesSide,
                Profile.side,
                poseJointsFront
            )
            val result = results.isSuccess && results.getOrNull()?.values?.all { it == InspectionResult.trueInContour } ?: true
            assertEquals(false, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenInvalidSideInputs_whenInspected_thenNotAllInspectionResultsAreTrueInContour() =
        runTest {
            val results = poseInspection.inspect(
                contourMaskSide,
                contourMaskSide,
                optimalZonesSide,
                Profile.side,
                invalidPoseJointsSide
            )
            val result = results.isSuccess && results.getOrNull()?.values?.all { it == InspectionResult.trueInContour } ?: true
            assertEquals(false, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenInvalidFrontInputs_whenInspected_thenNotAllInspectionResultsAreTrueInContour() =
        runTest {
            val results = poseInspection.inspect(
                contourMaskFront,
                contourMaskFront,
                optimalZonesFront,
                Profile.front,
                invalidPoseJointsFront
            )
            val result = results.isSuccess && results.getOrNull()?.values?.all { it == InspectionResult.trueInContour } ?: true
            assertEquals(false, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenAllFrontInspectionResultsAreTrueInContour_whenCheckedForAllIsInContour_thenTrue() =
        runTest {
            val inspectionResults = mapOf(
                "face" to InspectionResult.trueInContour,
                "la" to InspectionResult.trueInContour,
                "ra" to InspectionResult.trueInContour,
                "ll" to InspectionResult.trueInContour,
                "rl" to InspectionResult.trueInContour
            )
            val result = poseInspection.isInContour(Profile.front, inspectionResults)
            assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenAllSideInspectionResultsAreTrueInContour_whenCheckedForAllIsInContour_thenTrue() =
        runTest {
            val inspectionResults = mapOf(
                "face" to InspectionResult.trueInContour,
                "ub" to InspectionResult.trueInContour,
                "lb" to InspectionResult.trueInContour,
            )
            val result = poseInspection.isInContour(Profile.side, inspectionResults)
            assertEquals(true, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenNotAllFrontInspectionResultsAreTrueInContour_whenCheckedForAllIsInContour_thenFalse() =
        runTest {
            val inspectionResults = mapOf(
                "face" to InspectionResult.falseMultipleFaces,
                "la" to InspectionResult.trueInContour,
                "ra" to InspectionResult.trueInContour,
                "ll" to InspectionResult.falseNotInContour,
                "rl" to InspectionResult.trueInContour
            )
            val result = poseInspection.isInContour(Profile.front, inspectionResults)
            assertEquals(false, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenNotAllSideInspectionResultsAreTrueInContour_whenCheckedForAllIsInContour_thenFalse() =
        runTest {
            val inspectionResults = mapOf(
                "face" to InspectionResult.falseNotDetected,
                "ub" to InspectionResult.trueInContour,
                "lb" to InspectionResult.falseNotInContour,
            )
            val result = poseInspection.isInContour(Profile.side, inspectionResults)
            assertEquals(false, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenEmptyFrontInspectionResults_whenCheckedForAllIsInContour_thenFalse() =
        runTest {
            val result = poseInspection.isInContour(Profile.front, mapOf())
            assertEquals(false, result)
        }

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun givenEmptySideInspectionResults_whenCheckedForAllIsInContour_thenFalse() =
        runTest {
            val result = poseInspection.isInContour(Profile.side, mapOf())
            assertEquals(false, result)
        }
}