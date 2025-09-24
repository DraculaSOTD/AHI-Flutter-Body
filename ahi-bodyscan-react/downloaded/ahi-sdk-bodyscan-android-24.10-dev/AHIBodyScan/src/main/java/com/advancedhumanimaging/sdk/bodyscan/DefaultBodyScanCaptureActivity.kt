//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan

import android.Manifest
import android.annotation.SuppressLint
import android.content.pm.PackageManager
import android.content.res.Configuration
import android.content.res.Configuration.UI_MODE_NIGHT_YES
import android.os.Bundle
import android.os.CountDownTimer
import android.view.WindowManager
import androidx.activity.OnBackPressedCallback
import androidx.activity.compose.setContent
import androidx.camera.core.CameraSelector
import androidx.compose.animation.Crossfade
import androidx.compose.animation.core.tween
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.shape.CornerSize
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.runtime.*
import androidx.compose.runtime.livedata.observeAsState
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.unit.dp
import androidx.core.content.ContextCompat
import androidx.lifecycle.lifecycleScope
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.helpers.AHISoundUtils
import com.advancedhumanimaging.sdk.bodyscan.helpers.AHISounds
import com.advancedhumanimaging.sdk.bodyscan.partalignment.AHIMotionSensor
import com.advancedhumanimaging.sdk.bodyscan.partcamera.CameraCapture
import com.advancedhumanimaging.sdk.bodyscan.partui.screens.*
import com.advancedhumanimaging.sdk.bodyscan.partui.theme.AHIBSTheme
import com.advancedhumanimaging.sdk.common.AHIThemes
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.collectLatest

internal class DefaultBodyScanCaptureActivity : BodyScanCaptureActivity() {
    private lateinit var viewModel: BodyScanCaptureViewModel
    private lateinit var ahiMotionSensor: AHIMotionSensor
    private lateinit var cameraCapture: CameraCapture
    private lateinit var alignmentCompleteTimer: CountDownTimer
    private lateinit var alignmentInfoBottomSheet: AHIBSAlignmentInfoBottomSheet
    private lateinit var phoneHeightTipsBottomSheet: AHIBSPhoneHeightTipsBottomSheet
    private var captureStateJob: Job? = null
    private var alignmentCheckJob: Job? = null
    private lateinit var ahiSoundUtils: AHISoundUtils

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        viewModel = getBodyScanViewModel()
        ahiSoundUtils = AHISoundUtils(applicationContext = applicationContext)
        ahiMotionSensor = AHIMotionSensor(this)
        cameraCapture = CameraCapture()
        cameraCapture.setConfig(mapOf("capture_times" to 4))
        val ahiTheme = AHIThemes.getCurrentTheme(
            resources.configuration.uiMode and Configuration.UI_MODE_NIGHT_MASK == UI_MODE_NIGHT_YES
        )
        alignmentInfoBottomSheet = AHIBSAlignmentInfoBottomSheet(ahiTheme)
        phoneHeightTipsBottomSheet = AHIBSPhoneHeightTipsBottomSheet(ahiTheme)
        checkPreConditions()
        val motionStateFlow = ahiMotionSensor.deviceMotion()
        lifecycleScope.launch {
            motionStateFlow.collectLatest { motionData ->
                motionData?.let { it -> viewModel.motionData.value = it }
            }
        }

        setContent {
            MainLayout(viewModel)
        }

        viewModel.scanState.observe(this) { state ->
            captureStateJob?.cancel()
            when (state) {
                BodyScanState.POSITIONING -> {
                    captureStateJob = lifecycleScope.launch {
                        viewModel.positioning(this@DefaultBodyScanCaptureActivity)
                    }
                }

                BodyScanState.FRONT_CAPTURE -> {
                    captureStateJob = lifecycleScope.launch {
                        viewModel.frontCapture(this@DefaultBodyScanCaptureActivity, cameraCapture)
                    }
                }

                BodyScanState.SIDE_CAPTURE -> {
                    captureStateJob = lifecycleScope.launch {
                        viewModel.sideCapture(this@DefaultBodyScanCaptureActivity, cameraCapture)
                    }
                }

                else -> {}
            }
        }

