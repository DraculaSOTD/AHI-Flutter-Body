//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partinversion.example.viewmodel

import android.net.Uri
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import com.advancedhumanimaging.sdk.bodyscan.common.SexType

class AHIBSInversionViewModel : ViewModel() {
    var fileName by mutableStateOf("test")
    var sex by mutableStateOf(SexType.female)
    var height by mutableStateOf(190f)
    var weight by mutableStateOf(70f)
    var chest by mutableStateOf(95.33f)
    var waist by mutableStateOf(73.39f)
    var hip by mutableStateOf(90.77f)
    var inseam by mutableStateOf(86.05f)
    var fitness by mutableStateOf(5f)
    var meshLocation: Uri? by mutableStateOf(null)
}