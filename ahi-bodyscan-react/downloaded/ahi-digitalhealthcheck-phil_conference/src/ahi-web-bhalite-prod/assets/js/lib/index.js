/* global rppg */

// Global settings
const authToken = "2b87039d-d352-4526-b245-4dbefc5cf636";
const socketUrl = "wss://vm-production.xyz/vp/bgr_signal_socket";
const pathToWasmData = "/assets/js/lib";
const useFrontCamera = true;
let isFormSubmitted = false;

// Webcam settings
const videoWidth = 940;
const videoHeight = 705;

const canvasChartElement = document.querySelector("#chart");
const canvasChartCtx = canvasChartElement.getContext("2d");

const canvasChartWidth = 100;
const canvasChartHeight = 25;
const canvasChartLineWidth = 1;

// Multilanguage support
const language = navigator.language.slice(0, 2) || "en";
const messages = {
  en: {
    faceGuideMessage: "Place your face in the outline...",
    faceGuideContext: "Make sure your face is well lit",
    calibrationMessage: "Calibrating System...",
    calibrationContext: "Please keep still",
    blinkCheckMessage: "Checking...",
    blinkCheckContext: "Please blink",
    signalInterpretationMessage: "Interpreting Vital Signs...",
    signalInterpretationContext: "This won't take long",
    analyzingBiometricsMessage: "Analyzing Biometrics...",
    analyzingBiometricsContext: "Thank you for your patience...",
    finalizingResultsMessage: "Finalizing Results...",
    finalizingResultsContext: "Thank you for your patience",
    faceNotDetectedMessage: "No Face Detected",
    faceNotDetectedContext: "Please face the camera",
    faceSizeMessage: "Too Far Away",
    faceSizeContext: "Please move closer to camera",
    faceOrientMessage: "Face not straight",
    faceOrientContext: "Please face camera directly",
  },
  es: {    
    calibrationMessage: "Calibrando el sistema...",
    calibrationContext: "Por favor, mantente quieto",
  },
};

