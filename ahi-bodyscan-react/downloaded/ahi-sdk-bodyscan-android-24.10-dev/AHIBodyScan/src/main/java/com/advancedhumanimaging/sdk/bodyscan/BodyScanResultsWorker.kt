//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import androidx.work.CoroutineWorker
import androidx.work.Data
import androidx.work.WorkerParameters
import com.advancedhumanimaging.sdk.bodyscan.common.*
import com.advancedhumanimaging.sdk.bodyscan.helpers.AHIBSCache
import com.advancedhumanimaging.sdk.bodyscan.model.CaptureData
import com.advancedhumanimaging.sdk.bodyscan.partclassification.Classification
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.Segmentation
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.File
import java.util.*

class BodyScanResultsWorker(
    private val context: Context,
    params: WorkerParameters
) : CoroutineWorker(context, params) {
    override suspend fun doWork(): Result {
        return withContext(Dispatchers.IO) {
            try {
                val decryptionKey = inputData.getString("decryptionKey")
                val resources = Resources(decryptionKey)
                val classification = Classification()
                val uid = inputData.getString("uid") ?: ""
                val sex = inputData.getString("sex")?.let {
                    try {
                        SexType.valueOf(it)
                    } catch (e: Exception) {
                        null
                    }
                }
                val heightCm = inputData.getDouble("heightCm", -1.0)
                val weightKg = inputData.getDouble("weightKg", -1.0)
                // classification to use average calculation and default set as true.
                val useAverage = inputData.getBoolean("useAverage", true)
                if (sex == null || heightCm < 0 || weightKg < 0) {
                    return@withContext Result.failure(
                        Data.Builder().putAll(mapOf("error_code" to BodyScanError.BODY_SCAN_WORKER_MISSING_USER_INPUTS.code())).build()
                    )
                }
                val frontSegmentationDataLocation = inputData.getString("front_segmentation_data_location")
                val frontSegmentationFile = frontSegmentationDataLocation?.let { File(it) }
                val sideCaptureDataLocation = inputData.getString("side_capture_data_location")
                val sideCaptureFile = sideCaptureDataLocation?.let { File(it) }
                // Keep checking every 100ms for 2 minutes to see if the side capture data and front segmentation data has been written to a file
                for (i in 1..(10 * 60 * 2)) {
                    if (sideCaptureFile?.isFile == true && frontSegmentationFile?.isFile == true) {
                        break
                    }
                    delay(100)
                }
                // After the wait, if file is still not ready, exit with error
                if (sideCaptureFile?.isFile != true || frontSegmentationFile?.isFile != true) {
                    return@withContext Result.failure(
                        Data.Builder().putAll(mapOf("error_code" to BodyScanError.BODY_SCAN_WORKER_NO_CAPTURE_DATA_FOUND.code())).build()
                    )
                }
                val frontSegmentationDataJson = JSONObject(frontSegmentationFile.readText())
                val sideCaptureDataJson = JSONObject(sideCaptureFile.readText())
                // Delete all temporary files
                AHIBSCache.deleteAllData(context)
                val frontSegmentationData = CaptureData.fromJson(frontSegmentationDataJson)
                val frontCaptures = mutableListOf<Capture>()
                frontSegmentationData?.let {
                    it.images.forEach { silhouette ->
                        frontCaptures.add(Capture(silhouette, mapOf("joints" to frontSegmentationData.poseJoints)))
                    }
                }
                val sideCaptureData = CaptureData.fromJson(sideCaptureDataJson)
                val sideCaptures = mutableListOf<Capture>()
                if (sideCaptureData != null) {
                    val captures = mutableListOf<Bitmap>()
                    val contours = mutableListOf<Bitmap>()
                    val poseJoints = mutableListOf<Map<String, PointF>>()
                    val profiles = mutableListOf<Profile>()
                    for (i in 0 until sideCaptureData.images.size) {
                        captures.add(sideCaptureData.images[i])
                        contours.add(sideCaptureData.contourMask)
                        poseJoints.add(sideCaptureData.poseJoints)
                        profiles.add(Profile.side)
                    }
                    val segmentationResult = Segmentation.segment(
                        captures.toTypedArray(),
                        contours.toTypedArray(),
                        profiles.toTypedArray(),
                        poseJoints.toTypedArray(),
                        context,
                        resources
                    )
                    if (segmentationResult.isSuccess) {
                        segmentationResult.getOrNull()?.let {
                            it.forEach { silhouette ->
                                sideCaptures.add(Capture(silhouette, mapOf("joints" to sideCaptureData.poseJoints)))
                            }
                        }
                    } else {
                        return@withContext Result.failure(
                            Data.Builder().putAll(mapOf("error_code" to segmentationResult.error()?.code())).build()
                        )
                    }
                }
                if (frontCaptures.size != sideCaptures.size) {
                    return@withContext Result.failure(
                        Data.Builder()
                            .putAll(mapOf("error_code" to BodyScanError.BODY_SCAN_WORKER_NUMBER_OF_FRONT_AND_SIDE_CAPTURES_NOT_THE_SAME.code()))
                            .build()
                    )
                }
                val captures = mutableListOf<CaptureGrouping>()
                for (i in 0 until frontCaptures.size) {
                    captures.add(CaptureGrouping(frontCaptures[i], sideCaptures[i]))
                }
                val classificationResult = classification.classify(
                    context = context,
                    resources = resources,
                    sex = sex,
                    heightCM = heightCm,
                    weightKG = weightKg,
                    captures = captures.toTypedArray(),
                    useAverage = useAverage
                )
                val classificationScores = classificationResult.getOrNull() ?: return@withContext Result.failure(
                    Data.Builder().putAll(mapOf("error_code" to classificationResult.error()?.code())).build()
                )
                val result = mapOf(
                    "id" to UUID.randomUUID().toString(),
                    "uid" to uid,
                    "date" to System.currentTimeMillis() / 1000,
                    "type" to AHIBSScanName,
                    "ver" to AHIBSVersion,
                    "kg_ent_weight" to weightKg,
                    "cm_ent_height" to heightCm,
                    "enum_ent_sex" to sex.name
                ) + classificationScores
                val workData = Data.Builder().putAll(result).build()
                return@withContext Result.success(workData)
            } catch (e: Exception) {
                return@withContext Result.failure(
                    Data.Builder().putAll(mapOf("error_code" to BodyScanError.BODY_SCAN_RESULTS_WORKER_EXCEPTION.code())).build()
                )
            }
        }
    }
}
