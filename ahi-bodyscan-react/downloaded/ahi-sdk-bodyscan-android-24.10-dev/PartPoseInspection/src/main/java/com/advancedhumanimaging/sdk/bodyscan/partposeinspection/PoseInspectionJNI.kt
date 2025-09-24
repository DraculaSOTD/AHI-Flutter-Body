//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposeinspection

import android.graphics.Bitmap

internal object PoseInspectionJNI {

    // Used to load the 'poseInspection' library on application startup.
    init {
        System.loadLibrary("poseInspection")
    }

    external fun getExtremePointsFromBinaryImage(
        binaryImage: Bitmap
    ): Map<String, Int>
}