package com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.view

import androidx.compose.foundation.Image
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.ArrowForward
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
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
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.ui.theme.BodyscanTheme
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.example.viewmodel.AHIBSSegmentationViewModel

@Composable
@OptIn(ExperimentalMaterial3Api::class)
fun SelectJointsView(
    naviHostController: NavHostController,
    viewModel: AHIBSSegmentationViewModel,
) {
    val index = remember { mutableStateOf(0) }
    val currentPoint = remember {
        mutableStateOf(
            Offset(
                viewModel.poseJoints.getValue(viewModel.poseJoints.keys.elementAt(index.value)).x,
                viewModel.poseJoints.getValue(viewModel.poseJoints.keys.elementAt(index.value)).y,
            )
        )
    }
    val humanPainter = remember {
        object : Painter() {
            override val intrinsicSize: Size
                get() = Size(
                    viewModel.humanBitmap.width.toFloat(),
                    viewModel.humanBitmap.height.toFloat()
                )

            override fun DrawScope.onDraw() {
                viewModel.humanBitmap.let {
                    scale(
                        scaleX = 2f,
                        scaleY = 2f,
                        pivot = Offset(0F, 0F)
                    ) {
                        drawImage(it.asImageBitmap())
                    }
                    drawCircle(color = Color.Red, center = currentPoint.value * 2F, radius = 15f)
                }
            }
        }
    }
    BodyscanTheme {
        Scaffold(
            topBar = {
                CenterAlignedTopAppBar(
                    title = {
                        Text(text = viewModel.poseJoints.keys.elementAt(index.value))
                    },
                    navigationIcon = {
                        IconButton(onClick = {
                            if (index.value > 0) index.value--
                            currentPoint.value =
                                Offset(
                                    viewModel.poseJoints.getValue(
                                        viewModel.poseJoints.keys.elementAt(
                                            index.value
                                        )
                                    ).x,
                                    viewModel.poseJoints.getValue(
                                        viewModel.poseJoints.keys.elementAt(
                                            index.value
                                        )
                                    ).y,
                                )
                        }
                        ) {
                            Icon(Icons.Filled.ArrowBack, "")
                        }
                    },
                    actions = {
                        IconButton(onClick = {
                            if (index.value < viewModel.poseJoints.size - 1) index.value++
                            currentPoint.value =
                                Offset(
                                    viewModel.poseJoints.getValue(
                                        viewModel.poseJoints.keys.elementAt(
                                            index.value
                                        )
                                    ).x,
                                    viewModel.poseJoints.getValue(
                                        viewModel.poseJoints.keys.elementAt(
                                            index.value
                                        )
                                    ).y,
                                )
                        }) {
                            Icon(Icons.Filled.ArrowForward, "Localized description")
                        }
                    }
                )
            }, content = { padding ->
                Box(Modifier.fillMaxSize())
                {
                    Image(
                        painter = humanPainter,
                        contentDescription = "Gallery Image",
                        contentScale = ContentScale.FillBounds,
                        modifier = Modifier.fillMaxSize().padding(padding).pointerInput(Unit) {
                            detectTapGestures(
                                onTap = { offset ->
                                    viewModel.poseJoints.getValue(
                                        viewModel.poseJoints.keys.elementAt(index.value)
                                    ).x = offset.x / 2
                                    viewModel.poseJoints.getValue(
                                        viewModel.poseJoints.keys.elementAt(
                                            index.value
                                        )
                                    ).y = offset.y / 2
                                    currentPoint.value = offset / 2F
                                }
                            )
                        }
                    )
                }
            })
    }
}