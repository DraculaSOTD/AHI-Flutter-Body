package com.advancedhumanimaging.sdk.bodyscan.common.utils

import android.graphics.PointF

fun getProminentPersonJoints(joints: Map<String, List<Map<String, PointF>>>): Map<String, PointF> {
    val faceJoints = joints["face"]?.maxByOrNull { it.size } ?: mapOf()
    val bodyJoints = joints["body"]?.maxByOrNull { it.size } ?: mapOf()
    return faceJoints + bodyJoints
}