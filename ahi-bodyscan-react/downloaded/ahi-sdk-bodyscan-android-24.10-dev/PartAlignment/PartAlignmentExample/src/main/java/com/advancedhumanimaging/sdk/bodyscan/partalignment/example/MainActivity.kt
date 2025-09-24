//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partalignment.example

import android.annotation.SuppressLint
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Text
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.ConstraintSet
import androidx.lifecycle.ViewModelProvider
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.MotionData
import com.advancedhumanimaging.sdk.bodyscan.partalignment.AHIMotionSensor
import com.advancedhumanimaging.sdk.bodyscan.partalignment.example.view.DisplayLabel
import com.advancedhumanimaging.sdk.bodyscan.partalignment.example.viewmodel.AHIAlignmentViewModel
import kotlinx.coroutines.flow.collectLatest
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity() {
    private lateinit var ahiMotionSensor: AHIMotionSensor
    private lateinit var viewModel: AHIAlignmentViewModel

    @SuppressLint("CoroutineCreationDuringComposition")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewModel = ViewModelProvider(this).get(AHIAlignmentViewModel::class.java)
        ahiMotionSensor = AHIMotionSensor(baseContext)
        setContent {
            if (ahiMotionSensor.isDeviceMotionAvailable()) {
                if (ahiMotionSensor.isDeviceMotionActive()) {
                    val scope = rememberCoroutineScope()
                    val motionStateFlow = ahiMotionSensor.deviceMotion()
                    scope.launch {
                        // collect latest data and put into view model
                        motionStateFlow.collectLatest {
                            it?.let(getData)
                        }
                    }
                    BoxWithConstraints(modifier = Modifier.fillMaxSize()) {
                        ConstraintLayout(
                            constraintSet = constraints(),
                            modifier = Modifier.fillMaxSize()
                        ) {
                            viewModel.motionData.let {
                                Column(modifier = Modifier.layoutId("gravity")) {
                                    DisplayLabel(title = "X", data = it.gravity.x)
                                    DisplayLabel(title = "Y", data = it.gravity.y)
                                    DisplayLabel(title = "Z", data = it.gravity.z)
                                }
                                DisplayLabel(
                                    modifier = Modifier.layoutId("angle"),
                                    title = "Angle",
                                    data = it.angle
                                )
                                if (ahiMotionSensor.isDeviceAligned()) {
                                    Text(
                                        text = "Device is aligned",
                                        modifier = Modifier.layoutId("align"),
                                        style = TextStyle(
                                            Color.Green,
                                            fontWeight = FontWeight.ExtraBold,
                                            fontSize = 30.sp
                                        )
                                    )
                                } else {
                                    Text(
                                        text = "Device is not aligned",
                                        modifier = Modifier.layoutId("align"),
                                        style = TextStyle(
                                            Color.Red,
                                            fontWeight = FontWeight.Bold,
                                            fontSize = 25.sp
                                        )
                                    )
                                }
                            }
                        }
                    }
                } else {
                    Text(text = "GRAVITY SENSOR IS INACTIVE")
                }
            } else {
                Box(modifier = Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
                    Text(text = "THIS DEVICE HASN'T GRAVITY SENSOR")
                }
            }
        }
    }

    // Register Motion Sensor
    override fun onResume() {
        super.onResume()
        ahiMotionSensor.start()
    }

    // Unregister Motion Sensor
    override fun onPause() {
        super.onPause()
        ahiMotionSensor.stop()
    }

    // Callback -> get sensor manager data and put into the view model, for future multi views implementation.
    private val getData: (MotionData) -> Unit = { motionData ->
        viewModel.motionData = motionData
    }

    private fun constraints(): ConstraintSet {
        return ConstraintSet {
            val gravityDataLabel = createRefFor("gravity")
            val angle = createRefFor("angle")
            val alignLabel = createRefFor("align")
            constrain(gravityDataLabel) {
                start.linkTo(parent.start)
                end.linkTo(parent.end)
                bottom.linkTo(angle.top, margin = 30.dp)
            }
            constrain(angle) {
                top.linkTo(parent.top)
                start.linkTo(parent.start)
                end.linkTo(parent.end)
                bottom.linkTo(parent.bottom)
            }
            constrain(alignLabel) {
                top.linkTo(angle.bottom, margin = 30.dp)
                start.linkTo(parent.start)
                end.linkTo(parent.end)
            }
        }
    }
}