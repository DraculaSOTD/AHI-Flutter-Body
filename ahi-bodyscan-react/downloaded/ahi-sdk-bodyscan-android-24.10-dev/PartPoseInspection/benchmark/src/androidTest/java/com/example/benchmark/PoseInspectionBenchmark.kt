package com.example.benchmark

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PointF
import android.graphics.RectF
import androidx.benchmark.junit4.BenchmarkRule
import androidx.benchmark.junit4.measureRepeated
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.InspectionResult
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.PoseInspection
import kotlinx.coroutines.runBlocking
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Benchmark, which will execute on an Android device.
 *
 * The body of [BenchmarkRule.measureRepeated] is measured in a loop, and Studio will
 * output the result. Modify your code to see how it affects performance.
 */
@RunWith(AndroidJUnit4::class)
class PoseInspectionBenchmark {

    @get:Rule
    val benchmarkRule = BenchmarkRule()
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext
    private val bmp = Bitmap.createScaledBitmap(
        BitmapFactory.decodeStream(appContext.assets.open("front.png")),
        720,
        1280,
        true
    )
    private val contourBmp = Bitmap.createScaledBitmap(
        BitmapFactory.decodeStream(appContext.assets.open("contourMaskFront.png")),
        720,
        1280,
        true
    )
    private val poseInspection = PoseInspection()
    private val poseJoints = mapOf(
        "body" to listOf(
            // default joints from Dave
            mapOf("CentroidRightAnkle" to PointF(268.8666F, 1122.775F)),
            mapOf("CentroidLeftAnkle" to PointF(453.10526F, 1125.771F)),
            mapOf("CentroidRightKnee" to PointF(283.39487F, 889.2501F)),
            mapOf("CentroidLeftKnee" to PointF(433.1378F, 886.6291F)),
            mapOf("CentroidRightHip" to PointF(292.84134F, 628.79407F)),
            mapOf("CentroidLeftHip" to PointF(417.66504F, 629.3525F)),
            mapOf("CentroidRightHand" to PointF(98.42963F, 652.48303F)),
            mapOf("CentroidLeftHand" to PointF(621.12915F, 651.6245F)),
            mapOf("CentroidRightElbow" to PointF(177.2645F, 471.7944F)),
            mapOf("CentroidLeftElbow" to PointF(548.3079F, 468.18817F)),
            mapOf("CentroidRightShoulder" to PointF(249.55823F, 309.0467F)),
            mapOf("CentroidLeftShoulder" to PointF(476.70828F, 309.1195F)),
            mapOf("CentroidNose" to PointF(348.65778F, 160.92563F))
        )
    )
    private val inspectionResult = mapOf(
        "face" to InspectionResult.trueInContour,
        "la" to InspectionResult.trueInContour,
        "ra" to InspectionResult.trueInContour,
        "ll" to InspectionResult.trueInContour,
        "rl" to InspectionResult.trueInContour
    )
    private val optimalZones = mapOf(
        "zoneHead" to RectF(10F, 10F, 10F, 10F),
        "zoneHeadTarget" to RectF(20F, 20F, 20F, 20F),
        "zoneAnkles" to RectF(30F, 30F, 30F, 30F),
        "zoneAnklesTarget" to RectF(40F, 40F, 40F, 40F)
    )

    @Test
    fun benchmarkInspect() {
        benchmarkRule.measureRepeated {
            runBlocking {
                poseInspection.inspect(bmp, contourBmp, optimalZones, Profile.front, poseJoints)
            }
        }
    }

    @Test
    fun benchmarkIsInContour() {
        benchmarkRule.measureRepeated {
            runBlocking {
                poseInspection.isInContour(Profile.front, inspectionResult)
            }
        }
    }
}