//  AHI
//
//  Copyright (c) AHI. All rights reserved.
//

#ifndef ahiModelsZoo_H_
#define ahiModelsZoo_H_

#include "Types.hpp"
#include "AutoLock.hpp"
#include "Thread.hpp"

#include "AssetManager.hpp"
#include "log2022.h"


//////////
typedef enum ahiModelGender {
    Male,
    Female,
    Either
} ahiModelGender;

typedef enum ahiModelsZooModelId {
    ModelUndef,
    ModelFaceF,
    ModelFaceB,
    ModelPose, // poseLight
    ModelAHIPose, //moveNetPose
    ModelSegment,
    ModelSegment256,
    ModelSegment512,
    ModelJoints,
    ModelClassV1,
    ModelClassV2male,
    ModelClassV2female,
    ModelClassV2p5,
    ModelClassV3male,
    ModelClassV3female,
    ModelClassV3p1,
    ModelClassTBFIM1,
    ModelClassTBFIM2,
    ModelClassExmeasImageBased,
    ModelClassExmeasFeatBasedM23,
    ModelClassExmeasFeatBasedM60A,
    ModelClassExmeasFeatBasedM60B,
    ModelClassHeighWeightFeatBased
} ahiModelsZooModelId;

typedef std::map<std::pair<ahiModelsZooModelId, std::string>, ahiModelGender> ahiModelNameGender_map;

typedef enum ahiModelsZooModelType {
    ModelTypeUnknown,
    ModelTypeTF,
    ModelTypeStub
} ahiModelsZooModelType;


typedef struct {
    std::string namesUWA[4] = {"chest", "waist", "hips", "inseam"};
    std::string namesMKH[23] = {
            "bodyVol",
            "bodyArea",
            "chest",
            "underbustCirc",
            "frontChestDist",
            "shoulderDist",
            "waistAtBelly",
            "hips",
            "thighCirc",
            "inseam",
            "neckCirc",
            "neckHeight",
            "upperArmCirc",
            "upperArmLength",
            "lowerArmLength",
            "wristCirc",
            "napeToWaistDist",
            "waistToHipDist",
            "upperLegLength",
            "lowerLegLength",
            "calfCirc",
            "ankleCirc",
            "kneeCirc"
    };

    std::string namesIBV[63] = {
            "fullBodyVolume",
            "cervicalHeight",
            "distanceNeckHip",
            "trunkLength",
            "sevenCVToKneeHeight",
            "cervicaleToKnee",
            "waistToKneeHeight",
            "waistHeight",
            "highWaistHeight",
            "bellyHeight",
            "hipHeightButtock",
            "seatHeight",
            "inseam",
            "crotchHeight",
            "kneeHeight",
            "ankleHeight",
            "bustOrChestHeight",
            "frontNeckHeight",
            "midNeckGirth",
            "headGirth",
            "waistFrontalContour",
            "horizontalShoulderWidthBetweenAcromions",
            "shoulderLength",
            "shoulderBiacromicalBreadth",
            "frontalArmpitContour",
            "bustWidth",
            "frontWaistLength",
            "centredFrontWaistLength",
            "neckShoulderPointToBreastPoint",
            "bustOrChestGirth",
            "underBustGirth",
            "backArmpitsContour",
            "scyeDepth",
            "cackWaistLength",
            "neckShoulderPointToWaist",
            "waistToButtockLength",
            "waistGirth",
            "highWaistGirth",
            "bellyGirth",
            "hipGirthButtock",
            "seatGirth",
            "middleHipGirth",
            "neckShoulderPointToWrist",
            "armLength",
            "upperArmLength",
            "foreArmLength",
            "upperArmGirth",
            "elbowGirth",
            "foreArmGirth",
            "wristGirth",
            "insideLegAnkle",
            "maxThighGirth",
            "kneeGirth",
            "minimumLegGirth",
            "ankleGirth",
            "bellyDepth",
            "hipBreadth",
            "waistBreadth",
            "lowerNeck",
            "lowerNeckWidth",
            "lowerNeckDepth",
            "bodyArea",
            "bodyVol"};

} extraMeasImageBasedMeasStruct;

