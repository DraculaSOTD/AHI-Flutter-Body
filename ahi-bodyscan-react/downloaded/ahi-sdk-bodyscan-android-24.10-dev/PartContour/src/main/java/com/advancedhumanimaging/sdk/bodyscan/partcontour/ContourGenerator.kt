//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcontour

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import androidx.core.graphics.contains
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSOptimalContourZone
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.AHIBSResourceType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IContourGenerator
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IResources
import com.advancedhumanimaging.sdk.common.models.AHILogLevel
import com.advancedhumanimaging.sdk.common.models.AHILogging
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min

class ContourGenerator : IContourGenerator {

    private val cvModels = mapOf(
        "MvnMu" to AHIBSResourceType.AHIBSResourceTypeVF,
        "AvgVerts" to AHIBSResourceType.AHIBSResourceTypeVF,
        "VertsInv" to AHIBSResourceType.AHIBSResourceTypeVF,
        "Ranges" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "Cov" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "SkV" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "BonW" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "BonWInv" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "Sv" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "SvInv" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "Faces" to AHIBSResourceType.AHIBSResourceTypeVI,
        "FacesInv" to AHIBSResourceType.AHIBSResourceTypeVI,
        "LaplacianRings" to AHIBSResourceType.AHIBSResourceTypeVI,
        "LaplacianRingsAsVectors" to AHIBSResourceType.AHIBSResourceTypeVI
    )
    private val cvModelsGenderless = mapOf(
        "InvRightCalf" to AHIBSResourceType.AHIBSResourceTypeVI,
        "InvRightThigh" to AHIBSResourceType.AHIBSResourceTypeVI,
        "InvRightUpperArm" to AHIBSResourceType.AHIBSResourceTypeVI
    )
    private val cvModelsAll = cvModels + cvModelsGenderless

    private val cvModelsMapMale = mutableMapOf<String, Pair<ByteArray, Int>>()
    private val cvModelsMapFemale = mutableMapOf<String, Pair<ByteArray, Int>>()

    private suspend fun getCvModelsMap(
        context: Context,
        resources: IResources,
        sex: SexType,
    ): Map<String, Pair<ByteArray, Int>> {
        return if (sex == SexType.male) {
            if (cvModelsMapMale.size != cvModelsAll.size) {
                cvModelsMapMale.clear()
                cvModelsAll.forEach { (name, resourceType) ->
                    val genderName = "male"
                    val isGenderless = cvModelsGenderless.containsKey(name)
                    val modelName = if (isGenderless) name else "${name}_${genderName}"
                    val buffer = resources.getResource(modelName, resourceType, context).getOrNull()
                    if (buffer != null) {
                        cvModelsMapMale[name] = Pair(buffer, buffer.size)
                    }
                }
            }
            cvModelsMapMale
        } else {
            if (cvModelsMapFemale.size != cvModelsAll.size) {
                cvModelsMapFemale.clear()
                cvModelsAll.forEach { (name, resourceType) ->
                    val genderName = "female"
                    val isGenderless = cvModelsGenderless.containsKey(name)
                    val modelName = if (isGenderless) name else "${name}_${genderName}"
                    val buffer = resources.getResource(modelName, resourceType, context).getOrNull()
                    if (buffer != null) {
                        cvModelsMapFemale[name] = Pair(buffer, buffer.size)
                    }
                }
            }
            cvModelsMapFemale
        }
    }

    override suspend fun generateIdealContour(
        context: Context,
        resources: IResources,
        sex: SexType,
        heightCM: Float,
        weightKG: Float,
        imageSize: Resolution,
        alignmentZRadians: Float,
        profile: Profile,
    ): Array<PointF>? {
        return withContext(Dispatchers.IO) {
            try {
                val cvModelsMapMale = getCvModelsMap(context, resources, SexType.male)
                val cvModelsMapFemale = getCvModelsMap(context, resources, SexType.female)
                when {
                    cvModelsMapMale.size != cvModelsAll.size -> {
                        AHILogging.log(AHILogLevel.ERROR, "Contour generation failed due to some missing resources")
                        null
                    }

                    cvModelsMapFemale.size != cvModelsAll.size -> {
                        AHILogging.log(AHILogLevel.ERROR, "Classification failed due to some missing resources")
                        null
                    }

                    else -> {
                        ContourGeneratorJNI.generateIdealContour(
                            sex,
                            heightCM,
                            weightKG,
                            imageSize,
                            alignmentZRadians,
                            profile,
                            cvModelsMapMale,
                            cvModelsMapFemale
                        )
                    }
                }
            } catch (e: Exception) {
                AHILogging.log(AHILogLevel.ERROR, "Generating ideal contour failed")
                null
            }
        }
    }

