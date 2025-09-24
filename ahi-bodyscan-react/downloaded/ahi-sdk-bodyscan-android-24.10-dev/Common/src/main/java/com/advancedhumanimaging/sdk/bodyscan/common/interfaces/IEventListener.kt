//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.common.interfaces

/**
 * Interface declaration of Event Listener. This allows the app to be notified of certain events, such as
 * photo captures, UX transitions, image inspection results; for such purposes as analytics and support.
 */
interface IEventListener {

    /**
     * Is called when an event occurs. This is non-blocking so as to not interrupt the usual scan flow.
     * @param name Event name.
     * @param meta Data that's attributed to the event. For example, photo capture would include the image.
     */
    suspend fun event(
        name: String,
        meta: Map<String, Any>
    )

}