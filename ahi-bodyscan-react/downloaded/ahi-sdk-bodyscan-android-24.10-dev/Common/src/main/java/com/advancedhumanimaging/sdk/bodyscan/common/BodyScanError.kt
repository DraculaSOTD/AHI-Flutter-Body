//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common

import com.advancedhumanimaging.sdk.common.models.AHIError

/**
 * AHIBodyScanError represents all error codes for BodyScan.
 * 2000-2999 = BodyScan error code range.
 */
enum class BodyScanError(private val code: Int) : AHIError {

    /** BodyScan was canceled*/
    BODY_SCAN_CANCELED(2000),

    /** BodyScan input height out of range ( 50-255 )*/
    BODY_SCAN_INPUT_HEIGHT_OUT_OF_RANGE(2001),

    /** BodyScan input weight out of range ( 16-300 )*/
    BODY_SCAN_INPUT_WEIGHT_OUT_OF_RANGE(2002),

    /** BodyScan initiate scan is called but MultiScan Setup is not done*/
    BODY_SCAN_SETUP_NOT_DONE(2003),

    /** BodyScan initiate download assets but BodyScan Setup is not done*/
    BODY_SCAN_REMOTE_ASSET_NOT_INIT(2004),

    /** BodyScan initiate download assets but files list not found*/
    BODY_SCAN_REMOTE_NO_ASSET_FILES_LIST(2005),

    /** BodyScan initiate download assets but not application context in MultiScan*/
    BODY_SCAN_REMOTE_NO_APP_CONTEXT(2006),

    /** BodyScan attempted to access an asset that could not be found*/
    BODY_SCAN_REMOTE_ASSET_NOT_FOUND(2007),

    /** BodyScan segmentation failed*/
    BODY_SCAN_SEGMENTATION_FAILED(2008),

    /** BodyScan segmentation failed as incorrect number of joints were provided*/
    BODY_SCAN_SEGMENTATION_MISSING_JOINTS(2009),

    /** BodyScan segmentation failed as capture image was not of the expected size*/
    BODY_SCAN_SEGMENTATION_INCORRECT_CAPTURE_RESOLUTION(2010),

    /** BodyScan segmentation failed as contour image was not of the expected size*/
    BODY_SCAN_SEGMENTATION_INCORRECT_CONTOUR_RESOLUTION(2011),

    /** BodyScan segmentation failed as model could not be retrieved from resources*/
    BODY_SCAN_SEGMENTATION_MODEL_MISSING(2012),

    /** BodyScan segmentation failed as model could not be loaded from disk*/
    BODY_SCAN_SEGMENTATION_MODEL_LOAD_FAILED(2013),

    /** BodyScan inversion failed as provided name was empty*/
    BODY_SCAN_INVERSION_NAME_MISSING(2014),

    /** BodyScan inversion failed as provided height, weight, or both were invalid*/
    BODY_SCAN_INVERSION_INVALID_HEIGHT_OR_WEIGHT(2015),

    /** BodyScan inversion failed in inverter*/
    BODY_SCAN_INVERSION_FAILED_IN_INVERSION(2016),

    /** BodyScan pose inspection failed*/
    BODY_SCAN_POSE_INSPECTION_FAILED(2017),

    /** BodyScan pose inspection failed as extreme points of the contour could not be obtained*/
    BODY_SCAN_POSE_INSPECTION_FAILED_TO_GET_EXTREME_POINTS(2018),

    /** BodyScan pose detection failed in face detect*/
    BODY_SCAN_POSE_DETECTION_FAILED_IN_FACE_DETECTION(2019),

    /** BodyScan pose detection failed in body detect*/
    BODY_SCAN_POSE_DETECTION_FAILED_IN_BODY_DETECTION(2020),

    /** BodyScan classification failed*/
    BODY_SCAN_CLASSIFICATION_FAILED(2021),

    /** BodyScan classification failed due to invalid height or weight*/
    BODY_SCAN_CLASSIFICATION_INVALID_HEIGHT_WEIGHT(2022),

    /** BodyScan classification failed due to capture images having invalid dimensions*/
    BODY_SCAN_CLASSIFICATION_INVALID_CAPTURE_IMAGE_DIMENSIONS(2023),

    /** BodyScan classification failed due to invalid pose joints provided*/
    BODY_SCAN_CLASSIFICATION_MISSING_JOINTS(2024),

    /** BodyScan classification failed due to no captures provided*/
    BODY_SCAN_CLASSIFICATION_INVALID_CAPTURE(2025),

    /** BodyScan classification failed due to some SVR models missing from resources*/
    BODY_SCAN_CLASSIFICATION_MISSING_SVR_MODELS(2026),

    /** BodyScan classification failed due to some ML models missing from resources*/
    BODY_SCAN_CLASSIFICATION_MISSING_ML_MODELS(2027),

    /** BodyScan inversion failed in inverter due to missing male CV models*/
    BODY_SCAN_INVERSION_MISSING_CV_MODELS_MALE(2028),

    /** BodyScan inversion failed in inverter due to missing female CV models*/
    BODY_SCAN_INVERSION_MISSING_CV_MODELS_FEMALE(2029),

    /** BodyScan attempt to access an asset failed due to missing decryption key*/
    BODY_SCAN_REMOTE_ASSET_DECRYPTION_KEY_NOT_PROVIDED(2030),

    /** BodyScan attempt to access an asset failed due to failed decryption*/
    BODY_SCAN_REMOTE_ASSET_DECRYPTION_FAILED(2031),

