//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.view

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
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
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.Landmarks
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.navigate.NavRoute
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.example.viewmodel.AHIBSPoseDetectionViewModel

@Composable
fun AHIBSPoseDetectionResultView(
    navHostController: NavHostController,
    viewModel: AHIBSPoseDetectionViewModel,
) {
    val faces = viewModel.landmark?.get("face")
    val bodies = viewModel.landmark?.get("body")
    BoxWithConstraints {
        ConstraintLayout(
            modifier = Modifier.fillMaxSize(), constraintSet = constraints()
        ) {

            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .layoutId("list"), horizontalAlignment = Alignment.CenterHorizontally
            ) {
                faces?.let {
                    Text(
                        text = "Faces",
                        modifier = Modifier.padding(6.dp),
                        style = TextStyle(Color.Red, fontWeight = FontWeight.Bold, fontSize = 20.sp)
                    )
                    faces.forEachIndexed { index, map ->
                        Text(
                            text = "Face ${index + 1}",
                            modifier = Modifier.padding(6.dp),
                            style = TextStyle(
                                color = Color.Blue,
                                fontWeight = FontWeight.Bold,
                                fontSize = 20.sp
                            )
                        )
                        LazyColumn(
                            modifier = Modifier.wrapContentSize()
                        ) {
                            items(items = map.toList(), itemContent = { item ->
                                Text(text = "${item.first}: ${item.second}")
                            })
                        }
                    }
                }
                bodies?.let {
                    var missingJoints = Landmarks.landmarksBody
                    Spacer(modifier = Modifier.height(10.dp))
                    Text(
                        text = "Bodies",
                        modifier = Modifier.padding(6.dp),
                        style = TextStyle(Color.Red, fontWeight = FontWeight.Bold, fontSize = 20.sp)
                    )
                    Text(
                        text = "Found Body Joints",
                        modifier = Modifier.padding(6.dp),
                        style = TextStyle(
                            color = Color.Blue,
                            fontWeight = FontWeight.Bold,
                            fontSize = 20.sp
                        )
                    )
                    bodies.forEach {
                        missingJoints = Landmarks.landmarksBody.minus(it.keys)
                        LazyColumn(
                            modifier = Modifier.wrapContentSize()
                        ) {
                            items(items = it.toList(), itemContent = { item ->
                                Text(text = "${item.first}: ${item.second}")
                            })
                        }
                    }
                    Text(
                        text = "Missing Body Joints",
                        modifier = Modifier.padding(6.dp),
                        style = TextStyle(
                            color = Color.Blue,
                            fontWeight = FontWeight.Bold,
                            fontSize = 20.sp
                        )
                    )
                    repeat(bodies.size) {
                        LazyColumn(
                            modifier = Modifier.wrapContentSize()
                        ) {
                            items(items = missingJoints.toList(), itemContent = { item ->
                                Text(text = item.first)
                            })
                        }
                    }
                }
            }

            Button(
                onClick = {
                    viewModel.landmark = null
                    navHostController.navigate(NavRoute.HOME_SCREEN)
                },
                modifier = Modifier.layoutId("clear")
            ) {
                Text(text = "Clear")
            }
        }
    }
}

private fun constraints(): ConstraintSet {
    return ConstraintSet {
        val list = createRefFor("list")
        val back = createRefFor("clear")
        val error = createRefFor("error")
        constrain(list) {
            top.linkTo(parent.top)
            end.linkTo(parent.end)
            start.linkTo(parent.start)
            bottom.linkTo(parent.bottom)
        }
        constrain(back) {
            bottom.linkTo(parent.bottom, margin = 30.dp)
            end.linkTo(parent.end, margin = 30.dp)
        }
        constrain(error) {
            top.linkTo(parent.top)
            start.linkTo(parent.start)
            end.linkTo(parent.end)
            bottom.linkTo(parent.bottom)
        }
    }
}