//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.viewmodel

import android.graphics.PointF
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import com.advancedhumanimaging.sdk.bodyscan.common.DetectionType

class AHIBSPoseDetectionViewModel : ViewModel() {
    var landmark by mutableStateOf<Map<String, List<Map<String, PointF>>>?>(null)
    var selectType by mutableStateOf(DetectionType.body)
}