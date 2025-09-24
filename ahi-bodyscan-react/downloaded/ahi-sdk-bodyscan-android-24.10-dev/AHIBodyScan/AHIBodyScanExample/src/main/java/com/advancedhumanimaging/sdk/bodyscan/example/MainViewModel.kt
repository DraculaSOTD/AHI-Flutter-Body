//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example

import android.app.Application
import android.net.Uri
import androidx.activity.result.ActivityResultRegistry
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.viewModelScope
import androidx.navigation.NavHostController
import com.advancedhumanimaging.sdk.bodyscan.BodyScan
import com.advancedhumanimaging.sdk.bodyscan.example.navigate.NavRoutes
import com.advancedhumanimaging.sdk.bodyscan.example.view.SmoothingOption
import com.advancedhumanimaging.sdk.common.AHIThemes
import com.advancedhumanimaging.sdk.common.IAHIPersistence
import com.advancedhumanimaging.sdk.common.IAHIScan
import com.advancedhumanimaging.sdk.common.models.AHIResult
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScan
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScanConstants
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import kotlin.math.abs

enum class SdkOperationStatus {
    SUCCESS,
    IN_PROGRESS,
    NOT_DONE
}

data class MainScreenState(
    var sdkSetupState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var sdkAuthZState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var sdkReleaseState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var sdkResourceDownloadState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var meshGenerationState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var scanState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var resultMessage: String = "",
    var classificationResults: Map<String, Any> = mutableMapOf(),
    var meshLocation: Uri? = null
)

class MainViewModel(application: Application) : AndroidViewModel(application) {

    private val mainScreenState = MutableLiveData(MainScreenState())
    private val currentScreenState get() = mainScreenState.value?.copy() ?: MainScreenState()

    var isDebug by mutableStateOf(false)
    var lastClassificationResults: Map<String, Any> = mapOf()
    var smoothingOption by mutableStateOf(SmoothingOption.NONE)
    var styleApplied by mutableStateOf(AHIThemes.style.ordinal)

    private val bodyScanPlugin = BodyScan()

    private val persistence = object : IAHIPersistence {
        override fun request(scanType: String, options: Map<String, Any>, completionBlock: (result: AHIResult<Array<Map<String, Any>>>) -> Unit) {
            val data: Array<Map<String, Any>> = when (smoothingOption) {
                SmoothingOption.NONE -> arrayOf()
                else -> {
                    Array(10) { index ->
                        val mIndex = if (smoothingOption == SmoothingOption.NORMAL) {
                            index + 1
                        } else {
                            index * 10
                        }
                        lastClassificationResults.mapValues { (key, value) ->
                            if (key == "date") {
                                val mValue = value.toString().toLongOrNull()
                                if (mValue != null) {
                                    mValue - (index * 10)
                                } else {
                                    value
                                }
                            } else {
                                val mValue = if (key.contains("_raw_")) {
                                    value.toString().toDoubleOrNull()
                                } else {
                                    null
                                }
                                if (mValue != null) {
                                    if (index < 5) {
                                        abs(mValue - mIndex)
                                    } else {
                                        mValue + mIndex
                                    }
                                } else {
                                    value
                                }
                            }
                        }
                    }
                }
            }
            completionBlock(AHIResult.success(data))
        }

    }

    fun getMainScreenState(): LiveData<MainScreenState> {
        return mainScreenState
    }

