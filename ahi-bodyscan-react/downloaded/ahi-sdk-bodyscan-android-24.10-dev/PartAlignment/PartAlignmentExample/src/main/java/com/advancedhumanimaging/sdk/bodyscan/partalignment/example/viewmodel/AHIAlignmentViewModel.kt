//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partalignment.example.viewmodel

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.GravityData
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.MotionData

class AHIAlignmentViewModel : ViewModel() {
    // observe motion data
    var motionData by mutableStateOf(MotionData(GravityData(0.0, 0.0, 0.0), 0.0))
}