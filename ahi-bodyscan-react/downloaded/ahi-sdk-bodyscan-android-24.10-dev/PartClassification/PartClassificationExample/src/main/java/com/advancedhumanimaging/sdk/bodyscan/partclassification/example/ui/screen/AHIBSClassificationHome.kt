//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partclassification.example.ui.screen

import android.widget.Toast
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.selection.selectableGroup
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Alignment.Companion.Center
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Color.Companion.White
import androidx.compose.ui.graphics.PointMode
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.graphics.painter.Painter
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.ConstraintSet
import androidx.constraintlayout.compose.Dimension
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.navigate.NavRoutes
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.viewmodel.AHIBSClassificationViewModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AHIBSClassificationHome(
    modifier: Modifier = Modifier,
    navHostController: NavHostController,
    viewModel: AHIBSClassificationViewModel,
    useAverage:Boolean = true
) {
    var showDialog by remember { mutableStateOf(false) }
    var frontImageSize by remember { mutableStateOf(IntSize.Zero) }
    var sideImageSize by remember { mutableStateOf(IntSize.Zero) }
    val context = LocalContext.current
    val frontPainter = object : Painter() {
        val scale = frontImageSize.width / AHIBSImageCaptureWidth
        override val intrinsicSize: Size
            get() = Size(scale * AHIBSImageCaptureWidth, scale * AHIBSImageCaptureHeight)

        override fun DrawScope.onDraw() {
            scale(
                scaleX = scale,
                scaleY = scale,
                Offset(0F, 0F)
            ) {
                drawImage(viewModel.frontSilhouette.value.asImageBitmap())
            }
            val posePointsUi = viewModel.frontJoints.value.map {
                Offset(
                    it.value.x,
                    it.value.y
                )
            }.toList()
            scale(
                scaleX = scale,
                scaleY = scale,
                Offset(0F, 0F)
            ) {
                drawPoints(
                    posePointsUi, strokeWidth = 16f,
                    pointMode = PointMode.Points,
                    color = Color.Red
                )
            }
        }
    }
    val sidePainter = object : Painter() {
        val scale = sideImageSize.width / AHIBSImageCaptureWidth
        override val intrinsicSize: Size
            get() = Size(scale * AHIBSImageCaptureWidth, scale * AHIBSImageCaptureHeight)

        override fun DrawScope.onDraw() {
            scale(
                scaleX = scale,
                scaleY = scale,
                Offset(0F, 0F)
            ) {
                drawImage(viewModel.sideSilhouette.value.asImageBitmap())
            }
            val posePointsUi = viewModel.sideJoints.value.filter { joint -> viewModel.filterJoints.find { it == joint.key } != null }.map {
                Offset(
                    it.value.x,
                    it.value.y
                )
            }.toList()
            scale(
                scaleX = scale,
                scaleY = scale,
                Offset(0F, 0F)
            ) {
                drawPoints(
                    posePointsUi, strokeWidth = 16f,
                    pointMode = PointMode.Points,
                    color = Color.Red
                )
            }
        }
    }
    val scope = rememberCoroutineScope()
    Scaffold(
        modifier = modifier.padding(8.dp),
        topBar = {
            CenterAlignedTopAppBar(
                title = { Text("Home") }
            )
        }
    ) { contentPadding ->
        BoxWithConstraints(
            modifier = Modifier
                .padding(contentPadding)
                .fillMaxSize()
        ) {
            ConstraintLayout(modifier = modifier, constraintSet = constraints()) {
                Row(
                    Modifier
                        .layoutId("sex")
                        .padding(4.dp)
                        .fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(text = "Sex:")
                    Row(
                        Modifier
                            .selectableGroup()
                    ) {
                        Row(
                            Modifier
                                .selectable(
                                    selected = viewModel.sex == SexType.male,
                                    onClick = {
                                        viewModel.sex = SexType.male
                                    },
                                    role = Role.RadioButton
                                )
                                .padding(4.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            RadioButton(
                                selected = viewModel.sex == SexType.male,
                                onClick = null
                            )
                            Text(
                                text = "Male",
                                modifier = Modifier.padding(start = 8.dp)
                            )
                        }
                        Row(
                            Modifier
                                .selectable(
                                    selected = viewModel.sex == SexType.female,
                                    onClick = {
                                        viewModel.sex = SexType.female
                                    },
                                    role = Role.RadioButton
                                )
                                .padding(4.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            RadioButton(
                                selected = viewModel.sex == SexType.female,
                                onClick = null
                            )
                            Text(
                                text = "Female",
                                modifier = Modifier.padding(start = 8.dp)
                            )
                        }
                    }
                }
                SliderWithLabel("height", "Height", viewModel.height, "cm", 50f..255f) { value ->
                    viewModel.height = value
                }
                SliderWithLabel("weight", "Weight", viewModel.weight, "kg", 16f..300f) { value ->
                    viewModel.weight = value
                }
                Button(
                    modifier = Modifier
                        .layoutId("classification")
                        .padding(4.dp),
                    onClick = {
                        scope.launch(Dispatchers.Main) {
                            showDialog = true
                            val result = viewModel.classify(context,useAverage)
                            showDialog = false
                            if (result && !viewModel.classificationResult.value.isNullOrEmpty()) {
                                navHostController.navigate(NavRoutes.RESULT_SCREEN)
                            } else {
                                Toast.makeText(context, "Classification failed", Toast.LENGTH_LONG).show()
                            }
                        }
                    }
                ) {
                    Text(text = "Classify")
                }
                Box(
                    modifier = Modifier
                        .layoutId("image_front")
                        .padding(4.dp)
                        .onSizeChanged {
                            frontImageSize = it
                        },
                    contentAlignment = Center
                ) {
                    Card(
                        colors = CardDefaults.cardColors(
                            containerColor = Color.Transparent,
                            contentColor = Color.Transparent
                        ),
                        onClick = {
                            navHostController.navigate("${NavRoutes.PICK_SILHOUETTE_SCREEN}/${Profile.front.name}")
                        }
                    ) {
                        Image(
                            painter = frontPainter,
                            contentDescription = "",
                            alignment = Center,
                            contentScale = ContentScale.Fit,
                        )
                    }
                }
                Box(
                    modifier = Modifier
                        .layoutId("image_side")
                        .padding(4.dp)
                        .onSizeChanged {
                            sideImageSize = it
                        },
                    contentAlignment = Center
                ) {
                    Card(
                        colors = CardDefaults.cardColors(
                            containerColor = Color.Transparent,
                            contentColor = Color.Transparent
                        ),
                        onClick = {
                            navHostController.navigate("${NavRoutes.PICK_SILHOUETTE_SCREEN}/${Profile.side.name}")
                        }
                    ) {
                        Image(
                            painter = sidePainter,
                            contentDescription = "",
                            alignment = Center,
                            contentScale = ContentScale.Fit,
                        )
                    }
                }
            }
        }
    }

    if (showDialog) {
        Dialog(
            onDismissRequest = { showDialog = false },
            DialogProperties(dismissOnBackPress = false, dismissOnClickOutside = false)
        ) {
            Box(
                contentAlignment = Center,
                modifier = Modifier
                    .padding(16.dp)
                    .background(White, shape = RoundedCornerShape(8.dp))
            ) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center
                ) {
                    CircularProgressIndicator(modifier = Modifier.padding(4.dp))
                    Text(modifier = Modifier.padding(4.dp), text = "Classifying, please wait...")
                }
            }
        }
    }
}