    fun triggerSdkInit(useCustomDelegate: Boolean) {
        val exampleConfig = mapOf(
            AHIMultiScanConstants.AHISetupTokenKey to "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJicTNKYXN5M0hMVmRxMkRja05MR1FXdGk2WEZIM2dVUElndUVNOXIyUTdXR05WRGVHTkV5ajR0T09XYXZxa3Y2VHEvYXJEVmtJNUZFSzZ4STFJUmcwRlVMK0t1MVdZV1UrOUtqSGsvS3pHVFk4bllES29WU2J6WEw1eVljOGprMndkMlNmeFBhYXJmT3pPZS9odFJvdUJwa2RSM09BS1V3TGtITTJYT3Fyays4d0MyenE2Y01pczFNZThTVWtGZ2RDS1ZsNVFLbkU0b2xGYzBNRTBVOEVqTW5mUUZSd1ZSZ2VVWlVOdkVWL3R2K1B0aEpJZGJKUnpxa0JUaE5YNDZBaXJaT2ZPV0tsczVoalZXMzNDS2xSczgxd3VwRWYrU0FQY3MxRmUwV2Urd1diN3VKSTdqTXRJeE1pcEdWdUQxMXNEODVKaW1GNm51bnBYRlY3NWc5TmVBR0JpdzZETDVnbXRPenRVNFVTaStKbWhvc1lIc3ZLMWhZay9YRTVKREFEeHV3eWJVaVlxVFJwVWdUeWRYUU02V1BpVUMvaVRmWEJIc1BKK3liRlBTcVpSVWhWa21EYUZ4TjJvNXluL1ZzRkNQU3IvSEs3WmcweEhZUGh4UHVpby8ydkRaZjRNZk1RU0tZaVhDbUxUNks0dmQwYy94OUg1ZngrYzdhMWlTRmR2dTNjWnV0aG1jdG5aYjVLU2prblVkcFlWamJaaytWb1B6eWczUVVpRXhqWDNuaVRZR0NaS3M5RkYyVFNUTW9PenZmVnVYcHpoVGhYMm5IeEpGdzNwN2phcGNvN21tZVhpYzF2VWpVVDhhQ09Xc3hqK1o1MUVRRHBielNyTjllSVV2N3pRZmtsNDhLQXpUUng3M0dkMWZuUmVMM0xzTWExcDBEc2hDakRlcz0iLCJleHAiOjE4MTQ3MDc1MzcsInZlciI6M30.EJTaMwDwzMtgIuY1DtwnCDyCnp9_2NauN7n91O7UIco8YpBbSxXWQWbbcvT-72pbpAQagAQ7hgKkjWEce5kKniDh3KhVa546kdQeneuWd1vu7iV1s3nZsMnPCxmFcv5uL30Cv8HWIkDEZb5UozZTgOcHDFAKyKQVT7Tmw0XA5GOjZqKZRtYtRXjzkQw7nidI_I0YkAshC6H8eakeyrVp-u-SilU95_ppIgZOtZHk1nSo04HATGXxl0inTDa57bi_UTU_OHLLlzKm-4GYtSKKdxyHcAipZklkQic4I_g4K43FeLNlWEyGpHt0dnabJm4auU033RZa5L87qCWFDSkTlBDTb3xTfSVtPtsyXTpHDMXS1-1WzQm97P8QcZ2zJl4z1wDk1WClwYRvk4E2JiA9ZwTef0OG2nQVlRJw0O4IpUWI8NLfg-2YcZR9-DZs2iC2QAIV0K1MVc9Ro2LMkgIm0nycgvYagmn-PZLuq9pATjVlW3oF1pJahEJcltoBc-JE0b_NjMsOFMeH3FQNf7_fCBYbRZA3SkOX87uPOqn2BXkYP77sHJ91mFw89TDilx4iVJSbErUej7tr8ZdagCVG1x0ltJ8qVAFCK5wnHZxp5FdfVBuaZNA2e7VzCFB7cYEFUfsuHd9NK2TzLcxizw7L2PxYHP2RdNCKMR3ebR572-w"
        )

        if (useCustomDelegate) {
            bodyScanPlugin.activityToDisplay = {
                CustomBodyScanActivity()
            }
        }
        val exampleScans = arrayOf<IAHIScan>(bodyScanPlugin)
        val pendingState = mainScreenState.value?.copy() ?: MainScreenState()
        pendingState.sdkSetupState = SdkOperationStatus.IN_PROGRESS
        mainScreenState.value = pendingState

        AHIMultiScan.setup(this.getApplication(), exampleConfig, exampleScans) {
            val newState = mainScreenState.value?.copy() ?: MainScreenState()
            newState.sdkSetupState =
                if (it.isSuccess) {
                    SdkOperationStatus.SUCCESS
                } else {
                    SdkOperationStatus.NOT_DONE
                }
            mainScreenState.value = newState
        }
    }

    fun triggerUserAuthZ() {
        mainScreenState.value = currentScreenState.copy(sdkAuthZState = SdkOperationStatus.IN_PROGRESS)
        AHIMultiScan.userAuthorize("testId", "testSalt", arrayOf("testclaim")) {
            val newState = currentScreenState
            newState.sdkAuthZState =
                if (it.isSuccess) SdkOperationStatus.SUCCESS else SdkOperationStatus.NOT_DONE
            checkDownloadedResources(newState)
        }
    }

    fun triggerSdkRelease() {
        val pendingState = mainScreenState.value?.copy() ?: MainScreenState()
        pendingState.sdkReleaseState = SdkOperationStatus.IN_PROGRESS
        mainScreenState.value = pendingState
        AHIMultiScan.releaseSdk {
            mainScreenState.value = if (it.isSuccess) MainScreenState() else mainScreenState.value
        }
    }

    private fun checkDownloadedResources(screenState: MainScreenState) {
        AHIMultiScan.areResourcesDownloaded {
            mainScreenState.value =
                screenState.copy(sdkResourceDownloadState = if (it.getOrNull() == true) SdkOperationStatus.SUCCESS else SdkOperationStatus.NOT_DONE)
        }
    }

