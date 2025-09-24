//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common

import org.junit.jupiter.api.Assertions
import org.junit.jupiter.api.Assertions.assertEquals
import org.junit.jupiter.api.Test

const val BODYSCAN_ERROR_MIN = 2000
const val BODYSCAN_ERROR_MAX = 2999

class BodyScanErrorCodeTests {
    @Test
    fun BodyScanErrorCode_inRange() {
        for (error in BodyScanError.values()) {
            Assertions.assertTrue(error.code() in BODYSCAN_ERROR_MIN..BODYSCAN_ERROR_MAX)
        }
    }

    @Test
    fun givenAllErrorCodes_whenCheckingForDuplicates_thenNoneFound() {
        val errorCodes = BodyScanError.values().map { it.code() }
        assertEquals(errorCodes.distinct(), errorCodes)
    }
}