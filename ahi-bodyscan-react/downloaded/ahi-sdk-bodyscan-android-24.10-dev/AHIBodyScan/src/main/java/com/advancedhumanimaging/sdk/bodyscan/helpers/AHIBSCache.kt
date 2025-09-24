package com.advancedhumanimaging.sdk.bodyscan.helpers

import android.content.Context
import java.io.File
import java.util.*

object AHIBSCache {
    fun getDirectory(context: Context): File {
        val directory = File(context.filesDir, "AHIBSCache")
        if (!directory.exists())
            directory.mkdir()
        return directory
    }

    fun createFile(context: Context, fileName: String? = null): File {
        val directory = getDirectory(context)
        val mFileName = fileName ?: UUID.randomUUID().toString()
        return File(directory, mFileName)
    }

    fun deleteAllData(context: Context) {
        getDirectory(context).listFiles()?.forEach { file ->
            file.delete()
        }
    }
}