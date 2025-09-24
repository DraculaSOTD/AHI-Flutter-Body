# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is the AHI BodyScan iOS SDK - a computer vision-based health technology application that enables accurate body measurement and composition analysis using a smartphone's front-facing camera. The app provides clinical-grade accuracy for body measurements and composition analysis through advanced machine learning models.

## Key Business Context

- **Target Accuracy**: ±2-3cm for circumferences, ±3-5% for body fat percentage
- **Processing Time**: Complete scan in under 60 seconds
- **Privacy**: On-device processing with no network dependencies
- **Platform**: iOS 13.4+ (iPhone 6s+ recommended)

## Architecture Overview

The application follows a modular, delegate-based architecture:

```
AHIBodyScan (Main orchestrator)
├── Common (shared interfaces and error handling)
├── PartCamera (image capture and processing)
├── PartPoseDetection (joint detection using CoreML)
├── PartSegmentation (person silhouette extraction)
├── PartClassification (body measurement calculation)
├── PartContour (optimal positioning guidance)
├── PartInversion (3D mesh generation)
├── PartAlignment (device motion sensing)
├── PartResources (ML model management)
├── PartPoseInspection (pose validation)
└── PartUI (user interface components)
```

## Technology Stack

- **Languages**: Objective-C/C++ (core), Swift (UI components)
- **ML Framework**: CoreML 3.0+ with Neural Engine optimization
- **Computer Vision**: OpenCV for image processing
- **Build System**: CocoaPods with private repository distribution
- **Testing**: Kiwi framework

## Application Flow

1. **Setup Phase**: SDK initialization and authentication
2. **Configuration Phase**: User input (height, weight, gender) and validation
3. **Capture Phase**: Dual-phase camera capture (front + side poses)
4. **Processing Phase**: ML analysis pipeline with 48 specialized SVR models
5. **Results Phase**: Measurement display and local storage

## Key Components

### User Interface Structure

- **BodyScanViewController**: Main entry point with user input form
- **CameraViewController**: Real-time camera capture with pose guidance
- **ResultsViewController**: Measurement display and data visualization

### Machine Learning Pipeline

- **Joint Detection**: 14-point pose estimation with confidence scoring
- **Person Segmentation**: Silhouette extraction from background
- **Body Classification**: Gender-specific SVR models (48 models total)
- **3D Mesh Generation**: Optional avatar creation

### Data Management

- **Local Storage**: SQLite database for scan sessions, user profiles, and measurement history
- **ML Model Management**: 54+ MB of embedded models with version management
- **Offline Operation**: Complete functionality without network dependencies

## Error Handling

Error codes in 2000-2999 range for SDK-specific issues:
- Input validation errors (height/weight ranges)
- Camera access and capture quality issues
- ML model processing failures
- Resource management errors

## Key Measurements Provided

- **Circumferences**: Chest, waist, hip, thigh, inseam
- **Body Composition**: Body fat %, fat-free mass %, visceral fat
- **Weight Predictions**: Validation and composition adjustments

## Development Notes

- All processing is done locally on-device for privacy
- Gender-specific models require proper user input validation
- Camera capture requires specific positioning (head in zone, ankles visible, arms away from body)
- Device angle must be within ±1.5 degrees for accurate measurements
- Processing latency target: 500-800ms total pipeline