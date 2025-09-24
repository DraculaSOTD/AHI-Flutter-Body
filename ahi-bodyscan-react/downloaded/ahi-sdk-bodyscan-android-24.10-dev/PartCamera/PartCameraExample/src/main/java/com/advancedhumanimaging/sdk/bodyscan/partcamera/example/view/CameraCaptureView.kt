//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcamera.example.view

import android.graphics.Bitmap
import android.media.MediaScannerConnection
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.camera.core.CameraSelector
import androidx.camera.core.UseCase
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material.DropdownMenu
import androidx.compose.material.DropdownMenuItem
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalLifecycleOwner
import androidx.compose.ui.unit.dp
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.ConstraintSet
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.partcamera.CameraAnalyzer
import com.advancedhumanimaging.sdk.bodyscan.partcamera.CameraUseCase
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.navigate.NavRoute
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.utils.FileUtilsImpl
import com.advancedhumanimaging.sdk.bodyscan.partcamera.example.viewmodel.AHICameraViewModel
import com.advancedhumanimaging.sdk.bodyscan.partcamera.util.getCameraProvider
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.io.FileOutputStream
import java.util.concurrent.Executors

@Composable
fun CameraCaptureView(
    modifier: Modifier = Modifier.fillMaxSize(),
    navHostController: NavHostController,
    viewModel: AHICameraViewModel,
    cameraSelector: CameraSelector = CameraSelector.DEFAULT_FRONT_CAMERA,
) {
    val launcher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
        onResult = {
            viewModel.uri = it
            if (viewModel.uri != null) {
                navHostController.navigate(NavRoute.REVIEW_SCREEN)
            }
        }
    )
    val context = LocalContext.current
    val lifecycleOwner = LocalLifecycleOwner.current
    val coroutineScope = rememberCoroutineScope()
    val analyzer = remember {
        CameraAnalyzer()
    }
    var previewUseCase by remember {
        mutableStateOf<UseCase>(
            CameraUseCase.getPreviewBuilder()
        )
    }
    val imageAnalysis by remember {
        mutableStateOf(
            CameraUseCase.getImageAnalysisBuilder()
        )
    }
    BoxWithConstraints {
        ConstraintLayout(
            modifier = modifier, constraintSet = constraints()
        ) {
            var expanded by remember { mutableStateOf(false) }
            val items = listOf<Int>(1, 2, 3, 4)
            var captureTimes by remember { mutableStateOf(1) }
            val textValue = remember { mutableStateOf("$captureTimes") }
            CameraPreview(
                modifier = Modifier
                    .fillMaxSize()
                    .layoutId("preview_view"),
                onUseCase = {
                    previewUseCase = it
                }
            )
            Button(
                modifier = Modifier.layoutId("capture_button"),
                onClick = {
                    coroutineScope.launch(Dispatchers.IO) {
                        val captureSetting = analyzer.setConfig(mapOf("capture_times" to captureTimes))
                        if (captureSetting) {
                            // Get captures.
                            val pictures = analyzer.takeCapture(mapOf())
                            // Save captures for QA debug propose.
                            val file = FileUtilsImpl
                            file.createDirectoryIfNotExist(context)
                            pictures?.forEach {
                                try {
                                    // Create file object
                                    val fileObject = file.createFile(context)
                                    val out = FileOutputStream(fileObject)
                                    val bitmap = it.image
                                    bitmap.compress(Bitmap.CompressFormat.PNG, 100, out)
                                    bitmap.recycle()
                                    MediaScannerConnection.scanFile(context, arrayOf(fileObject.toString()), null, null)
                                } catch (e: Exception) {
                                    e.printStackTrace()
                                }
                            }
                        }
                    }
                }
            ) {
                Text(text = "Capture")
            }
            Button(
                modifier = Modifier.layoutId("gallery_button"),
                onClick = {
                    launcher.launch("image/*")
                }
            ) {
                Text(text = "Gallery")
            }
            LaunchedEffect(previewUseCase) {
                val cameraProvider = context.getCameraProvider()
                try {
                    cameraProvider.unbindAll()
                    imageAnalysis.setAnalyzer(Executors.newSingleThreadExecutor(), analyzer)
                    cameraProvider.bindToLifecycle(
                        lifecycleOwner,
                        cameraSelector,
                        previewUseCase,
                        imageAnalysis
                    )
                } catch (e: Exception) {
                    e.printStackTrace()
                }
            }
            Box(modifier = Modifier.layoutId("dropdown_menu")) {
                Button(onClick = { expanded = true }) {
                    Text(text = textValue.value)
                }
                DropdownMenu(
                    expanded = expanded,
                    onDismissRequest = {
                        expanded = false
                    },
                ) {
                    items.forEachIndexed { index, i ->
                        DropdownMenuItem(
                            onClick = {
                                captureTimes = index + 1
                                expanded = false
                                textValue.value = "$i"
                            }
                        ) {
                            Text(text = "$i")
                        }
                    }
                }
            }
        }
    }
}

private fun constraints(): ConstraintSet {
    return ConstraintSet {
        val preview_view = createRefFor("preview_view")
        val capture_button = createRefFor("capture_button")
        val gallery_button = createRefFor("gallery_button")
        val dropdown_menu = createRefFor("dropdown_menu")
        constrain(preview_view) {
            top.linkTo(parent.top)
            start.linkTo(parent.start)
            end.linkTo(parent.end)
            bottom.linkTo(parent.bottom)
        }
        constrain(capture_button) {
            start.linkTo(parent.start)
            end.linkTo(parent.end)
            bottom.linkTo(parent.bottom, margin = 30.dp)
        }
        constrain(gallery_button) {
            end.linkTo(parent.end, margin = 30.dp)
            top.linkTo(parent.top, margin = 30.dp)
        }
        constrain(dropdown_menu) {
            end.linkTo(parent.end, margin = 30.dp)
            top.linkTo(gallery_button.bottom, margin = 30.dp)
        }
    }
}