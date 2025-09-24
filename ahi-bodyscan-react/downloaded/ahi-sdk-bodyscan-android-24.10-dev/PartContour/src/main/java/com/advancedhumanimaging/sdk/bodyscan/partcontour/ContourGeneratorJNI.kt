//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcontour

import android.graphics.Bitmap
import android.graphics.PointF
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution
import com.advancedhumanimaging.sdk.bodyscan.common.SexType

internal object ContourGeneratorJNI {

    // Used to load the 'poseInspection' library on application startup.
    init {
        System.loadLibrary("contourGenerator")
    }

    // ContourGenerator
    external fun generateIdealContour(
        sex: SexType,
        heightCM: Float,
        weightKG: Float,
        imageSize: Resolution,
        alignmentZRadians: Float,
        profile: Profile,
        cvModelsMale: Map<String, Pair<ByteArray, Int>>,
        cvModelsFemale: Map<String, Pair<ByteArray, Int>>
    ): Array<PointF>?

    external fun generateContourMask(contour: Array<PointF>, imageSize: Resolution): Bitmap?
}