async function initRPPG() {
  try {
    const videoElement = document.querySelector("#video");
    const formElement = document.querySelector("#post-scan");

    const sex = formElement.dataset.sex;
    const age = formElement.dataset.age;
    const weight = formElement.dataset.weight;
    const height = formElement.dataset.height;

    const calcData = {
      frameData: {
        rppgTrackerData: {
          status: -1,
          bgr1d: [],
          landmarks: [],
          face: [],
          eyeBlinkStatus: false,
          imageQualityFlags: [],
        },
        instantFps: -1,
        averageFps: -1,
        timestamp: -1,
      },
      measurementProgress: 0,
      measurementMeanData: {
        bloodPressureStatus: "-1",
        bpm: -1,
        oxygen: -1,
        rr: -1,
        stressStatus: "-1",
      },
      measurementSignal: {
        signal: [],
        ecg: [],
      },
      stressIndex: -1,
      hrvMetrics: {
        ibi: -1,
        rmssd: -1,
        sdnn: -1,
      },
      signalQuality: {
        snr: 0,
      },
      measurementStatus: {
        statusCode: "",
        statusMessage: "",
      },
      bloodPressure: {
        systolic: -1,
        diastolic: -1,
      },
      FaceState: {
        isFaceDetected: true,
        isFaceOriented: true,
        isFaceNear: true,
        blinkCount: 0,
      },
    };

    const errorWindow = 10;
    let faceDetectionLog = Array(errorWindow).fill(true);
    let faceSizeFlagLog = Array(errorWindow).fill(true);
    let faceOrientFlagLog = Array(errorWindow).fill(true);

    // Custom AHI function to determine if we have a face per frame
    function updateFaceStateLog(faceStateLog, newValue, errorWindow) {
      faceStateLog.push(newValue);
      if (faceStateLog.length > errorWindow) {
        faceStateLog.shift();
      }
      return !faceStateLog.every((val) => val === false);
    }

    const rppgInstance = (window.rppgInstance = new rppg({
      onFrame: (data) => {
        // Increment only for unique blink detected
        calcData.FaceState.blinkCount += (data.rppgTrackerData.eyeBlinkStatus && (!calcData.frameData.rppgTrackerData.eyeBlinkStatus || calcData.FaceState.blinkCount == 0)) ? 1 : 0;

        calcData.frameData = data;

        calcData.FaceState.isFaceDetected = updateFaceStateLog(
          faceDetectionLog,
          data.rppgTrackerData.status !== 2 &&
            data.rppgTrackerData.status !== 1,
          errorWindow
        );

        calcData.FaceState.blinkCount = !calcData.FaceState.isFaceDetected ? 0 : calcData.FaceState.blinkCount;

        calcData.FaceState.isFaceNear = updateFaceStateLog(
          faceSizeFlagLog,
          data.rppgTrackerData.imageQualityFlags.faceSizeFlag,
          errorWindow
        );

        calcData.FaceState.isFaceOriented = updateFaceStateLog(
          faceOrientFlagLog,
          data.rppgTrackerData.imageQualityFlags.faceOrientFlag,
          errorWindow
        );

        // No errors so we good
        render(calcData);
      },
      onMeasurementProgress: (data) => {
        // console.log("onMeasurementProgress", data);
        calcData.measurementProgress = data.progressPercent;
      },
      onMeasurementMeanData: (data) => {
        // console.log("onMeasurementMeanData", data);
        // {bpm: 0, rr: 0, oxygen: 0, bloodPressureStatus: "NO_DATA", stressStatus: "NO_DATA" }
        calcData.measurementMeanData = data;
      },
      onMeasurementSignal: (data) => {
        // console.log("onMeasurementSignal", data.signal);
        // 256 samples for the chart
        calcData.measurementSignal = data;
      },
      onStressIndex: (data) => {
        // console.log("onStressIndex:", data);
        // Returns-1 or a float per frame
        calcData.stressIndex = data.stress;
      },
      onHrvMetrics: (data) => {
        // console.log("onHrvMetrics:", data);
        // {"ibi": 0.0, "rmssd": 0.0, "sdnn": 0.0, "rr": [26], "stress_index": 0.0, "stress_label": ""}
        calcData.hrvMetrics = data;
      },
      onBloodPressure: (data) => {
        // console.log("onBloodPressure:", data);
        // {systolic: 0, diastolic: 0}
        calcData.bloodPressure = data;
      },
      onSignalQuality: (data) => {
        // console.log("onSignalQuality:", data);
        // {snr: 0}
        calcData.signalQuality = data;
      },
      onMeasurementStatus: (data) => {
        // console.log("onMeasurementStatus:", data);
        // {statusCode: "", statusMessage: ""}
        // Use only for debug
        calcData.measurementStatus = data;
      },
      onInterferenceWarning: (data) => {
        // console.log("onInterferenceWarning:", data);
      },
      onUnstableConditionsWarning: (data) => {
        // console.log("onUnstableConditionsWarning:", data);
      },
    }));

    await rppgInstance.initCamera({
      width: videoWidth,
      height: videoHeight,
      videoElement,
      useFrontCamera,
    });

    await rppgInstance.initTracker({
      pathToWasmData,
    });

    await rppgInstance.initSocket({
      authToken,
      url: socketUrl,
      query: `sex=${sex}&age=${age}&weight=${weight}&height=${height}`,
      onConnect: () => {
        console.log("Socket connection established");
      },
      onClose: (event) => console.log("Socket connection closed", event),
      onError: (event) => console.log("Socket connection error", event),
    });

    rppgInstance.start();
  } catch (error) {
    console.error("Error initializing RPPG:", error);
  }
}

