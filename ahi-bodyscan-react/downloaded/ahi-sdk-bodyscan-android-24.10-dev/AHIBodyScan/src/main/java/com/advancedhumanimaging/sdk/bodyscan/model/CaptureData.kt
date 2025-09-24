//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.model

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PointF
import android.util.Base64
import com.advancedhumanimaging.sdk.common.extensions.toList
import org.json.JSONObject
import java.io.ByteArrayOutputStream

data class CaptureData(
    val images: List<Bitmap>,
    val poseJoints: Map<String, PointF>,
    val contourMask: Bitmap
) {
    fun toJson(): JSONObject? {
        val imagesBase64 = images.mapNotNull { bitmapToBase64(it) }
        val serializedJoints = poseJoints.entries.associate {
            it.key to mapOf(
                "x" to it.value.x,
                "y" to it.value.y
            )
        }
        val contourBase64 = bitmapToBase64(contourMask)
        return if (contourBase64 != null) {
            JSONObject(
                mapOf(
                    "images" to imagesBase64,
                    "poseJoints" to serializedJoints,
                    "contourMask" to contourBase64
                )
            )
        } else {
            null
        }
    }

    private fun bitmapToBase64(bitmap: Bitmap): String? {
        return try {
            val byteArrayOutputStream = ByteArrayOutputStream()
            bitmap.compress(Bitmap.CompressFormat.PNG, 100, byteArrayOutputStream)
            val byteArray: ByteArray = byteArrayOutputStream.toByteArray()
            Base64.encodeToString(byteArray, Base64.DEFAULT)
        } catch (e: Exception) {
            null
        }
    }

    companion object {
        fun fromJson(json: JSONObject): CaptureData? {
            val jointsObj = json.optJSONObject("poseJoints")
            val poseJoints = mutableMapOf<String, PointF>()
            jointsObj?.keys()?.forEach { key ->
                val value = jointsObj[key]
                val pointObj = value as? JSONObject
                val pointX = pointObj?.optDouble("x")?.toFloat()
                val pointY = pointObj?.optDouble("y")?.toFloat()
                if (pointX != null && pointY != null) {
                    poseJoints[key] = PointF(pointX, pointY)
                }
            }
            val imagesObj = json.optJSONArray("images")
            val images = imagesObj?.toList()?.mapNotNull { base64ToBitmap(it.toString()) }
            val contourObj = json.optString("contourMask")
            val contourMask = base64ToBitmap(contourObj)
            return if (contourMask != null && images != null) {
                CaptureData(images, poseJoints, contourMask)
            } else {
                null
            }
        }

        private fun base64ToBitmap(base64: String): Bitmap? {
            val decoded = Base64.decode(base64, Base64.DEFAULT)
            return BitmapFactory.decodeByteArray(decoded, 0, decoded.size)
        }
    }
}
