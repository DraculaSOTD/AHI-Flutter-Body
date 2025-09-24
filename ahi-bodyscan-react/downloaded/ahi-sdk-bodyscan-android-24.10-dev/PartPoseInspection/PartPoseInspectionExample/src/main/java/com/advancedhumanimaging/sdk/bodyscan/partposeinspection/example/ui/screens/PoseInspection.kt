//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.ui.screens

import android.graphics.Bitmap
import android.graphics.ImageDecoder
import android.graphics.Paint
import android.os.Build
import android.provider.MediaStore
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.*
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.drawscope.drawIntoCanvas
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewModelScope
import androidx.navigation.NavController
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.InspectionResult
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.example.viewmodel.InspectionViewModel
import kotlinx.coroutines.launch

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun PoseInspection(navController: NavController, inspectionViewModel: InspectionViewModel) {
    var imageSize by remember { mutableStateOf(IntSize.Zero) }
    var borderColor by remember { mutableStateOf(Color.Transparent) }
    val poseInspectionInfo = inspectionViewModel.poseInspection.lastInspectionResultDetails.observeAsState()

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
                if (inspectionViewModel.profile == Profile.front) {
                    inspectionViewModel.humanFront = Bitmap.createScaledBitmap(bitmap, 720, 1280, true)
                } else {
                    inspectionViewModel.humanSide = Bitmap.createScaledBitmap(bitmap, 720, 1280, true)
                }
            }
        }
    )

    val painter = object : Painter() {
        val scale = imageSize.height / AHIBSImageCaptureHeight
        override val intrinsicSize: Size
            get() = Size(scale * AHIBSImageCaptureWidth, scale * AHIBSImageCaptureHeight)

        override fun DrawScope.onDraw() {
            scale(
                scaleX = scale,
                scaleY = scale,
                Offset(0F, 0F)
            ) {
                val bitmap = if (inspectionViewModel.profile == Profile.front) inspectionViewModel.humanFront else inspectionViewModel.humanSide
                drawImage(bitmap.asImageBitmap())
                val poseJoints = inspectionViewModel.poseJoints
                val faceJoints = poseJoints["face"]?.maxByOrNull { it.size } ?: mapOf()
                val bodyJoints = poseJoints["body"]?.maxByOrNull { it.size } ?: mapOf()
                val posePoints = (faceJoints + bodyJoints).filter { it.key != "FaceSize" && it.key != "FacePosition" }
                    .mapKeys { it.key.replace("Centroid", "", true) }
                val posePointsUi = posePoints.map {
                    Offset(
                        it.value.x,
                        it.value.y
                    )
                }.toList()
                drawPoints(
                    posePointsUi, strokeWidth = 16f,
                    pointMode = PointMode.Points,
                    color = Color.Blue
                )
                posePoints.forEach { (key, point) ->
                    drawIntoCanvas {
                        val paint = Paint()
                        paint.textAlign = Paint.Align.CENTER
                        paint.textSize = 24.sp.value
                        paint.color = Color.Magenta.toArgb()
                        it.nativeCanvas.drawText(key, point.x, point.y, paint)
                    }
                }
                val contourPoints = inspectionViewModel.idealContourPoints
                val contourPointsUi = contourPoints?.map {
                    Offset(
                        it.x,
                        it.y
                    )
                }?.toList() ?: listOf()
                drawPoints(
                    contourPointsUi,
                    strokeWidth = 4f,
                    pointMode = PointMode.Points,
                    color = Color.Cyan
                )
                poseInspectionInfo.value?.values?.forEach {
                    val box = it.boundingBox
                    val color =
                        when (it.result) {
                            InspectionResult.trueInContour -> Color.Green
                            InspectionResult.falseNotInContour -> Color.Red
                            else -> Color.Yellow
                        }
                    if (box != null) {
                        drawRect(
                            color = color,
                            topLeft = Offset(box.left, box.top),
                            size = Size(box.width(), box.height()),
                            style = Stroke(width = 5F)
                        )
                    }
                }
                val isInContour = inspectionViewModel.isInContour
                borderColor = if (isInContour) Color.Green else Color.Red

            }
        }
    }
    Scaffold(
        modifier = Modifier
            .fillMaxSize()
            .padding(8.dp),
        topBar = {
            CenterAlignedTopAppBar(
                title = { Text("Pose Inspection") },
                navigationIcon = {
                    IconButton(onClick = {
                        navController.popBackStack()
                        inspectionViewModel.refreshContour = false
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
                            .border(width = 4.dp, color = borderColor),
                        painter = painter,
                        contentDescription = "",
                        alignment = Alignment.Center,
                        contentScale = ContentScale.Fit,
                    )
                }
            }
            Button(
                modifier = Modifier.padding(8.dp),
                onClick = {
                    inspectionViewModel.viewModelScope.launch {
                        inspectionViewModel.inspectPose(inspectionViewModel.profile)
                    }
                }
            )
            { Text("Inspect Pose") }
        }
    }

    LaunchedEffect(inspectionViewModel.profile, inspectionViewModel.humanFront, inspectionViewModel.humanSide) {
        inspectionViewModel.inspectPose(inspectionViewModel.profile)
    }
}