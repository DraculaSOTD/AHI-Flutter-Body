package com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.utils

import android.graphics.Bitmap
import androidx.compose.ui.unit.IntSize

fun Bitmap.resizeBitmap(screenSize: IntSize): Bitmap {
    if (screenSize.height > 0 && screenSize.width > 0) {
        var dstW = screenSize.width
        var dstH = screenSize.height
        val srcW = this.width
        val srcH = this.height
        val srcRatio = srcW.toFloat() / srcH.toFloat()
        val dstRatio = dstW.toFloat() / dstH.toFloat()
        if (dstRatio > srcRatio) {
            dstW = (dstH * srcRatio).toInt()
        } else {
            dstH = (dstW / srcRatio).toInt()
        }
        return Bitmap.createScaledBitmap(this, dstW, dstH, true)
    } else {
        return this
    }
}