    fun updateDownloadResourcesState(status: AHIResult<Unit>) {
        if (status.isFailure) {
            mainScreenState.value =
                currentScreenState.copy(resultMessage = "Failed to download resources: ${status.error().toString()}")
        } else {
            AHIMultiScan.totalEstimatedDownloadSizeInBytes {
                it.fold({ downloadState ->
                    val mB = 1024.0 * 1024.0
                    val progress = downloadState.progressBytes / mB
                    val total = downloadState.totalBytes / mB
                    mainScreenState.value =
                        currentScreenState.copy(resultMessage = "Download state: ${progress.format(2)}MB / ${total.format(2)}MB")
                    if (progress == total) {
                        mainScreenState.value =
                            currentScreenState.copy(resultMessage = "", sdkResourceDownloadState = SdkOperationStatus.SUCCESS)
                    }
                }, {})
            }
        }
    }

    fun downloadResources() {
        mainScreenState.value = currentScreenState.copy(sdkResourceDownloadState = SdkOperationStatus.IN_PROGRESS)
        val result = AHIMultiScan.downloadResourcesInForeground(1)
        mainScreenState.value =
            currentScreenState.copy(sdkResourceDownloadState = if (result.isSuccess) SdkOperationStatus.IN_PROGRESS else SdkOperationStatus.NOT_DONE)
    }

    fun initiateScan(navHostController: NavHostController, registry: ActivityResultRegistry, scanOptions: Map<String, Any>) {
        mainScreenState.value = currentScreenState.copy(scanState = SdkOperationStatus.IN_PROGRESS)
        AHIMultiScan.delegatePersistence = if (smoothingOption == SmoothingOption.NONE) {
            null
        } else {
            persistence
        }
        AHIMultiScan.initiateScan("body", scanOptions, registry) { resultFuture ->
            viewModelScope.launch(Dispatchers.Main) {
                val newState = currentScreenState
                when (val result = withContext(Dispatchers.IO) { resultFuture.get() }) {
                    is AHIResult.Success -> {
                        newState.resultMessage = ""
                        newState.classificationResults = result.value
                        lastClassificationResults = result.value
                        newState.scanState = SdkOperationStatus.SUCCESS
                    }
                    else -> {
                        newState.resultMessage = "Error occurred: ${result.errorCode()} - ${result.error()}"
                        newState.classificationResults = mapOf()
                        newState.scanState = SdkOperationStatus.NOT_DONE
                    }
                }
                mainScreenState.value = newState
                navHostController.navigate(NavRoutes.RESULT_SCREEN)
            }
        }
    }

    fun directInitiateScan(navHostController: NavHostController, registry: ActivityResultRegistry, scanOptions: Map<String, Any>) {
        mainScreenState.value = currentScreenState.copy(scanState = SdkOperationStatus.IN_PROGRESS)
            viewModelScope.launch(Dispatchers.Main) {
                val resultFuture = bodyScanPlugin.initiateScan(scanOptions, registry)
                val newState = currentScreenState
                when (val result = withContext(Dispatchers.IO) { resultFuture.get() }) {
                    is AHIResult.Success -> {
                        newState.resultMessage = ""
                        newState.classificationResults = result.value
                        lastClassificationResults = result.value
                        newState.scanState = SdkOperationStatus.SUCCESS
                    }
                    else -> {
                        newState.resultMessage = "Error occurred: ${result.error()}"
                        newState.classificationResults = mapOf()
                        newState.scanState = SdkOperationStatus.NOT_DONE
                    }
                }
                mainScreenState.value = newState
                navHostController.navigate(NavRoutes.RESULT_SCREEN)
            }
    }

    fun generateMesh(navHostController: NavHostController) {
        mainScreenState.value = currentScreenState.copy(resultMessage = "", meshGenerationState = SdkOperationStatus.IN_PROGRESS)
        navHostController.navigate(NavRoutes.MESH_SCREEN)
        AHIMultiScan.getScanExtra(mainScreenState.value?.classificationResults ?: mapOf(), mapOf("extrapolate" to listOf("mesh"))) {
            it.fold(
                onSuccess = { result ->
                    val uri = (result["extrapolate"] as? List<Map<*, *>>)?.firstOrNull()?.get("mesh") as? Uri
                    mainScreenState.value =
                        currentScreenState.copy(
                            resultMessage = "",
                            meshGenerationState = SdkOperationStatus.SUCCESS,
                            meshLocation = uri
                        )
                },
                onFailure = { error ->
                    mainScreenState.value =
                        currentScreenState.copy(
                            resultMessage = "Mesh generation failed with error: ${error.error}",
                            meshGenerationState = SdkOperationStatus.NOT_DONE,
                            meshLocation = null
                        )
                }
            )
        }
    }

    private fun Double.format(digits: Int) = "%.${digits}f".format(this)
}