function drawChart({ signal = [], startX = 0, startY = 0, color = "white" }) {
  canvasChartElement.width = canvasChartWidth;
  canvasChartElement.height = canvasChartHeight;
  canvasChartCtx.lineWidth = canvasChartLineWidth;
  canvasChartCtx.fillStyle = "transparent";
  // Limit the number of entries in the signal
  const maxEntries = 75; // Change this to the number of entries you want
  signal = signal.slice(Math.max(signal.length - maxEntries, 0));

  canvasChartCtx.beginPath();
  let px2 = startX;
  let py2 = canvasChartElement.height / 2 / 4 + startY;
  canvasChartCtx.strokeStyle = color;

  // Find the maximum and minimum signal values
  const maxSignal = Math.max(...signal);
  const minSignal = Math.min(...signal);

  // Scale the x-axis to fit within the canvas width
  const xScale = canvasChartElement.width / signal.length;

  signal.forEach((y, x) => {
    if (!canvasChartCtx) {
      return;
    }
    // Normalize the signal to the range of the canvas height
    const normalizedY =
      ((y - minSignal) / (maxSignal - minSignal)) * canvasChartElement.height;

    canvasChartCtx.moveTo(px2, py2);
    canvasChartCtx.lineTo(x * xScale + startX, normalizedY + startY);
    px2 = x * xScale + startX;
    py2 = normalizedY + startY;
  });
  canvasChartCtx.stroke();
}

function updateRiveCompletionStatuses({
  bpm,
  rr,
  oxygen,
  hrvMetrics,
  stressIndex,
  bloodPressure,
}) {
  const completionStatuses = {
    bpm_isComplete: bpm > 0,
    rr_isComplete: rr > 0,
    spo2_isComplete: oxygen > 0,
    hrv_isComplete:
      hrvMetrics.ibi > 0 && hrvMetrics.rmssd > 0 && hrvMetrics.sdnn > 0,
    si_isComplete: stressIndex > 0,
    bp_isComplete: bloodPressure.systolic > 0 && bloodPressure.diastolic > 0,
  };

  Object.entries(completionStatuses).forEach(([buttonName, isComplete]) => {
    if (isComplete) {
      setInputValue(buttonName, "true");
    }
  });
}

const fallbackLang = "en";

const getSafeTranslation = (key, defaultText = `[[${key}]]`) => {
  try {
    const lang = window.language || fallbackLang;

    const value =
      window.messages?.[lang]?.[key] ??
      window.messages?.[fallbackLang]?.[key] ??
      defaultText;

    return value;
  } catch (error) {
    console.error(`Error getting translation for ${key}:`, error);
    return defaultText;
  }
};

function checkState({
  progress,
  measurementSignal,
  bloodPressure,
  measurementMeanData,
  hrvMetrics,
  stressIndex,
  FaceState,
}) {
  // Set a default state object to update...
  let state = {
    haloState: 0,
    guideText: getSafeTranslation("faceGuideMessage", "Position your face in the frame"),
    guideContext: getSafeTranslation("faceGuideContext", "Stay still for the scan"),
    isCalibrated: false,
    isError: false,
    shouldDrawChart: false,
    submitForm: false,
  };

  // CHECK FOR ERRORS...
  // Do you have a face?
  if (!FaceState.isFaceDetected) {
    state.haloState = 3;    
    state.guideText = getSafeTranslation("faceNotDetectedMessage", "No Face Detected");
    state.guideContext = getSafeTranslation("faceNotDetectedContext", "Please face the camera");
    state.shouldDrawChart = false;
    state.isCalibrated = false;
    state.isError = true;
    return state;
  }

  // Is it close enough? This must be before Orientation check
  if (!FaceState.isFaceNear) {
    state.haloState = 3;        
    state.guideText = getSafeTranslation("faceSizeMessage", "Too Far Away");
    state.guideContext = getSafeTranslation("faceSizeContext", "Please move closer to camera");
    state.shouldDrawChart = false;
    state.isCalibrated = false;
    state.isError = true;
    return state;
  }

  // Is it pointing the right way?
  if (!FaceState.isFaceOriented) {
    state.haloState = 3;        
    state.guideText = getSafeTranslation("faceOrientMessage", "Face not straight");
    state.guideContext = getSafeTranslation("faceOrientContext", "Please face camera directly");
    state.shouldDrawChart = false;
    state.isCalibrated = false;
    state.isError = true;
    return state;
  }

  // Is person blinking?
  if (progress > 50 && progress < 90 && FaceState.blinkCount < 1) {
    state.haloState = 3;        
    state.guideText = getSafeTranslation("blinkCheckMessage", "Checking...");
    state.guideContext = getSafeTranslation("blinkCheckContext", "Please blink");
    state.shouldDrawChart = false;
    state.isCalibrated = false;
    state.isError = true;
    return state;
  }

  // If NO errors then check progress
  if (progress > 0 && progress < 100) {
    state.haloState = 1;        
    state.guideText = getSafeTranslation("calibrationMessage", "Calibrating System...");
    state.guideContext = getSafeTranslation("calibrationContext", "Please keep still");
    state.isError = false;
  }

  if (measurementSignal.signal.length > 50) {        
    state.guideText = getSafeTranslation("signalInterpretationMessage", "Interpreting Vital Signs...");
    state.guideContext = getSafeTranslation("signalInterpretationContext", "This won't take long");
    state.shouldDrawChart = true;
    state.isError = false;
  }

  if (progress > 100) {
    state.haloState = 2;        
    state.guideText = getSafeTranslation("analyzingBiometricsMessage", "Analyzing Biometrics...");
    state.guideContext = getSafeTranslation("analyzingBiometricsContext", "Thank you for your patience...");
    state.isCalibrated = true;
    state.isError = false;
  }

  if (
    bloodPressure.systolic > 0 &&
    bloodPressure.diastolic > 0 &&
    measurementMeanData.bpm > 0 &&
    measurementMeanData.oxygen > 0 &&
    measurementMeanData.rr > 0 &&
    hrvMetrics.ibi > 0 &&
    hrvMetrics.rmssd > 0 &&
    hrvMetrics.sdnn > 0 &&
    stressIndex > 0
  ) {
    state.haloState = 4;
    state.shouldDrawChart = false;
    state.guideText = getSafeTranslation("finalizingResultsMessage", "Finalizing Results...");
    state.guideContext = getSafeTranslation("finalizingResultsContext", "Thank you for your patience");
    state.isCalibrated = false;
    state.submitForm = true;
  }

  return state;
}

