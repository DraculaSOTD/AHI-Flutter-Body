//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import androidx.appcompat.app.AppCompatActivity
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.core.snap
import androidx.compose.animation.core.tween
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.LocalTextStyle
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.geometry.center
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.partui.R
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.AHIBSGray
import com.advancedhumanimaging.sdk.common.AHITheme
import kotlin.math.abs

const val COUNTDOWN_DURATION = 3000

@Composable
fun AHIBSAlignment(
    activity: AppCompatActivity,
    ahiTheme: AHITheme,
    angle: Double,
    isAligned: Boolean,
    alignmentCompleteProgress: Float,
    isTextVisible: Boolean,
    infoBottomSheet: AHIBSAlignmentInfoBottomSheet
) {
    var viewSize by remember { mutableStateOf(IntSize.Zero) }
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(color = Color.Black.copy(alpha = 0.9F))
            .onSizeChanged {
                viewSize = it
            }
    ) {
        val textSize = viewSize.width / 11F
        AHIBSBorder(cornerColor = Color.White, dashColor = AHIBSGray)
        AnimatedVisibility(
            modifier = Modifier
                .padding(top = 50.dp)
                .align(Alignment.TopCenter),
            visible = isTextVisible,
            enter = fadeIn(snap()),
            exit = fadeOut(tween(300))
        ) {
            val headingTexts = stringResource(id = R.string.alignment_heading).split("\n")
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.Center
            ) {
                headingTexts.forEach { text ->
                    Text(
                        text = text,
                        color = Color.White,
                        style = LocalTextStyle.current.copy(
                            fontWeight = FontWeight.ExtraBold,
                            fontSize = with(LocalDensity.current) { textSize.toSp() },
                            textAlign = TextAlign.Center,
                        )
                    )
                }
            }
        }
        //This is calculated and scaled from Figma designs.
        val circleRadius = viewSize.width * (170f / 375f) / 2
        val strokeWidth = 6.dp
        if (alignmentCompleteProgress == 0F) {
            Canvas(modifier = Modifier.fillMaxSize()) {
                drawCircle(
                    color = Color.White,
                    style = Stroke(
                        width = strokeWidth.toPx(),
                        pathEffect = PathEffect.dashPathEffect(floatArrayOf(60.dp.value, 20.dp.value), 0f)
                    ),
                    radius = circleRadius,
                    center = Offset(size.center.x, size.center.y)
                )
                drawCircle(
                    color = Color.Black.copy(alpha = 0.7F),
                    radius = circleRadius,
                    center = getCirclePosition(circleRadius.toDouble(), angle, size, isAligned)
                )
                drawCircle(
                    color = Color(ahiTheme.primaryTintColor),
                    radius = circleRadius,
                    style = Stroke(
                        width = strokeWidth.toPx()
                    ),
                    center = getCirclePosition(circleRadius.toDouble(), angle, size, isAligned)
                )
            }
        } else {
            CircularProgressIndicator(
                color = Color(ahiTheme.primaryTintColor),
                modifier = Modifier
                    .size(with(LocalDensity.current) { (circleRadius * 2).toDp() } + strokeWidth)
                    .align(Alignment.Center),
                progress = alignmentCompleteProgress,
                strokeWidth = strokeWidth
            )
        }

        LaunchedEffect(true) {
            if (!infoBottomSheet.isVisible) {
                infoBottomSheet.show(activity.supportFragmentManager, AHIBSAlignmentInfoBottomSheet.TAG)
            }
        }
    }
}

private fun getCirclePosition(circleRadius: Double, angle: Double, size: Size, isAligned: Boolean): Offset {
    val mAngle =
        if (abs(angle) > 90.0) {
            90.0
        } else {
            abs(angle)
        }
    return if (isAligned) {
        Offset(size.center.x, size.center.y)
    } else {
        val halfHeight = (size.height / 2)
        var y = ((mAngle - 1.5) / 90) * halfHeight
        y =
            if (angle < 0) {
                halfHeight - y
            } else {
                halfHeight + y
            }
        if (y >= size.height - circleRadius) {
            y = size.height - circleRadius
        } else if (y <= circleRadius) {
            y = circleRadius
        }
        Offset(size.center.x, y.toFloat())
    }
}