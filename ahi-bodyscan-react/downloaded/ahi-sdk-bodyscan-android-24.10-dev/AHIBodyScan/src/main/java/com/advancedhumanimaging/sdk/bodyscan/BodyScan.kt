//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan

import android.net.Uri
import androidx.activity.result.ActivityResultRegistry
import androidx.lifecycle.LiveData
import androidx.lifecycle.Observer
import androidx.work.WorkInfo
import androidx.work.WorkManager
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSScanName
import com.advancedhumanimaging.sdk.bodyscan.common.AHIBSVersion
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.helpers.BodyScanRemoteAssets
import com.advancedhumanimaging.sdk.bodyscan.keys.AHIKeysUtil
import com.advancedhumanimaging.sdk.bodyscan.partinversion.Inversion
import com.advancedhumanimaging.sdk.bodyscan.partresources.Resources
import com.advancedhumanimaging.sdk.common.AHILicense
import com.advancedhumanimaging.sdk.common.AHIResultSmoothing
import com.advancedhumanimaging.sdk.common.IAHIMultiScan
import com.advancedhumanimaging.sdk.common.IAHIScan
import com.advancedhumanimaging.sdk.common.models.AHIKey
import com.advancedhumanimaging.sdk.common.models.AHILogLevel
import com.advancedhumanimaging.sdk.common.models.AHILogging
import com.advancedhumanimaging.sdk.common.models.AHIResult
import com.advancedhumanimaging.sdk.common.uidelegation.AHIScanCaptureResultContract
import com.advancedhumanimaging.sdk.common.uidelegation.ScanFinishedPendingResults
import com.advancedhumanimaging.sdk.common.uidelegation.ScanFinishedState
import com.advancedhumanimaging.sdk.common.uidelegation.ScanFinishedWithError
import com.advancedhumanimaging.sdk.common.uidelegation.ScanFinishedWithResults
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.util.Base64
import java.util.UUID
import java.util.concurrent.CompletableFuture
import kotlin.coroutines.suspendCoroutine
import kotlin.math.max
import kotlin.math.min

class BodyScan : IAHIScan {

    private lateinit var resultsLiveData: LiveData<WorkInfo>
    private lateinit var resultsObserver: Observer<WorkInfo>
    private lateinit var remoteAssets: BodyScanRemoteAssets
    private lateinit var ahiMultiScan: IAHIMultiScan
    private var uid: String = ""
    private var error: BodyScanError? = null
    private val multiScanImageName = "ahi-logo-multiscan.png"
    private val delimiters = "AHIKEY:"

    internal companion object {
        const val RESULTS_WORKER_NAME = "body-scan-results"
    }

    private val decryptionKey: String
        get() {
            val appContext =
                ahiMultiScan.getApplicationContext().getOrNull() ?: return ""
            var key = ""
            val ahiLicenceKey: AHIKey = try {
                AHIKeysUtil(multiScanImageName, delimiters).keyAHILicenseToken(appContext)
            } catch (exception: Exception) {
                AHIKey("", "")
            }
            ahiMultiScan.getDetails {
                it.getOrNull()?.let { details ->
                    val configKey = AHILicense.getScanConfigFromDetails(details, ahiLicenceKey)?.optString("key")
                    val configCloud = details["config"] as? Map<String, Any>
                    val secretKey = configCloud?.get("sec") as? String
                    if (configKey != null && secretKey != null) {
                        val keyData = AHILicense.decryptConfigKey(configKey, secretKey)
                        key = String(Base64.getEncoder().encode(keyData))
                    }
                }
            }
            return key
        }
    var activityToDisplay: () -> BodyScanCaptureActivity = {
        DefaultBodyScanCaptureActivity()
    }

    override fun areResourcesDownloaded(): Boolean {
        if (::remoteAssets.isInitialized) {
            return remoteAssets.areResourcesDownloaded()
        }
        return false
    }

