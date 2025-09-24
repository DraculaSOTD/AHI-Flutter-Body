//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partclassification.example.viewmodel

import android.content.Context
import android.content.res.AssetManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PointF
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import com.advancedhumanimaging.sdk.bodyscan.common.Capture
import com.advancedhumanimaging.sdk.bodyscan.common.CaptureGrouping
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partclassification.Classification
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

class AHIBSClassificationViewModel(private val assetManager: AssetManager) : ViewModel() {
    private val resources = Resources()
    private val classification = Classification()
    var sex by mutableStateOf(SexType.male)
    var height by mutableStateOf(190.0F)
    var weight by mutableStateOf(70.0F)
    val frontSilhouette = mutableStateOf(getSilhouette(Profile.front))
    val sideSilhouette = mutableStateOf(getSilhouette(Profile.side))
    val filterJoints = listOf("CentroidHeadTop", "CentroidNeck", "CentroidRightKnee", "CentroidRightAnkle", "CentroidLeftKnee", "CentroidLeftAnkle")
    val frontJoints = mutableStateOf(
        mapOf(
            "CentroidHeadTop" to PointF(350.479F, 78.79068F),
            "CentroidNeck" to PointF(350.479F, 225.30754F),
            "CentroidRightAnkle" to PointF(268.8666F, 1122.775F),
            "CentroidLeftAnkle" to PointF(453.10526F, 1125.771F),
            "CentroidRightKnee" to PointF(283.39487F, 889.2501F),
            "CentroidLeftKnee" to PointF(433.1378F, 886.6291F),
            "CentroidRightHip" to PointF(292.84134F, 628.79407F),
            "CentroidLeftHip" to PointF(417.66504F, 629.3525F),
            "CentroidRightHand" to PointF(98.42963F, 652.48303F),
            "CentroidLeftHand" to PointF(621.12915F, 651.6245F),
            "CentroidRightElbow" to PointF(177.2645F, 471.7944F),
            "CentroidLeftElbow" to PointF(548.3079F, 468.18817F),
            "CentroidRightShoulder" to PointF(249.55823F, 309.0467F),
            "CentroidLeftShoulder" to PointF(476.70828F, 309.1195F),
            "CentroidNose" to PointF(348.65778F, 160.92563F)
        )
    )
    val sideJoints = mutableStateOf(
        mapOf(
            "CentroidHeadTop" to PointF(368.21106F, 68.45384F),
            "CentroidNeck" to PointF(368.21106F, 228.95241F),
            "CentroidRightAnkle" to PointF(345.386F, 1149.2936F),
            "CentroidLeftAnkle" to PointF(337.06787F, 1120.261F),
            "CentroidRightKnee" to PointF(359.0062F, 887.20996F),
            "CentroidLeftKnee" to PointF(355.96588F, 872.59674F),
            "CentroidRightHip" to PointF(367.0083F, 620.60876F),
            "CentroidLeftHip" to PointF(371.98386F, 623.1391F),
            "CentroidRightHand" to PointF(395.06506F, 677.1813F),
            "CentroidLeftHand" to PointF(388.1402F, 662.5207F),
            "CentroidRightElbow" to PointF(343.24966F, 463.985F),
            "CentroidLeftElbow" to PointF(356.23596F, 460.41724F),
            "CentroidRightShoulder" to PointF(328.8516F, 273.66187F),
            "CentroidLeftShoulder" to PointF(344.94974F, 281.6684F),
            "CentroidNose" to PointF(382.90192F, 161.08696F)
        )
    )
    val classificationResult = mutableStateOf<Map<String, Any>?>(null)

    suspend fun classify(context: Context, useAverage:Boolean): Boolean {
        return withContext(Dispatchers.IO) {
            val frontCapture = Capture(
                image = frontSilhouette.value,
                meta = mapOf("joints" to frontJoints.value)
            )
            val sideCapture = Capture(
                sideSilhouette.value,
                meta = mapOf("joints" to sideJoints.value)
            )
            val result = classification.classify(
                context = context,
                resources = resources,
                sex = sex,
                heightCM = height.toDouble(),
                weightKG = weight.toDouble(),
                captures = arrayOf(
                    CaptureGrouping(
                        frontCapture,
                        sideCapture
                    )
                ),
                useAverage = useAverage
            )
            classificationResult.value = result.getOrNull()
            result.isSuccess
        }
    }

    private fun getSilhouette(profile: Profile): Bitmap {
        val img =
            assetManager.open(if (profile == Profile.front) "front_silhouette.JPEG" else "side_silhouette.JPEG")
        return BitmapFactory.decodeStream(img)
    }
}