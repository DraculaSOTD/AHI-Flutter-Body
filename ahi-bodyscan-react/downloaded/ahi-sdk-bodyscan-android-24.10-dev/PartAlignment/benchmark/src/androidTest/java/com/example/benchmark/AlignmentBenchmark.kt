package com.example.benchmark

import androidx.benchmark.junit4.BenchmarkRule
import androidx.benchmark.junit4.measureRepeated
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.partalignment.AHIMotionSensor
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
class AlignmentBenchmark {

    @get:Rule
    val benchmarkRule = BenchmarkRule()
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext
    private val ahiMotionSensor = AHIMotionSensor(appContext)

    @Test
    fun benchmarkIsDeviceMotionAvailable() {
        benchmarkRule.measureRepeated {
            ahiMotionSensor.start()
            runWithTimingDisabled { ahiMotionSensor.stop() }
        }
    }

    @Test
    fun benchmarkIsDeviceMotionActive() {
        benchmarkRule.measureRepeated {
            runWithTimingDisabled { ahiMotionSensor.start() }
            ahiMotionSensor.stop()
        }
    }

    @Test
    fun benchmarkStart() {
        benchmarkRule.measureRepeated {
            ahiMotionSensor.start()
            runWithTimingDisabled { ahiMotionSensor.stop() }
        }
    }

    @Test
    fun benchmarkStop() {
        benchmarkRule.measureRepeated {
            runWithTimingDisabled { ahiMotionSensor.start() }
            ahiMotionSensor.stop()
        }
    }

    @Test
    fun benchmarkDeviceMotion() {
        benchmarkRule.measureRepeated {
            ahiMotionSensor.deviceMotion()
        }
    }

    @Test
    fun benchmarkIsDeviceAligned() {
        benchmarkRule.measureRepeated {
            ahiMotionSensor.isDeviceAligned()
        }
    }
}