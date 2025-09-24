//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

import kotlinx.coroutines.flow.MutableStateFlow

/**
 * Interface declaration of Alignment service. This provide motion sensor requirements, such as tracking movement and orientation.
 * */
interface IAlignment {
    /**
     * Indicates if the device has motion sensors.
     * @return If true, the device has motion sensors and can provide motion data, otherwise false.
     * */
    fun isDeviceMotionAvailable(): Boolean

    /**
     * Indicates is the device motion sensor is running and tracking movement/orientation data.
     * */
    fun isDeviceMotionActive(): Boolean

    /**
     * When called, provide that motion sensor exists, will make the sensor start tracking motion/orientation data.
     * */
    fun start()

    /**
     * When called, will stop the sensor from tracking motion/orientation data.
     * */
    fun stop()

    /**
     *  Request the latest motion/orientation sensor data sample.
     *  @return The latest motion/orientation sensor data sample, provided that the motion sensor is running/active.
     * */
    fun deviceMotion(): MutableStateFlow<MotionData?>

    /**
     * Request the motion/orientation sensor is aligned or not aligned.
     * @return The current state of motion/orientation sensor is aligned(true) or not aligned(false).
     * */
    fun isDeviceAligned(): Boolean
}

data class GravityData(
    var x:Double,
    var y:Double,
    var z:Double,
)

data class MotionData(
    val gravity: GravityData,
    val angle: Double
)