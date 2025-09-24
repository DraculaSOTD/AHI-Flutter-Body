//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common

import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import android.util.Size

/**
 * The name of BodyScan.
 */
const val AHIBSScanName = "body"

/**
 * The current version of BodyScan.
 */
const val AHIBSVersion = BuildConfig.SDK_VERSION_CODE

/**
 * Required width of image captured.
 */
const val AHIBSImageCaptureWidth = 720.0F

/**
 * Required height of image captured.
 */
const val AHIBSImageCaptureHeight = 1280.0F

/**
 * Resolution defines the pixel height and weight for a 2D surface or image.
 * @property width The count of pixels across the U dimension, or width.
 * @property height The count of pixels across the V dimension, or height.
 */
typealias Resolution = Size

/**
 * Container of an instantaneous capture data.
 * @property image The photo captured.
 * @property meta Data to attach to the capture, which should include alignment angles, profile, and
 * pose detection values as they become available.
 */
data class Capture(var image: Bitmap, var meta: Map<String, Any>) {
    val joints get() = meta["joints"]?.let { it as? Map<String, PointF> } ?: mapOf()
}

/**
 * Pairs a front and side capture together. The expectation is for the meta to include pre-process data.
 */
data class CaptureGrouping(var front: Capture, var side: Capture)

/**
 * Represents the profile of the user (e.g. how they are standing and facing the camera).
 * Either as front or side profile orientation of their body.
 */
enum class Profile {
    /** Represents front user profile orientation. */
    front,

    /** Represents side user profile orientation. */
    side
}

/**
 * Represents sex type. Either male or female.
 */
enum class SexType {
    /** Male type. */
    male,

    /** Female type. */
    female
}

/**
 * Represents inspection result from checking user's detected pose joints to the provided contour.
 */
enum class InspectionResult {
    /** Indicates that the detected joint is IN the expected area, relative to the contour. */
    trueInContour,

    /** Indicates that the detected joint is NOT in the expected area, relative to the contour. */
    falseNotInContour,

    /** Indicates that the specific user appendage was not provided in the pose detection result. */
    falseNotDetected,

    /** Indicates that multiple faces were detected in the capture, rendering the pose detection as unreliable for inspection. */
    falseMultipleFaces
}

/**
 * Represents the detection type to ascertain from a photo.
 */
enum class DetectionType {
    /** Detect face landmarks. */
    face,

    /** Detect body landmarks. */
    body,

    /** Detect both face and body landmarks. */
    faceAndBody
}

/**
 * Represents the optimal contour zone types.
 */
enum class AHIBSOptimalContourZone(val key: String) {
    HEAD("zoneHead"),
    HEAD_TARGET("zoneHeadTarget"),
    ANKLES("zoneAnkles"),
    ANKLES_TARGET("zoneAnklesTarget")
}

/**
 * Pose inspection details for debug purposes
 */
data class PoseInspectionInfo(
    val boundingBox: RectF?,
    val point: PointF?,
    val result: InspectionResult
)