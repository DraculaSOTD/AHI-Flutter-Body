//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcamera

import androidx.camera.core.ImageAnalysis
import androidx.camera.core.Preview
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution

object CameraUseCase {
    private val resolution = Resolution(720, 1280)

    fun getPreviewBuilder() = Preview.Builder()
        .setTargetResolution(resolution)
        .build()

    fun getImageAnalysisBuilder() = ImageAnalysis.Builder()
        .setTargetResolution(resolution)
        .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
        .build()
}