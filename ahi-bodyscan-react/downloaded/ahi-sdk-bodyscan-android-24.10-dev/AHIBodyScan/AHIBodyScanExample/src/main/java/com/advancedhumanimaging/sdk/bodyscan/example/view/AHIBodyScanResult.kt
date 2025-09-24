//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example.view

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.example.MainViewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun AHIBodyScanResult(
    navHostController: NavHostController,
    mainViewModel: MainViewModel,
    modifier: Modifier = Modifier
) {
    Scaffold(
        modifier = modifier,
        topBar = {
            CenterAlignedTopAppBar(
                title = { Text("BodyScan Results") },
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
        val results = mainViewModel.getMainScreenState().observeAsState().value?.classificationResults
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(contentPadding),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            if (!results.isNullOrEmpty()) {
                Button(
                    modifier = Modifier.padding(8.dp),
                    onClick = {
                        mainViewModel.generateMesh(navHostController)
                    }) {
                    Text(text = "Generate Mesh")
                }
                Spacer(modifier = Modifier.height(16.dp))
                LazyColumn(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(8.dp),
                    horizontalAlignment = Alignment.Start,
                ) {
                    results.toList().sortedBy { it.first.substringAfterLast("_") }.toMap().forEach { (key, value) ->
                        item {
                            Spacer(modifier = Modifier.height(8.dp))
                            Card {
                                Column(
                                    modifier = Modifier
                                        .padding(8.dp),
                                    horizontalAlignment = Alignment.Start
                                ) {
                                    Text(text = "$key:", fontStyle = FontStyle.Italic)
                                    Text(text = value.toString())
                                }
                            }
                        }
                    }
                }
            } else {
                Text(
                    modifier = Modifier.padding(8.dp),
                    text = mainViewModel.getMainScreenState().value?.resultMessage ?: ""
                )
            }
        }
    }
}