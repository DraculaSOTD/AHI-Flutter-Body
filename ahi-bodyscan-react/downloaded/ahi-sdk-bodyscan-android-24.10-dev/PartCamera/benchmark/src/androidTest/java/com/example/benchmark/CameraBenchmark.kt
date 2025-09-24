package com.example.benchmark

import androidx.benchmark.junit4.BenchmarkRule
import androidx.benchmark.junit4.measureRepeated
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.rule.GrantPermissionRule
import com.advancedhumanimaging.sdk.bodyscan.partcamera.CameraAnalyzer
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
class CameraBenchmark {

    @get:Rule
    var mRuntimePermissionRule: GrantPermissionRule =
        GrantPermissionRule.grant(android.Manifest.permission.CAMERA)

    @get:Rule
    val benchmarkRule = BenchmarkRule()
    private val cameraAnalyzer = CameraAnalyzer()
    private val config = mapOf("capture_times" to 4)

    @Test
    fun benchmarkSetConfig() {
        benchmarkRule.measureRepeated {
            cameraAnalyzer.setConfig(config)
        }
    }
}