@Composable
fun SliderWithLabel(
    layoutId: String,
    name: String,
    value: Float,
    unit: String,
    valueRange: ClosedFloatingPointRange<Float>,
    onUpdateValue: (Float) -> Unit
) {
    Row(
        Modifier
            .layoutId(layoutId)
            .padding(4.dp)
            .fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            modifier = Modifier.weight(0.4F, true),
            text = "$name: ${value.format(2)}$unit"
        )
        Slider(
            modifier = Modifier.weight(0.6F, true),
            value = value,
            onValueChange = { onUpdateValue(it) },
            valueRange = valueRange
        )
    }
}

fun Float.format(digits: Int) = "%.${digits}f".format(this)

private fun constraints() = ConstraintSet {
    val imageFrontRef = createRefFor("image_front")
    val imageSideRef = createRefFor("image_side")
    val pickFrontRef = createRefFor("pick_front")
    val pickSideRef = createRefFor("pick_side")
    val classificationRef = createRefFor("classification")
    val sexRef = createRefFor("sex")
    val weightRef = createRefFor("weight")
    val heightRef = createRefFor("height")
    constrain(sexRef) {
        top.linkTo(parent.top)
        end.linkTo(parent.end)
        start.linkTo(parent.start)
        bottom.linkTo(heightRef.top)
    }
    constrain(heightRef) {
        top.linkTo(sexRef.bottom)
        end.linkTo(parent.end)
        start.linkTo(parent.start)
        bottom.linkTo(weightRef.top)
    }
    constrain(weightRef) {
        top.linkTo(heightRef.bottom)
        end.linkTo(parent.end)
        start.linkTo(parent.start)
        bottom.linkTo(imageSideRef.top)
        bottom.linkTo(imageFrontRef.top)
    }
    constrain(imageSideRef) {
        top.linkTo(weightRef.bottom)
        end.linkTo(parent.end)
        start.linkTo(imageFrontRef.end)
        bottom.linkTo(classificationRef.top)
        width = Dimension.fillToConstraints
        height = Dimension.fillToConstraints
    }
    constrain(imageFrontRef) {
        top.linkTo(weightRef.bottom)
        start.linkTo(parent.start)
        end.linkTo(imageSideRef.start)
        bottom.linkTo(classificationRef.top)
        width = Dimension.fillToConstraints
        height = Dimension.fillToConstraints
    }
    constrain(classificationRef) {
        start.linkTo(parent.start)
        bottom.linkTo(parent.bottom)
        end.linkTo(parent.end)
        width = Dimension.percent(0.6F)
    }
    constrain(pickFrontRef) {
        start.linkTo(parent.start)
        bottom.linkTo(parent.bottom)
    }
    constrain(pickSideRef) {
        end.linkTo(parent.end)
        bottom.linkTo(parent.bottom)
    }
}