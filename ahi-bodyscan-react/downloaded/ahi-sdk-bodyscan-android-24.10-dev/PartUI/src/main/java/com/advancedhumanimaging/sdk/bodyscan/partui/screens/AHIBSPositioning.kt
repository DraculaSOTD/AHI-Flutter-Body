//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partui.screens

import androidx.compose.animation.*
import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.tween
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.res.vectorResource
import androidx.compose.ui.unit.dp
import com.advancedhumanimaging.sdk.bodyscan.partui.R
import com.advancedhumanimaging.sdk.bodyscan.partui.components.AHIBSPrompt
import com.advancedhumanimaging.sdk.common.AHITheme

enum class PositioningState {
    FIT_WHOLE_BODY, MOVE_BACK, MOVE_FORWARD, PERFECT_SPOT, ONE_PERSON_ONLY
}

@OptIn(ExperimentalAnimationApi::class)
@Composable
fun AHIBSPositioning(
    modifier: Modifier,
    ahiTheme: AHITheme,
    state: PositioningState,
    isVisible: Boolean
) {
    Box(
        modifier = modifier
    ) {
        AHIBSBorder(cornerColor = Color(ahiTheme.primaryTintColor), dashColor = Color.White)
        AnimatedVisibility(
            visible = isVisible,
            modifier = Modifier
                .fillMaxWidth()
                .align(Alignment.Center),
            enter = fadeIn(animationSpec = tween(300, easing = LinearEasing)),
            exit = fadeOut(
                animationSpec = tween(
                    durationMillis = 300, easing = LinearEasing
                )
            )
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth(),
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.Center
            ) {
                val texts = when (state) {
                    PositioningState.FIT_WHOLE_BODY -> {
                        stringResource(id = R.string.fit_in_frame).split("\n")
                    }
                    PositioningState.MOVE_BACK -> {
                        stringResource(id = R.string.move_back).split("\n")
                    }
                    PositioningState.MOVE_FORWARD -> {
                        stringResource(id = R.string.move_forward).split("\n")
                    }
                    PositioningState.PERFECT_SPOT -> {
                        AHIBSPrompt(
                            modifier = Modifier,
                            texts = stringResource(id = R.string.perfect_spot).split("\n"),
                            backgroundColor = Color(ahiTheme.primaryTintColor),
                            icon = ImageVector.vectorResource(id = R.drawable.ic_baseline_back_hand_24),
                            iconShape = CircleShape,
                            animateIcon = true
                        )
                        Spacer(modifier = Modifier.size(8.dp))
                        stringResource(id = R.string.hold_still).split("\n")
                    }
                    PositioningState.ONE_PERSON_ONLY -> {
                        stringResource(id = R.string.one_person).split("\n")
                    }
                }
                AHIBSPrompt(
                    modifier = Modifier,
                    texts = texts,
                    backgroundColor = Color(ahiTheme.primaryTintColor),
                    iconShape = CircleShape
                )
            }
        }
    }
}