//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partcamera.example.utils

import android.content.Context
import android.os.Environment
import java.io.File

const val QA_TESTING_FOLDER = "/QA_Testing_Folder/"

interface FileUtils {
    fun createDirectoryIfNotExist(context: Context)
    fun createFile(context: Context): File
}

/**
 * Save path: /storage/emulated/0/Pictures/QA_Testing_Folder
 * */
object FileUtilsImpl : FileUtils {

    override fun createDirectoryIfNotExist(context: Context) {
        val folder = File(
            Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES)?.absolutePath + QA_TESTING_FOLDER
        )
        if (!folder.exists()) {
            folder.mkdirs()
        }
    }

    override fun createFile(context: Context): File {
        return File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES)?.absolutePath + QA_TESTING_FOLDER + System.currentTimeMillis() + ".jpg")
    }
}