async function submitForm(
  measurementMeanData,
  hrvMetrics,
  stressIndex,
  bloodPressure
) {
  let form = document.getElementById("post-scan");
  let formData = {
    bpm_ent_restingHeartRate: measurementMeanData.bpm,
    int_raw_rr: measurementMeanData.rr,
    int_raw_oxygen: measurementMeanData.oxygen,
    flt_raw_ibi: hrvMetrics.ibi,
    flt_raw_sdnn: hrvMetrics.sdnn,
    flt_raw_rmssd: hrvMetrics.rmssd,
    flt_raw_stressIndex: stressIndex,
    mmHg_ent_systolicBP: bloodPressure.systolic,
    mmHg_ent_diastolicBP: bloodPressure.diastolic,
  };
  for (let key in formData) {
    if (formData.hasOwnProperty(key)) {
      let hiddenField = document.createElement("input");
      hiddenField.type = "hidden";
      hiddenField.name = key;
      hiddenField.value = JSON.stringify(formData[key]);
      form.appendChild(hiddenField);
    }
  }
  // Stop the RPPG instance
  rppgInstance.stop();
  // Submit the form
  form.submit();
}

async function render({
  measurementProgress,
  frameData,
  measurementMeanData,
  bloodPressure,
  stressIndex,
  hrvMetrics,
  measurementStatus,
  measurementSignal,
  signalQuality,
  FaceState,
}) {
  const currentState = checkState({
    progress: measurementProgress,
    measurementSignal,
    bloodPressure,
    measurementMeanData,
    hrvMetrics,
    stressIndex,
    FaceState,
  });

  // UPDATE HUD with current state
  setInputValue("haloState", currentState.haloState);
  updateRiveText("guideText", currentState.guideText);
  updateRiveText("guideTextContext", currentState.guideContext);
  updateRiveText("bpmValue", measurementMeanData.bpm);
  setInputValue("isCalibrated", currentState.isCalibrated);
  updateRiveCompletionStatuses({
    bpm: measurementMeanData.bpm,
    rr: measurementMeanData.rr,
    oxygen: measurementMeanData.oxygen,
    hrvMetrics,
    stressIndex,
    bloodPressure,
  });

  // PAUSE RPPG if there is an error state
  rppgInstance.pause(currentState.isError);

  // UPDATE CHART
  if (currentState.shouldDrawChart) {
    canvasChartElement.style.display = "inline";
    drawChart({
      signal: measurementSignal.signal,
      startX: 0,
      startY: 0,
      color: "white",
    });
  } else {
    canvasChartElement.style.display = "none";
  }

  // SUBMIT FORM
  if (currentState.submitForm) {
    if (!isFormSubmitted) {
      console.table("Measurement Mean Data: ", measurementMeanData);
      console.table("HRV Metrics: ", hrvMetrics);
      console.table("Stress Index: ", stressIndex);
      console.table("Blood Pressure: ", bloodPressure);
      console.table("Frame Data: ", frameData);
      console.table("Measurement Status: ", measurementStatus);
      console.table("Signal Quality: ", signalQuality);
      // Prevent multiple form submissions
      isFormSubmitted = true;
      submitForm(measurementMeanData, hrvMetrics, stressIndex, bloodPressure);
    }
  }
}

