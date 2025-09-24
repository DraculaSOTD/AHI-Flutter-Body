//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//
package com.advancedhumanimaging.sdk.bodyscan.partclassification.example.ui.screen

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.layout.layoutId
import androidx.compose.ui.unit.dp
import androidx.constraintlayout.compose.ConstraintLayout
import androidx.constraintlayout.compose.ConstraintSet
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.partclassification.example.viewmodel.AHIBSClassificationViewModel

@Composable
fun AHIBSClassificationResult(
    modifier: Modifier = Modifier,
    navHostController: NavHostController,
    viewModel: AHIBSClassificationViewModel
) {
    viewModel.classificationResult.value?.let {
        DisplayClassificationResult(
            list = it.toList(),
            modifier = modifier,
            navHostController = navHostController
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DisplayClassificationResult(
    list: List<Pair<String, Any>>,
    modifier: Modifier = Modifier,
    navHostController: NavHostController
) {
    fun constraints(): ConstraintSet = ConstraintSet {
        val mList = createRefFor("list")
        val backButton = createRefFor("back_button")
        constrain(mList) {
            top.linkTo(parent.top, margin = 20.dp)
            start.linkTo(parent.start, margin = 20.dp)
            end.linkTo(parent.end, margin = 20.dp)
            bottom.linkTo(parent.bottom, margin = 20.dp)
        }
        constrain(backButton) {
            end.linkTo(parent.end, margin = 30.dp)
            bottom.linkTo(parent.bottom, margin = 30.dp)
        }
    }
    Scaffold(
        modifier = modifier.padding(8.dp),
        topBar = {
            CenterAlignedTopAppBar(
                title = { Text("Results") },
                navigationIcon = {
                    IconButton(onClick = {
                        navHostController.popBackStack()
                    }) {
                        Icon(
                            imageVector = Icons.Filled.ArrowBack,
                            contentDescription = "Localized description"
                        )
                    }
                }
            )
        }
    ) { contentPadding ->
        BoxWithConstraints(
            modifier = Modifier
                .padding(contentPadding)
                .fillMaxSize()
        ) {
            ConstraintLayout(constraintSet = constraints(), modifier = modifier) {
                LazyColumn(modifier = modifier.layoutId("list")) {
                    items(list, itemContent = { item: Pair<String, Any> ->
                        Row(
                            modifier = modifier.padding(15.dp),
                            horizontalArrangement = Arrangement.SpaceBetween
                        ) {
                            Text(
                                text = "${item.first}:"
                            )
                            Text(
                                text = (item.second as? Float)?.let { "%.${2}f".format(it) } ?: ""
                            )
                        }
                    })
                }
            }
        }
    }
}