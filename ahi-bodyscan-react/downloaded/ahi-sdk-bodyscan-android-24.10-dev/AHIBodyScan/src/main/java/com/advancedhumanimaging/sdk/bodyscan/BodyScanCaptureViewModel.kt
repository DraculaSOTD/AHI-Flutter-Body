//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan

import android.content.Context
import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import android.media.Image
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import androidx.work.*
import com.advancedhumanimaging.sdk.bodyscan.common.*
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.GravityData
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.MotionData
import com.advancedhumanimaging.sdk.bodyscan.common.utils.getProminentPersonJoints
import com.advancedhumanimaging.sdk.bodyscan.helpers.AHIBSCache
import com.advancedhumanimaging.sdk.bodyscan.model.CaptureData
import com.advancedhumanimaging.sdk.bodyscan.partcamera.CameraCapture
import com.advancedhumanimaging.sdk.bodyscan.partcontour.ContourGenerator
import com.advancedhumanimaging.sdk.bodyscan.partposedetection.BodyScanPoseDetection
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import com.advancedhumanimaging.sdk.bodyscan.partsegmentation.Segmentation
import com.advancedhumanimaging.sdk.bodyscan.partui.screens.CaptureState
import com.advancedhumanimaging.sdk.bodyscan.partui.screens.CriticalErrorState
import com.advancedhumanimaging.sdk.bodyscan.partui.screens.PositioningState
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.ANIMATION_DELAY
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.COUNTDOWN_DELAY
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.PROMPT_DELAY
import com.advancedhumanimaging.sdk.common.models.AHIResult
import com.advancedhumanimaging.sdk.common.uidelegation.AHIScanViewModel
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import java.io.File
import java.io.FileOutputStream
import java.text.SimpleDateFormat
import java.util.*

enum class BodyScanState {
    ALIGNMENT,
    POSITIONING,
    FRONT_CAPTURE,
    SIDE_CAPTURE,
    CRITICAL_ERROR,
    COMPLETE
}

@Suppress("UNCHECKED_CAST")
internal class BodyScanViewModelFactory(private val inputs: BodyScanValidInput) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        return BodyScanCaptureViewModel(inputs) as T
    }
}

