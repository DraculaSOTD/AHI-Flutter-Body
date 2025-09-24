//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcontour.example.viewmodel

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
import com.advancedhumanimaging.sdk.bodyscan.common.*
import com.advancedhumanimaging.sdk.bodyscan.partcontour.ContourGenerator
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.BodyScanPoseDetection
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources

class ContourViewModel(private val assetManager: AssetManager) : ViewModel() {
    private val resources = Resources()
    private val poseDetection = BodyScanPoseDetection()
    private val contourGenerator = ContourGenerator()
    var profile by mutableStateOf(Profile.front)
    var idealContourPointsFront by mutableStateOf<Array<PointF>?>(null)
    var idealContourPointsSide by mutableStateOf<Array<PointF>?>(null)
    var contourMaskFront by mutableStateOf<Bitmap?>(null)
    var contourMaskSide by mutableStateOf<Bitmap?>(null)
    var humanFront by mutableStateOf(getHumanBitmap(Profile.front))
    var humanSide by mutableStateOf(getHumanBitmap(Profile.side))
    var poseJointsFront by mutableStateOf(mapOf<String, PointF>())
    var poseJointsSide by mutableStateOf(mapOf<String, PointF>())
    var scaledContourPointsFront by mutableStateOf<Array<PointF>?>(null)
    var scaledContourPointsSide by mutableStateOf<Array<PointF>?>(null)
    var optimalZonesFront by mutableStateOf<Map<String, RectF>?>(null)
    var optimalZonesSide by mutableStateOf<Map<String, RectF>?>(null)
    var isUserInOptimalZoneFrontHead by mutableStateOf(false)
    var isUserInOptimalZoneFrontAnkles by mutableStateOf(false)
    var isUserInOptimalZoneSideHead by mutableStateOf(false)
    var isUserInOptimalZoneSideAnkles by mutableStateOf(false)
    private val imageSize =
        Resolution(AHIBSImageCaptureWidth.toInt(), AHIBSImageCaptureHeight.toInt())

    private fun getHumanBitmap(profile: Profile): Bitmap {
        val humanImg =
            assetManager.open(if (profile == Profile.front) "front.png" else "side.png")
        return Bitmap.createScaledBitmap(BitmapFactory.decodeStream(humanImg), 720, 1280, true)
    }

    suspend fun generateIdealContour(
        context: Context,
        sex: SexType,
        height: Float,
        weight: Float,
        theta: Float
    ) {
        val thetaInRadians = Math.toRadians(theta.toDouble()).toFloat()
        if (profile == Profile.front) {
            idealContourPointsFront = contourGenerator.generateIdealContour(
                context,
                resources,
                sex,
                height,
                weight,
                imageSize,
                thetaInRadians,
                profile
            )
        } else {
            idealContourPointsSide = contourGenerator.generateIdealContour(
                context,
                resources,
                sex,
                height,
                weight,
                imageSize,
                thetaInRadians,
                profile
            )
        }
        generateContourMask()
    }

    private suspend fun generateContourMask() {
        if (profile == Profile.front) {
            contourMaskFront = idealContourPointsFront?.let { contourGenerator.generateContourMask(it, imageSize) }
        } else {
            contourMaskSide = idealContourPointsSide?.let { contourGenerator.generateContourMask(it, imageSize) }
        }
    }

    suspend fun detectPose() {
        if (profile == Profile.front) {
            val joints = poseDetection.detect(DetectionType.faceAndBody, humanFront) ?: mapOf()
            val faceJoints = joints["face"]?.maxByOrNull { it.size } ?: mapOf()
            val bodyJoints = joints["body"]?.maxByOrNull { it.size } ?: mapOf()
            poseJointsFront = faceJoints + bodyJoints
        } else {
            val joints = poseDetection.detect(DetectionType.faceAndBody, humanSide) ?: mapOf()
            val faceJoints = joints["face"]?.maxByOrNull { it.size } ?: mapOf()
            val bodyJoints = joints["body"]?.maxByOrNull { it.size } ?: mapOf()
            poseJointsSide = faceJoints + bodyJoints
        }
    }

    suspend fun generateScaledContour() {
        if (profile == Profile.front) {
            scaledContourPointsFront = contourGenerator.generateScaledContour(
                idealContourPointsFront ?: emptyArray(),
                poseJointsFront
            )
        } else {
            scaledContourPointsSide = contourGenerator.generateScaledContour(
                idealContourPointsSide ?: emptyArray(),
                poseJointsSide
            )
        }
    }

    suspend fun generateOptimalZones() {
        if (profile == Profile.front) {
            optimalZonesFront = idealContourPointsFront?.let {
                contourGenerator.generateOptimalZones(
                    it,
                    imageSize,
                )
            }
        } else {
            optimalZonesSide =
                idealContourPointsSide?.let { contourGenerator.generateOptimalZones(it, imageSize) }
        }
    }

    suspend fun isUserInOptimalZone() {
        if (profile == Profile.front) {
            val headArea = optimalZonesFront?.get(AHIBSOptimalContourZone.HEAD.key)
            isUserInOptimalZoneFrontHead =
                headArea?.let {
                    contourGenerator.isUserInOptimalZone(
                        AHIBSOptimalContourZone.HEAD.key,
                        it,
                        poseJointsFront
                    )
                } == true
            val anklesArea = optimalZonesFront?.get(AHIBSOptimalContourZone.ANKLES.key)
            isUserInOptimalZoneFrontAnkles =
                anklesArea?.let {
                    contourGenerator.isUserInOptimalZone(
                        AHIBSOptimalContourZone.ANKLES.key,
                        it,
                        poseJointsFront
                    )
                } == true
        } else {
            val headArea = optimalZonesSide?.get(AHIBSOptimalContourZone.HEAD.key)
            isUserInOptimalZoneSideHead =
                headArea?.let {
                    contourGenerator.isUserInOptimalZone(
                        AHIBSOptimalContourZone.HEAD.key,
                        it,
                        poseJointsSide
                    )
                } == true
            val anklesArea = optimalZonesSide?.get(AHIBSOptimalContourZone.ANKLES.key)
            isUserInOptimalZoneSideAnkles =
                anklesArea?.let {
                    contourGenerator.isUserInOptimalZone(
                        AHIBSOptimalContourZone.ANKLES.key,
                        it,
                        poseJointsSide
                    )
                } == true
        }
    }

}