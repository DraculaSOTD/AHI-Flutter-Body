package com.example.benchmark

import android.graphics.PointF
import androidx.benchmark.junit4.BenchmarkRule
import androidx.benchmark.junit4.measureRepeated
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partcontour.ContourGenerator
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
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
class ContourBenchmark {

    @get:Rule
    val benchmarkRule = BenchmarkRule()
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext
    private val contourGenerator = ContourGenerator()
    private val resources = Resources()
    private val exampleContour = runBlocking {
        contourGenerator.generateIdealContour(
            appContext,
            resources,
            SexType.male,
            189F,
            80F,
            Resolution(720, 1280),
            0F,
            Profile.side
        )
    }
    private val poseJoints = mapOf(
        // default joints from Dave
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
    private val optimalZones = runBlocking {
        contourGenerator.generateOptimalZones(
            exampleContour!!,
            Resolution(720, 1280)
        )
    }

    @Test
    fun benchmarkGenerateIdealContour() {
        benchmarkRule.measureRepeated {
            runBlocking {
                contourGenerator.generateIdealContour(
                    appContext,
                    resources,
                    SexType.male,
                    189F,
                    80F,
                    Resolution(720, 1280),
                    0F,
                    Profile.side
                )
            }
        }
    }

    @Test
    fun benchmarkGenerateContourMask() {
        benchmarkRule.measureRepeated {
            runBlocking {
                contourGenerator.generateContourMask(
                    exampleContour!!,
                    Resolution(720, 1280)
                )
            }
        }
    }

    @Test
    fun benchmarkGenerateScaledContour() {
        benchmarkRule.measureRepeated {
            runBlocking {
                contourGenerator.generateScaledContour(
                    exampleContour!!,
                    poseJoints
                )
            }
        }
    }

    @Test
    fun benchmarkGenerateOptimalZones() {
        benchmarkRule.measureRepeated {
            runBlocking {
                contourGenerator.generateOptimalZones(
                    exampleContour!!,
                    Resolution(720, 1280)
                )
            }
        }
    }

    @Test
    fun benchmarkIsUserInOptimalZone() {
        benchmarkRule.measureRepeated {
            runBlocking {
                contourGenerator.isUserInOptimalZone(
                    "zoneHead",
                    optimalZones!!["zoneHead"]!!,
                    poseJoints
                )
            }
        }
    }
}