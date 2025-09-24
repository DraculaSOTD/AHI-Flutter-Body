package com.example.benchmark

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PointF
import androidx.benchmark.junit4.BenchmarkRule
import androidx.benchmark.junit4.measureRepeated
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.Segmentation
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.runTest
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
class SegmentationBenchmark {

    @get:Rule
    val benchmarkRule = BenchmarkRule()
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext

    private val joints = mapOf(
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
    private val captureBmp: Bitmap =
        Bitmap.createScaledBitmap(
            BitmapFactory.decodeStream(appContext.assets.open("front.png")),
            720,
            1280,
            true
        )
    private val contourBmp: Bitmap =
        Bitmap.createScaledBitmap(
            BitmapFactory.decodeStream(appContext.assets.open("contourMaskFront.png")),
            720,
            1280,
            true
        )

    @OptIn(ExperimentalCoroutinesApi::class)
    @Test
    fun benchmarkSegment() {
        benchmarkRule.measureRepeated {
            runTest {
                Segmentation.segment(
                    captureBmp,
                    contourBmp,
                    Profile.front,
                    joints,
                    appContext,
                    MockResources()
                )
            }
        }
    }
}