    override suspend fun authorize(userId: String): AHIResult<Unit> {
        uid = userId
        return AHIResult.success()
    }

    override suspend fun initiateScan(
        options: Map<String, Any>, registry: ActivityResultRegistry,
    ): CompletableFuture<AHIResult<Map<String, Any>>> = withContext(Dispatchers.Main) {
        // cloud feature enabled collector
        val featureCollector = mutableMapOf<String, Boolean>()
        var accuracyThreshold = 0.94f
        // Here we collection each feature configs.
        // Put feature: classification to "useAverage", default set as true.
        val useAverageOnClassification = ahiMultiScan.featureEnabledFor(scan = "body", key = "useAverage", defaults = true)
        val useFastHeightValidation = ahiMultiScan.featureEnabledFor(scan = "body", key = "useFastHeightValidation", defaults = true)
        featureCollector["useAverage"] = useAverageOnClassification
        featureCollector["useFastHeightValidation"] = useFastHeightValidation
        // Put feature: configurable green zone.
        val accuracyMapFromCloud = ahiMultiScan.featureFor(scan = "body", key = "threshold", defaults = mapOf("threshold" to 0.94))

        // Here we check the accuracyMapFromCloud map that it contains "threshold" key.
        if (accuracyMapFromCloud.isNotEmpty() && accuracyMapFromCloud.containsKey("threshold")) {
            // If value is null then set default value as 0.94
            val cloudThresholdValue = accuracyMapFromCloud["threshold"] as? Float ?: 0.94f
            // Calculate threshold value
            accuracyThreshold = min(max(cloudThresholdValue, 0.92f), 0.97f)
        }

        var initiatedViaMultiScan = false
        for (trace in Thread.currentThread().stackTrace) {
            if (trace.className.contains("com.advancedhumanimaging.sdk.multiscan.capture") && trace.methodName == "onInitiateScan" && trace.fileName == "ScanCaptureManager.kt") {
                initiatedViaMultiScan = true
                break
            }
        }
        if (!initiatedViaMultiScan) {
            AHILogging.log(AHILogLevel.ERROR, "initiateScan NOT clalled from multiscan");
            return@withContext CompletableFuture.completedFuture(AHIResult.Companion.failure(BodyScanError.BODY_SCAN_FAILED_DIRECT_SCAN_INITIATED));
        }

        if (!areResourcesDownloaded()) {
            AHILogging.log(
                AHILogLevel.ERROR, "BodyScan failed due to resources not downloaded"
            )
            return@withContext CompletableFuture.completedFuture(AHIResult.failure(BodyScanError.BODY_SCAN_RESOURCES_NOT_DOWNLOADED))
        }
        val validInputs = validateScanInputs(options) ?: return@withContext CompletableFuture.completedFuture(AHIResult.failure(error!!))
        // override featuresEnabled by feature collector.
        validInputs.featuresEnabled = featureCollector
        validInputs.accuracyThreshold = accuracyThreshold

        val activityResult = getResultFromActivity(registry, validInputs)
        return@withContext handleCaptureActivityFinished(activityResult)
    }

    private suspend fun getResultFromActivity(
        registry: ActivityResultRegistry, validInputs: BodyScanValidInput,
    ): ScanFinishedState {
        return suspendCoroutine { continuation ->
            // launch activity and register result listener
            registry.register(
                "BodyScanCapture", AHIScanCaptureResultContract(activityToDisplay()::class.java)
            ) {
                kotlin.runCatching {
                    continuation.resumeWith(Result.success(it))
                }
            }.launch(validInputs)
        }
    }

