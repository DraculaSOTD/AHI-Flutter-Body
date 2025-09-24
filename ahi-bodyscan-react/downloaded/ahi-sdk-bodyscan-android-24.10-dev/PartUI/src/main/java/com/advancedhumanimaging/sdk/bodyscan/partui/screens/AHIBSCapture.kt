//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import android.graphics.PointF
import androidx.compose.animation.Crossfade
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.tween
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.BiasAlignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.res.vectorResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureHeight
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSImageCaptureWidth
import com.advancedhumanimaging.sdk.bodyscan.common.Resolution
import com.advancedhumanimaging.sdk.bodyscan.partui.R
import com.advancedhumanimaging.sdk.bodyscan.partui.components.AHIBSPrompt
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.ANIMATION_DELAY
import com.advancedhumanimaging.sdk.common.AHITheme

enum class CaptureState {
    FIT_OUTLINE, TURN_LEFT, FEET_ARMS_INSIDE, IN_CONTOUR, NONE, ONE_PERSON_ONLY
}

@Composable
fun AHIBSCapture(
    modifier: Modifier,
    ahiTheme: AHITheme,
    state: CaptureState,
    timer: Int,
    contourPoints: Array<PointF>,
) {
    Box(
        modifier = modifier
    ) {
        when (state) {
            CaptureState.ONE_PERSON_ONLY -> {
                AHIBSBorder(cornerColor = Color(ahiTheme.primaryTintColor), dashColor = Color.White)
                AHIBSPrompt(
                    modifier = Modifier.align(Alignment.Center),
                    texts = stringResource(id = R.string.one_person).split("\n"),
                    backgroundColor = Color(ahiTheme.primaryTintColor),
                    iconShape = CircleShape
                )
            }
            CaptureState.FIT_OUTLINE -> {
                AHIBSPrompt(
                    modifier = Modifier.align(Alignment.Center),
                    texts = stringResource(id = R.string.fit_into_outline).split("\n"),
                    backgroundColor = Color(ahiTheme.primaryTintColor),
                    iconShape = CircleShape
                )
            }
            CaptureState.TURN_LEFT -> {
                AHIBSPrompt(
                    modifier = Modifier.align(Alignment.Center),
                    texts = stringResource(id = R.string.turn_left).split("\n"),
                    backgroundColor = Color(ahiTheme.primaryTintColor),
                    icon = ImageVector.vectorResource(id = R.drawable.ic_back_arrow),
                    iconShape = CircleShape
                )
            }
            CaptureState.FEET_ARMS_INSIDE -> {
                AHIBSContourRenderer(
                    modifier = modifier,
                    points = contourPoints,
                    imageSize = Resolution(AHIBSImageCaptureWidth.toInt(), AHIBSImageCaptureHeight.toInt()),
                    backgroundColor = Color.Black.copy(0.6F),
                    foregroundColor = Color.Transparent,
                    lineSolid = false,
                    lineColor = Color(ahiTheme.primaryTintColor),
                    lineDashColor = Color.White
                )
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .align(Alignment.Center),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Center
                ) {
                    AHIBSPrompt(
                        modifier = Modifier,
                        texts = stringResource(id = R.string.feet_together).split("\n"),
                        backgroundColor = Color(ahiTheme.primaryTintColor),
                        iconShape = CircleShape
                    )
                    Spacer(modifier = Modifier.size(24.dp))
                    AHIBSPrompt(
                        modifier = Modifier,
                        texts = stringResource(id = R.string.arms_by_side).split("\n"),
                        backgroundColor = Color(ahiTheme.primaryTintColor),
                        iconShape = CircleShape
                    )
                }
            }
            else -> {
                AHIBSContourRenderer(
                    modifier = modifier,
                    points = contourPoints,
                    imageSize = Resolution(AHIBSImageCaptureWidth.toInt(), AHIBSImageCaptureHeight.toInt()),
                    backgroundColor = Color.Black.copy(0.6F),
                    foregroundColor = Color.Transparent,
                    lineSolid = timer > 0,
                    lineColor = Color(ahiTheme.primaryTintColor),
                    lineDashColor = Color.White
                )
                if (timer > 0) {
                    Box(
                        modifier = Modifier
                            .offset(21.dp, 21.dp)
                            .size(100.dp)
                            .clip(CircleShape)
                            .background(Color(ahiTheme.primaryTintColor)),
                        contentAlignment = Alignment.Center
                    ) {
                        Crossfade(
                            targetState = timer,
                            animationSpec = tween(durationMillis = ANIMATION_DELAY.toInt(), easing = LinearEasing)
                        ) { timerState ->
                            Text(
                                text = timerState.toString(),
                                color = Color.White,
                                fontSize = with(LocalDensity.current) { 60.dp.toSp() },
                                fontWeight = FontWeight.Black,
                                textAlign = TextAlign.Center,
                            )
                        }
                    }
                }
            }
        }
    }
}