// const initRive = async () => {
//   let r;

//   const riveLoaded = new Promise((resolve) => {
//     const baseUrl = window.location.protocol + "//" + window.location.host;
//     // const riveFilePath = baseUrl + "/assets/rive/hud.riv";
//     const riveFilePath = "https://webfiles.ahi.zone/vm/hud.riv?Expires=1756783577&Signature=VOBVq8~hrxeLNBpeLsFST2FgAzo3Kjy21p14rxi3et6HwzrJthOT8wG71yXUSpHOgus2a7GaRGa7G2C3UibIZFe5rBIFLUFYPsXWfqkBDJe4GoyCSv3a52q9UdgueKUbo9RdTEDagUc6GR1fk-ehqbiJm9UEaHb-c8GfaTSAZFfZAdhDi8GLjd1ZFFa-ASrBewdr5-ILwCotzFj8JClA2jbJEKPWf8EDUvlUB082rS~wTxDhntnxfrpiiAclfTokE~kcbSktL33AT~DFE5h4BBykmcJgTn7u0MYN65qJ~gSMfJkh~OfpP776Yjak1lDPBY2oBflazmHbEApg7saBHw__&Key-Pair-Id=K2KJYYC3132IRB";

//     r = new rive.Rive({
//       src: riveFilePath,
//       canvas: document.getElementById("canvas"),
//       autoplay: true,
//       stateMachines: "SM",
//       automaticallyHandleEvents: true,
//       onLoad: async () => {
//         r.resizeDrawingSurfaceToCanvas();
//         resolve(true);

//         updateRiveText("guideText", messages[language].faceGuideMessage);
//         updateRiveText("guideTextContext", messages[language].faceGuideContext);
//         updateRiveText("bpmValue", "-");

//         r.on("backTapped", () => {
//           console.log("backTapped");
//         });
//       },
//     });
//   });

