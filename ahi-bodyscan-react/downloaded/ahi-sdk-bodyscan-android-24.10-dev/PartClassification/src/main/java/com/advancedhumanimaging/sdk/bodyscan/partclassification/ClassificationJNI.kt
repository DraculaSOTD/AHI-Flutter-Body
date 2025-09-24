package com.advancedhumanimaging.sdk.bodyscan.partclassification

import android.graphics.Bitmap
import android.graphics.PointF
import com.advancedhumanimaging.sdk.bodyscan.common.SexType

internal object ClassificationJNI {
    init {
        System.loadLibrary("classification")
    }

    external fun classify(
        height: Double,
        weight: Double,
        sex: SexType,
        frontSilhouette: Bitmap,
        sideSilhouette: Bitmap,
        frontJoints: Map<String, PointF>,
        sideJoints: Map<String, PointF>,
        tfModels: Map<String, Pair<ByteArray, Int>>,
        svrModels: Map<String, Pair<ByteArray, Int>>,
        useAverage:Boolean
    ): Map<String, Any>?

    external fun classifyMultiple(
        height: Double,
        weight: Double,
        sex: SexType,
        frontSilhouettes: Array<Bitmap>,
        sideSilhouette: Array<Bitmap>,
        frontJoints: Array<Map<String, PointF>>,
        sideJoints: Array<Map<String, PointF>>,
        tfModels: Map<String, Pair<ByteArray, Int>>,
        svrModels: Map<String, Pair<ByteArray, Int>>,
        useAverage:Boolean
    ): Map<String, Any>?

    external fun getTfLiteModelNames(): Array<String>

    external fun getSvrModelNames(): Array<String>

}