package com.example.benchmark

import androidx.benchmark.junit4.BenchmarkRule
import androidx.benchmark.junit4.measureRepeated
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partinversion.Inversion
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import kotlinx.coroutines.runBlocking
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
class InversionBenchmark {

    @get:Rule
    val benchmarkRule = BenchmarkRule()
    private val inversion = Inversion()
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext

    @Test
    fun benchmarkInvert() {
        benchmarkRule.measureRepeated {
            runBlocking {
                inversion.invert(
                    "test",
                    SexType.female,
                    170.0,
                    60.0,
                    100.0,
                    68.0,
                    110.0,
                    50.0,
                    5.0,
                    appContext,
                    Resources()
                )
            }
        }
    }
}