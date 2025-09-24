package com.example.benchmark

import androidx.benchmark.junit4.BenchmarkRule
import androidx.benchmark.junit4.measureRepeated
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
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
class ClassificationBenchmark {

    @get:Rule
    val benchmarkRule = BenchmarkRule()
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext

    @Test
    fun benchmarkClassify() {
        benchmarkRule.measureRepeated {
            // TODO benchmark Classification.classify() here
        }
    }
}