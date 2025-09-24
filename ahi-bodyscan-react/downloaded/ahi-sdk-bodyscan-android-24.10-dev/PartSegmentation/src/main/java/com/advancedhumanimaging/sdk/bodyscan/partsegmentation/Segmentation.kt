//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partsegmentation

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.AHIBSResourceType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IResources
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.ISegmentation
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.jni.SegmentationJNI
import com.advancedhumanimaging.sdk.common.models.AHIResult
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

object Segmentation : ISegmentation {
    private val expectedJoints = listOf(
        "CentroidHeadTop",
        "CentroidNeck",
        "CentroidRightAnkle",
        "CentroidLeftAnkle",
        "CentroidRightKnee",
        "CentroidLeftKnee",
        "CentroidRightHip",
        "CentroidLeftHip",
        "CentroidRightHand",
        "CentroidLeftHand",
        "CentroidRightElbow",
        "CentroidLeftElbow",
        "CentroidRightShoulder",
        "CentroidLeftShoulder",
        "CentroidNose"
    )

    override suspend fun segment(
        capture: Bitmap,
        contourMask: Bitmap,
        profile: Profile,
        poseJoints: Map<String, PointF>,
        context: Context,
        resources: IResources
    ): AHIResult<Bitmap> {
        return withContext(Dispatchers.IO) {
            if (capture.width != 720 || capture.height != 1280) {
                return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENTATION_INCORRECT_CAPTURE_RESOLUTION)
            }
            if (contourMask.width != 720 || contourMask.height != 1280) {
                return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENTATION_INCORRECT_CONTOUR_RESOLUTION)
            }
            if (expectedJoints.any { !poseJoints.containsKey(it) }) {
                return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENTATION_MISSING_JOINTS)
            }
            val modelBuffer = resources.getResource("segnet", AHIBSResourceType.AHIBSResourceTypeML, context).getOrNull()
            if (modelBuffer != null) {
                val img = SegmentationJNI.segment(
                    capture,
                    contourMask,
                    profile,
                    poseJoints,
                    modelBuffer,
                    modelBuffer.size
                )
                if (img != null) {
                    AHIResult.success(img)
                } else {
                    AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENTATION_FAILED)
                }
            } else {
                AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENTATION_MODEL_MISSING)
            }
        }
    }

    suspend fun segment(
        captures: Array<Bitmap>,
        contourMasks: Array<Bitmap>,
        profiles: Array<Profile>,
        poseJoints: Array<Map<String, PointF>>,
        context: Context,
        resources: IResources
    ): AHIResult<Array<Bitmap>> {
        return withContext(Dispatchers.IO) {
            captures.forEach {
                if (it.width != 720 || it.height != 1280) {
                    return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENT_LIST_INCORRECT_CAPTURE_RESOLUTION)
                }
            }
            contourMasks.forEach {
                if (it.width != 720 || it.height != 1280) {
                    return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENT_LIST_INCORRECT_CONTOUR_RESOLUTION)
                }
            }
            poseJoints.forEach { joints ->
                if (expectedJoints.any { !joints.containsKey(it) }) {
                    return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENT_LIST_MISSING_JOINTS)
                }
            }
            val modelBuffer = resources.getResource("segnet", AHIBSResourceType.AHIBSResourceTypeML, context).getOrNull()
            if (modelBuffer != null) {
                val img = SegmentationJNI.segmentAll(
                    captures,
                    contourMasks,
                    profiles,
                    poseJoints,
                    modelBuffer,
                    modelBuffer.size
                )
                if (img != null) {
                    AHIResult.success(img)
                } else {
                    AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENT_LIST_FAILED)
                }
            } else {
                AHIResult.failure(BodyScanError.BODY_SCAN_SEGMENT_LIST_MODEL_MISSING)
            }
        }
    }
}