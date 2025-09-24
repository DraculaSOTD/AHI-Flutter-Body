package com.advancedhumanimaging.sdk.bodyscan.partclassification

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import com.advancedhumanimaging.sdk.bodyscan.common.*
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.AHIBSResourceType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IClassification
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IResources
import com.advancedhumanimaging.sdk.common.models.AHILogLevel
import com.advancedhumanimaging.sdk.common.models.AHILogging
import com.advancedhumanimaging.sdk.common.models.AHIResult
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

class Classification : IClassification {
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

    override suspend fun classify(
        context: Context,
        resources: IResources,
        sex: SexType,
        heightCM: Double,
        weightKG: Double,
        captures: Array<CaptureGrouping>,
        useAverage:Boolean
    ): AHIResult<Map<String, Any>?> {
        return withContext(Dispatchers.IO) {
            try {
                if (heightCM < MIN_HEIGHT || heightCM > MAX_HEIGHT || weightKG < MIN_WEIGHT || weightKG > MAX_WEIGHT) {
                    AHILogging.log(AHILogLevel.ERROR, "Classification failed due to invalid height or weight")
                    return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_CLASSIFICATION_INVALID_HEIGHT_WEIGHT)
                }
                if (captures.isEmpty()) {
                    AHILogging.log(AHILogLevel.ERROR, "Classification failed due to no captures provided")
                    return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_CLASSIFICATION_INVALID_CAPTURE)
                }
                captures.forEach { capture ->
                    val frontImage = capture.front.image
                    val sideImage = capture.side.image
                    if (
                        frontImage.width.toFloat() != AHIBSImageCaptureWidth ||
                        frontImage.height.toFloat() != AHIBSImageCaptureHeight ||
                        sideImage.width.toFloat() != AHIBSImageCaptureWidth ||
                        sideImage.height.toFloat() != AHIBSImageCaptureHeight
                    ) {
                        AHILogging.log(AHILogLevel.ERROR, "Classification failed due to capture images having invalid dimensions")
                        return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_CLASSIFICATION_INVALID_CAPTURE_IMAGE_DIMENSIONS)
                    }
                    if (expectedJoints.any { !capture.front.joints.containsKey(it) } || expectedJoints.any { !capture.side.joints.containsKey(it) }) {
                        AHILogging.log(AHILogLevel.ERROR, "Classification failed due to missing pose joints")
                        return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_CLASSIFICATION_MISSING_JOINTS)
                    }
                }
                val tfModelsMap = mutableMapOf<String, Pair<ByteArray, Int>>()
                val tfModelNames = ClassificationJNI.getTfLiteModelNames().asList()
                tfModelNames.forEach { name ->
                    val buffer = resources.getResource(name, AHIBSResourceType.AHIBSResourceTypeML, context).getOrNull()
                    if (buffer != null) {
                        tfModelsMap[name] = Pair(buffer, buffer.size)
                    }
                }
                val svrModelsMap = mutableMapOf<String, Pair<ByteArray, Int>>()
                val svrModelNames = ClassificationJNI.getSvrModelNames().asList()
                svrModelNames.forEach { name ->
                    val buffer = resources.getResource(name, AHIBSResourceType.AHIBSResourceTypeSVR, context).getOrNull()
                    if (buffer != null) {
                        svrModelsMap[name] = Pair(buffer, buffer.size)
                    }
                }
                if (tfModelNames.size != tfModelsMap.size) {
                    AHILogging.log(AHILogLevel.ERROR, "Classification failed due to some missing resources")
                    return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_CLASSIFICATION_MISSING_ML_MODELS)
                }
                if (svrModelNames.size != svrModelsMap.size) {
                    AHILogging.log(AHILogLevel.ERROR, "Classification failed due to some missing resources")
                    return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_CLASSIFICATION_MISSING_SVR_MODELS)
                }
                val frontSilhouettes = mutableListOf<Bitmap>()
                val sideSilhouettes = mutableListOf<Bitmap>()
                val frontJoints = mutableListOf<Map<String, PointF>>()
                val sideJoints = mutableListOf<Map<String, PointF>>()
                captures.forEach { capture ->
                    frontSilhouettes.add(capture.front.image)
                    sideSilhouettes.add(capture.side.image)
                    frontJoints.add(capture.front.joints)
                    sideJoints.add(capture.side.joints)
                }
                val result = ClassificationJNI.classifyMultiple(
                    heightCM,
                    weightKG,
                    sex,
                    frontSilhouettes.toTypedArray(),
                    sideSilhouettes.toTypedArray(),
                    frontJoints.toTypedArray(),
                    sideJoints.toTypedArray(),
                    tfModelsMap,
                    svrModelsMap,
                    useAverage
                )
                AHIResult.success(result)
            } catch (e: Exception) {
                AHILogging.log(AHILogLevel.ERROR, "Classification failed")
                AHIResult.failure(BodyScanError.BODY_SCAN_CLASSIFICATION_FAILED)
            }
        }
    }

    companion object {
        private const val MIN_HEIGHT = 50
        private const val MAX_HEIGHT = 255
        private const val MIN_WEIGHT = 16
        private const val MAX_WEIGHT = 300
    }
}