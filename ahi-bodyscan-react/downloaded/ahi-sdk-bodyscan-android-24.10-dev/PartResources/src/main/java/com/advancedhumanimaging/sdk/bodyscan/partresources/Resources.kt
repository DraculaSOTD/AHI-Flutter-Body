//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partresources

import android.content.Context
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.AHIBSResourceType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IResources
import com.advancedhumanimaging.sdk.common.helpers.DecryptionHelper
import com.advancedhumanimaging.sdk.common.models.AHIFile
import com.advancedhumanimaging.sdk.common.models.AHILogLevel
import com.advancedhumanimaging.sdk.common.models.AHILogging
import com.advancedhumanimaging.sdk.common.models.AHIResult
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

/**
 * Resources service.
 * @param decryptionKey The AES secret key, encoded as a base64 string.
 */
class Resources(private var decryptionKey: String? = null) : IResources {
    private val encryptedExtList = listOf(".bin", ".test_bin")

    override suspend fun getResource(
        name: String,
        type: AHIBSResourceType,
        context: Context
    ): AHIResult<ByteArray> {
        return withContext(Dispatchers.IO) {
            try {
                val file = getFile(name, type, context)
                when {
                    file == null -> {
                        AHILogging.log(AHILogLevel.DEBUG, "Resource ($name) not found")
                        AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_ASSET_NOT_FOUND)
                    }
                    encryptedExtList.find { file.name.endsWith(it) } != null -> {
                        if (decryptionKey == null) {
                            AHILogging.log(AHILogLevel.ERROR, "Accessing resource failed due to missing decryption key")
                            return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_ASSET_DECRYPTION_KEY_NOT_PROVIDED)
                        }
                        val decryptedBuffer = decryptResource(file.readBytes(), decryptionKey!!)
                        if (decryptedBuffer != null) {
                            AHIResult.success(decryptedBuffer)
                        } else {
                            AHILogging.log(AHILogLevel.ERROR, "Accessing resource failed due to failed decryption")
                            return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_ASSET_DECRYPTION_FAILED)
                        }
                    }
                    else -> {
                        AHIResult.success(file.readBytes())
                    }
                }
            } catch (e: Exception) {
                AHILogging.log(AHILogLevel.ERROR, "Failed to access resource")
                AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_ASSET_ACCESS_FAILED)
            }
        }
    }

    override fun releaseResource(name: String) {
        TODO("Not yet implemented")
    }

    private fun getFile(
        name: String,
        type: AHIBSResourceType,
        context: Context
    ): File? {
        val extList = listOf("") + encryptedExtList
        val nameWithExtension = if (type == AHIBSResourceType.AHIBSResourceTypeML) "$name.tflite" else "$name.cereal"
        // First check for resources in local storage. This should find downloaded resources in production builds.
        extList.forEach { ext ->
            val nameWithExtensionFull = "$nameWithExtension$ext"
            val localFilePath = AHIFile.getLocalPath(context, nameWithExtensionFull)
            val localFile = localFilePath?.let { File(it) } ?: File(context.filesDir, nameWithExtensionFull)
            if (localFile.exists()) {
                return localFile
            }
        }
        val assetsPath = "Encoded"
        val assetsList = context.assets.list(assetsPath)?.toList() ?: listOf()
        // Then fallback to checking for debug resources in assets.
        // Expect demo/internal apps to provide models as assets - they will never exist in release.
        extList.forEach { ext ->
            val nameWithExtensionFull = "$nameWithExtension$ext"
            if (assetsList.contains(nameWithExtensionFull)) {
                // Copy file from assets directory to data/data folder
                // This allows the built-in debug assets to be in the same place as we'd expect the downloaded assets
                val copiedFile = File(context.filesDir, nameWithExtensionFull)
                context.assets.open("$assetsPath/$nameWithExtensionFull").use { input ->
                    copiedFile.outputStream().use { output ->
                        input.copyTo(output, 1024)
                    }
                    input.close()
                }
                return copiedFile
            }
        }
        return null
    }

    private fun decryptResource(resourceBuffer: ByteArray, decryptionKey: String): ByteArray? {
        return try {
            DecryptionHelper.decode(resourceBuffer, decryptionKey, trim = false, useIV = true)
        } catch (e: Exception) {
            null
        }
    }
}