    override suspend fun generateContourMask(contour: Array<PointF>, imageSize: Resolution): Bitmap? {
        return withContext(Dispatchers.IO) {
            try {
                if (contour.isEmpty()) {
                    AHILogging.log(AHILogLevel.ERROR, "Generating contour mask failed due to empty contour provided")
                    return@withContext null
                }
                ContourGeneratorJNI.generateContourMask(contour, imageSize)
            } catch (e: Exception) {
                AHILogging.log(AHILogLevel.ERROR, "Generating contour mask failed")
                null
            }
        }
    }

    override suspend fun generateScaledContour(
        contourIdeal: Array<PointF>,
        poseJoints: Map<String, PointF>,
    ): Array<PointF>? {
        return withContext(Dispatchers.IO) {
            try {
                val (minY, maxY) = getYAxisExtreme(contourIdeal)
                // Get vertices of interest
                val chinY = poseJoints["CentroidNeck"]?.y
                val headTopY = poseJoints["CentroidHeadTop"]?.y
                val rightAnkleY = poseJoints["CentroidRightAnkle"]?.y ?: -1F
                val leftAnkleY = poseJoints["CentroidLeftAnkle"]?.y ?: -1F
                val ankleY = max(rightAnkleY, leftAnkleY)
                if (chinY != null && headTopY != null && ankleY > -1F) {
                    // Calculate insights
                    val faceLength = chinY - headTopY
                    // Add face length to ankles to get bottom of feet
                    val feetBottomY = ankleY + faceLength
                    // Adjust bottom of feet to avoid going beyond image size
                    val adjFeetBottomY = max(0.7F * AHIBSImageCaptureHeight, min(AHIBSImageCaptureHeight - 30F, feetBottomY))
                    val deltaPixels = adjFeetBottomY - headTopY
                    val scale = deltaPixels / (maxY - minY)
                    val contourScaled = Array(contourIdeal.size) { i ->
                        var x = scale * (contourIdeal[i].x - AHIBSImageCaptureWidth / 2) + AHIBSImageCaptureWidth / 2
                        var y = scale * (contourIdeal[i].y - minY) + headTopY
                        x = max(30F, min(x, AHIBSImageCaptureWidth - 30F))
                        y = max(30F, min(y, AHIBSImageCaptureHeight - 30F))
                        PointF(x, y)
                    }
                    contourScaled
                } else {
                    null
                }
            } catch (e: Exception) {
                AHILogging.log(AHILogLevel.ERROR, "Generating scaled contour failed")
                null
            }
        }
    }

