//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.partinversion

import android.content.Context
import android.net.Uri
import com.advancedhumanimaging.sdk.bodyscan.common.BodyScanError
import com.advancedhumanimaging.sdk.bodyscan.common.SexType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.AHIBSResourceType
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IInversion
import com.advancedhumanimaging.sdk.bodyscan.common.interfaces.IResources
import com.advancedhumanimaging.sdk.common.models.AHILogLevel
import com.advancedhumanimaging.sdk.common.models.AHILogging
import com.advancedhumanimaging.sdk.common.models.AHIResult
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.io.FileWriter

class Inversion : IInversion {

    private val cvModels = mapOf(
        "MvnMu" to AHIBSResourceType.AHIBSResourceTypeVF,
        "AvgVerts" to AHIBSResourceType.AHIBSResourceTypeVF,
        "VertsInv" to AHIBSResourceType.AHIBSResourceTypeVF,
        "Ranges" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "Cov" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "SkV" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "BonW" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "BonWInv" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "Sv" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "SvInv" to AHIBSResourceType.AHIBSResourceTypeVVF,
        "Faces" to AHIBSResourceType.AHIBSResourceTypeVI,
        "FacesInv" to AHIBSResourceType.AHIBSResourceTypeVI,
        "LaplacianRings" to AHIBSResourceType.AHIBSResourceTypeVI,
        "LaplacianRingsAsVectors" to AHIBSResourceType.AHIBSResourceTypeVI
    )
    private val cvModelsGenderless = mapOf(
        "InvRightCalf" to AHIBSResourceType.AHIBSResourceTypeVI,
        "InvRightThigh" to AHIBSResourceType.AHIBSResourceTypeVI,
        "InvRightUpperArm" to AHIBSResourceType.AHIBSResourceTypeVI
    )
    private val cvModelsAll = cvModels + cvModelsGenderless

    private val cvModelsMapMale = mutableMapOf<String, Pair<ByteArray, Int>>()
    private val cvModelsMapFemale = mutableMapOf<String, Pair<ByteArray, Int>>()

    private suspend fun getCvModelsMap(
        context: Context,
        resources: IResources,
        sex: SexType
    ): Map<String, Pair<ByteArray, Int>> {
        return if (sex == SexType.male) {
            if (cvModelsMapMale.size != cvModelsAll.size) {
                cvModelsMapMale.clear()
                cvModelsAll.forEach { (name, resourceType) ->
                    val genderName = "male"
                    val isGenderless = cvModelsGenderless.containsKey(name)
                    val modelName = if (isGenderless) name else "${name}_${genderName}"
                    val buffer = resources.getResource(modelName, resourceType, context).getOrNull()
                    if (buffer != null) {
                        cvModelsMapMale[name] = Pair(buffer, buffer.size)
                    }
                }
            }
            cvModelsMapMale
        } else {
            if (cvModelsMapFemale.size != cvModelsAll.size) {
                cvModelsMapFemale.clear()
                cvModelsAll.forEach { (name, resourceType) ->
                    val genderName = "female"
                    val isGenderless = cvModelsGenderless.containsKey(name)
                    val modelName = if (isGenderless) name else "${name}_${genderName}"
                    val buffer = resources.getResource(modelName, resourceType, context).getOrNull()
                    if (buffer != null) {
                        cvModelsMapFemale[name] = Pair(buffer, buffer.size)
                    }
                }
            }
            cvModelsMapFemale
        }
    }

    override suspend fun invert(
        name: String,
        sex: SexType,
        heightCM: Double,
        weightKG: Double,
        chestCM: Double,
        waistCM: Double,
        hipCM: Double,
        inseamCM: Double,
        fitness: Double,
        context: Context,
        resources: IResources
    ): AHIResult<Uri> {
        return withContext(Dispatchers.IO) {
            if (name.isEmpty()) {
                AHILogging.log(AHILogLevel.ERROR, "Inversion failed due to no name provided")
                return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_INVERSION_NAME_MISSING)
            }
            if (heightCM <= 0 || weightKG <= 0) {
                AHILogging.log(AHILogLevel.ERROR, "Inversion failed due to invalid height or weight")
                return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_INVERSION_INVALID_HEIGHT_OR_WEIGHT)
            }
            val cvModelsMapMale = getCvModelsMap(context, resources, SexType.male)
            if (cvModelsMapMale.size != cvModelsAll.size) {
                AHILogging.log(AHILogLevel.ERROR, "Inversion failed due to some missing resources")
                return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_INVERSION_MISSING_CV_MODELS_MALE)
            }
            val cvModelsMapFemale = getCvModelsMap(context, resources, SexType.female)
            if (cvModelsMapFemale.size != cvModelsAll.size) {
                AHILogging.log(AHILogLevel.ERROR, "Inversion failed due to some missing resources")
                return@withContext AHIResult.failure(BodyScanError.BODY_SCAN_INVERSION_MISSING_CV_MODELS_FEMALE)
            }
            val meshData =
                InversionJNI.invert(sex, heightCM, weightKG, chestCM, waistCM, hipCM, inseamCM, fitness, cvModelsMapMale, cvModelsMapFemale)
            val file = File(context.filesDir, "$name.obj")
            val writer = FileWriter(file)
            writer.append(meshData)
            writer.flush()
            writer.close()
            if (meshData.isBlank()) {
                AHIResult.failure(BodyScanError.BODY_SCAN_INVERSION_FAILED_IN_INVERSION)
            } else {
                AHIResult.success(Uri.fromFile(file))
            }
        }
    }
}