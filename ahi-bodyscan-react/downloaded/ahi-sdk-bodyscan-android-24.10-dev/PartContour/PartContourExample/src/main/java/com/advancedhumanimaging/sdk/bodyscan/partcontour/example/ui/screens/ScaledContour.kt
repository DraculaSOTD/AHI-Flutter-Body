//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcontour.example.ui.screens

import android.annotation.SuppressLint
import android.graphics.Bitmap
import android.graphics.ImageDecoder
import android.os.Build
import android.provider.MediaStore
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PointMode
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewModelScope
import androidx.navigation.NavController
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.partcontour.example.viewmodel.ContourViewModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

private const val TAG = "ScaledContour"

@SuppressLint("CoroutineCreationDuringComposition")
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ScaledContour(navController: NavController, viewModel: ContourViewModel) {
    var imageSize by remember { mutableStateOf(IntSize.Zero) }
    val coroutineScope = viewModel.viewModelScope

    val humanImagePickerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
        onResult = { imageUri ->
            imageUri?.let {
                val bitmap = if (Build.VERSION.SDK_INT < 28) {
                    MediaStore.Images.Media.getBitmap(navController.context.contentResolver, it).copy(Bitmap.Config.ARGB_8888, true)
                } else {
                    val source = ImageDecoder.createSource(navController.context.contentResolver, it)
                    ImageDecoder.decodeBitmap(source) { decoder, _, _ ->
                        decoder.isMutableRequired = true
                    }
                }
                if (viewModel.profile == Profile.front) {
                    viewModel.humanFront = Bitmap.createScaledBitmap(bitmap, 720, 1280, true)
                } else {
                    viewModel.humanSide = Bitmap.createScaledBitmap(bitmap, 720, 1280, true)
                }
                coroutineScope.launch {
                    viewModel.detectPose()
                    viewModel.generateScaledContour()
                }
            }
        }
    )

    val painter = object : Painter() {
        val scale = imageSize.height / AHIBSImageCaptureHeight
        override val intrinsicSize: Size
            get() = Size(scale * AHIBSImageCaptureWidth, scale * AHIBSImageCaptureHeight)

        override fun DrawScope.onDraw() {
            val bitmap = if (viewModel.profile == Profile.front) viewModel.humanFront else viewModel.humanSide
            scale(
                scaleX = scale,
                scaleY = scale,
                Offset(0F, 0F)
            ) {
                drawImage(bitmap.asImageBitmap())
            }
            val posePoints = if (viewModel.profile == Profile.front) viewModel.poseJointsFront else viewModel.poseJointsSide
            val posePointsUi = posePoints.filter { it.key != "FaceSize" }.map {
                Offset(
                    it.value.x,
                    it.value.y
                )
            }.toList()
            val contourPoints =
                if (viewModel.profile == Profile.front) viewModel.scaledContourPointsFront else viewModel.scaledContourPointsSide
            val contourPointsUi = contourPoints?.map {
                Offset(
                    it.x,
                    it.y
                )
            }?.toList() ?: listOf()
            scale(
                scaleX = scale,
                scaleY = scale,
                Offset(0F, 0F)
            ) {
                drawPoints(
                    contourPointsUi,
                    strokeWidth = 4f,
                    pointMode = PointMode.Points,
                    color = Color.Red
                )
            }
            scale(
                scaleX = scale,
                scaleY = scale,
                Offset(0F, 0F)
            ) {
                drawPoints(
                    posePointsUi, strokeWidth = 16f,
                    pointMode = PointMode.Points,
                    color = Color.Blue
                )
            }
        }
    }

    Scaffold(
        modifier = Modifier
            .fillMaxSize()
            .padding(8.dp),
        topBar = {
            CenterAlignedTopAppBar(
                title = { Text("Scaled Contour") },
                navigationIcon = {
                    IconButton(onClick = { navController.popBackStack() }) {
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
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Box(
                modifier = Modifier
                    .weight(1F, true)
                    .padding(8.dp)
                    .onSizeChanged {
                        imageSize = it
                    },
                contentAlignment = Alignment.Center
            ) {
                Card(
                    colors = CardDefaults.cardColors(
                        containerColor = Color.Transparent,
                        contentColor = Color.Transparent
                    ),
                    onClick = {
                        humanImagePickerLauncher.launch("image/*")
                    }
                ) {
                    Image(
                        modifier = Modifier
                            .background(Color.Black),
                        painter = painter,
                        contentDescription = "",
                        alignment = Alignment.Center,
                        contentScale = ContentScale.Fit,
                    )
                }
            }
            Button(
                modifier = Modifier
                    .fillMaxWidth(0.8F)
                    .padding(8.dp),
                onClick = {
                    viewModel.viewModelScope.launch {
                        viewModel.detectPose()
                        viewModel.generateScaledContour()
                    }
                }
            )
            { Text("Generate Scaled Contour") }
        }
    }

    LaunchedEffect(viewModel.profile, viewModel.humanFront, viewModel.humanSide) {
        viewModel.detectPose()
        viewModel.generateScaledContour()
    }
}