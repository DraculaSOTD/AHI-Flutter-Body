package com.example.benchmark

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import androidx.benchmark.junit4.BenchmarkRule
import androidx.benchmark.junit4.measureRepeated
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.DetectionType
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.BodyScanPoseDetection
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
class PoseDetectionBenchmark {

    @get:Rule
    val benchmarkRule = BenchmarkRule()
    private val poseDetection = BodyScanPoseDetection()
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext
    private val bmp = Bitmap.createScaledBitmap(
        BitmapFactory.decodeStream(appContext.assets.open("front.png")),
        720,
        1280,
        true
    )

    @Test
    fun benchmarkDetect() {
        benchmarkRule.measureRepeated {
            runBlocking {
                poseDetection.detect(DetectionType.faceAndBody, bmp)
            }
        }
    }
}