const initRive = async () => {
  let r;
  
  // Define fallback messages that will always be available
  const fallbackMessages = {
    faceGuideMessage: "Position your face in the frame",
    faceGuideContext: "Stay still for the scan",
  };
  
  // Safely get translation text with guaranteed fallback
  const getTranslationText = (key) => {
    try {
      // Handle potentially undefined messages or language
      if (!window.messages || !window.language || !window.messages[window.language]) {
        return fallbackMessages[key] || key || "";
      }
      
      // Return the translation if it exists, otherwise fallback
      return window.messages[window.language][key] || fallbackMessages[key] || key || "";
    } catch (error) {
      console.error(`Error getting translation for ${key}:`, error);
      return fallbackMessages[key] || key || "";
    }
  };
  
  // Safe wrapper for updating Rive text that won't throw errors
  const safeUpdateRiveText = (textFieldName, content) => {
    try {
      if (r && typeof r.setTextRunValue === 'function') {
        r.setTextRunValue(textFieldName, content || "");
      }
    } catch (e) {
      console.error(`Failed to update Rive text '${textFieldName}':`, e);
    }
  };

  const riveLoaded = new Promise((resolve) => {
    const riveFilePath = "https://webfiles.ahi.zone/vm/hud.riv?Expires=1756783577&Signature=VOBVq8~hrxeLNBpeLsFST2FgAzo3Kjy21p14rxi3et6HwzrJthOT8wG71yXUSpHOgus2a7GaRGa7G2C3UibIZFe5rBIFLUFYPsXWfqkBDJe4GoyCSv3a52q9UdgueKUbo9RdTEDagUc6GR1fk-ehqbiJm9UEaHb-c8GfaTSAZFfZAdhDi8GLjd1ZFFa-ASrBewdr5-ILwCotzFj8JClA2jbJEKPWf8EDUvlUB082rS~wTxDhntnxfrpiiAclfTokE~kcbSktL33AT~DFE5h4BBykmcJgTn7u0MYN65qJ~gSMfJkh~OfpP776Yjak1lDPBY2oBflazmHbEApg7saBHw__&Key-Pair-Id=K2KJYYC3132IRB";

    try {
      const canvas = document.getElementById("canvas");
      
      // Check if canvas exists
      if (!canvas) {
        console.error("Canvas element not found");
        // Create a fallback canvas to avoid errors
        const canvasContainer = document.querySelector("#canvas-container") || document.body;
        const fallbackCanvas = document.createElement("canvas");
        fallbackCanvas.id = "canvas";
        canvasContainer.appendChild(fallbackCanvas);
      }
      
      r = new rive.Rive({
        src: riveFilePath,
        canvas: document.getElementById("canvas"),
        autoplay: true,
        stateMachines: "SM",
        automaticallyHandleEvents: true,
        onLoad: async () => {
          try {
            r.resizeDrawingSurfaceToCanvas();
          } catch (resizeError) {
            console.error("Error resizing Rive canvas:", resizeError);
          }
          
          // Always resolve the promise to prevent hanging
          resolve(true);
          
          try {
            // Use our safe translation getter instead of direct access
            safeUpdateRiveText("guideText", getTranslationText("faceGuideMessage"));
            safeUpdateRiveText("guideTextContext", getTranslationText("faceGuideContext"));
            safeUpdateRiveText("bpmValue", "-");
          } catch (textError) {
            console.error("Error updating Rive text:", textError);
            // Try with fallbacks directly if something went wrong
            safeUpdateRiveText("guideText", fallbackMessages.faceGuideMessage);
            safeUpdateRiveText("guideTextContext", fallbackMessages.faceGuideContext);
          }
          
          try {
            // Set up event handlers
            r.on("backTapped", () => {
              console.log("backTapped");
            });
          } catch (eventError) {
            console.error("Error setting up Rive events:", eventError);
          }
        },
        // Add error handling for loading failures
        onError: (error) => {
          console.error("Rive loading error:", error);
          resolve(false); // Resolve with false to indicate failure
        }
      });
    } catch (riveInitError) {
      console.error("Failed to initialize Rive:", riveInitError);
      resolve(false); // Make sure the promise resolves even if Rive init fails completely
    }
  });

  // Safe resize handler
  const safeResize = () => {
    try {
      if (r && typeof r.resizeDrawingSurfaceToCanvas === 'function') {
        r.resizeDrawingSurfaceToCanvas();
      }
    } catch (e) {
      console.error("Error during resize:", e);
    }
  };

  window.addEventListener("resize", safeResize, false);

  // Set to global object so index.js can see it.
  window.riveInstance = r;

  // Add language change handler to update text if language changes later
  const onLanguageChange = () => {
    safeUpdateRiveText("guideText", getTranslationText("faceGuideMessage"));
    safeUpdateRiveText("guideTextContext", getTranslationText("faceGuideContext"));
  };
  
  // If a language change event exists, subscribe to it
  if (window.addEventListener && window.languageChangeEvent) {
    window.addEventListener(window.languageChangeEvent, onLanguageChange, false);
  }

  return riveLoaded;
};

// RIVE FUNCTIONS

const setInputValue = async (inputName, newValue) => {
  const r = window.riveInstance;
  if (!riveInstance) return;

  const inputs = r.stateMachineInputs("SM");
  const stateInput = inputs.find((i) => i.name === inputName);
  stateInput.value = newValue;
};

function updateRiveText(itemName, statusMessage) {
  if (window.riveInstance) {
    window.riveInstance.setTextRunValue(
      String(itemName),
      String(statusMessage)
    );
  }
}

function init() {
  initRive();
  initRPPG(false);
}

document.addEventListener("DOMContentLoaded", init);
