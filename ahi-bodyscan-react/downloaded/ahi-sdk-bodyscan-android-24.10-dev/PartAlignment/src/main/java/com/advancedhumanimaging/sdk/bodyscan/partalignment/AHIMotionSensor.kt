//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partalignment

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import androidx.appcompat.app.AppCompatActivity
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.GravityData
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IAlignment
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.MotionData
import kotlinx.coroutines.flow.MutableStateFlow
import kotlin.math.abs

class AHIMotionSensor(
    context: Context,
) : SensorEventListener, IAlignment {
    private val sensorManager: SensorManager
    private var gravityData: GravityData? = null
    private val motionDataStateFlow = MutableStateFlow<MotionData?>(null)
    private var activeListen = false
    private val GRAVITY = 9.81

    /* Sensors */
    private var graSensor: Sensor? = null
    private var accSensor: Sensor? = null
    private var magSensor: Sensor? = null

    /* Sensor datas */
    private val gravityReading = DoubleArray(3)
    private val accelerometerReading = FloatArray(3)
    private val magnetometerReading = FloatArray(3)

    /* Matrix & Angles */
    private val rotationMatrix = FloatArray(9)
    private val orientationAngles = FloatArray(3)

    /* Angle degrees */
    private var pitch = 0.0
    private var roll = 0.0
    private var yaw = 0.0
    private val outGravity = FloatArray(9)
    private var angle = 0.0

    init {
        sensorManager = context.getSystemService(AppCompatActivity.SENSOR_SERVICE) as SensorManager
        graSensor = sensorManager.getDefaultSensor(Sensor.TYPE_GRAVITY)
        accSensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)
        magSensor = sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD)
    }

    override fun onSensorChanged(event: SensorEvent?) {
        event?.let {
            when (it.sensor.type) {
                Sensor.TYPE_GRAVITY -> {
                    /* Gravity only use for display x,y,z data */
                    gravityReading[0] = -event.values[0] / GRAVITY
                    gravityReading[1] = -event.values[1] / GRAVITY
                    gravityReading[2] = -event.values[2] / GRAVITY
                    gravityData = GravityData(gravityReading[0], gravityReading[1], gravityReading[2])
                }
                Sensor.TYPE_ACCELEROMETER -> {
                    System.arraycopy(event.values, 0, accelerometerReading, 0, accelerometerReading.size)
                }
                Sensor.TYPE_MAGNETIC_FIELD -> {
                    System.arraycopy(event.values, 0, magnetometerReading, 0, magnetometerReading.size)
                }
            }
            SensorManager.getRotationMatrix(rotationMatrix, null, accelerometerReading, magnetometerReading)
            SensorManager.remapCoordinateSystem(rotationMatrix, SensorManager.AXIS_X, SensorManager.AXIS_Z, outGravity)
            SensorManager.getOrientation(outGravity, orientationAngles)
            pitch = Math.toDegrees(orientationAngles[1].toDouble())
            roll = Math.toDegrees(orientationAngles[2].toDouble())
            yaw = Math.toDegrees(orientationAngles[0].toDouble())
            angle = if (abs(pitch) in 0.0..5.0 && abs(roll) in 0.0..5.0) {
                if (pitch > roll) {
                    pitch
                } else {
                    roll
                }
            } else if (abs(pitch) > abs(roll)) {
                pitch
            } else if (abs(roll) > abs(pitch)) {
                roll
            } else {
                roll
            }
            gravityData?.let {
                motionDataStateFlow.value = MotionData(it, angle)
            }
        }
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) {}

    override fun isDeviceMotionAvailable(): Boolean {
        return graSensor != null && accSensor != null && magSensor != null
    }

    override fun isDeviceMotionActive(): Boolean {
        // Check if sensorGravity still null return false
        return isDeviceMotionAvailable() && activeListen
    }

    override fun start() {
        activeListen = true
        sensorManager.registerListener(this, graSensor, SensorManager.SENSOR_DELAY_UI)
        sensorManager.registerListener(this, magSensor, SensorManager.SENSOR_DELAY_UI)
        sensorManager.registerListener(this, accSensor, SensorManager.SENSOR_DELAY_UI)
    }

    override fun stop() {
        activeListen = false
        sensorManager.unregisterListener(this)
    }

    override fun deviceMotion(): MutableStateFlow<MotionData?> {
        // Return state flow emit queue.
        return motionDataStateFlow
    }

    override fun isDeviceAligned(): Boolean {
        return angle in -2.0..2.0
    }
}