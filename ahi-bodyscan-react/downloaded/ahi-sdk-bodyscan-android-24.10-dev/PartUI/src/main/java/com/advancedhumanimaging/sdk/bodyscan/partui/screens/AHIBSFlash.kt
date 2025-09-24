package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import androidx.compose.animation.core.TargetBasedAnimation
import androidx.compose.animation.core.VectorConverter
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.shape.CornerSize
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.ANIMATION_DELAY
import com.advancedhumanimaging.sdk.common.AHITheme

@Composable
fun AHIBSFlash(
    ahiTheme: AHITheme,
    modifier: Modifier
) {
    val anim = remember {
        TargetBasedAnimation(
            animationSpec = tween(ANIMATION_DELAY.toInt()),
            typeConverter = Float.VectorConverter,
            initialValue = 1f,
            targetValue = 25f
        )
    }
    var playTime by remember { mutableStateOf(0L) }
    var scale by remember { mutableStateOf(0F) }
    Box(
        modifier = modifier
            .background(Color.Black)
    ) {
        Canvas(
            modifier = modifier
                .clip(RoundedCornerShape(corner = CornerSize(30.dp)))
                .background(Color(ahiTheme.primaryTintColor))
        ) {
            val viewRadius = size.width * 0.5F
            scale(scale) {
                drawCircle(
                    color = Color.White.copy(0.7F),
                    radius = viewRadius * 0.1F
                )
                drawCircle(
                    color = Color.White.copy(0.3F),
                    radius = viewRadius * 0.31F
                )
                drawCircle(
                    color = Color.White.copy(0.2F),
                    radius = viewRadius * 0.77F
                )
                drawCircle(
                    color = Color.White.copy(0.1F),
                    radius = viewRadius * 1.15F
                )
            }
        }
    }
    LaunchedEffect(anim) {
        val startTime = withFrameNanos { it }
        do {
            playTime = withFrameNanos { it } - startTime
            scale = anim.getValueFromNanos(playTime)
        } while (!anim.isFinishedFromNanos(playTime))
    }
}