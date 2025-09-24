//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.viewmodel

import android.graphics.Bitmap
import android.graphics.PointF
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import com.advancedhumanimaging.sdk.bodyscan.common.Profile

class AHIBSSegmentationViewModel : ViewModel() {
    var profile by mutableStateOf(Profile.front)
    var humanBitmap by mutableStateOf<Bitmap>(Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888))
    var contourBitmap by mutableStateOf<Bitmap>(Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888))
    var poseJoints by mutableStateOf(
        mapOf(
            // default joints from Dave
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
}