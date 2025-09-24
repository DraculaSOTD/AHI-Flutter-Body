//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposeinspection

import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import androidx.core.graphics.contains
import androidx.lifecycle.MutableLiveData
import com.advancedhumanimaging.sdk.bodyscan.common.*
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IPoseInspection
import com.advancedhumanimaging.sdk.bodyscan.common.utils.getProminentPersonJoints
import com.advancedhumanimaging.sdk.common.models.AHILogLevel
import com.advancedhumanimaging.sdk.common.models.AHILogging
import com.advancedhumanimaging.sdk.common.models.AHIResult
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlin.math.abs
import kotlin.math.max

class PoseInspection : IPoseInspection {
    // Provides details of latest inspection for debug purposes
    val lastInspectionResultDetails = MutableLiveData<Map<String, PoseInspectionInfo>>(mutableMapOf())

    private suspend fun updateLastInspectionResultDetails(key: String, detail: PoseInspectionInfo) {
        val details = lastInspectionResultDetails.value?.toMutableMap()
        details?.set(key, detail)
        withContext(Dispatchers.Main) {
            lastInspectionResultDetails.value = details
        }
    }

    override suspend fun inspect(
        capture: Bitmap,
        contourMask: Bitmap,
        optimalZones: Map<String, RectF>,
        profile: Profile,
        poseJoints: Map<String, List<Map<String, PointF>>>
    ): AHIResult<Map<String, InspectionResult>> {
        return withContext(Dispatchers.IO) {
            try {
                withContext(Dispatchers.Main) {
                    lastInspectionResultDetails.value = mutableMapOf()
                }
                val allPoseJoints = getProminentPersonJoints(poseJoints)
                val extremePoints = getExtremePointsFromBinaryImage(contourMask)
                if (extremePoints.isEmpty()) {
                    AHILogging.log(AHILogLevel.ERROR, "Pose inspection failed due to failure to get extreme points from contour")
                    return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_POSE_INSPECTION_FAILED_TO_GET_EXTREME_POINTS)
                }
                val results = if (profile == Profile.front) {
                    inspectFrontPoseWithDetectedPose(
                        extremePoints,
                        optimalZones,
                        allPoseJoints
                    ).toMutableMap()
                } else {
                    inspectSidePoseWithDetectedPose(
                        extremePoints,
                        optimalZones,
                        allPoseJoints
                    ).toMutableMap()
                }
                if (poseJoints["face"]?.let { it.size > 1 } == true) {
                    results["face"] = InspectionResult.falseMultipleFaces
                }
                AHIResult.success(results)
            } catch (e: Exception) {
                AHILogging.log(AHILogLevel.ERROR, "Pose inspection failed")
                AHIResult.failure(BodyScanError.BODY_SCAN_POSE_INSPECTION_FAILED)
            }
        }
    }

    override suspend fun isInContour(
        profile: Profile,
        inspectionResult: Map<String, InspectionResult>
    ): Boolean {
        return withContext(Dispatchers.IO) {
            val checkingParts =
                if (profile == Profile.front) {
                    listOf("face", "la", "ra", "ll", "rl")
                } else {
                    listOf("face", "ub", "lb")
                }
            checkingParts.forEach {
                if (inspectionResult[it] != InspectionResult.trueInContour) {
                    return@withContext false
                }
            }
            inspectionResult.isNotEmpty()
        }
    }

    private fun getExtremePointsFromBinaryImage(
        binaryImage: Bitmap
    ): Map<String, Float> = PoseInspectionJNI.getExtremePointsFromBinaryImage(binaryImage)
        .mapValues { it.value.toFloat() }

    private suspend fun inspectFrontPoseWithDetectedPose(
        extremePoints: Map<String, Float>,
        optimalZones: Map<String, RectF>,
        poseJoints: Map<String, PointF>
    ): Map<String, InspectionResult> {
        val inspectionResult = mutableMapOf<String, InspectionResult>()
        val optimalZoneHead = optimalZones[AHIBSOptimalContourZone.HEAD.key]
        val optimalZoneAnkles = optimalZones[AHIBSOptimalContourZone.ANKLES.key]
        for (countJoints in 0..4) {
            var resultPart = ""
            var binaryImageBoxTL: RectF? = null
            var posePoint: PointF? = null
            when (countJoints) {
                0 -> {
                    // Face
                    resultPart = "face"
                    posePoint = poseJoints["CentroidHeadTop"]
                    val top = optimalZoneHead?.top ?: extremePoints["yTop"]
                    val xContourFaceCenter = extremePoints["xContourFaceCenter"]
                    val yContourFaceCenter = extremePoints["yContourFaceCenter"]
                    val contourFaceWidth = extremePoints["ContourFaceWidth"]
                    val contourFaceHeight =
                        optimalZoneHead?.height() ?: extremePoints["ContourFaceHeight"]
                    if (xContourFaceCenter != null && contourFaceHeight != null && contourFaceWidth != null && top != null && yContourFaceCenter != null) {
                        val halfWidth = (contourFaceWidth * 0.5F)
                        val halfHeight = (contourFaceHeight * 0.5F)
                        val left = xContourFaceCenter - halfWidth
                        val right = xContourFaceCenter + halfWidth
                        val bottom = yContourFaceCenter + halfHeight
                        binaryImageBoxTL = RectF(left, top, right, bottom)
                    }
                }

                1 -> {
                    // Left Hand
                    resultPart = "la"
                    posePoint = poseJoints["CentroidLeftHand"]
                    val yTop = extremePoints["yTop"]
                    val yBottom = extremePoints["yBottom"]
                    val xLeft = extremePoints["xLeft"]
                    val xRight = extremePoints["xRight"]
                    val yLeft = extremePoints["yLeft"]
                    if (yTop != null && yBottom != null && xLeft != null && xRight != null && yLeft != null) {
                        val lengthTemplateBox = (AHI_INSPECT_HAND_BOX_H_FRONT * abs(yTop - yBottom))
                        val widthTemplateBox = (AHI_INSPECT_HAND_BOX_W_FRONT * abs(xLeft - xRight))
                        val left = xLeft - widthTemplateBox
                        val top = yLeft - (lengthTemplateBox * 0.7F)
                        val bottom = top + lengthTemplateBox
                        val right = left + widthTemplateBox
                        binaryImageBoxTL = RectF(left, top, right, bottom)
                    }
                }

                2 -> {
                    // Right Hand
                    resultPart = "ra"
                    posePoint = poseJoints["CentroidRightHand"]
                    val yTop = extremePoints["yTop"]
                    val yBottom = extremePoints["yBottom"]
                    val xLeft = extremePoints["xLeft"]
                    val xRight = extremePoints["xRight"]
                    val yRight = extremePoints["yRight"]
                    if (yTop != null && yBottom != null && xLeft != null && xRight != null && yRight != null) {
                        val lengthTemplateBox = (AHI_INSPECT_HAND_BOX_H_FRONT * abs(yTop - yBottom))
                        val widthTemplateBox = (AHI_INSPECT_HAND_BOX_W_FRONT * abs(xLeft - xRight))
                        val left = xRight
                        val top = yRight - (lengthTemplateBox * 0.7F)
                        val bottom = top + lengthTemplateBox
                        val right = left + widthTemplateBox
                        binaryImageBoxTL = RectF(left, top, right, bottom)
                    }
                }

                3 -> {
                    // Left Leg
                    resultPart = "ll"
                    posePoint = poseJoints["CentroidLeftAnkle"]
                    val yTop = extremePoints["yTop"]
                    val yBottom = extremePoints["yBottom"]
                    val xLeft = extremePoints["xLeft"]
                    val xRight = extremePoints["xRight"]
                    val xBottomLeft = extremePoints["xBottomLeft"]
                    if (yTop != null && yBottom != null && xLeft != null && xRight != null && xBottomLeft != null) {
                        var lengthTemplateBox = (AHI_INSPECT_LEG_BOX_H_FRONT * abs(yTop - yBottom))
                        val widthTemplateBox = (AHI_INSPECT_LEG_BOX_W_FRONT * abs(xLeft - xRight))
                        val left = xBottomLeft - widthTemplateBox
                        var top = yBottom - lengthTemplateBox
                        if (optimalZoneAnkles != null) {
                            top = optimalZoneAnkles.top
                            lengthTemplateBox = optimalZoneAnkles.height()
                        }
                        val bottom = top + lengthTemplateBox
                        val right = left + widthTemplateBox
                        binaryImageBoxTL = RectF(left, top, right, bottom)
                    }
                }

                4 -> {
                    // Right Leg
                    resultPart = "rl"
                    posePoint = poseJoints["CentroidRightAnkle"]
                    val yTop = extremePoints["yTop"]
                    val yBottom = extremePoints["yBottom"]
                    val xLeft = extremePoints["xLeft"]
                    val xRight = extremePoints["xRight"]
                    val xBottomRight = extremePoints["xBottomRight"]
                    if (yTop != null && yBottom != null && xLeft != null && xRight != null && xBottomRight != null) {
                        var lengthTemplateBox = (AHI_INSPECT_LEG_BOX_H_FRONT * abs(yTop - yBottom))
                        val widthTemplateBox = (AHI_INSPECT_LEG_BOX_W_FRONT * abs(xLeft - xRight))
                        val left = xBottomRight
                        var top = yBottom - lengthTemplateBox
                        if (optimalZoneAnkles != null) {
                            top = optimalZoneAnkles.top
                            lengthTemplateBox = optimalZoneAnkles.height()
                        }
                        val bottom = top + lengthTemplateBox
                        val right = left + widthTemplateBox
                        binaryImageBoxTL = RectF(left, top, right, bottom)
                    }
                }
            }
            val result =
                when {
                    posePoint == null -> {
                        InspectionResult.falseNotDetected
                    }

                    binaryImageBoxTL?.contains(posePoint) == true -> {
                        InspectionResult.trueInContour
                    }

                    else -> {
                        InspectionResult.falseNotInContour
                    }
                }
            inspectionResult[resultPart] = result
            updateLastInspectionResultDetails(resultPart, PoseInspectionInfo(binaryImageBoxTL, posePoint, result))

        }
        return inspectionResult
    }

    private suspend fun inspectSidePoseWithDetectedPose(
        extremePoints: Map<String, Float>,
        optimalZones: Map<String, RectF>,
        poseJoints: Map<String, PointF>
    ): Map<String, InspectionResult> {
        val inspectionResult = mutableMapOf<String, InspectionResult>()
        val optimalZoneHead = optimalZones[AHIBSOptimalContourZone.HEAD.key]
        val optimalZoneAnkles = optimalZones[AHIBSOptimalContourZone.ANKLES.key]
        for (countJoints in 0..2) {
            var resultPart = ""
            var binaryImageBoxTL: RectF? = null
            var posePoint: PointF? = null
            when (countJoints) {
                0 -> {
                    // Face
                    resultPart = "face"
                    posePoint = poseJoints["CentroidHeadTop"]
                    val top = optimalZoneHead?.top ?: extremePoints["yTop"]
                    val xContourFaceCenter = extremePoints["xContourFaceCenter"]
                    val yContourFaceCenter = extremePoints["yContourFaceCenter"]
                    val contourFaceWidth = extremePoints["ContourFaceWidth"]
                    val contourFaceHeight =
                        optimalZoneHead?.height() ?: extremePoints["ContourFaceHeight"]
                    if (xContourFaceCenter != null && contourFaceHeight != null && contourFaceWidth != null && top != null && yContourFaceCenter != null) {
                        val halfWidth = (contourFaceWidth * 0.5F)
                        val halfHeight = (contourFaceHeight * 0.5F)
                        val left = xContourFaceCenter - halfWidth
                        val right = xContourFaceCenter + halfWidth
                        val bottom = yContourFaceCenter + halfHeight
                        binaryImageBoxTL = RectF(left, top, right, bottom)
                    }
                }

                1 -> {
                    // Upper Body
                    resultPart = "ub"
                    posePoint = poseJoints["CentroidRightHand"]
                    val yTop = extremePoints["yTop"]
                    val yBottom = extremePoints["yBottom"]
                    val xLeft = extremePoints["xLeft"]
                    val xRight = extremePoints["xRight"]
                    if (yTop != null && yBottom != null && xLeft != null && xRight != null) {
                        val lengthTemplateBox =
                            (AHI_INSPECT_HAND_BOX_H_SIDE * abs(yTop - yBottom))
                        val widthTemplateBox =
                            (AHI_INSPECT_HAND_BOX_W_SIDE * abs(xLeft - xRight))
                        val left =
                            max(0F, (AHIBSImageCaptureWidth / 2) - widthTemplateBox)
                        val top =
                            max(0F, (AHIBSImageCaptureHeight / 2) - lengthTemplateBox)
                        val bottom = top + (lengthTemplateBox * 2F)
                        val right = left + (widthTemplateBox * 2F)
                        binaryImageBoxTL = RectF(left, top, right, bottom)
                    }
                }

                2 -> {
                    // Lower Body
                    resultPart = "lb"
                    posePoint = poseJoints["CentroidRightAnkle"]
                    val yTop = extremePoints["yTop"]
                    val yBottom = extremePoints["yBottom"]
                    val xLeft = extremePoints["xLeft"]
                    val xRight = extremePoints["xRight"]
                    val xBottomRight = extremePoints["xBottomRight"]
                    if (yTop != null && yBottom != null && xLeft != null && xRight != null && xBottomRight != null) {
                        var lengthTemplateBox =
                            (AHI_INSPECT_LEG_BOX_H_SIDE * abs(yTop - yBottom))
                        val widthTemplateBox =
                            (AHI_INSPECT_LEG_BOX_W_SIDE * abs(xLeft - xRight))
                        val left = max(0F, xBottomRight)
                        var top = max(0F, yBottom - lengthTemplateBox)
                        if (optimalZoneAnkles != null) {
                            top = optimalZoneAnkles.top
                            lengthTemplateBox = optimalZoneAnkles.height()
                        }
                        val bottom = top + lengthTemplateBox
                        val right = left + widthTemplateBox
                        binaryImageBoxTL = RectF(left, top, right, bottom)
                    }
                }
            }
            val result =
                when {
                    posePoint == null -> {
                        InspectionResult.falseNotDetected
                    }

                    binaryImageBoxTL?.contains(posePoint) == true -> {
                        InspectionResult.trueInContour
                    }

                    else -> {
                        InspectionResult.falseNotInContour
                    }
                }
            inspectionResult[resultPart] = result
            updateLastInspectionResultDetails(resultPart, PoseInspectionInfo(binaryImageBoxTL, posePoint, result))
        }
        return inspectionResult
    }

    companion object {
        // Segment regions
        private const val AHI_INSPECT_HAND_BOX_W_FRONT = 0.175F
        private const val AHI_INSPECT_HAND_BOX_W_SIDE = 0.4F
        private const val AHI_INSPECT_HAND_BOX_H_FRONT = 0.3F
        private const val AHI_INSPECT_HAND_BOX_H_SIDE = 0.1F
        private const val AHI_INSPECT_LEG_BOX_H_FRONT = 0.1464F
        private const val AHI_INSPECT_LEG_BOX_H_SIDE = 0.225F
        private const val AHI_INSPECT_LEG_BOX_W_FRONT = 0.246F
        private const val AHI_INSPECT_LEG_BOX_W_SIDE = 0.7F
    }
}