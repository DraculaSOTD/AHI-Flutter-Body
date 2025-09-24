//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.helpers

import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.keys.AHIKeysUtil
import com.advancedhumanimaging.sdk.common.IAHIMultiScan
import com.advancedhumanimaging.sdk.common.extensions.toMap
import com.advancedhumanimaging.sdk.common.helpers.AHIRemoteAssetsHelper
import com.advancedhumanimaging.sdk.common.helpers.IAHIRemoteAssets
import com.advancedhumanimaging.sdk.common.helpers.TokenHelper
import com.advancedhumanimaging.sdk.common.models.AHIKey
import com.advancedhumanimaging.sdk.common.models.AHILogLevel
import com.advancedhumanimaging.sdk.common.models.AHILogging
import com.advancedhumanimaging.sdk.common.models.AHIResult

private const val AHI_BODYSCAN_FILES_ID = "1"
private const val multiScanImageName = "ahi-logo-multiscan.png"
private const val delimiters = "AHIKEY:"

internal class BodyScanRemoteAssets(private val multi: IAHIMultiScan) : IAHIRemoteAssets {
    override fun getFilesConfig(completionBlock: (files: Map<String, Any>?) -> Unit) {
        multi.getDetails {
            val config = it.getOrNull()
            val setup = config?.get("setup") as? Map<*, *>
            val token = setup?.get("TOKEN") as? String
            if (token == null) {
                completionBlock(null)
                return@getDetails
            }
            val appContext = multi.getApplicationContext().getOrNull() ?: return@getDetails
            val ahiAppToken: AHIKey
            try {
                ahiAppToken = AHIKeysUtil(multiScanImageName, delimiters).keyAHIAppToken(appContext)
            } catch (exception: Exception) {
                completionBlock(null)
                return@getDetails
            }
            if (ahiAppToken.privateKey.isEmpty() || ahiAppToken.verifyKey.isEmpty()) {
                completionBlock(null)
                return@getDetails
            }
            val appExtract = TokenHelper.decode(token, ahiAppToken)
            val appConfig = appExtract?.firstOrNull()?.toMap()
            if (appConfig == null) {
                completionBlock(null)
                return@getDetails
            }
            val cloudConfig = config["config"] as Map<*, *>?
            val files = cloudConfig?.get("files") as Map<String, Any>?
            completionBlock(files)
        }
    }

    fun areResourcesDownloaded(): Boolean {
        return AHIRemoteAssetsHelper.areResourcesDownloaded(multi, true, AHI_BODYSCAN_FILES_ID)
    }

    fun currentDownloadedSizeInBytes(): Long {
        val filesList = multi.getFilesList().getOrNull()
        if (filesList == null) {
            AHILogging.log(AHILogLevel.ERROR, "No files for the scan module.")
            return 0
        }
        val appContext = multi.getApplicationContext().getOrNull() ?: return 0
        return AHIRemoteAssetsHelper.getSizeInBytes(
            appContext,
            AHI_BODYSCAN_FILES_ID,
            true,
            filesList
        )
    }

    fun totalRemoteAssetsSizeInBytes(): Long {
        val filesList = multi.getFilesList().getOrNull()
        if (filesList == null) {
            AHILogging.log(AHILogLevel.ERROR, "No files for the scan module.")
            return 0
        }
        val appContext = multi.getApplicationContext().getOrNull() ?: return 0
        return AHIRemoteAssetsHelper.getSizeInBytes(
            appContext,
            AHI_BODYSCAN_FILES_ID,
            false,
            filesList
        )
    }

    fun initiateDownloadResources(inBackground: Boolean, downloadRetryCount: Int): AHIResult<Unit> {
        val filesList = getBodyScanFilesList() ?: return AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_NO_ASSET_FILES_LIST)
        //        download resources
        val appContext = multi.getApplicationContext().getOrNull() ?: return AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_NO_APP_CONTEXT)
        AHIRemoteAssetsHelper.downloadRequest(
            filesList,
            this,
            multi.getDownloadProgressDelegate(),
            appContext,
            inBackground,
            downloadRetryCount,
            completionBlock = {
                if (it.isFailure) {
                    AHILogging.log(AHILogLevel.ERROR, "Failed to download resources")
                } else {
                    AHILogging.log(AHILogLevel.INFO, "Starts downloading")
                }
            })
        return AHIResult.success()
    }

    private fun getBodyScanFilesList(): List<Map<String, Any>>? {
        val filesConfigList = multi.getFilesList().getOrNull()?.get(AHI_BODYSCAN_FILES_ID)
        if (AHIRemoteAssetsHelper.areResourcesDownloaded(multi, true, AHI_BODYSCAN_FILES_ID)) {
            // all resources are downloaded and present
            return null
        }
        return filesConfigList
    }
}