typedef struct {
    float scalesUWAmkh[5] = {157.0, 164.0, 177.0, 108.0, 105.0};
    std::string namesUWAmkh[5] = {"chest", "waist", "hips", "thighs", "inseam"};

    float scalesMH[23] = {
            182.0,
            3.5,
            157.0,
            139.0,
            42.0,
            17.0,
            164.0,
            177.0,
            108.0,
            105.0,
            61.0,
            12.0,
            52.0,
            38.0,
            29.0,
            28.0,
            51.0,
            24.0,
            49.0,
            66.0,
            64.0,
            32.0,
            60.0
    };
    string namesMH[23] = {
            "bodyVol",
            "bodyArea",
            "chest",
            "underbustCirc",
            "frontChestDist",
            "shoulderDist",
            "waistAtBelly",
            "hips",
            "thighCirc",
            "inseam",
            "neckCirc",
            "neckHeight",
            "upperArmCirc",
            "upperArmLength",
            "lowerArmLength",
            "wristCirc",
            "napeToWaistDist",
            "waistToHipDist",
            "upperLegLength",
            "lowerLegLength",
            "calfCirc",
            "ankleCirc",
            "kneeCirc"
    };

    float scalesUWAibv[7] = {210.0, 170.0, 164.0, 163.0, 172.0, 100.0, 110.0};
    std::string namesUWAibv[7] = {"heightCheck", "weightCheck", "chest", "waist", "hips", "thighs", "inseam"};

    float scalesIBV[70] = {210.0, 171.0, 184.0, 79.0, 97.0, 127.0, 130.0, 78.0, 139.0, 143.0, 143.0,
                           118.0, 119.0, 110.0, 110.0, 64.0, 10.0, 159.0, 177.0, 54.0, 66.0, 89.0,
                           66.0, 23.0, 55.0, 71.0, 32.0, 67.0, 48.0, 51.0, 164.0, 145.0, 56.0, 32.0,
                           56.0, 60.0, 25.0, 163.0, 153.0, 154.0, 172.0, 173.0, 174.0, 101.0, 80.0,
                           46.0, 37.0, 53.0, 39.0, 39.0, 23.0, 100.0, 100.0, 60.0, 32.0, 34.0, 48.0,
                           63.0, 52.0, 57.0, 20.0, 19.0, 3.0, 169.0, 168.0, 88.0, 112.0, 18.0,
                           173.0, 83.0
    };
    std::string namesIBV[70] = {
            "heightCheck",
            "fullBodyVolume",
            "cervicalHeight",
            "distanceNeckHip",
            "trunkLength",
            "sevenCVToKneeHeight",
            "cervicaleToKnee",
            "waistToKneeHeight",
            "waistHeight",
            "highWaistHeight",
            "bellyHeight",
            "hipHeightButtock",
            "seatHeight",
            "inseam",
            "crotchHeight",
            "kneeHeight",
            "ankleHeight",
            "bustOrChestHeight",
            "frontNeckHeight",
            "midNeckGirth",
            "headGirth",
            "waistFrontalContour",
            "horizontalShoulderWidthBetweenAcromions",
            "shoulderLength",
            "shoulderBiacromicalBreadth",
            "frontalArmpitContour",
            "bustWidth",
            "frontWaistLength",
            "centredFrontWaistLength",
            "neckShoulderPointToBreastPoint",
            "bustOrChestGirth",
            "underbustGirth",
            "backArmpitsContour",
            "scyeDepth",
            "backWaistLength",
            "neckShoulderPointToWaist",
            "waistToButtockLength",
            "waistGirth",
            "highWaistGirth",
            "bellyGirth",
            "hipGirthButtock",
            "seatGirth",
            "middleHipGirth",
            "neckShoulderPointToWrist",
            "armLength",
            "upperArmLength",
            "forearmLength",
            "upperArmGirth",
            "elbowGirth",
            "forearmGirth",
            "wristGirth",
            "insideLeg",
            "maxThighGirth",
            "kneeGirth",
            "minimumLegGirth",
            "ankleGirth",
            "bellyDepth",
            "hipBreadth",
            "waistBreadth",
            "lowerNeck",
            "lowerNeckWidth",
            "lowerNeckDepth",
            "bodyArea",
            "bodyVol",
            "weightFromAnthrop",
            "fatFromAnthrop",
            "musclesFromAnthrop",
            "bonesFromAnthrop",
            "weightsFromBCT",
            "fatsFromBCT"};
} extraMeasFeatBasedMeasStruct;

class ahiModelsZoo {
public:
    ahiModelNameGender_map ahiModelGenderMap = {
            {std::make_pair(ModelFaceF, "faceDetectionFront"),    ahiModelGender::Either},
            {std::make_pair(ModelFaceB, "faceDetectionBack"),     ahiModelGender::Either},
            {std::make_pair(ModelPose, "ahiPoseLight"),           ahiModelGender::Either},
            {std::make_pair(ModelAHIPose, "ahiMoveNetPoseModel"), ahiModelGender::Either},
            {std::make_pair(ModelSegment, "segnet"),              ahiModelGender::Either},
            {std::make_pair(ModelSegment256, "seg_model_256"),    ahiModelGender::Either},
            {std::make_pair(ModelSegment512, "seg_model_512"),    ahiModelGender::Either},
            {std::make_pair(ModelJoints, "jointnet"),             ahiModelGender::Either},
    };

    ahiModelNameGender_map ahiShapeModelGenderMap = {
            {std::make_pair(ModelClassV1, "classV1Model"),             ahiModelGender::Either},
            {std::make_pair(ModelClassV2male, "classV2Modelmale"),     ahiModelGender::Male},
            {std::make_pair(ModelClassV2female, "classV2Modelfemale"), ahiModelGender::Female},
            {std::make_pair(ModelClassV2p5, "classV2p5Model"),         ahiModelGender::Either},
            {std::make_pair(ModelClassV3male, "classV3Modelmale"),     ahiModelGender::Male},
            {std::make_pair(ModelClassV3female, "classV3Modelfemale"), ahiModelGender::Female},
            {std::make_pair(ModelClassV3p1, "classV3p1Model"),         ahiModelGender::Either}
    };

    ahiModelNameGender_map ahiCompositionModelGenderMap = {
            {std::make_pair(ModelClassTBFIM1, "classTBFIM1Model"), ahiModelGender::Either}
    };


    uint8_t mKeydata[32];
    uint8_t mNoncedata[12];

    ahiModelsZoo() = default;

    std::vector<std::string> getSvrModelList(std::string measCatagory);
};

#endif
