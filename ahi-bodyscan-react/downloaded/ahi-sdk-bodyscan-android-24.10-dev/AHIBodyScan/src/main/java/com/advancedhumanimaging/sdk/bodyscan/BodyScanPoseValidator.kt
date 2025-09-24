package com.advancedhumanimaging.sdk.bodyscan

import android.graphics.Bitmap
import android.graphics.PointF
import android.graphics.RectF
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSOptimalContourZone
import com.advancedhumanimaging.sdk.bodyscan.common.InspectionResult
import com.advancedhumanimaging.sdk.bodyscan.common.Profile
import com.advancedhumanimaging.sdk.bodyscan.common.utils.getProminentPersonJoints
import com.advancedhumanimaging.sdk.bodyscan.partcontour.ContourGenerator
import com.advancedhumanimaging.sdk.bodyscan.partposeinspection.PoseInspection
import com.advancedhumanimaging.sdk.bodyscan.partui.screens.CriticalErrorState
import com.advancedhumanimaging.sdk.bodyscan.partui.screens.PositioningState
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.util.Date

class BodyScanPoseValidator(
    private val useFastHeightValidation: Boolean,
) {
    val poseInspection = PoseInspection()
    private val placeHolderBitmap: Bitmap = Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888)
    private val criticalErrorTimes = mutableMapOf<CriticalErrorState, Long>()
    var isUserInOptimalZoneHead by mutableStateOf(false)
    var isUserInOptimalZoneAnkles by mutableStateOf(false)
    private val contourGenerator = ContourGenerator()

    private fun updateCriticalErrorTime(criticalErrorState: CriticalErrorState) {
        criticalErrorTimes[criticalErrorState] = Date().time
    }

    fun getCriticalErrorDuration(criticalErrorState: CriticalErrorState): Long {
        return criticalErrorTimes[criticalErrorState]?.let { Date().time - it } ?: 0
    }

    fun resetCriticalErrorTimes() {
        CriticalErrorState.values().forEach { updateCriticalErrorTime(it) }
    }

    suspend fun checkPosition(
        contourMask: Bitmap, optimalZones: Map<String, RectF>, poseJoints: Map<String, List<Map<String, PointF>>>
    ): PositioningState {
        return withContext(Dispatchers.IO) {
            val singleJoints = getProminentPersonJoints(poseJoints)
            val areHeadAndAnklesDetected =
                singleJoints.filterKeys { it in listOf("CentroidHeadTop", "CentroidRightAnkle", "CentroidLeftAnkle") }.size == 3
            val isOnePersonOnly = isOnePersonOnly(contourMask, optimalZones, Profile.front, poseJoints)
            val headArea = optimalZones[AHIBSOptimalContourZone.HEAD.key]
            isUserInOptimalZoneHead = headArea?.let {
                contourGenerator.isUserInOptimalZone(
                    AHIBSOptimalContourZone.HEAD.key, it, singleJoints
                )
            } == true
            val anklesArea = optimalZones[AHIBSOptimalContourZone.ANKLES.key]
            isUserInOptimalZoneAnkles = anklesArea?.let {
                contourGenerator.isUserInOptimalZone(
                    AHIBSOptimalContourZone.ANKLES.key, it, singleJoints
                )
            } == true

            val isFaceMissing = !singleJoints.containsKey("CentroidHeadTop")
            val isAnkleMissing = !singleJoints.keys.containsAll(listOf("CentroidRightAnkle", "CentroidLeftAnkle"))

            var shouldComeCloser = false
            var isPhoneTooHigh = false
            var isPhoneTooLow = false
            var isPhoneTooHighIndeterminate = false
            var isPhoneTooLowIndeterminate = false
            if (headArea != null && anklesArea != null) {
                val bottomOfHeadZoneBuffer = headArea.bottom + 50
                val topOfAnklesZoneBuffer = anklesArea.top - 50
                val pointHeadTop = singleJoints["CentroidHeadTop"]
                val pointAnkles = getAverageAnklePoints(singleJoints["CentroidRightAnkle"], singleJoints["CentroidLeftAnkle"])
                val bottomOfMiddleZoneBuffer = ((anklesArea.bottom - headArea.bottom) / 2) + headArea.bottom
                val topOfMiddleZoneBuffer = ((anklesArea.top - headArea.top) / 2) + headArea.top

                val hasHeadTop = pointHeadTop != null
                val hasAnkles = pointAnkles != null
                val headBelowZoneBuffer = pointHeadTop?.y?.let { it > bottomOfHeadZoneBuffer } == true
                val headBelowZone = pointHeadTop?.y?.let { it > headArea.bottom } == true
                val anklesBelowHeadZone = pointAnkles?.y?.let { it > headArea.bottom } == true
                val anklesBelowZone = pointAnkles?.y?.let { it > anklesArea.bottom } == true
                val headAboveZone = pointHeadTop?.y?.let { it < headArea.top } == true
                val headAboveZoneBuffer = pointHeadTop?.y?.let { it < bottomOfHeadZoneBuffer } == true
                val anklesAboveZoneBuffer = pointAnkles?.y?.let { it < topOfAnklesZoneBuffer } == true
                val anklesBelowZoneBuffer = pointAnkles?.y?.let { it > topOfAnklesZoneBuffer } == true
                val anklesAboveZone = pointAnkles?.y?.let { it < anklesArea.top } == true
                val anklesAboveInverseZoneBuffer = pointAnkles?.y?.let { it < anklesArea.top + 50 } == true

                // Commented because cases 2 and 3 are not valid and will never trigger
//                val shouldComeCloserCase1 = headBelowZone && anklesAboveZone
//                val shouldComeCloserCase2 = isUserInOptimalZoneHead && anklesAboveZone && isUserInOptimalZoneAnkles && !anklesBelowZoneBuffer
//                val shouldComeCloserCase3 = headBelowZone && isUserInOptimalZoneHead && headAboveZoneBuffer && isUserInOptimalZoneAnkles
//                val shouldComeCloserCase4 = headBelowZone && headAboveZoneBuffer && isUserInOptimalZoneAnkles
//                shouldComeCloser = shouldComeCloserCase1 || shouldComeCloserCase2 || shouldComeCloserCase3 || shouldComeCloserCase4

                //Come closer cases
                val shouldComeCloserCase1 = headBelowZone && anklesAboveZone
                val shouldComeCloserCase2 = isUserInOptimalZoneHead && anklesAboveZone
                val shouldComeCloserCase3 = headBelowZone && isUserInOptimalZoneAnkles
                shouldComeCloser = shouldComeCloserCase1 || shouldComeCloserCase2 || shouldComeCloserCase3

                // Phone too high cases
                val phoneTooHighCase1 = headBelowZoneBuffer && anklesBelowHeadZone
                val phoneTooHighCase2 = headBelowZone && !hasAnkles
                val phoneTooHighCase3 = headBelowZone && headBelowZoneBuffer && !isUserInOptimalZoneHead && isUserInOptimalZoneAnkles
                val phoneTooHighCase4 = headBelowZone && headBelowZoneBuffer && isUserInOptimalZoneAnkles
                val phoneTooHighCase5 = isUserInOptimalZoneHead && !hasAnkles
                val phoneTooHighCase6 = isUserInOptimalZoneHead && anklesBelowZone
                isPhoneTooHighIndeterminate =
                    phoneTooHighCase1 || phoneTooHighCase2 || phoneTooHighCase3 || phoneTooHighCase4 || phoneTooHighCase5 || phoneTooHighCase6

                // Phone too low cases
                val phoneTooLowCase1 = headAboveZone && anklesAboveZoneBuffer
                val phoneTooLowCase2 = !hasHeadTop && anklesAboveZoneBuffer
                val phoneTooLowCase3 = isUserInOptimalZoneHead && anklesAboveZone && anklesAboveInverseZoneBuffer
                val phoneTooLowCase4 = isUserInOptimalZoneHead && anklesAboveZone && !isUserInOptimalZoneAnkles
                val phoneTooLowCase5 = isUserInOptimalZoneHead && anklesAboveZone && anklesBelowZoneBuffer
                val phoneTooLowCase6 = !hasHeadTop && isUserInOptimalZoneAnkles
                val phoneTooLowCase7 = headAboveZone && isUserInOptimalZoneAnkles
                val phoneTooLowCase8 = !hasHeadTop && isUserInOptimalZoneAnkles
                isPhoneTooLowIndeterminate =
                    phoneTooLowCase1 || phoneTooLowCase2 || phoneTooLowCase3 || phoneTooLowCase4 || phoneTooLowCase5 || phoneTooLowCase6 || phoneTooLowCase7 || phoneTooLowCase8

                // For fast detection of phone too high and phone too low
                if (useFastHeightValidation) {
                    if (pointHeadTop != null && pointAnkles != null) {
                        val middle = ((pointAnkles.y - pointHeadTop.y) / 2) + pointHeadTop.y
                        if (middle < topOfMiddleZoneBuffer) {
                            isPhoneTooLow = true
                        } else if (middle > bottomOfMiddleZoneBuffer) {
                            isPhoneTooHigh = true
                        }
                    }
                }
            }

            // Critical errors checks
            // Update time if error is not detected
            if (!isFaceMissing) {
                updateCriticalErrorTime(CriticalErrorState.PERSON_MISSING)
            }
            if (!isAnkleMissing) {
                updateCriticalErrorTime(CriticalErrorState.ANKLES_MISSING)
            }
            if (!isPhoneTooHigh) {
                updateCriticalErrorTime(CriticalErrorState.PHONE_TOO_HIGH)
            }
            if (!isPhoneTooLow) {
                updateCriticalErrorTime(CriticalErrorState.PHONE_TOO_LOW)
            }
            if (!isPhoneTooHighIndeterminate) {
                updateCriticalErrorTime(CriticalErrorState.PHONE_TOO_HIGH_INDETERMINATE)
            }
            if (!isPhoneTooLowIndeterminate) {
                updateCriticalErrorTime(CriticalErrorState.PHONE_TOO_LOW_INDETERMINATE)
            }

            val positioningState = when {
                singleJoints.isEmpty() -> PositioningState.FIT_WHOLE_BODY
                !isOnePersonOnly -> PositioningState.ONE_PERSON_ONLY
                isUserInOptimalZoneHead && isUserInOptimalZoneAnkles -> PositioningState.PERFECT_SPOT
                !areHeadAndAnklesDetected -> PositioningState.MOVE_BACK
                shouldComeCloser -> PositioningState.MOVE_FORWARD
                else -> PositioningState.FIT_WHOLE_BODY
            }
            positioningState
        }
    }

    suspend fun isOnePersonOnly(
        contourMask: Bitmap, optimalZones: Map<String, RectF>, profile: Profile, poseJoints: Map<String, List<Map<String, PointF>>>
    ): Boolean {
        val inspectionResult = poseInspection.inspect(placeHolderBitmap, contourMask, optimalZones, profile, poseJoints).getOrNull() ?: mapOf()
        return inspectionResult.values.find { it == InspectionResult.falseMultipleFaces } == null
    }

    private fun getAverageAnklePoints(rightAnkle: PointF?, leftAnkle: PointF?): PointF? {
        return if (rightAnkle == null || leftAnkle == null) {
            rightAnkle ?: leftAnkle
        } else {
            PointF((rightAnkle.x + leftAnkle.x) / 2F, (rightAnkle.y + leftAnkle.y) / 2F)
        }
    }
}