    private suspend fun observeWorkerForScanResults(
        workerId: UUID,
    ): CompletableFuture<AHIResult<Map<String, Any>>> {
        val completableFuture = CompletableFuture<AHIResult<Map<String, Any>>>()
        if (::resultsObserver.isInitialized && ::resultsLiveData.isInitialized) {
            resultsLiveData.removeObserver(resultsObserver)
        }
        val appContext = ahiMultiScan.getApplicationContext().getOrNull()
        if (appContext == null) {
            completableFuture.complete(AHIResult.failure(BodyScanError.BODY_SCAN_SETUP_NOT_DONE))
        } else {
            resultsLiveData = WorkManager.getInstance(appContext).getWorkInfoByIdLiveData(workerId)
            resultsObserver = Observer {
                if (it == null) return@Observer
                when (it.state) {
                    WorkInfo.State.SUCCEEDED -> {
                        completableFuture.complete(AHIResult.success(it.outputData.keyValueMap))
                    }

                    WorkInfo.State.CANCELLED -> {
                        completableFuture.complete(AHIResult.failure(BodyScanError.BODY_SCAN_WORKER_CANCELED))
                    }

                    WorkInfo.State.FAILED -> {
                        val errorCode = it.outputData.getInt("error_code", -1)
                        val error = if (errorCode != -1) {
                            BodyScanError.getError(errorCode)
                        } else {
                            BodyScanError.BODY_SCAN_WORKER_FAILED
                        }
                        completableFuture.complete(AHIResult.failure(error))
                    }

                    else -> {}
                }
            }
            resultsLiveData.observeForever(resultsObserver)
        }
        return completableFuture
    }

    private suspend fun handleCaptureActivityFinished(
        scanFinishedState: ScanFinishedState,
    ): CompletableFuture<AHIResult<Map<String, Any>>> {
        val scanResults: CompletableFuture<AHIResult<Map<String, Any>>> = when (scanFinishedState) {
            is ScanFinishedWithError -> {
                CompletableFuture.completedFuture(AHIResult.failure(BodyScanError.getError(scanFinishedState.errorCode)))
            }

            is ScanFinishedWithResults -> {
                CompletableFuture.completedFuture(AHIResult.success(scanFinishedState.results))
            }

            is ScanFinishedPendingResults -> {
                observeWorkerForScanResults(scanFinishedState.workerId)
            }
        }
        return smoothenScanResults(scanResults)
    }

    private fun smoothenScanResults(scanResults: CompletableFuture<AHIResult<Map<String, Any>>>): CompletableFuture<AHIResult<Map<String, Any>>> =
        scanResults.thenApply { results ->
            val resultsMap = results.getOrNull() ?: return@thenApply results
            val smoothedResults = getSmoothedClassificationResults(resultsMap)
            return@thenApply AHIResult.success(smoothedResults)
        }

    private fun validateScanInputs(options: Map<String, Any>): BodyScanValidInput? {
        val sex = (options["enum_ent_sex"] as? String)?.let {
            try {
                SexType.valueOf(it)
            } catch (e: Exception) {
                null
            }
        }
        if (sex == null) {
            AHILogging.log(AHILogLevel.ERROR, "Sex input is invalid or missing")
            return null
        }
        val height = options["cm_ent_height"].toString().toDoubleOrNull()
        if (height == null || height !in 50.0..255.0) {
            AHILogging.log(AHILogLevel.ERROR, "Height input is invalid or missing")
            error = BodyScanError.BODY_SCAN_INPUT_HEIGHT_OUT_OF_RANGE
            return null
        }
        val weight = options["kg_ent_weight"].toString().toDoubleOrNull()
        if (weight == null || weight !in 16.0..300.0) {
            AHILogging.log(AHILogLevel.ERROR, "Weight input is invalid or missing")
            error = BodyScanError.BODY_SCAN_INPUT_WEIGHT_OUT_OF_RANGE
            return null
        }
        val isDebug = options["debug_isDebug"] as? Boolean ?: false
        return BodyScanValidInput(height, weight, sex, uid, decryptionKey, isDebug)
    }

