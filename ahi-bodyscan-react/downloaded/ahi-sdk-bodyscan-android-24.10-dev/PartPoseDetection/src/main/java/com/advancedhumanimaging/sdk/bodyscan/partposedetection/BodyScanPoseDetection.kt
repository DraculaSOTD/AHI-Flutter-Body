//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposedetection

import android.graphics.Bitmap
import android.graphics.PointF
import android.media.Image
import com.advancedhumanimaging.sdk.bodyscan.common.DetectionType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IPoseDetection
import com.google.mlkit.vision.common.InputImage
import com.google.mlkit.vision.face.FaceDetection
import com.google.mlkit.vision.face.FaceDetectorOptions
import com.google.mlkit.vision.face.FaceLandmark
import com.google.mlkit.vision.pose.PoseDetection.getClient
import com.google.mlkit.vision.pose.PoseLandmark
import com.google.mlkit.vision.pose.defaults.PoseDetectorOptions
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

class BodyScanPoseDetection : IPoseDetection {
    private var leftEye: PointF? = null
    private var rightEye: PointF? = null
    private var leftMouth: PointF? = null
    private val poseOptions = PoseDetectorOptions.Builder()
        .setDetectorMode(PoseDetectorOptions.STREAM_MODE)
        .build()
    private val poseDetector = getClient(poseOptions)
    private val faceOptions = FaceDetectorOptions.Builder()
        .setPerformanceMode(FaceDetectorOptions.PERFORMANCE_MODE_ACCURATE)
        .setLandmarkMode(FaceDetectorOptions.CONTOUR_MODE_NONE)
        .setLandmarkMode(FaceDetectorOptions.LANDMARK_MODE_ALL)
        .build()
    private val faceDetector = FaceDetection.getClient(faceOptions)

    override suspend fun detect(
        type: DetectionType,
        bitmap: Bitmap,
    ): Map<String, List<Map<String, PointF>>>? {
        return withContext(Dispatchers.IO) {
            try {
                val image = InputImage.fromBitmap(bitmap, 0)
                detect(type, image)
            } catch (e: Exception) {
                e.printStackTrace()
                null
            }
        }
    }

    suspend fun detect(
        type: DetectionType,
        image: Image,
        rotationDegrees: Int
    ): Map<String, List<Map<String, PointF>>>? {
        return withContext(Dispatchers.IO) {
            try {
                val inputImage = InputImage.fromMediaImage(image, rotationDegrees)
                detect(type, inputImage)
            } catch (e: Exception) {
                e.printStackTrace()
                null
            }
        }
    }

    private suspend fun detect(type: DetectionType, inputImage: InputImage): Map<String, List<Map<String, PointF>>> {
        val poseJoints: MutableMap<String, List<Map<String, PointF>>> = mutableMapOf()
        val poseResult =
            poseDetector.process(inputImage)
                .addOnCompleteListener { result ->
                    val pose = result.result
                    val joints = mutableMapOf<String, PointF>()
                    Landmarks.landmarksBody.forEach { poseLandmark ->
                        pose.getPoseLandmark(poseLandmark.value)?.let { landmark ->
                            validateLandmark(landmark)?.let {
                                joints[poseLandmark.key] = it
                            }
                        }
                    }
                    poseJoints["body"] = listOf(joints)
                    leftEye = pose.getPoseLandmark(PoseLandmark.LEFT_EYE)?.let { validateLandmark(it) }
                    rightEye = pose.getPoseLandmark(PoseLandmark.RIGHT_EYE)?.let { validateLandmark(it) }
                    leftMouth = pose.getPoseLandmark(PoseLandmark.LEFT_MOUTH)?.let { validateLandmark(it) }
                }
        val faceResult =
            if (type == DetectionType.face || type == DetectionType.faceAndBody) {
                faceDetector.process(inputImage)
                    .addOnCompleteListener { faces ->
                        val jointsList = mutableListOf<Map<String, PointF>>()
                        faces.result.forEach { face ->
                            val joints = mutableMapOf<String, PointF>()
                            joints["FaceSize"] = PointF(
                                face.boundingBox.width().toFloat(),
                                face.boundingBox.height().toFloat()
                            )
                            joints["FacePosition"] = PointF(
                                face.boundingBox.exactCenterX(),
                                face.boundingBox.exactCenterY()
                            )
                            val leftEye = face.getLandmark(FaceLandmark.LEFT_EYE)?.position
                            val rightEye = face.getLandmark(FaceLandmark.RIGHT_EYE)?.position
                            val leftMouth = face.getLandmark(FaceLandmark.MOUTH_LEFT)?.position
                            if (leftEye != null && rightEye != null && leftMouth != null) {
                                val (headTop, neck) = getFaceTopAndNeck(rightEye, leftEye, leftMouth)
                                joints["CentroidHeadTop"] = headTop
                                joints["CentroidNeck"] = neck
                            }
                            jointsList.add(joints)
                        }
                        poseJoints["face"] = jointsList
                    }
            } else {
                null
            }
        poseResult.await()
        faceResult?.await()
        if ((type == DetectionType.face || type == DetectionType.faceAndBody) && poseJoints["face"].isNullOrEmpty() && leftEye != null && rightEye != null && leftMouth != null) {
            val (headTop, neck) = getFaceTopAndNeck(rightEye!!, leftEye!!, leftMouth!!)
            poseJoints["face"] = listOf(
                mapOf(
                    "CentroidHeadTop" to headTop,
                    "CentroidNeck" to neck
                )
            )
        }
        when (type) {
            DetectionType.face -> poseJoints.remove("body")
            DetectionType.body -> poseJoints.remove("face")
            else -> {}
        }
        return poseJoints
    }

    private fun validateLandmark(landmark: PoseLandmark): PointF? {
        return if (landmark.inFrameLikelihood > 0.9f) {
            landmark.position
        } else {
            null
        }
    }

    private fun getFaceTopAndNeck(rightEye: PointF, leftEye: PointF, leftMouth: PointF): Pair<PointF, PointF> {
        val x = rightEye.x + ((leftEye.x - rightEye.x) * 0.5F)
        val y = leftMouth.y - rightEye.y
        val yHead = rightEye.y - (y * 1.6F)
        val yNeck = leftMouth.y + y
        val headTop = PointF(x, yHead)
        val neck = PointF(x, yNeck)
        return Pair(headTop, neck)
    }
}

object Landmarks {
    val landmarksBody = mapOf(
        "CentroidRightAnkle" to PoseLandmark.RIGHT_ANKLE,
        "CentroidLeftAnkle" to PoseLandmark.LEFT_ANKLE,
        "CentroidRightKnee" to PoseLandmark.RIGHT_KNEE,
        "CentroidLeftKnee" to PoseLandmark.LEFT_KNEE,
        "CentroidRightHip" to PoseLandmark.RIGHT_HIP,
        "CentroidLeftHip" to PoseLandmark.LEFT_HIP,
        "CentroidRightHand" to PoseLandmark.RIGHT_THUMB,
        "CentroidLeftHand" to PoseLandmark.LEFT_THUMB,
        "CentroidRightElbow" to PoseLandmark.RIGHT_ELBOW,
        "CentroidLeftElbow" to PoseLandmark.LEFT_ELBOW,
        "CentroidRightShoulder" to PoseLandmark.RIGHT_SHOULDER,
        "CentroidLeftShoulder" to PoseLandmark.LEFT_SHOULDER,
        "CentroidNose" to PoseLandmark.NOSE,
    )
}
