//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partsegmentation.jni

import android.graphics.Bitmap
import android.graphics.PointF
import com.advancedhumanimaging.sdk.bodyscan.common.Profile

interface ISegmentationJNI {
    fun segment(
        capture: Bitmap,
        contourMask: Bitmap,
        profile: Profile,
        poseJoints: Map<String, PointF>,
        buffer: ByteArray,
        buffer_size: Int
    ): Bitmap?

    fun segmentAll(
        captures: Array<Bitmap>,
        contourMasks: Array<Bitmap>,
        profiles: Array<Profile>,
        poseJoints: Array<Map<String, PointF>>,
        buffer: ByteArray,
        buffer_size: Int
    ): Array<Bitmap>?
}