    /** BodyScan attempt to access an asset failed*/
    BODY_SCAN_REMOTE_ASSET_ACCESS_FAILED(2032),

    /** BodyScan failed due to failure to start camera*/
    BODY_SCAN_CAMERA_START_FAILURE(2033),

    /** BodyScan worker failed due to the number of front and side captures not being the same*/
    BODY_SCAN_WORKER_NUMBER_OF_FRONT_AND_SIDE_CAPTURES_NOT_THE_SAME(2034),

    /** BodyScan failed to generate ideal contour*/
    BODY_SCAN_CONTOUR_GENERATOR_FAILED_TO_GENERATE_IDEAL_CONTOUR(2035),

    /** BodyScan failed to generate optimal zones*/
    BODY_SCAN_CONTOUR_GENERATOR_FAILED_TO_GENERATE_OPTIMAL_ZONES(2036),

    /** BodyScan failed to generate ideal contour mask*/
    BODY_SCAN_CONTOUR_GENERATOR_FAILED_TO_GENERATE_IDEAL_CONTOUR_MASK(2037),

    /** BodyScan failed to generate scaled contour*/
    BODY_SCAN_CONTOUR_GENERATOR_FAILED_TO_GENERATE_SCALED_CONTOUR(2038),

    /** BodyScan failed to worker manager canceled*/
    BODY_SCAN_WORKER_CANCELED(2039),

    /** BodyScan failed to worker manager failed*/
    BODY_SCAN_WORKER_FAILED(2040),

    /** BodyScan failed to motion sensor unavailable*/
    BODY_SCAN_MOTION_SENSOR_ERROR_UNAVAILABLE(2041),

    /** BodyScan failed to camera permission not granted*/
    BODY_SCAN_CAMERA_PERMISSION_ERROR(2042),

    /** BodyScan worker failed due to missing user inputs*/
    BODY_SCAN_WORKER_MISSING_USER_INPUTS(2043),

    /** BodyScan failed to get scan extras due to missing params*/
    BODY_SCAN_MISSING_SCAN_EXTRA_PARAMS(2044),

    /** BodyScan failed due to resources not downloaded*/
    BODY_SCAN_RESOURCES_NOT_DOWNLOADED(2045),

    /** BodyScan failed to generate mesh as scan extras due to missing inputs*/
    BODY_SCAN_MISSING_MESH_GENERATION_INPUTS_FROM_SCAN_EXTRA_INPUTS(2046),

    /** BodyScan worker failed due to no capture data found at file location*/
    BODY_SCAN_WORKER_NO_CAPTURE_DATA_FOUND(2047),

    /** BodyScan failed due to failure to capture front pose*/
    BODY_SCAN_FRONT_CAPTURE_FAILURE(2048),

    /** BodyScan failed due to failure to capture side pose*/
    BODY_SCAN_SIDE_CAPTURE_FAILURE(2049),

    /** BodyScan failed due to failure to prepare capture data*/
    BODY_SCAN_CAPTURE_DATA_PREPARATION_FAILURE(2050),

    /** Headless not yet supported. */
    HEADLESS_NOT_YET_SUPPORTED(2051),

    /** BodyScan not initiated through multiscan*/
    BODY_SCAN_FAILED_DIRECT_SCAN_INITIATED(2054),

    /** BodyScan failed due to worker manager exception*/
    BODY_SCAN_RESULTS_WORKER_EXCEPTION(2055),

    /** BodyScan initiate download assets but BodyScan Setup is not done*/
    BODY_SCAN_REMOTE_ASSET_BACKGROUND_NOT_INIT(2056),

    /** ScanExtra failed due to resources not downloaded*/
    BODY_SCAN_SCANEXTRA_RESOURCES_NOT_DOWNLOADED(2057),

    /** BodyScan generate mesh is called but MultiScan Setup is not done*/
    BODY_SCAN_MESH_SETUP_NOT_DONE(2058),

    /** BodyScan segmentation failed as capture image was not of the expected size*/
    BODY_SCAN_SEGMENT_LIST_INCORRECT_CAPTURE_RESOLUTION(2059),

    /** BodyScan segmentation failed as contour image was not of the expected size*/
    BODY_SCAN_SEGMENT_LIST_INCORRECT_CONTOUR_RESOLUTION(2060),

    /** BodyScan segmentation failed as incorrect number of joints were provided*/
    BODY_SCAN_SEGMENT_LIST_MISSING_JOINTS(2061),

    /** BodyScan segmentation failed*/
    BODY_SCAN_SEGMENT_LIST_FAILED(2062),

    /** BodyScan segmentation failed as model could not be retrieved from resources*/
    BODY_SCAN_SEGMENT_LIST_MODEL_MISSING(2063),

    /** BodyScan error code limit. No error code from BodyScan should equal or be greater than this error code value. */
    UNKNOWN(2999);

    /**
     * `code()` is overridden to get local codes.
     * @return The corresponding error code integer value.
     */
    override fun code(): Int {
        return this.code
    }

    /**
     * `getError()` is overridden to get AHIError by value.
     */
    companion object {
        /**
         * Get AHIError from matching error code integer value.
         * @param code The error code integer value.
         * @return The corresponding AHIError, or `UNKNOWN`.
         */
        fun getError(code: Int): BodyScanError {
            return values().find { s -> s.code == code } ?: UNKNOWN
        }
    }
}
