package com.example.benchmark

import android.content.Context
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.AHIBSResourceType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IResources
import com.advancedhumanimaging.sdk.common.models.AHIResult
import java.io.File

class MockResources : IResources {
    override suspend fun getResource(name: String, type: AHIBSResourceType, context: Context): AHIResult<ByteArray> {
        val nameWithExtension = if (type == AHIBSResourceType.AHIBSResourceTypeML) "$name.tflite" else "$name.cereal"
        val assetsList = context.assets.list("Encoded")?.toList() ?: listOf()
        return if (assetsList.contains(nameWithExtension)) {
            // copy file from assets directory to data/data folder
            // this allows the built-in debug assets to be in the same place as we'd expect the downloaded assets
            val copiedFile = File(context.filesDir, nameWithExtension)
            context.assets.open("Encoded/$nameWithExtension").use { input ->
                copiedFile.outputStream().use { output ->
                    input.copyTo(output, 1024)
                }
            }
            AHIResult.success(copiedFile.readBytes())
        } else {
            AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_ASSET_ACCESS_FAILED)
        }
    }

    override fun releaseResource(name: String) {
        TODO("Not yet implemented")
    }
}