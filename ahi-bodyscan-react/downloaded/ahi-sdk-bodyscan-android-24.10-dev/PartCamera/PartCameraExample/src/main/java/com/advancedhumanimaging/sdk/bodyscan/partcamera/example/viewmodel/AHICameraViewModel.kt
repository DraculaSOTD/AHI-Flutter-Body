package com.advancedhumanimaging.sdk.bodyscan.partcamera.example.viewmodel

import android.net.Uri
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel

class AHICameraViewModel : ViewModel() {
    var uri by mutableStateOf<Uri?>(null)
}