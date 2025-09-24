package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import android.graphics.PointF
import androidx.compose.foundation.Canvas
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.scale
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.*
import androidx.compose.ui.graphics.drawscope.Fill
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.drawscope.clipPath
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution

@Composable
fun AHIBSContourRenderer(
    modifier: Modifier,
    points: Array<PointF>,
    imageSize: Resolution,
    backgroundColor: Color,
    foregroundColor: Color,
    lineSolid: Boolean,
    lineColor: Color,
    lineDashColor: Color
) {
    val path = generateContourPath(points)
    Canvas(
        modifier = modifier
            .scale(scaleX = -1f, scaleY = 1f)
    ) {
        val strokeWidth = 6.dp.toPx()
        val scaleX = size.width / AHIBSImageCaptureWidth
        val scaleY = size.height / AHIBSImageCaptureHeight
        val scale = if (scaleX * AHIBSImageCaptureHeight <= size.height) {
            scaleX
        } else {
            scaleY
        }
        val yOffset = size.height - (scale * imageSize.height)
        val xOffset = size.width - (scale * imageSize.width)
        scale(
            scale = scale,
            Offset(-xOffset, -yOffset)
        ) {
            drawPath(
                path = path,
                color = foregroundColor,
                style = Fill
            )
            clipPath(
                path = path,
                clipOp = ClipOp.Difference
            ) {
                drawRect(
                    color = backgroundColor,
                    topLeft = Offset(0F, (-yOffset * 0.5F) / scale)
                )
                drawPath(
                    path = path,
                    color = lineColor,
                    style = Stroke(
                        width = strokeWidth,
                        cap = StrokeCap.Round,
                    )
                )
                if (!lineSolid) {
                    drawPath(
                        path = path,
                        color = lineDashColor,
                        style = Stroke(
                            width = strokeWidth,
                            cap = StrokeCap.Butt,
                            pathEffect = PathEffect.dashPathEffect(floatArrayOf(5.dp.toPx(), 10.dp.toPx()), 0f)
                        )
                    )
                }
            }
        }
    }
}

private fun generateContourPath(points: Array<PointF>): Path {
    return Path().apply {
        points.forEachIndexed { index, point ->
            if (index == 0) {
                moveTo(point.x, point.y)
            } else {
                lineTo(point.x, point.y)
            }
        }
        close()
    }
}