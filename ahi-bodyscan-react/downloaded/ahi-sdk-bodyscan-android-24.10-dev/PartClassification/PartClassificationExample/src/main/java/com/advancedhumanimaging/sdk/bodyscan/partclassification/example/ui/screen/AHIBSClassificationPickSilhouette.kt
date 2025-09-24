//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partclassification.example.ui.screen

import android.annotation.SuppressLint
import android.graphics.Bitmap
import android.graphics.ImageDecoder
import android.graphics.PointF
import android.os.Build
import android.provider.MediaStore
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.layout.*
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.viewmodel.AHIBSClassificationViewModel
import kotlin.math.roundToInt

private const val TAG = "PoseInspection"

@SuppressLint("CoroutineCreationDuringComposition")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AHIBSClassificationPickSilhouette(
    modifier: Modifier = Modifier,
    navHostController: NavHostController,
    viewModel: AHIBSClassificationViewModel,
    profile: Profile
) {
    var imageSize by remember { mutableStateOf(IntSize.Zero) }
    val imageScale = imageSize.height / AHIBSImageCaptureHeight
    val imagePickerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
        onResult = { imageUri ->
            imageUri?.let {
                val bitmap = if (Build.VERSION.SDK_INT < 28) {
                    MediaStore.Images.Media.getBitmap(navHostController.context.contentResolver, it).copy(Bitmap.Config.ARGB_8888, true)
                } else {
                    val source = ImageDecoder.createSource(navHostController.context.contentResolver, it)
                    ImageDecoder.decodeBitmap(source) { decoder, _, _ ->
                        decoder.isMutableRequired = true
                    }
                }
                if (profile == Profile.front) {
                    viewModel.frontSilhouette.value = Bitmap.createScaledBitmap(bitmap, 720, 1280, true)
                } else {
                    viewModel.sideSilhouette.value = Bitmap.createScaledBitmap(bitmap, 720, 1280, true)
                }
            }
        }
    )

    val painter = object : Painter() {
        override val intrinsicSize: Size
            get() = Size(imageScale * AHIBSImageCaptureWidth, imageScale * AHIBSImageCaptureHeight)

        override fun DrawScope.onDraw() {
            val bitmap = if (profile == Profile.front) viewModel.frontSilhouette else viewModel.sideSilhouette
            scale(
                scaleX = imageScale,
                scaleY = imageScale,
                Offset(0F, 0F)
            ) {
                drawImage(bitmap.value.asImageBitmap())
            }
        }
    }

    Scaffold(
        modifier = modifier.padding(8.dp),
        topBar = {
            val title = if (profile == Profile.front) "Front" else "Side"
            CenterAlignedTopAppBar(
                title = { Text("$title Silhouette") },
                navigationIcon = {
                    IconButton(onClick = {
                        navHostController.popBackStack()
                    }) {
                        Icon(
                            imageVector = Icons.Filled.ArrowBack,
                            contentDescription = "Localized description"
                        )
                    }
                }
            )
        }
    ) { contentPadding ->
        Column(
            modifier = Modifier
                .padding(contentPadding)
                .fillMaxSize(),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Box(
                modifier = Modifier
                    .weight(1F, true)
                    .padding(8.dp)
                    .onSizeChanged {
                        imageSize = it
                    },
                contentAlignment = Alignment.TopStart
            ) {
                Card(
                    colors = CardDefaults.cardColors(
                        containerColor = Color.Transparent,
                        contentColor = Color.Transparent
                    )
                ) {
                    Image(
                        painter = painter,
                        contentDescription = "",
                        alignment = Alignment.Center,
                        contentScale = ContentScale.Fit,
                    )
                }
                val joints =
                    if (profile == Profile.front) {
                        viewModel.frontJoints.value
                    } else {
                        viewModel.sideJoints.value.filter { joint -> viewModel.filterJoints.find { it == joint.key } != null }
                    }
                joints.forEach {
                    PosePoints(
                        viewModel = viewModel,
                        imageScale = imageScale,
                        profile = profile,
                        joint = Pair(it.key, it.value)
                    )
                }
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceEvenly
            ) {
                Button(
                    modifier = Modifier
                        .weight(1F, true)
                        .padding(8.dp),
                    onClick = { imagePickerLauncher.launch("image/*") }
                )
                { Text("Pick Image") }
                Button(
                    modifier = Modifier
                        .weight(1F, true)
                        .padding(8.dp),
                    onClick = { navHostController.popBackStack() }
                )
                { Text("Done") }
            }
        }
    }
}

@Composable
fun PosePoints(
    viewModel: AHIBSClassificationViewModel,
    imageScale: Float,
    profile: Profile,
    joint: Pair<String, PointF>
) {
    var offsetX by remember { mutableStateOf(0F) }
    var offsetY by remember { mutableStateOf(0F) }
    var scale by remember { mutableStateOf(0F) }
    scale = imageScale
    offsetX = (joint.second.x * scale) - 30.dp.value
    offsetY = joint.second.y * scale
    Box(
        modifier = Modifier
            .absoluteOffset { IntOffset(offsetX.roundToInt(), offsetY.roundToInt()) }
            .background(Color.Red)
            .size(20.dp)
            .pointerInput(Unit) {
                detectDragGestures { change, dragAmount ->
                    change.consume()
                    offsetX += dragAmount.x
                    offsetY += dragAmount.y
                    if (profile == Profile.front) {
                        val joints = viewModel.frontJoints.value.toMutableMap()
                        val point = PointF((offsetX + 30.dp.value) / scale, offsetY / scale)
                        joints[joint.first] = point
                        viewModel.frontJoints.value = joints
                    } else {
                        val joints = viewModel.sideJoints.value.toMutableMap()
                        val point = PointF((offsetX + 30.dp.value) / scale, offsetY / scale)
                        joints[joint.first] = point
                        viewModel.sideJoints.value = joints
                    }
                }
            }
    )
}