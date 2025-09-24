//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.CornerRadius
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.graphics.drawscope.DrawScope
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.unit.dp
import kotlin.math.abs

private val STROKE_WIDTH = 12.dp

@Composable
fun AHIBSBorder(cornerColor: Color, dashColor: Color) {
    Canvas(modifier = Modifier.fillMaxSize()) {
        val strokeWith = STROKE_WIDTH.toPx()
        val strokeGap = strokeWith / 2
        val dashOn = size.width / 13
        val dashOffHorizontal = (dashOn * 2) / 6
        val cornerLength = dashOn * 3
        val cornerRadius = cornerLength * 0.5F
        val availHeight = (size.height - (cornerLength * 2))
        val dashOnTimes = availHeight / dashOn
        val rem = abs(dashOnTimes.toInt().toFloat() - dashOnTimes) * dashOn
        val dashOffTimes = ((dashOnTimes.toInt() * 2) / 5) - 2
        val fullRem = (dashOffTimes * dashOn) + rem
        val dashOffVertical = fullRem / (dashOnTimes.toInt() - dashOffTimes + 1)
        val dashIntervalsHorizontal = floatArrayOf(dashOn, dashOffHorizontal)
        val dashIntervalsVertical = floatArrayOf(dashOn, dashOffVertical)
        val topRightPath = Path().apply {
            moveTo(size.width - cornerLength, strokeGap)
            lineTo(size.width - strokeGap, strokeGap)
            lineTo(size.width - strokeGap, cornerLength)
        }
        drawRoundRect(
            color = Color.Black,
            topLeft = Offset(strokeGap, strokeGap),
            size = Size(size.width - strokeWith, size.height - strokeWith),
            style = Stroke(width = strokeWith),
            cornerRadius = CornerRadius(cornerRadius * 0.75F)
        )
        drawPath(
            path = topRightPath,
            cornerColor = cornerColor,
            cornerLength * 0.5F
        )
        val bottomRightPath = Path().apply {
            moveTo(size.width - cornerLength, size.height - strokeGap)
            lineTo(size.width - strokeGap, size.height - strokeGap)
            lineTo(size.width - strokeGap, size.height - cornerLength)
        }
        drawPath(
            path = bottomRightPath,
            cornerColor = cornerColor,
            cornerRadius
        )
        val topLeftPath = Path().apply {
            moveTo(cornerLength, strokeGap)
            lineTo(strokeGap, strokeGap)
            lineTo(strokeGap, cornerLength)
        }
        drawPath(
            path = topLeftPath,
            cornerColor = cornerColor,
            cornerRadius
        )
        val bottomLeftPath = Path().apply {
            moveTo(strokeGap, size.height - cornerLength)
            lineTo(strokeGap, size.height - strokeGap)
            lineTo(cornerLength, size.height - strokeGap)
        }
        drawPath(
            path = bottomLeftPath,
            cornerColor = cornerColor,
            cornerRadius
        )
        drawLine(
            start = Offset(cornerLength + dashOffHorizontal, strokeGap),
            end = Offset(size.width - cornerLength - dashOffHorizontal, strokeGap),
            dashIntervals = dashIntervalsHorizontal,
            dashColor = dashColor
        )
        drawLine(
            start = Offset(cornerLength + dashOffHorizontal, size.height - strokeGap),
            end = Offset(size.width - cornerLength - dashOffHorizontal, size.height - strokeGap),
            dashIntervals = dashIntervalsHorizontal,
            dashColor = dashColor
        )
        drawLine(
            start = Offset(strokeGap, cornerLength + dashOffVertical),
            end = Offset(strokeGap, size.height - cornerLength - dashOffVertical),
            dashIntervals = dashIntervalsVertical,
            dashColor = dashColor
        )
        drawLine(
            start = Offset(size.width - strokeGap, cornerLength + dashOffVertical),
            end = Offset(size.width - strokeGap, size.height - cornerLength - dashOffVertical),
            dashIntervals = dashIntervalsVertical,
            dashColor = dashColor
        )
    }
}

private fun DrawScope.drawPath(path: Path, cornerColor: Color, cornerRadius: Float) {
    val cornerStroke = Stroke(
        width = STROKE_WIDTH.toPx(),
        pathEffect = PathEffect.cornerPathEffect(cornerRadius)
    )
    drawPath(
        path = path,
        color = cornerColor,
        style = cornerStroke,
    )
}

private fun DrawScope.drawLine(start: Offset, end: Offset, dashIntervals: FloatArray, dashColor: Color) {
    drawLine(
        color = dashColor,
        start = start,
        end = end,
        strokeWidth = STROKE_WIDTH.toPx(),
        pathEffect = PathEffect.dashPathEffect(dashIntervals, 0f),
    )
}