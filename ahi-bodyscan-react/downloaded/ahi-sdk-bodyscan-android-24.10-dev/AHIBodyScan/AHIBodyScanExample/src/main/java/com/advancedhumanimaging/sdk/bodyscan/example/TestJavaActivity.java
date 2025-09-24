//
//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

package com.advancedhumanimaging.sdk.bodyscan.example;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import com.advancedhumanimaging.sdk.bodyscan.BodyScan;
import com.advancedhumanimaging.sdk.common.IAHIDownloadProgress;
import com.advancedhumanimaging.sdk.common.IAHIScan;
import com.advancedhumanimaging.sdk.common.models.AHIResult;
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScan;
import com.advancedhumanimaging.sdk.multiscan.AHIMultiScanConstants;
import java.util.HashMap;
import java.util.Map;
import kotlin.Unit;

public class TestJavaActivity extends AppCompatActivity implements IAHIDownloadProgress {

    private MainScreenState mainScreenState = new MainScreenState();
    private TextView statusLabel;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.test_android_activity);
        Map<String, String> config = new HashMap<>();
        config.put(AHIMultiScanConstants.AHISetupTokenKey, "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJicTNKYXN5M0hMVmRxMkRja05MR1FXdGk2WEZIM2dVUElndUVNOXIyUTdXR05WRGVHTkV5ajR0T09XYXZxa3Y2VHEvYXJEVmtJNUZFSzZ4STFJUmcwRlVMK0t1MVdZV1UrOUtqSGsvS3pHVFk4bllES29WU2J6WEw1eVljOGprMndkMlNmeFBhYXJmT3pPZS9odFJvdUJwa2RSM09BS1V3TGtITTJYT3Fyays4d0MyenE2Y01pczFNZThTVWtGZ2RDS1ZsNVFLbkU0b2xGYzBNRTBVOEVqTW5mUUZSd1ZSZ2VVWlVOdkVWL3R2K1B0aEpJZGJKUnpxa0JUaE5YNDZBaXJaT2ZPV0tsczVoalZXMzNDS2xSczgxd3VwRWYrU0FQY3MxRmUwV2Urd1diN3VKSTdqTXRJeE1pcEdWdUQxMXNEODVKaW1GNm51bnBYRlY3NWc5TmVBR0JpdzZETDVnbXRPenRVNFVTaStKbWhvc1lIc3ZLMWhZay9YRTVKREFEeHV3eWJVaVlxVFJwVWdUeWRYUU02V1BpVUMvaVRmWEJIc1BKK3liRlBTcVpSVWhWa21EYUZ4TjJvNXluL1ZzRkNQU3IvSEs3WmcweEhZUGh4UHVpby8ydkRaZjRNZk1RU0tZaVhDbUxUNks0dmQwYy94OUg1ZngrYzdhMWlTRmR2dTNjWnV0aG1jdG5aYjVLU2prblVkcFlWamJaaytWb1B6eWczUVVpRXhqWDNuaVRZR0NaS3M5RkYyVFNUTW9PenZmVnVYcHpoVGhYMm5IeEpGdzNwN2phcGNvN21tZVhpYzF2VWpVVDhhQ09Xc3hqK1o1MUVRRHBielNyTjllSVV2N3pRZmtsNDhLQXpUUng3M0dkMWZuUmVMM0xzTWExcDBEc2hDakRlcz0iLCJleHAiOjE4MTQ3MDc1MzcsInZlciI6M30.EJTaMwDwzMtgIuY1DtwnCDyCnp9_2NauN7n91O7UIco8YpBbSxXWQWbbcvT-72pbpAQagAQ7hgKkjWEce5kKniDh3KhVa546kdQeneuWd1vu7iV1s3nZsMnPCxmFcv5uL30Cv8HWIkDEZb5UozZTgOcHDFAKyKQVT7Tmw0XA5GOjZqKZRtYtRXjzkQw7nidI_I0YkAshC6H8eakeyrVp-u-SilU95_ppIgZOtZHk1nSo04HATGXxl0inTDa57bi_UTU_OHLLlzKm-4GYtSKKdxyHcAipZklkQic4I_g4K43FeLNlWEyGpHt0dnabJm4auU033RZa5L87qCWFDSkTlBDTb3xTfSVtPtsyXTpHDMXS1-1WzQm97P8QcZ2zJl4z1wDk1WClwYRvk4E2JiA9ZwTef0OG2nQVlRJw0O4IpUWI8NLfg-2YcZR9-DZs2iC2QAIV0K1MVc9Ro2LMkgIm0nycgvYagmn-PZLuq9pATjVlW3oF1pJahEJcltoBc-JE0b_NjMsOFMeH3FQNf7_fCBYbRZA3SkOX87uPOqn2BXkYP77sHJ91mFw89TDilx4iVJSbErUej7tr8ZdagCVG1x0ltJ8qVAFCK5wnHZxp5FdfVBuaZNA2e7VzCFB7cYEFUfsuHd9NK2TzLcxizw7L2PxYHP2RdNCKMR3ebR572-w");
        IAHIScan[] scans = {new BodyScan()};
        statusLabel = findViewById(R.id.java_sdk_status);
        statusLabel.setText("SDK Setup Pending...");
        AHIMultiScan.INSTANCE.setup(
                getApplication(),
                config,
                scans,
                (sdkResult) -> {
                    Log.d("BodyScan_JavaTest", "sdk setup result: " + sdkResult);
                    downloadResources();
                    onSdkInitialised(sdkResult);
                    return null;
                });
    }

    private void onSdkInitialised(AHIResult<Unit> sdkResult) {
        Log.d("BodyScan_JavaTest", "sdk setup result: " + sdkResult.toString());
        mainScreenState.setSdkSetupState(SdkOperationStatus.SUCCESS);
        statusLabel.setText("SDK Setup finished. Result: " + sdkResult);
    }

    private void downloadResources() {
        AHIMultiScan.INSTANCE.areResourcesDownloaded(result -> {
            result.fold(downloaded -> {
                if (downloaded) {
                    statusLabel.setText("Resource are already downloaded");
                } else {
                    statusLabel.setText("Starting resource downloading");
                    AHIMultiScan.INSTANCE.setDelegateDownloadProgress(this);
                    AHIMultiScan.INSTANCE.downloadResourcesInForeground(1);
                }
                return null;
            }, exception -> {
                statusLabel.setText(exception.toString());
                return null;
            });
            return null;
        });
    }

    @Override
    public void onPointerCaptureChanged(boolean hasCapture) {
        super.onPointerCaptureChanged(hasCapture);
    }

    @Override
    public void downloadProgressReport(@NonNull AHIResult<Unit> ahiResult) {
        if (ahiResult.isFailure()) {
            Log.d("Downloading", "Failed to download resources: ${status.error().toString()}");
            return;
        }
        AHIMultiScan.INSTANCE.totalEstimatedDownloadSizeInBytes(downloadStateAHIResult -> {
            downloadStateAHIResult.fold(downloadState -> {
                double progress = downloadState.getProgressBytes() / 0x100000;
                double total = downloadState.getTotalBytes() / 0x100000;
                Log.i("Downloading", "Download state: " + progress + "/" + total + " MiB");
                statusLabel.setText("Download state: " + progress + "/" + total + " MiB");
                if (downloadState.getProgressBytes() == downloadState.getTotalBytes()) {
                    Log.d("Downloading", "Downloading finished");
                    statusLabel.setText("Downloading finished");
                }
                return null;
            }, exception -> {

                return null;
            });

            return null;
        });
    }
}