    private fun getSmoothedClassificationResults(classificationResults: Map<String, Any>): Map<String, Any> {
        val previousResults = getPreviousResults() ?: return classificationResults
        val smoothedResults = mutableMapOf<String, Any>()

        // for each raw value, we get the current and previous values and use this to calculate a smoothed (or adjusted value)
        classificationResults.forEach { (key, value) ->
            val currentValue =
                if (key.contains("_raw_")) {
                    value.toString().toDoubleOrNull()
                } else {
                    null
                }
            if (currentValue != null) {
                val previousValues = mapPreviousResultsForKey(key, previousResults)
                val smoothedValue = AHIResultSmoothing.smoothResultsWithCurrentValue(currentValue, previousValues)
                val adjustedKey = key.replace("_raw_", "_adj_")
                smoothedResults[adjustedKey] = smoothedValue
            }
        }

        return classificationResults + smoothedResults
    }

    private fun mapPreviousResultsForKey(key: String, previousResults: Array<Map<String, Any>>): Map<Long, Double> {
        val resultsMap = mutableMapOf<Long, Double>()
        previousResults.forEach { previousResult ->
            val date = previousResult["date"].toString().toLongOrNull()
            val value = previousResult[key].toString().toDoubleOrNull()
            if (date != null && value != null) {
                resultsMap[date] = value
            }
        }
        return resultsMap
    }

    private fun getPreviousResults(): Array<Map<String, Any>>? {
        return try {
            var previousResults: Array<Map<String, Any>>? = null
            val persistence = ahiMultiScan.getPersistenceDelegate() ?: return null
            persistence.request("body", mapOf()) {
                when (it) {
                    is AHIResult.Failure -> {
                        AHILogging.log(AHILogLevel.ERROR, "Could not get previous results (error code: ${it.errorCode()})")
                    }

                    is AHIResult.Success -> {
                        previousResults = it.value
                    }
                }
            }
            previousResults
        } catch (e: Exception) {
            AHILogging.log(AHILogLevel.ERROR, "Could not get previous results")
            null
        }
    }

    override fun scanName(): String = AHIBSScanName

    override fun version(): String {
        return AHIBSVersion
    }

    override fun billingOffset(): Int = 200

    override fun currentDownloadedSizeInBytes(): Long {
        if (::remoteAssets.isInitialized) {
            return remoteAssets.currentDownloadedSizeInBytes()
        }
        return 0
    }

