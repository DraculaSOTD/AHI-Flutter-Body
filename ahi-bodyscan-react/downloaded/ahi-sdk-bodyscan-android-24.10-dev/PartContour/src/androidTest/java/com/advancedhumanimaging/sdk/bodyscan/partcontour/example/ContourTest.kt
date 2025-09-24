package com.advancedhumanimaging.sdk.bodyscan.partcontour.example

import android.graphics.PointF
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSOptimalContourZone
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partcontour.ContourGenerator
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import kotlinx.coroutines.runBlocking
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see [Testing documentation](http://d.android.com/tools/testing)
 */
@RunWith(AndroidJUnit4::class)
class ContourTest {
    // Context of the app under test.
    private val appContext: android.content.Context =
        InstrumentationRegistry.getInstrumentation().targetContext
    private val contourGenerator = ContourGenerator()
    private val resources = Resources()

    private val poseJoints = mapOf(
        "CentroidLeftAnkle" to PointF(472.60F, 1129.9F),
        "CentroidLeftElbow" to PointF(541.50F, 433.91F),
        "CentroidLeftHand" to PointF(618.26F, 564.56F),
        "CentroidLeftHip" to PointF(422.10F, 617.55F),
        "CentroidLeftKnee" to PointF(449.97F, 883.97F),
        "CentroidLeftShoulder" to PointF(461.82F, 293.04F),
        "CentroidNose" to PointF(354.75F, 139.99F),
        "CentroidHeadTop" to PointF(354.75F, 129.99F),
        "CentroidNeck" to PointF(354.75F, 149.99F),
        "CentroidRightAnkle" to PointF(289.08F, 1135.66F),
        "CentroidRightElbow" to PointF(175.38F, 425.55F),
        "CentroidRightHand" to PointF(109.50F, 570.86F),
        "CentroidRightHip" to PointF(304.54F, 613.36F),
        "CentroidRightKnee" to PointF(305.05F, 885.98F),
        "CentroidRightShoulder" to PointF(254.81F, 288.37F)
    )

    private val defaultContour = runBlocking {
        contourGenerator.generateIdealContour(
            appContext,
            resources,
            SexType.male,
            173.5F,
            80.12F,
            Resolution(720, 1280),
            0.01F,
            Profile.front
        )!!
    }

    @Test
    fun givenCorrectParameters_whenGenerateIdealContour_thenReturnNotNull() {
        runBlocking {
            val result = contourGenerator.generateIdealContour(
                appContext,
                resources,
                SexType.male,
                173.5F,
                80.12F,
                Resolution(720, 1280),
                0.01F,
                Profile.front
            )
            assertNotNull(result)
            assertTrue(result!!.size > 10)
        }
    }

    @Test
    fun givenCorrectParameters_whenGenerateContourMask_thenReturnNotNullCorrectSize() {
        runBlocking {
            val result = contourGenerator.generateContourMask(defaultContour, Resolution(720, 1280))
            assertNotNull(result)
            assertEquals(result!!.height, 1280)
            assertEquals(result.width, 720)
        }
    }

    @Test
    fun givenCorrectParameters_whenGenerateScaledContour_thenReturnNotNull() {
        runBlocking {
            val result = contourGenerator.generateScaledContour(defaultContour, poseJoints)
            assertNotNull(result)
            assertTrue(result!!.size > 10)
            assertNotEquals(result, defaultContour)
        }
    }

    @Test
    fun givenCorrectFrontProfileParameters_whenGenerateOptimalZones_thenExpectedZonesInReasonablePositions() {
        for (i in 60..120 step 5) {
            // image is 16 * 9, so easy to generate a bunch of different sizes
            val imageSize = Resolution(i * 9, i * 16)
            val contour = runBlocking {
                contourGenerator.generateIdealContour(
                    appContext,
                    resources,
                    SexType.male,
                    173.5F,
                    80.12F,
                    imageSize,
                    0.01F,
                    Profile.front
                )!!
            }
            val result =
                runBlocking { contourGenerator.generateOptimalZones(contour, imageSize) }
            assertNotNull(result)

            val headZone = result!![AHIBSOptimalContourZone.HEAD.key]
            val anklesZone = result[AHIBSOptimalContourZone.ANKLES.key]
            assertNotNull(headZone)
            assertNotNull(anklesZone)

            // head at the top, ankles at the bottom
            assertTrue(headZone!!.centerY() < anklesZone!!.centerY())

            // zones don't extend outsize the image rect
            assertTrue(headZone.right <= imageSize.width)
            assertTrue(headZone.bottom <= imageSize.height)
            assertTrue(anklesZone.right <= imageSize.width)
            assertTrue(anklesZone.bottom <= imageSize.height)
        }
    }

    @Test
    fun givenCorrectSideProfileParameters_whenGenerateOptimalZones_thenExpectedZonesInReasonablePositions() {
        for (i in 60..120 step 5) {
            // image is 16 * 9, so easy to generate a bunch of different sizes
            val imageSize = Resolution(i * 9, i * 16)
            val contour = runBlocking {
                contourGenerator.generateIdealContour(
                    appContext,
                    resources,
                    SexType.male,
                    173.5F,
                    80.12F,
                    imageSize,
                    0.01F,
                    Profile.side
                )!!
            }
            val result =
                runBlocking { contourGenerator.generateOptimalZones(contour, imageSize) }
            assertNotNull(result)

            val headZone = result!![AHIBSOptimalContourZone.HEAD.key]
            val anklesZone = result[AHIBSOptimalContourZone.ANKLES.key]
            assertNotNull(headZone)
            assertNotNull(anklesZone)

            // head at the top, ankles at the bottom
            assertTrue(headZone!!.centerY() < anklesZone!!.centerY())

            // zones don't extend outsize the image rect
            assertTrue(headZone.right <= imageSize.width)
            assertTrue(headZone.bottom <= imageSize.height)
            assertTrue(anklesZone.right <= imageSize.width)
            assertTrue(anklesZone.bottom <= imageSize.height)
        }
    }

    @Test
    fun givenCompleteJoints_whenIsUserInOptimalZone_thenHeadIsInZoneTrue() {
        val imageSize = Resolution(720, 1280)
        val optimalZones = runBlocking { contourGenerator.generateOptimalZones(defaultContour, imageSize)!! }
        val isInZone = runBlocking {
            contourGenerator.isUserInOptimalZone(
                AHIBSOptimalContourZone.HEAD.key,
                optimalZones[AHIBSOptimalContourZone.HEAD.key]!!,
                poseJoints
            )
        }
        assertTrue(isInZone)
    }

    @Test
    fun givenCompleteJoints_whenIsUserInOptimalZone_thenAnklesIsInZoneTrue() {
        val imageSize = Resolution(720, 1280)
        val optimalZones = runBlocking { contourGenerator.generateOptimalZones(defaultContour, imageSize)!! }
        val isInZone = runBlocking {
            contourGenerator.isUserInOptimalZone(
                AHIBSOptimalContourZone.ANKLES.key,
                optimalZones[AHIBSOptimalContourZone.ANKLES.key]!!,
                poseJoints
            )
        }
        assertTrue(isInZone)
    }

    @Test
    fun givenEmptyJoints_whenIsUserInOptimalZone_thenHeadIsInZoneFalse() {
        val imageSize = Resolution(720, 1280)
        val optimalZones = runBlocking { contourGenerator.generateOptimalZones(defaultContour, imageSize)!! }
        val isInZone = runBlocking {
            contourGenerator.isUserInOptimalZone(
                AHIBSOptimalContourZone.HEAD.key,
                optimalZones[AHIBSOptimalContourZone.HEAD.key]!!,
                mapOf()
            )
        }
        assertFalse(isInZone)
    }

    @Test
    fun givenEmptyJoints_whenIsUserInOptimalZone_thenAnklesIsInZoneFalse() {
        val imageSize = Resolution(720, 1280)
        val optimalZones = runBlocking { contourGenerator.generateOptimalZones(defaultContour, imageSize)!! }
        val isInZone = runBlocking {
            contourGenerator.isUserInOptimalZone(
                AHIBSOptimalContourZone.ANKLES.key,
                optimalZones[AHIBSOptimalContourZone.ANKLES.key]!!,
                mapOf()
            )
        }
        assertFalse(isInZone)
    }
}