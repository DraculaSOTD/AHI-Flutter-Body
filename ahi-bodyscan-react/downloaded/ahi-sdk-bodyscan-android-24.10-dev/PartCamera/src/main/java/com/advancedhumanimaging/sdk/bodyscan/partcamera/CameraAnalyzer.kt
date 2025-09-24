//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcamera

import android.annotation.SuppressLint
import android.graphics.Bitmap
import android.graphics.Matrix
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import com.advancedhumanimaging.sdk.bodyscan.common.Capture
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.ICameraCapture
import com.advancedhumanimaging.sdk.bodyscan.partcamera.util.yubToBitmap
import kotlinx.coroutines.delay

class CameraAnalyzer : ICameraCapture, ImageAnalysis.Analyzer {
    private lateinit var capture: Capture
    private var rotationDegree = 0
    private lateinit var pictures: ArrayList<Capture>
    private var captureTimes = 1
    override fun setConfig(config: Map<String, Any>): Boolean {
        if (config.isEmpty()) {
            return false
        }
        (config["capture_times"] as? Int)?.let {
            captureTimes = it
        }
        return true
    }

    override suspend fun takeCapture(meta: Map<String, Any>): Array<Capture>? {
        pictures = ArrayList()
        repeat(captureTimes) {
            val matrix = Matrix()
            val (captureBitmap, captureMeta) = capture
            matrix.postRotate((captureMeta.get("rotationDegree") as Int).toFloat())
            matrix.postScale(-1f, 1f, captureBitmap.width / 2f, captureBitmap.height / 2f)
            val rotationBitmap = Bitmap.createBitmap(captureBitmap, 0, 0, captureBitmap.width, captureBitmap.height, matrix, true)
            pictures.add(Capture(rotationBitmap, meta))
            delay(1000L / captureTimes)
        }
        return pictures.toTypedArray()
    }

    @SuppressLint("UnsafeOptInUsageError")
    override fun analyze(imageProxy: ImageProxy) {
        imageProxy.let { imageProxy ->
            val image = imageProxy.image
            if (image != null) {
                rotationDegree = imageProxy.imageInfo.rotationDegrees
                capture = Capture(
                    image.yubToBitmap(), mutableMapOf(
                        "rotationDegree" to rotationDegree
                    )
                )
            }
            imageProxy.close()
        }
    }
}