    override fun downloadResourcesInBackground(downloadRetryCount: Int): AHIResult<Unit> {
        if (!::remoteAssets.isInitialized) {
            AHILogging.log(
                AHILogLevel.ERROR, "BodyScan: Failed to initiate downloadResources; setup is not done"
            )
            return AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_ASSET_BACKGROUND_NOT_INIT)
        }
        return remoteAssets.initiateDownloadResources(true, downloadRetryCount)
    }

    override fun downloadResourcesInForeground(downloadRetryCount: Int): AHIResult<Unit> {
        if (!::remoteAssets.isInitialized) {
            AHILogging.log(
                AHILogLevel.ERROR, "BodyScan: Failed to initiate downloadResources; setup is not done"
            )
            return AHIResult.failure(BodyScanError.BODY_SCAN_REMOTE_ASSET_NOT_INIT)
        }
        return remoteAssets.initiateDownloadResources(false, downloadRetryCount)
    }

    override suspend fun getScanExtra(
        scanResult: Map<String, Any>, options: Map<String, Any>,
    ): AHIResult<Map<String, Any>> {
        if (scanResult["type"] != AHIBSScanName || options.isEmpty()) {
            AHILogging.log(
                AHILogLevel.ERROR, "BodyScan failed to get scan extras due to missing params"
            )
            return AHIResult.failure(BodyScanError.BODY_SCAN_MISSING_SCAN_EXTRA_PARAMS)
        }
        if (!areResourcesDownloaded()) {
            AHILogging.log(
                AHILogLevel.ERROR, "BodyScan failed to get scan extras due to resources not downloaded"
            )
            return AHIResult.failure(BodyScanError.BODY_SCAN_SCANEXTRA_RESOURCES_NOT_DOWNLOADED)
        }
        val isMeshRequested = (options["extrapolate"] as? List<*>)?.contains("mesh") == true
        return if (isMeshRequested) {
            val meshResult = generateMesh(scanResult)
            val meshUri = meshResult.getOrNull() ?: return AHIResult.failure(BodyScanError.getError(meshResult.errorCode()))
            AHIResult.success(mapOf("extrapolate" to listOf(mapOf("mesh" to meshUri))))
        } else {
            AHIResult.success(mapOf())
        }
    }

    override suspend fun initiateHeadlessScan(options: Map<String, Any>, registry: ActivityResultRegistry): CompletableFuture<AHIResult<Map<String, Any>>> {
        return CompletableFuture.completedFuture(AHIResult.failure(BodyScanError.HEADLESS_NOT_YET_SUPPORTED))
    }

    private suspend fun generateMesh(scanResult: Map<String, Any>): AHIResult<Uri> {
        val sex = (scanResult["enum_ent_sex"] as? String)?.let {
            try {
                SexType.valueOf(it)
            } catch (e: Exception) {
                null
            }
        }
        val heightCm = scanResult["cm_ent_height"].toString().toDoubleOrNull()
        val weightKG = scanResult["kg_ent_weight"].toString().toDoubleOrNull()
        val chestCM = (scanResult["cm_adj_chest"] ?: scanResult["cm_raw_chest"]).toString().toDoubleOrNull()
        val waistCM = (scanResult["cm_adj_waist"] ?: scanResult["cm_raw_waist"]).toString().toDoubleOrNull()
        val hipCM = (scanResult["cm_adj_hips"] ?: scanResult["cm_raw_hips"]).toString().toDoubleOrNull()
        val inseamCM = (scanResult["cm_adj_inseam"] ?: scanResult["cm_raw_inseam"]).toString().toDoubleOrNull()
        val fitness = (scanResult["ml_gen_fitness"] ?: scanResult["ml_adj_fitness"] ?: scanResult["ml_raw_fitness"]).toString().toDoubleOrNull()
        if (sex == null || heightCm == null || weightKG == null || chestCM == null || waistCM == null || hipCM == null || inseamCM == null || fitness == null) {
            AHILogging.log(
                AHILogLevel.ERROR, "BodyScan failed to generate mesh (scan extras) due to missing inputs"
            )
            return AHIResult.failure(BodyScanError.BODY_SCAN_MISSING_MESH_GENERATION_INPUTS_FROM_SCAN_EXTRA_INPUTS)
        }
        val multiScanAppContext = ahiMultiScan.getApplicationContext()
        val appContext =
            multiScanAppContext.getOrNull() ?: return AHIResult.failure(multiScanAppContext.error() ?: BodyScanError.BODY_SCAN_MESH_SETUP_NOT_DONE)
        val name = scanResult["id"] as? String ?: ""
        return Inversion().invert(
            name,
            sex,
            heightCm,
            weightKG,
            chestCM,
            waistCM,
            hipCM,
            inseamCM,
            fitness,
            appContext,
            Resources(decryptionKey)
        )
    }

    override fun setMultiScan(multiScan: IAHIMultiScan) {
        ahiMultiScan = multiScan
        remoteAssets = BodyScanRemoteAssets(ahiMultiScan)
    }

    override suspend fun setup(): AHIResult<Unit> {
        return AHIResult.success()
    }

    override fun totalRemoteAssetsSizeInBytes(): Long {
        if (!::remoteAssets.isInitialized) {
            AHILogging.log(
                AHILogLevel.ERROR, "BodyScan: Failed to calculate totalRemoteAssetsSizeInBytes: setup is not done"
            )
            return 0
        }
        return remoteAssets.totalRemoteAssetsSizeInBytes()
    }

    override suspend fun release(): AHIResult<Unit> {
        return AHIResult.success()
    }
}
