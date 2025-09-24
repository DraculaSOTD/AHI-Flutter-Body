# Module AHI BodyScan Android

[![Build Status](https://app.bitrise.io/app/f75371a4-7931-4540-885d-89476b3c908c/status.svg?token=G7Maz5awRLy_PevyByBSkw&branch=24.5/dev)](https://app.bitrise.io/app/f75371a4-7931-4540-885d-89476b3c908c)
[![codecov](https://codecov.io/gh/ahi-dev/ahi-sdk-bodyscan-android/branch/24.5/dev/graph/badge.svg?token=Xim1496Qze)](https://codecov.io/gh/ahi-dev/ahi-sdk-bodyscan-android)

AHI (AHI) BodyScan SDK is intended to be used as a plugin to the AHI MultiScan SDK for Android. 

## Example

To run the example app, run the 'app' build configuration in android studio, or run `./gradlew installDebug`

## Installation

AHI BodyScan is available as a gradle dependency which is hosted on an S3 bucket owned by AHI.

For access to the public release gradle repository, you will need to set up the values for AHI_RELEASE_AWS_ACCESS_KEY and AHI_RELEASE_AWS_SECRET_KEY.

For access to the dev and staging gradle repositories, you will need to set up values for AHI_DEV_AWS_ACCESS_KEY and AHI_DEV_AWS_SECRET_KEY.

To use the AHI BodyScan module in your own project, add this to your build.gradle file:

` implementation 'com.advancedhumanimaging.sdk.bodyscan:ahi-sdk-bodyscan-android:24.5.+'`

## Author

AHI

## License

AHI BodyScan is propriety code and is distributed under the [AHI SDK EULA license](https://refdev.advancedhumanimaging.io/LICENSE.md).
