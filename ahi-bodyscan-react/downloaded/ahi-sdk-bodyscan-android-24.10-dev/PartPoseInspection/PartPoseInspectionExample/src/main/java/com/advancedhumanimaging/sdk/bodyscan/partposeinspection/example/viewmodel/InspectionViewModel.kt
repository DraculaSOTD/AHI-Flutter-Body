//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.viewmodel

import android.content.Context
import android.content.res.AssetManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PointF
import android.graphics.RectF
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.DetectionType
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partcontour.ContourGenerator
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.BodyScanPoseDetection
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.PoseInspection
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources

class InspectionViewModel(private val assetManager: AssetManager) : ViewModel() {
    private val resources = Resources()
    private val contourGenerator = ContourGenerator()
    var refreshContour by mutableStateOf(true)
    var profile by mutableStateOf(Profile.front)
    var idealContourPoints by mutableStateOf<Array<PointF>?>(null)
    var humanFront by mutableStateOf(getHumanBitmap(Profile.front))
    var humanSide by mutableStateOf(getHumanBitmap(Profile.side))
    var contourMask by mutableStateOf<Bitmap?>(null)
    var poseJoints by mutableStateOf(mapOf<String, List<Map<String, PointF>>>())
    var optimalZones by mutableStateOf<Map<String, RectF>?>(null)
    var isInContour by mutableStateOf(false)
    private val imageSize = Resolution(AHIBSImageCaptureWidth.toInt(), AHIBSImageCaptureHeight.toInt())
    private val poseDetection = BodyScanPoseDetection()
    val poseInspection = PoseInspection()

    private fun getHumanBitmap(profile: Profile): Bitmap {
        val humanImg =
            assetManager.open(if (profile == Profile.front) "front.png" else "side.png")
        return Bitmap.createScaledBitmap(BitmapFactory.decodeStream(humanImg), 720, 1280, true)
    }

    fun updateContourMask(mask: Bitmap) {
        contourMask = mask
    }

    suspend fun generateIdealContour(
        context: Context,
        sex: SexType,
        height: Float,
        weight: Float,
        theta: Float,
        profile: Profile
    ) {
        val thetaInRadians = Math.toRadians(theta.toDouble()).toFloat()
        idealContourPoints =
            contourGenerator.generateIdealContour(context, resources, sex, height, weight, imageSize, thetaInRadians, profile)
        generateContourMask()
        generateOptimalZones()
    }

    suspend fun inspectPose(profile: Profile) {
        detectPose(profile)
        if (contourMask != null) {
            val results = poseInspection.inspect(
                humanFront,
                contourMask!!,
                optimalZones ?: mapOf(),
                Profile.front,
                poseJoints
            )
            isInContour = poseInspection.isInContour(Profile.front, results.getOrNull() ?: mapOf())
        }
    }

    private suspend fun detectPose(profile: Profile) {
        val human = if (profile == Profile.front) {
            humanFront
        } else {
            humanSide
        }
        poseJoints = poseDetection.detect(DetectionType.faceAndBody, human) ?: mapOf()
    }

    private suspend fun generateContourMask() {
        contourMask = idealContourPoints?.let { contourGenerator.generateContourMask(it, imageSize) }
    }

    private suspend fun generateOptimalZones() {
        optimalZones = idealContourPoints?.let { contourGenerator.generateOptimalZones(it, imageSize) }
    }
}