//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partresources.example

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.viewModelScope
import com.advancedhumanimaging.sdk.bodyscan.BodyScan
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.AHIBSResourceType
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import com.advancedhumanimaging.sdk.common.IAHIScan
import com.advancedhumanimaging.sdk.common.models.AHIResult
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScan
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScanConstants
import kotlinx.coroutines.launch

enum class SdkOperationStatus {
    SUCCESS,
    IN_PROGRESS,
    NOT_DONE
}

data class MainScreenState(
    var sdkSetupState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var sdkReleaseState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var sdkResourceDownloadState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var sdkRemoveDownloadResourceState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var decryptResourceState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var meshGenerationState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var scanState: SdkOperationStatus = SdkOperationStatus.NOT_DONE,
    var resultMessage: String = "",
)

class MainViewModel(application: Application) : AndroidViewModel(application) {
    private val mainScreenState = MutableLiveData(MainScreenState())
    private val resources = Resources("SpO9g6T4BZAzj2YkjpvbYA==")
    private val currentScreenState get() = mainScreenState.value?.copy() ?: MainScreenState()

    fun getMainScreenState(): LiveData<MainScreenState> {
        return mainScreenState
    }

    fun triggerSdkInit() = viewModelScope.launch {
        val exampleConfig = mapOf(
            AHIMultiScanConstants.AHISetupTokenKey to "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJicTNKYXN5M0hMVmRxMkRja05MR1FXdGk2WEZIM2dVUElndUVNOXIyUTdXR05WRGVHTkV5ajR0T09XYXZxa3Y2VHEvYXJEVmtJNUZFSzZ4STFJUmcwRlVMK0t1MVdZV1UrOUtqSGsvS3pHVFk4bllES29WU2J6WEw1eVljOGprMndkMlNmeFBhYXJmT3pPZS9odFJvdUJwa2RSM09BS1V3TGtITTJYT3Fyays4d0MyenE2Y01pczFNZThTVWtGZ2RDS1ZsNVFLbkU0b2xGYzBNRTBVOEVqTW5mUUZSd1ZSZ2VVWlVOdkVWL3R2K1B0aEpJZGJKUnpxa0JUaE5YNDZBaXJaT2ZPV0tsczVoalZXMzNDS2xSczgxd3VwRWYrU0FQY3MxRmUwV2Urd1diN3VKSTdqTXRJeE1pcEdWdUQxMXNEODVKaW1GNm51bnBYRlY3NWc5TmVBR0JpdzZETDVnbXRPenRVNFVTaStKbWhvc1lIc3ZLMWhZay9YRTVKREFEeHV3eWJVaVlxVFJwVWdUeWRYUU02V1BpVUMvaVRmWEJIc1BKK3liRlBTcVpSVWhWa21EYUZ4TjJvNXluL1ZzRkNQU3IvSEs3WmcweEhZUGh4UHVpby8ydkRaZjRNZk1RU0tZaVhDbUxUNks0dmQwYy94OUg1ZngrYzdhMWlTRmR2dTNjWnV0aG1jdG5aYjVLU2prblVkcFlWamJaaytWb1B6eWczUVVpRXhqWDNuaVRZR0NaS3M5RkYyVFNUTW9PenZmVnVYcHpoVGhYMm5IeEpGdzNwN2phcGNvN21tZVhpYzF2VWpVVDhhQ09Xc3hqK1o1MUVRRHBielNyTjllSVV2N3pRZmtsNDhLQXpUUng3M0dkMWZuUmVMM0xzTWExcDBEc2hDakRlcz0iLCJleHAiOjE4MTQ3MDc1MzcsInZlciI6M30.EJTaMwDwzMtgIuY1DtwnCDyCnp9_2NauN7n91O7UIco8YpBbSxXWQWbbcvT-72pbpAQagAQ7hgKkjWEce5kKniDh3KhVa546kdQeneuWd1vu7iV1s3nZsMnPCxmFcv5uL30Cv8HWIkDEZb5UozZTgOcHDFAKyKQVT7Tmw0XA5GOjZqKZRtYtRXjzkQw7nidI_I0YkAshC6H8eakeyrVp-u-SilU95_ppIgZOtZHk1nSo04HATGXxl0inTDa57bi_UTU_OHLLlzKm-4GYtSKKdxyHcAipZklkQic4I_g4K43FeLNlWEyGpHt0dnabJm4auU033RZa5L87qCWFDSkTlBDTb3xTfSVtPtsyXTpHDMXS1-1WzQm97P8QcZ2zJl4z1wDk1WClwYRvk4E2JiA9ZwTef0OG2nQVlRJw0O4IpUWI8NLfg-2YcZR9-DZs2iC2QAIV0K1MVc9Ro2LMkgIm0nycgvYagmn-PZLuq9pATjVlW3oF1pJahEJcltoBc-JE0b_NjMsOFMeH3FQNf7_fCBYbRZA3SkOX87uPOqn2BXkYP77sHJ91mFw89TDilx4iVJSbErUej7tr8ZdagCVG1x0ltJ8qVAFCK5wnHZxp5FdfVBuaZNA2e7VzCFB7cYEFUfsuHd9NK2TzLcxizw7L2PxYHP2RdNCKMR3ebR572-w"
        )
        val exampleScans = arrayOf<IAHIScan>(BodyScan())
        val pendingState = mainScreenState.value?.copy() ?: MainScreenState()
        pendingState.sdkSetupState = SdkOperationStatus.IN_PROGRESS
        mainScreenState.value = pendingState
        AHIMultiScan.setup(getApplication(), exampleConfig, exampleScans) {
            val newState = mainScreenState.value?.copy() ?: MainScreenState()
            newState.sdkSetupState =
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
                        mainScreenState.value?.sdkResourceDownloadState = SdkOperationStatus.SUCCESS
                        mainScreenState.value =
                            currentScreenState.copy(resultMessage = "", sdkResourceDownloadState = SdkOperationStatus.SUCCESS)
                    }
                }, {
                    mainScreenState.value = currentScreenState.copy(resultMessage = "Download Failed: ${it.error.code()}")
                })
            }
        }
    }

    fun downloadResources() {
        mainScreenState.value = currentScreenState.copy(sdkResourceDownloadState = SdkOperationStatus.IN_PROGRESS)
        val result = AHIMultiScan.downloadResourcesInForeground(1)
        mainScreenState.value =
            currentScreenState.copy(sdkResourceDownloadState = if (result.isSuccess) SdkOperationStatus.IN_PROGRESS else SdkOperationStatus.NOT_DONE)
    }

    fun triggerReleaseDownloadResourceState() {
        mainScreenState.value = currentScreenState.copy(sdkRemoveDownloadResourceState = SdkOperationStatus.IN_PROGRESS)
        val fileDirPath = getApplication<Application>().applicationContext.filesDir
        if (fileDirPath.exists()) {
            val fileLists = fileDirPath.listFiles()
            if (fileLists != null) {
                fileLists.forEach {
                    if (it.name.contains(".bin")) it.delete()
                }
                mainScreenState.value = currentScreenState.copy(
                    sdkRemoveDownloadResourceState = if (fileLists.isEmpty()) SdkOperationStatus.SUCCESS else SdkOperationStatus.NOT_DONE,
                    sdkResourceDownloadState = SdkOperationStatus.NOT_DONE,
                    resultMessage = ""
                )
            }
        }
    }

    fun decryptResource(resourceName: String) {
        viewModelScope.launch {
            mainScreenState.value = currentScreenState.copy(decryptResourceState = SdkOperationStatus.IN_PROGRESS)
            val name = resourceName.substringBefore(".")
            val type = if (resourceName.contains(".tflite")) AHIBSResourceType.AHIBSResourceTypeML else AHIBSResourceType.AHIBSResourceTypeSVR
            val resourceResult = resources.getResource(name, type, getApplication<Application>().applicationContext)
            val isDecryptSuccessful = resourceResult.getOrNull()?.size?.let { it > 0 } == true
            mainScreenState.value = currentScreenState.copy(
                decryptResourceState = if (isDecryptSuccessful) SdkOperationStatus.SUCCESS else SdkOperationStatus.NOT_DONE,
                resultMessage = if (isDecryptSuccessful) "Resource decrypted successfully" else "Resource decryption failed"
            )
        }
    }

    fun resetResultMessage() {
        mainScreenState.value = currentScreenState.copy(resultMessage = "")
    }

    private fun Double.format(digits: Int) = "%.${digits}f".format(this)

}