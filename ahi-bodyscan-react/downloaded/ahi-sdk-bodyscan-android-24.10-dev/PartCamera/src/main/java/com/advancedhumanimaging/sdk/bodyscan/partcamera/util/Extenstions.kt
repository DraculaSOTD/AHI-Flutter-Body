//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcamera.util

import android.content.Context
import android.graphics.*
import android.media.Image
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.core.content.ContextCompat
import java.io.ByteArrayOutputStream
import java.util.concurrent.Executor
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine

fun Image.yubToBitmap(): Bitmap {
    val yBuffer = this.planes[0].buffer
    val vuBuffer = this.planes[2].buffer
    val ySize: Int = yBuffer.capacity()
    val vuSize: Int = vuBuffer.capacity()
    val nv21 = ByteArray(ySize + vuSize)
    yBuffer.get(nv21, 0, ySize)
    vuBuffer.get(nv21, ySize, vuSize)
    val baOutputStream = ByteArrayOutputStream()
    val yuvImage = YuvImage(nv21, ImageFormat.NV21, this.width, this.height, null)
    yuvImage.compressToJpeg(Rect(0, 0, this.width, this.height), 100, baOutputStream)
    val byteForBitmap = baOutputStream.toByteArray()
    val bitmap = BitmapFactory.decodeByteArray(byteForBitmap, 0, byteForBitmap.size)
    return bitmap
}

suspend fun Context.getCameraProvider(): ProcessCameraProvider = suspendCoroutine { continuation ->
    ProcessCameraProvider.getInstance(this).also { future ->
        future.addListener({
            continuation.resume(future.get())
        }, executor)
    }
}

val Context.executor: Executor
    get() = ContextCompat.getMainExecutor(this)