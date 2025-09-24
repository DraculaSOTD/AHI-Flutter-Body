//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.view

import android.graphics.Bitmap
import android.graphics.ImageDecoder
import android.os.Build
import android.provider.MediaStore
import android.util.Log
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.wrapContentSize
import androidx.compose.material.DropdownMenu
import androidx.compose.material.DropdownMenuItem
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PointMode
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.ConstraintSet
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.common.DetectionType
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.BodyScanPoseDetection
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.navigate.NavRoute
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.utils.resizeBitmap
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.viewmodel.AHIBSPoseDetectionViewModel
import kotlinx.coroutines.launch

@Composable
fun AHIBSPoseDetectionView(
    naviHostController: NavHostController,
    viewModel: AHIBSPoseDetectionViewModel,
) {
    val context = LocalContext.current
    val uiPoint = remember { mutableStateOf<List<Offset>?>(null) }
    val bitmap = remember { mutableStateOf<Bitmap?>(null) }
    val humanPainter = remember { mutableStateOf<Painter?>(null) }
    val screenSize = remember { mutableStateOf(IntSize.Zero) }
    val launcher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent(),
        onResult = {
            it?.let {
                uiPoint.value = null
                bitmap.value = null
                if (Build.VERSION.SDK_INT < 28) {
                    bitmap.value = MediaStore.Images.Media.getBitmap(context.contentResolver, it)
                } else {
                    val source = ImageDecoder.createSource(context.contentResolver, it)
                    val bm = ImageDecoder.decodeBitmap(source)
                    bitmap.value = bm
                    bitmap.value = bm.resizeBitmap(screenSize.value)
                }
            }
        }
    )
    val coroutineScope = rememberCoroutineScope()
    val poseDetection = BodyScanPoseDetection()
    fun constraints(): ConstraintSet {
        return ConstraintSet {
            val pickImage = createRefFor("pick")
            val detect = createRefFor("detect")
            val image = createRefFor("image")
            val result = createRefFor("result")
            val dropdown_menu = createRefFor("dropdown_menu")
            constrain(pickImage) {
                end.linkTo(parent.end, margin = 30.dp)
                bottom.linkTo(parent.bottom, margin = 20.dp)
            }
            constrain(detect) {
                start.linkTo(parent.start, margin = 30.dp)
                bottom.linkTo(parent.bottom, margin = 20.dp)
            }
            constrain(image) {
                top.linkTo(parent.top, margin = 30.dp)
                start.linkTo(parent.start, margin = 30.dp)
                end.linkTo(parent.end, margin = 30.dp)
                bottom.linkTo(parent.bottom, margin = 30.dp)
            }
            constrain(result) {
                top.linkTo(parent.top, margin = 20.dp)
                end.linkTo(parent.end, margin = 20.dp)
            }
            constrain(dropdown_menu) {
                top.linkTo(result.bottom, margin = 30.dp)
                end.linkTo(parent.end, margin = 30.dp)
            }
        }
    }
    BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
        ConstraintLayout(
            modifier = Modifier
                .fillMaxSize()
                .onSizeChanged {
                    // Get parent screen size.
                    screenSize.value = it
                },
            constraintSet = constraints()
        ) {
            val items = listOf(
                DetectionType.body,
                DetectionType.face,
                DetectionType.faceAndBody
            )
            bitmap.value?.let {
                Image(
                    modifier = Modifier
                        .background(Color.Red)
                        .layoutId("image")
                        .wrapContentSize(),
                    painter = humanPainter.value!!,
                    contentDescription = "",
                    contentScale = ContentScale.Crop
                )
            }
            Button(
                modifier = Modifier.layoutId("pick"),
                onClick = { launcher.launch("image/*") }) { Text(text = "Pick Image") }
            Button(
                modifier = Modifier.layoutId("detect"),
                onClick = {
                    coroutineScope.launch {
                        bitmap.value?.let {
                            viewModel.landmark =
                                poseDetection.detect(viewModel.selectType, it)
                        }
                        val pp = mutableListOf<Offset>()
                        viewModel.landmark?.let { it ->
                            for (lists in it.values) {
                                for (points in lists) {
                                    val filterPoints =
                                        points.filter { point -> point.key != "FaceSize" && point.key != "FacePosition" }
                                    filterPoints.values.forEach { point ->
                                        pp.add(
                                            Offset(
                                                point.x,
                                                point.y
                                            )
                                        )
                                    }
                                }
                            }
                        }
                        uiPoint.value = pp
                    }
                }
            ) {
                Text(text = "Pose Detect")
            }
            Button(
                modifier = Modifier.layoutId("result"), onClick = {
                    if (!viewModel.landmark.isNullOrEmpty()) {
                        naviHostController.navigate(NavRoute.RESULT_SCREEN)
                    }
                }) {
                Text(text = "View result")
            }
            LaunchedEffect(viewModel.landmark) {
                humanPainter.value = object : Painter() {
                    override val intrinsicSize: Size
                        get() = Size(
                            bitmap.value?.width?.toFloat() ?: 0f,
                            bitmap.value?.height?.toFloat() ?: 0f
                        )

                    override fun DrawScope.onDraw() {
                        bitmap.value?.let {
                            drawImage(it.asImageBitmap())
                            uiPoint.value?.let { list ->
                                drawPoints(
                                    list, strokeWidth = 16f,
                                    pointMode = PointMode.Points,
                                    color = Color.Red
                                )
                            }
                        }

                    }
                }
            }
            var expanded by remember { mutableStateOf(false) }
            Box(modifier = Modifier.layoutId("dropdown_menu")) {
                Button(onClick = { expanded = true }) {
                    Text(text = viewModel.selectType.name)
                }
                DropdownMenu(
                    expanded = expanded,
                    onDismissRequest = { expanded = false }
                ) {
                    items.forEachIndexed { index, type ->
                        DropdownMenuItem(onClick = {
                            viewModel.selectType = type
                            expanded = false
                        }) {
                            Text(text = type.name)
                        }
                    }
                }
            }
        }
    }
}