    override suspend fun generateOptimalZones(
        contourIdeal: Array<PointF>,
        imageSize: Resolution,
        accuracyThreshold: Float,
    ): Map<String, RectF>? {
        return withContext(Dispatchers.IO) {
            try {
                // Determine top and bottom Y values
                val (contTopY, contBottomY) = getYAxisExtreme(contourIdeal)
                // Acc surface model
                val ac = -211.2
                val bc = 356.3
                val cc = -205.8
                val dc = 3.775
                val ec = -3.168
                val fc = 99.98
                // to approx. contour ankle, very close actually
                val contourBottomYAdjusted = 0.9 * contBottomY + 0.1 * contTopY
                // this is not the whole contour it is from head to ankle
                val contHeight = contourBottomYAdjusted - contTopY
                // Create a single Mat Column
                val singleCol = Array(imageSize.height) { 0F }
                // here we mimic the head top change around the contour head top by -200, +400 and we do the same and mimic the user ankle variation around the approximate contour ankle (-400, till the end of the image)
                for (ty in max(0, contTopY.toInt() - 200) until (contTopY.toInt() + 400)) {
                    for (by in (contourBottomYAdjusted.toInt() - 400) until imageSize.height) {
                        val x = (ty - contTopY) / contHeight
                        val y = (by - contourBottomYAdjusted) / contHeight
                        val vc = ac * x * x + bc * x * y + cc * y * y + dc * x + ec * y + fc
                        // accumulated accuracy using superposition, where ankle is fixed then head change location w.r.t contour head (and the same for the head top)
                        singleCol[ty] = singleCol[ty] + vc.toFloat()
                        singleCol[by] = singleCol[by] + vc.toFloat()
                    }
                }
                var maxAccumTop = 0F
                var maxAccumBot = 0F
                for (col in 0 until imageSize.height) {
                    if (col < imageSize.height / 2) {
                        maxAccumTop = max(maxAccumTop, singleCol[col])
                    } else {
                        maxAccumBot = max(maxAccumBot, singleCol[col])
                    }
                }
                // any large value // or set this "later" to the top gap/offset, e.g. top offset
                var topP1 = 1000000
                var topP2 = -1
                // any large value
                var botP1 = 1000000
                // or set this "later" to the bottom gap/offset, e.g. (NSUInteger)MFZSdkImageCaptureHeight- bottom offset
                var botP2 = -1
                // e.g. 0.96 of the maximum possible accuracy
//                val accThreshold = 0.94 //adjusting this to increase/decrease the green zone
                // a tolerance of 30 pixels (not the offset), because we noticed that rare shapes may have a zone not 100% around the contour points
                val tolerance = 30
                for (col in 0 until imageSize.height) {
                    var expectedAccTop: Float
                    var expectedAccBot: Float
                    if (col < imageSize.height / 2) {
                        expectedAccTop = singleCol[col] / (maxAccumTop + 1e-6F)
                        if (expectedAccTop > accuracyThreshold) {
                            if (col < contTopY) {
                                topP1 = min(topP1, col)
                            }
                            if (col > contTopY - tolerance && col < imageSize.height / 2) {
                                topP2 = max(topP2, col)
                            }
                        }
                    } else {
                        expectedAccBot = singleCol[col] / (maxAccumBot + 1e-6F)
                        if (expectedAccBot > accuracyThreshold) {
                            if (col > imageSize.height / 2 && col < contourBottomYAdjusted + tolerance) {
                                botP1 = min(botP1, col)
                            }
                            if (col > contourBottomYAdjusted - tolerance) {
                                botP2 = max(botP2, col)
                            }
                        }
                    }
                }
                val greenZoneMinY = 25
                val greenZoneMaxY = imageSize.height - 60
                // Now we can have the dynamic green zone as rectangles
                topP1 = max(greenZoneMinY, topP1)
                val topH = abs(topP2 - topP1)
                val topMidY = topP1 + topH / 2.0F
                botP2 = min(greenZoneMaxY, botP2)
                val botH = abs(botP2 - botP1)
                val botMidY = botP1 + botH / 2.0F
                val topRect = RectF(0F, topP1.toFloat(), imageSize.width.toFloat(), (topP1 + topH).toFloat())
                val topTargetTop = topMidY - (topRect.height() * 0.95F * 0.5F)
                val topTarget = RectF(0F, topTargetTop, imageSize.width.toFloat(), topTargetTop + (topRect.height() * 0.95F))
                val bottomRect = RectF(0F, botP1.toFloat(), imageSize.width.toFloat(), (botP1 + botH).toFloat())
                val bottomTargetTop = botMidY - bottomRect.height() * 0.95F * 0.5F
                val bottomTarget = RectF(0F, bottomTargetTop, imageSize.width.toFloat(), bottomTargetTop + (bottomRect.height() * 0.95F))
                mapOf(
                    AHIBSOptimalContourZone.HEAD.key to topRect,
                    AHIBSOptimalContourZone.HEAD_TARGET.key to topTarget,
                    AHIBSOptimalContourZone.ANKLES.key to bottomRect,
                    AHIBSOptimalContourZone.ANKLES_TARGET.key to bottomTarget
                )
            } catch (e: Exception) {
                AHILogging.log(AHILogLevel.ERROR, "Generating optimal zones failed")
                null
            }
        }
    }

    override suspend fun isUserInOptimalZone(
        zone: String,
        area: RectF,
        poseJoints: Map<String, PointF>,
    ): Boolean {
        return withContext(Dispatchers.IO) {
            val posePoints = mutableListOf<PointF>()
            when (zone) {
                AHIBSOptimalContourZone.HEAD.key -> {
                    poseJoints["CentroidHeadTop"]?.let { posePoints.add(it) }
                }

                AHIBSOptimalContourZone.ANKLES.key -> {
                    poseJoints["CentroidRightAnkle"]?.let { posePoints.add(it) }
                    poseJoints["CentroidLeftAnkle"]?.let { posePoints.add(it) }
                }
            }
            posePoints.isNotEmpty() && posePoints.all { area.contains(it) }
        }
    }

    private fun getYAxisExtreme(points: Array<PointF>): Pair<Float, Float> {
        val yTop = points.minOfOrNull { it.y } ?: AHIBSImageCaptureHeight
        val yBottom = points.maxOfOrNull { it.y } ?: 0F
        return Pair(yTop, yBottom)
    }
}