class BodyScanCaptureViewModel(userInputs: BodyScanValidInput) : AHIScanViewModel<BodyScanValidInput>(userInputs) {
    private val placeHolderBitmap: Bitmap = Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888)
    val scanState = MutableLiveData(BodyScanState.ALIGNMENT)
    var positioningState by mutableStateOf(PositioningState.FIT_WHOLE_BODY)
    var positioningStateVisibility by mutableStateOf(false)
    var captureState by mutableStateOf(CaptureState.TURN_LEFT)
    var criticalErrorState = MutableLiveData<CriticalErrorState>()
    var criticalErrorDebugTimes by mutableStateOf(listOf<String>())
    var alignmentCompleteProgress by mutableStateOf(0F)
    var alignmentTextVisibility by mutableStateOf(true)
    var captureTimer by mutableStateOf(0)
    var showFlash by mutableStateOf(false)
    val motionData = MutableLiveData(MotionData(GravityData(0.0, 0.0, 0.0), 45.0))
    private val contourGenerator = ContourGenerator()
    private val poseDetector = BodyScanPoseDetection()
    private val resources = Resources(userInputs.decryptionKey)
    private val imageSize = Resolution(AHIBSImageCaptureWidth.toInt(), AHIBSImageCaptureHeight.toInt())
    var contourMask: Bitmap = placeHolderBitmap
    var contour by mutableStateOf<Array<PointF>>(arrayOf())
    var optimalZones by mutableStateOf(mapOf<String, RectF>())
    var poseJoints by mutableStateOf(mapOf<String, List<Map<String, PointF>>>())
    private var scaledContourPoseJoints by mutableStateOf(mapOf<String, PointF>())
    var frontCaptureData: CaptureData? = null
    var sideCaptureData: CaptureData? = null
    private var poseJointsLastUpdated = Date().time
    private var frontSegmentationJob: Job? = null
    private var frontSegmentationDataLocation: String? = null

    // get useAverage feature from cloud, default set as true.
    private var useAverage = mutableStateOf(userInputs.featuresEnabled["useAverage"] ?: true)

    // get useFastHeightValidation feature from cloud, default set as true.
    private var useFastHeightValidation = userInputs.featuresEnabled["useFastHeightValidation"] ?: true

    // get threshold value from cloud, default set as 0.94f
    private val accuracyThreshold = mutableStateOf(userInputs.accuracyThreshold ?: 0.94f)

    val poseValidator = BodyScanPoseValidator(useFastHeightValidation)

    // Alignment Sound trigger
    private val _isTriggerAlignedSound = MutableStateFlow(false)
    val isTriggerAlignedSound = _isTriggerAlignedSound.asStateFlow()

    // Capture Sound trigger
    private val _isTriggerCaptureSound = MutableStateFlow(false)
    val isTriggerCaptureSound = _isTriggerCaptureSound.asStateFlow()

    // Enable/disable debug mode
    val isDebug = userInputs.isDebug


    private suspend fun animateFlash() {
        showFlash = true
        delay(ANIMATION_DELAY)
        viewModelScope.launch {
            delay(50)
            showFlash = false
        }
    }

    suspend fun detectPose(image: Image, rotationDegrees: Int) {
        poseJoints = poseDetector.detect(DetectionType.faceAndBody, image, rotationDegrees) ?: mapOf()
        poseJointsLastUpdated = Date().time
    }

    private suspend fun checkSideCapture(): CaptureState {
        return withContext(Dispatchers.IO) {
            val singleJoints = getProminentPersonJoints(poseJoints)
            val rightHand = singleJoints["CentroidRightHand"]
            val leftHand = singleJoints["CentroidLeftHand"]
            val rightAnkle = singleJoints["CentroidRightAnkle"]
            val leftAnkle = singleJoints["CentroidLeftAnkle"]
            val extremeContourRight = contour.minOf { it.x }
            val extremeContourLeft = contour.maxOf { it.x }
            val isArmOutside =
                rightHand?.let { it.x < extremeContourRight || it.x > extremeContourLeft } == true || leftHand?.let { it.x < extremeContourRight || it.x > extremeContourLeft } == true
            val isLegOutside =
                rightAnkle?.let { it.x < extremeContourRight || it.x > extremeContourLeft } == true || leftAnkle?.let { it.x < extremeContourRight || it.x > extremeContourLeft } == true
            val inspectionResult = inspectPose(contourMask, Profile.side, poseJoints)
            val isOnePersonOnly = inspectionResult.values.find { it == InspectionResult.falseMultipleFaces } == null
            val isHeadInside = inspectionResult["face"] == InspectionResult.trueInContour
            val isInContour = poseValidator.poseInspection.isInContour(Profile.side, inspectionResult)
            captureState = when {
                !isOnePersonOnly -> CaptureState.ONE_PERSON_ONLY
                isHeadInside && (isArmOutside || isLegOutside) -> CaptureState.FEET_ARMS_INSIDE
                isInContour -> CaptureState.IN_CONTOUR
                else -> CaptureState.NONE
            }
            captureState
        }
    }

    private suspend fun checkFrontCapture(): CaptureState {
        return withContext(Dispatchers.IO) {
            val inspectionResult = inspectPose(contourMask, Profile.front, poseJoints)
            val isOnePersonOnly = inspectionResult.values.find { it == InspectionResult.falseMultipleFaces } == null
            val isInContour = poseValidator.poseInspection.isInContour(Profile.front, inspectionResult)
            captureState = when {
                !isOnePersonOnly -> CaptureState.ONE_PERSON_ONLY
                isInContour -> CaptureState.IN_CONTOUR
                else -> CaptureState.NONE
            }
            captureState
        }
    }

    private suspend fun generateIdealContour(context: Context, profile: Profile): Boolean {
        val contour = contourGenerator.generateIdealContour(context,
            resources,
            userInputs.sex,
            userInputs.heightCm.toFloat(),
            userInputs.weightKg.toFloat(),
            imageSize,
            motionData.value?.let { Math.toRadians(it.angle).toFloat() } ?: 0F,
            profile)
        return if (contour != null) {
            this.contour = contour
            true
        } else {
            finishedWithError(BodyScanError.BODY_SCAN_CONTOUR_GENERATOR_FAILED_TO_GENERATE_IDEAL_CONTOUR)
            false
        }
    }

    private suspend fun generateScaledContour(): Boolean {
        val joints = scaledContourPoseJoints.ifEmpty { getProminentPersonJoints(poseJoints) }
        val scaledContour = contourGenerator.generateScaledContour(
            contour, joints
        )
        return if (scaledContour != null) {
            this.contour = scaledContour
            true
        } else {
            finishedWithError(BodyScanError.BODY_SCAN_CONTOUR_GENERATOR_FAILED_TO_GENERATE_SCALED_CONTOUR)
            false
        }
    }

    private fun updateScaledContourPoseJoints() {
        scaledContourPoseJoints = getProminentPersonJoints(poseJoints)
    }

    private suspend fun updateContourMask(): Boolean {
        val contourMask = contourGenerator.generateContourMask(contour, imageSize)
        return if (contourMask != null) {
            this.contourMask = contourMask
            true
        } else {
            finishedWithError(BodyScanError.BODY_SCAN_CONTOUR_GENERATOR_FAILED_TO_GENERATE_IDEAL_CONTOUR_MASK)
            false
        }
    }

    private suspend fun updateOptimalZones(): Boolean {
        val zones = contourGenerator.generateOptimalZones(contour, imageSize, accuracyThreshold.value)
        return if (zones != null) {
            optimalZones = zones
            true
        } else {
            finishedWithError(BodyScanError.BODY_SCAN_CONTOUR_GENERATOR_FAILED_TO_GENERATE_OPTIMAL_ZONES)
            false
        }
    }

    private suspend fun inspectPose(
        contourMask: Bitmap,
        profile: Profile,
        poseJoints: Map<String, List<Map<String, PointF>>>,
    ): Map<String, InspectionResult> {
        // Using placeholder bitmap since it is not used in inspection
        val inspectionResult = poseValidator.poseInspection.inspect(placeHolderBitmap, contourMask, optimalZones, profile, poseJoints)
        if (inspectionResult is AHIResult.Failure) {
            finishedWithError(inspectionResult.reason.error as BodyScanError)
        }
        return inspectionResult.getOrNull() ?: mapOf()
    }

    fun progressToNextStage() {
        val nextStage = when (scanState.value) {
            BodyScanState.ALIGNMENT -> BodyScanState.POSITIONING
            BodyScanState.POSITIONING -> BodyScanState.FRONT_CAPTURE
            BodyScanState.FRONT_CAPTURE -> BodyScanState.SIDE_CAPTURE
            BodyScanState.SIDE_CAPTURE -> BodyScanState.COMPLETE
            BodyScanState.COMPLETE -> BodyScanState.COMPLETE
            else -> BodyScanState.ALIGNMENT
        }
        scanState.value = nextStage
    }

    private suspend fun updatePositioningState(state: PositioningState) {
        positioningStateVisibility = false
        delay(500)
        positioningState = state
        positioningStateVisibility = true
    }

    suspend fun positioning(context: Context) {
        poseValidator.resetCriticalErrorTimes()
        updatePositioningState(PositioningState.FIT_WHOLE_BODY)
        generateIdealContour(context, Profile.front)
        updateContourMask()
        updateOptimalZones()
        do {
            delay(PROMPT_DELAY)
            var position = poseValidator.checkPosition(contourMask, optimalZones, poseJoints)
            if (isDebug) {
                updateCriticalErrorDebugTimes()
            }
            val criticalError = checkCriticalErrors()

            // If critical error is detected, set state to CRITICAL_ERROR
            criticalError?.let {
                criticalErrorState.value = it
                poseValidator.resetCriticalErrorTimes()
                scanState.value = BodyScanState.CRITICAL_ERROR
                return
            }

            // If in perfect spot, check again in 2 seconds to be sure
            if (position == PositioningState.PERFECT_SPOT) {
                delay(2000)
                position = poseValidator.checkPosition(contourMask, optimalZones, poseJoints)
            }

            updatePositioningState(position)
        } while (positioningState != PositioningState.PERFECT_SPOT)
        updateScaledContourPoseJoints()
        delay(PROMPT_DELAY)
        generateScaledContour()
        updateContourMask()
        updateOptimalZones()
        progressToNextStage()
    }

    private suspend fun checkCriticalErrors(): CriticalErrorState? {
        return withContext(Dispatchers.IO) {
            val isPersonMissing = (poseValidator.getCriticalErrorDuration(CriticalErrorState.PERSON_MISSING) >= ERROR_DURATION_THRESHOLD)
            val isPhonePlacementError = (poseValidator.getCriticalErrorDuration(CriticalErrorState.ANKLES_MISSING) >= ERROR_DURATION_THRESHOLD)
            val isPhoneTooLowIndeterminateError = (poseValidator.getCriticalErrorDuration(CriticalErrorState.PHONE_TOO_LOW_INDETERMINATE) >= ERROR_DURATION_THRESHOLD)
            val isPhoneTooHighIndeterminateError = (poseValidator.getCriticalErrorDuration(CriticalErrorState.PHONE_TOO_HIGH_INDETERMINATE) >= ERROR_DURATION_THRESHOLD)
            val isPhoneTooLowError = (poseValidator.getCriticalErrorDuration(CriticalErrorState.PHONE_TOO_LOW) >= ERROR_IMMEDIATE_DURATION_THRESHOLD)
            val isPhoneTooHighError = (poseValidator.getCriticalErrorDuration(CriticalErrorState.PHONE_TOO_HIGH) >= ERROR_IMMEDIATE_DURATION_THRESHOLD)

            when {
                isPersonMissing -> CriticalErrorState.PERSON_MISSING
                isPhonePlacementError -> CriticalErrorState.ANKLES_MISSING
                isPhoneTooLowIndeterminateError -> CriticalErrorState.PHONE_TOO_LOW_INDETERMINATE
                isPhoneTooHighIndeterminateError -> CriticalErrorState.PHONE_TOO_HIGH_INDETERMINATE
                isPhoneTooLowError -> CriticalErrorState.PHONE_TOO_LOW
                isPhoneTooHighError -> CriticalErrorState.PHONE_TOO_HIGH
                else -> null
            }
        }
    }

    suspend fun frontCapture(context: Context, cameraCapture: CameraCapture) {
        captureState = CaptureState.FIT_OUTLINE
        delay(PROMPT_DELAY)
        resetFrontSegmentation()
        do {
            captureTimer = 0
            captureState = CaptureState.NONE
            do {
                checkFrontCapture()
            } while (captureState != CaptureState.IN_CONTOUR)
            countdownLoop@ for (i in 3 downTo 1) {
                _isTriggerCaptureSound.value = false
                delay(COUNTDOWN_DELAY)
                captureTimer = i
                delay(ANIMATION_DELAY)
                if (!poseValidator.isOnePersonOnly(contourMask, optimalZones, Profile.front, poseJoints)) {
                    do {
                        captureState = CaptureState.ONE_PERSON_ONLY
                    } while (!poseValidator.isOnePersonOnly(contourMask, optimalZones, Profile.front, poseJoints))
                    captureTimer = 0
                    captureState = CaptureState.NONE
                    break@countdownLoop
                } else if (checkFrontCapture() != CaptureState.IN_CONTOUR) {
                    break@countdownLoop
                }
            }
            if (captureState == CaptureState.IN_CONTOUR) {
                val frontCaptures = cameraCapture.takeCapture(mapOf())
                _isTriggerCaptureSound.value = true
                frontCaptureData = frontCaptures?.let {
                    if (isDebug) {
                        savePictures(context, frontCaptures, Profile.front)
                    }
                    CaptureData(frontCaptures.map { it.image }, getProminentPersonJoints(poseJoints), contourMask)
                }
                waitForNewDetectedPoseJoints()
                checkFrontCapture()
            }
        } while (captureState != CaptureState.IN_CONTOUR)
        if (frontCaptureData == null) {
            finishedWithError(BodyScanError.BODY_SCAN_FRONT_CAPTURE_FAILURE)
        }
        _isTriggerCaptureSound.value = false
        captureTimer = 0
        frontCaptureData?.let { doFrontSegmentation(context, it) }
        animateFlash()
        progressToNextStage()
    }

    suspend fun sideCapture(context: Context, cameraCapture: CameraCapture) {
        captureState = CaptureState.TURN_LEFT
        generateIdealContour(context, Profile.side)
        generateScaledContour()
        updateContourMask()
        updateOptimalZones()
        delay(PROMPT_DELAY)
        captureTimer = 0
        captureState = CaptureState.NONE
        delay(PROMPT_DELAY)
        do {
            captureTimer = 0
            do {
                if (captureState == CaptureState.FEET_ARMS_INSIDE) {
                    delay(PROMPT_DELAY)
                }
                checkSideCapture()
            } while (captureState != CaptureState.IN_CONTOUR)
            countdownLoop@ for (i in 3 downTo 1) {
                _isTriggerCaptureSound.value = false
                delay(COUNTDOWN_DELAY)
                captureTimer = i
                delay(ANIMATION_DELAY)
                if (!poseValidator.isOnePersonOnly(contourMask, optimalZones, Profile.side, poseJoints)) {
                    do {
                        captureState = CaptureState.ONE_PERSON_ONLY
                    } while (!poseValidator.isOnePersonOnly(contourMask, optimalZones, Profile.side, poseJoints))
                    captureTimer = 0
                    captureState = CaptureState.NONE
                    break@countdownLoop
                } else if (checkSideCapture() != CaptureState.IN_CONTOUR) {
                    break@countdownLoop
                }
            }
            if (captureState == CaptureState.IN_CONTOUR) {
                val sideCaptures = cameraCapture.takeCapture(mapOf())
                _isTriggerCaptureSound.value = true
                sideCaptureData = sideCaptures?.let {
                    if (isDebug) {
                        savePictures(context, sideCaptures, Profile.side)
                    }
                    CaptureData(sideCaptures.map { it.image }, getProminentPersonJoints(poseJoints), contourMask)
                }
                waitForNewDetectedPoseJoints()
                checkSideCapture()
            }
        } while (captureState != CaptureState.IN_CONTOUR)
        if (sideCaptureData == null) {
            finishedWithError(BodyScanError.BODY_SCAN_SIDE_CAPTURE_FAILURE)
        }
        _isTriggerCaptureSound.value = false
        captureTimer = 0
        val scanData = prepareScanData(context)
        animateFlash()
        progressToNextStage()
        if (scanData != null) {
            queueFinalRequestWithWorker(context, scanData)
        } else {
            finishedWithError(BodyScanError.BODY_SCAN_CAPTURE_DATA_PREPARATION_FAILURE)
        }
    }

    // Wait for up to 500ms until new pose points are detected
    private suspend fun waitForNewDetectedPoseJoints() {
        val now = Date().time
        val maxDelay = 500
        for (i in 1..maxDelay) {
            if (poseJointsLastUpdated > now) {
                break
            } else if (i < maxDelay) {
                delay(1)
            }
        }
    }

    private suspend fun prepareScanData(context: Context): Data? {
        return if (frontCaptureData != null && sideCaptureData != null) {
            val sideCaptureDataLocation = storeSideCaptureData(context, sideCaptureData!!)
            workDataOf(
                "decryptionKey" to userInputs.decryptionKey,
                "uid" to userInputs.uid,
                "sex" to userInputs.sex.name,
                "heightCm" to userInputs.heightCm,
                "weightKg" to userInputs.weightKg,
                "side_capture_data_location" to sideCaptureDataLocation,
                "front_segmentation_data_location" to frontSegmentationDataLocation,
                "useAverage" to useAverage.value
            )
        } else {
            null
        }
    }

    private fun doFrontSegmentation(context: Context, frontCaptureData: CaptureData) {
        val file = AHIBSCache.createFile(context)
        frontSegmentationDataLocation = file.absolutePath
        frontSegmentationJob = GlobalScope.launch(Dispatchers.IO) {
            val captures = mutableListOf<Bitmap>()
            val contours = mutableListOf<Bitmap>()
            val poseJoints = mutableListOf<Map<String, PointF>>()
            val profiles = mutableListOf<Profile>()
            for (i in 0 until frontCaptureData.images.size) {
                captures.add(frontCaptureData.images[i])
                contours.add(frontCaptureData.contourMask)
                poseJoints.add(frontCaptureData.poseJoints)
                profiles.add(Profile.front)
            }
            val segmentationResult = Segmentation.segment(
                captures.toTypedArray(), contours.toTypedArray(), profiles.toTypedArray(), poseJoints.toTypedArray(), context, resources
            )
            if (segmentationResult.isSuccess) {
                segmentationResult.getOrNull()?.let {
                    val json = CaptureData(it.toList(), frontCaptureData.poseJoints, frontCaptureData.contourMask).toJson().toString()
                    file.writeText(json)
                }
            }

        }
    }

    private fun resetFrontSegmentation() {
        frontSegmentationDataLocation = null
        frontSegmentationJob?.cancel()
    }

    private suspend fun storeSideCaptureData(context: Context, sideCaptureData: CaptureData): String {
        return withContext(Dispatchers.IO) {
            val file = AHIBSCache.createFile(context)
            // Write side capture data to file asynchronously and send file location to worker to read when ready
            GlobalScope.launch(Dispatchers.IO) {
                val sideDataJson = sideCaptureData.toJson().toString()
                file.writeText(sideDataJson)
            }
            file.absolutePath
        }
    }

    // Save captures for QA debug propose.
    private fun savePictures(context: Context, captures: Array<Capture>, profile: Profile) {
        GlobalScope.launch(Dispatchers.IO) {
            val formatter = SimpleDateFormat("yyyyMMddHHmmss", Locale.getDefault())
            val now = Date()
            val fileName = "${formatter.format(now)}_${profile.name}"
            captures.forEachIndexed { index, capture ->
                try {
                    val file = File(context.filesDir, "${fileName}_${index + 1}.png")
                    val out = FileOutputStream(file)
                    capture.image.compress(Bitmap.CompressFormat.PNG, 100, out)
                } catch (e: Exception) {
                    e.printStackTrace()
                }
            }
        }
    }

    private fun updateCriticalErrorDebugTimes() {
        val errorTimes = mutableListOf<String>()
        CriticalErrorState.values().forEach {
            val duration = poseValidator.getCriticalErrorDuration(it) / 1000L
            val text = when (it) {
                CriticalErrorState.PHONE_TOO_HIGH, CriticalErrorState.PHONE_TOO_HIGH_INDETERMINATE -> "PTHigh"
                CriticalErrorState.PHONE_TOO_LOW, CriticalErrorState.PHONE_TOO_LOW_INDETERMINATE -> "PTLow"
                CriticalErrorState.ANKLES_MISSING -> "CSAnkles"
                CriticalErrorState.PERSON_MISSING -> "CSYou"
            }
            errorTimes.add("$text: ${duration}s")
        }
        criticalErrorDebugTimes = errorTimes
    }

    private fun queueFinalRequestWithWorker(context: Context, data: Data) {
        val workRequest = OneTimeWorkRequestBuilder<BodyScanResultsWorker>().setInputData(data).build()
        WorkManager.getInstance(context).enqueueUniqueWork(
            BodyScan.RESULTS_WORKER_NAME, ExistingWorkPolicy.REPLACE, workRequest
        )
        onCaptureFinishedPendingResults(workRequest.id)
    }

    fun finishedWithError(error: BodyScanError) {
        viewModelScope.launch(Dispatchers.Main) {
            onCaptureError(error.code())
        }
    }

    fun setTriggerAlignSound(trigger: Boolean) {
        _isTriggerAlignedSound.value = trigger
    }

    companion object {
        private const val ERROR_DURATION_THRESHOLD = 60000L
        private const val ERROR_IMMEDIATE_DURATION_THRESHOLD = 2000L
        private const val ERROR_COUNT_THRESHOLD = 20
    }
}