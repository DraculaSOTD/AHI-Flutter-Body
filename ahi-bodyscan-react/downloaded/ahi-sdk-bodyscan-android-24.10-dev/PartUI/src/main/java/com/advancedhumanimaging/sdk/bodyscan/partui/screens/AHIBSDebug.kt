//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import android.graphics.PointF
import android.graphics.RectF
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.scale
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PointMode
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.drawscope.scale
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.common.*
import com.advancedhumanimaging.sdk.bodyscan.common.utils.getProminentPersonJoints
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.AHIBSBlue

@Composable
fun AHIBSDebug(
    contourPoints: Array<PointF>,
    poseJoints: Map<String, List<Map<String, PointF>>>,
    optimalZones: Map<String, RectF>,
    isUserInOptimalZoneHead: Boolean,
    isUserInOptimalZoneAnkles: Boolean,
    inspectionInfo: Map<String, PoseInspectionInfo>,
    criticalErrors: List<String>
) {
    Canvas(
        modifier = Modifier
            .fillMaxSize()
            .scale(scaleX = -1f, scaleY = 1f)
    ) {
        val scaleX = size.width / AHIBSImageCaptureWidth
        val scaleY = size.height / AHIBSImageCaptureHeight
        val scale = if (scaleX * AHIBSImageCaptureHeight <= size.height) {
            scaleX
        } else {
            scaleY
        }
        val yOffset = size.height - (scale * AHIBSImageCaptureHeight)
        val xOffset = size.width - (scale * AHIBSImageCaptureWidth)
        val posePoints = getProminentPersonJoints(poseJoints).filter { it.key != "FaceSize" && it.key != "FacePosition" }
            .mapKeys { it.key.replace("Centroid", "", true) }
        val posePointsUi = posePoints.map {
            Offset(
                it.value.x,
                it.value.y
            )
        }.toList()
        val contourPointsUi = contourPoints.map {
            Offset(
                it.x,
                it.y
            )
        }.toList()
        val zoneHead = optimalZones[AHIBSOptimalContourZone.HEAD.key]
        val zoneAnkle = optimalZones[AHIBSOptimalContourZone.ANKLES.key]
        scale(
            scaleX = scale,
            scaleY = scale,
            Offset(-xOffset, -yOffset)
        ) {
            try {
                drawPoints(
                    posePointsUi,
                    strokeWidth = 16f,
                    pointMode = PointMode.Points,
                    color = Color.Cyan,
                    cap = StrokeCap.Round
                )
                drawPoints(
                    contourPointsUi,
                    strokeWidth = 4f,
                    pointMode = PointMode.Points,
                    color = AHIBSBlue
                )
                zoneHead?.let {
                    val topLeft = Offset(it.left, it.top)
                    val size = Size(it.width(), it.height())
                    drawRect(if (isUserInOptimalZoneHead) Color.Green else Color.Red, topLeft, size, 0.25F)
                }
                zoneAnkle?.let {
                    val topLeft = Offset(it.left, it.top)
                    val size = Size(it.width(), it.height())
                    drawRect(if (isUserInOptimalZoneAnkles) Color.Green else Color.Red, topLeft, size, 0.25F)
                }
                inspectionInfo.values.forEach {
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
            } catch (e: Exception) {
                e.printStackTrace()
            }
        }
    }

    Box(
        modifier = Modifier.fillMaxSize()
    ) {
        Column(
            modifier = Modifier
                .align(Alignment.TopEnd)
                .padding(8.dp)
        ) {
            criticalErrors.forEachIndexed { index, text ->
                Text(
                    modifier = Modifier
                        .background(Color.Black.copy(alpha = 0.5F))
                        .padding(4.dp),
                    text = text,
                    color = Color.White,
                    style = TextStyle(
                        fontWeight = FontWeight.ExtraBold,
                        textAlign = TextAlign.Center,
                    ),
                )
                if (index != criticalErrors.lastIndex) Spacer(modifier = Modifier.size(4.dp))
            }
        }
    }
}