        alignmentCompleteTimer = object : CountDownTimer(COUNTDOWN_DURATION.toLong(), 1) {
            override fun onTick(millisUntilFinished: Long) {
                viewModel.alignmentTextVisibility = millisUntilFinished > 2000
                if (millisUntilFinished > 0) {
                    viewModel.alignmentCompleteProgress = millisUntilFinished.toFloat() / COUNTDOWN_DURATION
                }
            }

            override fun onFinish() {
                lifecycleScope.launch {
                    delay(500)
                    viewModel.alignmentCompleteProgress = 0F
                }
                if (viewModel.scanState.value == BodyScanState.ALIGNMENT) {
                    if (alignmentInfoBottomSheet.isVisible) {
                        alignmentInfoBottomSheet.dismiss()
                    }
                    viewModel.progressToNextStage()
                }
            }
        }
        lifecycleScope.launch {
            viewModel.isTriggerAlignedSound.collectLatest {
                if (it) {
                    ahiSoundUtils.play(AHISounds.SOUND_ALIGNMENT)
                }
            }
        }

        lifecycleScope.launch {
            viewModel.isTriggerCaptureSound.collectLatest {
                if (it) {
                    ahiSoundUtils.play(AHISounds.SOUND_CAPTURE)
                }
            }
        }

