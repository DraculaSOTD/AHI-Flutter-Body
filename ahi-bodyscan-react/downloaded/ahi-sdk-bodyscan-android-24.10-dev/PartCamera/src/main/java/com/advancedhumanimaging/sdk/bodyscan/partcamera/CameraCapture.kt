package com.advancedhumanimaging.sdk.bodyscan.partcamera

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Bitmap
import android.graphics.Matrix
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageProxy
import androidx.lifecycle.LifecycleOwner
import com.advancedhumanimaging.sdk.bodyscan.common.Capture
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.ICameraCapture
import com.advancedhumanimaging.sdk.bodyscan.partcamera.util.getCameraProvider
import com.advancedhumanimaging.sdk.bodyscan.partcamera.util.yubToBitmap
import com.advancedhumanimaging.sdk.common.models.AHILogLevel
import com.advancedhumanimaging.sdk.common.models.AHILogging
import kotlinx.coroutines.*
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import java.util.concurrent.Executors

// This is an alternative camera implementation. Looking to replace the original camera implementation (CameraAnalyzer) but trialing it for now
class CameraCapture : ICameraCapture {
    private val takeCaptureMutex = Mutex()
    private val startCameraMutex = Mutex()
    private val imageProxies = mutableListOf<ImageProxy>()
    private var isTakingCapture = false
    private var captureTimes = 1

    val previewUseCase = CameraUseCase.getPreviewBuilder()
    private val analysisUseCase = CameraUseCase.getImageAnalysisBuilder()

    override fun setConfig(config: Map<String, Any>): Boolean {
        if (config.isEmpty()) {
            return false
        }
        (config["capture_times"] as? Int)?.let {
            captureTimes = it
        }
        return true
    }

    @SuppressLint("UnsafeOptInUsageError")
    override suspend fun takeCapture(meta: Map<String, Any>): Array<Capture>? {
        takeCaptureMutex.withLock {
            return withContext(Dispatchers.IO) {
                try {
                    isTakingCapture = true
                    val captures = mutableListOf<Capture>()
                    repeat(captureTimes) {
                        var tries = 10
                        do {
                            val imageProxy = imageProxies.firstOrNull()
                            val image = imageProxy?.image
                            if (image != null) {
                                val matrix = Matrix()
                                val captureBitmap = image.yubToBitmap()
                                matrix.postRotate(imageProxy.imageInfo.rotationDegrees.toFloat())
                                val rotationBitmap = Bitmap.createBitmap(captureBitmap, 0, 0, captureBitmap.width, captureBitmap.height, matrix, true)
                                captures.add(Capture(rotationBitmap, meta))
                                tries = 0
                                imageProxy.close()
                                imageProxies.removeFirst()
                            } else {
                                tries--
                                delay(100)
                            }
                        } while (tries > 0)
                    }
                    isTakingCapture = false
                    imageProxies.forEach {
                        it.close()
                    }
                    imageProxies.clear()
                    captures.toTypedArray()
                } catch (e: Exception) {
                    AHILogging.log(AHILogLevel.ERROR, "Taking captures failed")
                    isTakingCapture = false
                    null
                }
            }
        }
    }

    suspend fun startCamera(
        context: Context,
        lifecycleOwner: LifecycleOwner,
        coroutineScope: CoroutineScope,
        cameraSelector: CameraSelector,
        onAnalyze: suspend (imageProxy: ImageProxy) -> Unit
    ): Boolean {
        startCameraMutex.withLock {
            return try {
                analysisUseCase.setAnalyzer(Executors.newSingleThreadExecutor()) { imageProxy ->
                    if (!isTakingCapture) {
                        coroutineScope.launch {
                            onAnalyze(imageProxy)
                            imageProxy.close()
                        }
                    } else {
                        if (imageProxies.size >= 2) {
                            imageProxies.last().close()
                            imageProxies.removeLast()
                        }
                        imageProxies.add(imageProxy)
                    }
                }
                val cameraProvider = context.getCameraProvider()
                cameraProvider.unbindAll()
                cameraProvider.bindToLifecycle(
                    lifecycleOwner, cameraSelector, previewUseCase, analysisUseCase
                )
                true
            } catch (e: Exception) {
                AHILogging.log(AHILogLevel.ERROR, "Camera failed to start")
                false
            }
        }
    }
}