        viewModel.motionData.observe(this) {
            if (viewModel.alignmentCompleteProgress == 0F && viewModel.scanState.value == BodyScanState.ALIGNMENT && ahiMotionSensor.isDeviceMotionActive() && ahiMotionSensor.isDeviceAligned()) {
                lifecycleScope.launch(Dispatchers.Main) {
                    withContext(Dispatchers.IO) {
                        delay(500)
                    }

                    if (viewModel.alignmentCompleteProgress == 0F && viewModel.scanState.value == BodyScanState.ALIGNMENT && ahiMotionSensor.isDeviceMotionActive() && ahiMotionSensor.isDeviceAligned()) {
                        viewModel.setTriggerAlignSound(true)
                        alignmentCompleteTimer.cancel()
                        alignmentCompleteTimer.start()

                    }
                }
            } else if (!ahiMotionSensor.isDeviceAligned()) {
                viewModel.setTriggerAlignSound(false)
                if (viewModel.scanState.value == BodyScanState.ALIGNMENT) {
                    alignmentCompleteTimer.cancel()
                    viewModel.alignmentCompleteProgress = 0F
                    viewModel.alignmentTextVisibility = true
                } else if (viewModel.scanState.value != BodyScanState.CRITICAL_ERROR) {
                    if (alignmentCheckJob?.isActive != true) {
                        alignmentCheckJob?.cancel()
                        alignmentCheckJob = lifecycleScope.launch(Dispatchers.Main) {
                            delay(2000)
                            if (!ahiMotionSensor.isDeviceAligned()) {
                                viewModel.scanState.value = BodyScanState.ALIGNMENT
                            }
                        }
                    }
                }
            }
        }

        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                viewModel.finishedWithError(BodyScanError.BODY_SCAN_CANCELED)
            }
        })
    }

    override fun onResume() {
        super.onResume()
        ahiMotionSensor.start()
    }

    override fun onPause() {
        super.onPause()
        ahiMotionSensor.stop()
    }

    @SuppressLint("UnsafeOptInUsageError")
    @Composable
    private fun MainLayout(viewModel: BodyScanCaptureViewModel) {
        val ahiTheme = AHIThemes.getCurrentTheme(isSystemInDarkTheme())
        val ahiThemeState by remember {
            mutableStateOf(ahiTheme)
        }
        val captureState = viewModel.scanState.observeAsState()
        val motionData = viewModel.motionData.observeAsState()
        val criticalErrorState = viewModel.criticalErrorState.observeAsState()
        val inspectionInfo = viewModel.poseValidator.poseInspection.lastInspectionResultDetails.observeAsState()
        AHIBSTheme {
            Box(
                modifier = Modifier.fillMaxSize()
            ) {
                AHIBSLayout(infoAction = {
                    if (!alignmentInfoBottomSheet.isVisible) {
                        alignmentInfoBottomSheet.show(supportFragmentManager, AHIBSAlignmentInfoBottomSheet.TAG)
                    }
                }, closeAction = {
                    viewModel.finishedWithError(BodyScanError.BODY_SCAN_CANCELED)
                }) {
                    Box(
                        modifier = Modifier.fillMaxSize()
                    ) {
                        AHIBSCameraPreview(
                            previewUseCase = cameraCapture.previewUseCase,
                            modifier = Modifier
                                .fillMaxSize()
                                .clip(RoundedCornerShape(corner = CornerSize(60.dp)))
                        )
                        Crossfade(
                            targetState = captureState.value, animationSpec = tween(durationMillis = 300)
                        ) { captureState ->
                            when (captureState) {
                                BodyScanState.ALIGNMENT -> AHIBSAlignment(
                                    this@DefaultBodyScanCaptureActivity,
                                    ahiThemeState,
                                    angle = motionData.value?.angle ?: 45.0,
                                    ahiMotionSensor.isDeviceAligned(),
                                    viewModel.alignmentCompleteProgress,
                                    viewModel.alignmentTextVisibility,
                                    alignmentInfoBottomSheet
                                )

                                BodyScanState.POSITIONING -> {
                                    AHIBSPositioning(
                                        modifier = Modifier.fillMaxSize(),
                                        ahiTheme = ahiThemeState,
                                        state = viewModel.positioningState,
                                        viewModel.positioningStateVisibility
                                    )
                                }

                                BodyScanState.FRONT_CAPTURE, BodyScanState.SIDE_CAPTURE -> {
                                    if (viewModel.captureTimer in 1..3) {
                                        ahiSoundUtils.play(AHISounds.SOUND_COUNTDOWN)
                                    }
                                    AHIBSCapture(
                                        modifier = Modifier.fillMaxSize(),
                                        ahiTheme = ahiThemeState,
                                        state = viewModel.captureState,
                                        timer = viewModel.captureTimer,
                                        contourPoints = viewModel.contour,
                                    )
                                }

                                BodyScanState.CRITICAL_ERROR -> {
                                    criticalErrorState.value?.let {
                                        AHIBSCriticalError(modifier = Modifier.fillMaxSize(), state = it, button1Action = {
                                            viewModel.scanState.value = BodyScanState.ALIGNMENT
                                        }, button2Action = {
                                            when (it) {
                                                CriticalErrorState.PHONE_TOO_HIGH, CriticalErrorState.PHONE_TOO_LOW,
                                                CriticalErrorState.PHONE_TOO_HIGH_INDETERMINATE, CriticalErrorState.PHONE_TOO_LOW_INDETERMINATE -> {
                                                    phoneHeightTipsBottomSheet.show(
                                                        supportFragmentManager, AHIBSPhoneHeightTipsBottomSheet.TAG
                                                    )
                                                }

                                                else -> {
                                                    // Placeholder, might change
                                                    phoneHeightTipsBottomSheet.show(
                                                        supportFragmentManager, AHIBSPhoneHeightTipsBottomSheet.TAG
                                                    )
                                                }
                                            }
                                        })
                                    }
                                }

                                else -> {

                                }
                            }
                        }

                        if (viewModel.isDebug && captureState.value != BodyScanState.ALIGNMENT) {
                            val mInspectionInfo =
                                if (captureState.value == BodyScanState.FRONT_CAPTURE || captureState.value == BodyScanState.SIDE_CAPTURE) {
                                    inspectionInfo.value ?: mapOf()
                                } else {
                                    mapOf()
                                }
                            val mCriticalErrorDebugTimes =
                                if (captureState.value == BodyScanState.POSITIONING || captureState.value == BodyScanState.CRITICAL_ERROR) {
                                    viewModel.criticalErrorDebugTimes
                                } else {
                                    listOf()
                                }
                            AHIBSDebug(
                                arrayOf(),
                                viewModel.poseJoints,
                                viewModel.optimalZones,
                                viewModel.poseValidator.isUserInOptimalZoneHead,
                                viewModel.poseValidator.isUserInOptimalZoneAnkles,
                                mInspectionInfo,
                                mCriticalErrorDebugTimes
                            )
                        }
                    }
                }
                if (viewModel.showFlash) {
                    AHIBSFlash(
                        ahiTheme = ahiThemeState, modifier = Modifier.fillMaxSize()
                    )
                }
            }
        }

        LaunchedEffect(true) {
            val cameraStart = cameraCapture.startCamera(
                this@DefaultBodyScanCaptureActivity, this@DefaultBodyScanCaptureActivity, lifecycleScope, CameraSelector.DEFAULT_FRONT_CAMERA
            ) {
                val image = it.image
                if (image != null) {
                    viewModel.detectPose(image, it.imageInfo.rotationDegrees)
                }
            }
            if (!cameraStart) {
                viewModel.finishedWithError(BodyScanError.BODY_SCAN_CAMERA_START_FAILURE)
            }
        }
    }

    private fun checkPreConditions() {
        if (!ahiMotionSensor.isDeviceMotionAvailable()) {
            viewModel.finishedWithError(BodyScanError.BODY_SCAN_MOTION_SENSOR_ERROR_UNAVAILABLE)
        }
        if (!hasCameraPermission()) {
            viewModel.finishedWithError(BodyScanError.BODY_SCAN_CAMERA_PERMISSION_ERROR)
        }
    }

    private fun hasCameraPermission(): Boolean = ContextCompat.checkSelfPermission(
        this, Manifest.permission.CAMERA
    ) == PackageManager.PERMISSION_GRANTED
}
