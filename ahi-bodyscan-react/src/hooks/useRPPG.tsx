import {
  useEffect,
  useReducer,
  useRef,
  useState,
} from 'react';
import RPPG from 'vastmindz-web-sdk/dist';
import { RPPGOnFrame } from 'vastmindz-web-sdk/dist/lib/RPPG.types';
import {
  HrvMetrics,
  MeasurementMeanData,
  MeasurementProgress,
  MeasurementSignal,
  MeasurementStatus,
  SignalQuality,
  BloodPressure,
  StressIndex,
  StatusCode,
  BloodPressureStatus,
  StressStatus,
} from 'vastmindz-web-sdk/dist/lib/RPPGEvents.types';

// Constants
const CALCULATION_TIMEOUT = 90000; // 90 seconds
const FPS_CHECK_DONE_TIMEOUT = 3000;
const FPS_CHECK_THRESHOLD = 15;
const FPS_CHECK_TIMEOUT = 5000;
const AUTH_TOKEN = '2b87039d-d352-4526-b245-4dbefc5cf636';
const SOCKET_URL = 'wss://vm-production.xyz/vp/bgr_signal_socket';

export interface RPPGData {
  frameData?: RPPGOnFrame;
  measurementData?: MeasurementMeanData;
  measurementStatus?: MeasurementStatus;
  measurementProgress?: MeasurementProgress;
  signalQuality?: SignalQuality;
  measurementSignal?: MeasurementSignal;
  hrvMetrics?: HrvMetrics;
  bloodPressure?: BloodPressure;
  stressIndex?: StressIndex;
}

export interface Fps {
  instantFps: number;
  averageFps: number;
  maxFps: number;
  unsupported: boolean;
}

export interface UseRPPG {
  videoElement: React.RefObject<HTMLVideoElement>;
  canvasElement?: React.RefObject<HTMLCanvasElement>;
  useFrontCamera?: boolean;
  userData: {
    sex: string;
    age: number;
    weight: number;
    height: number;
  };
  onUnsupportedDeviceCb?: () => void;
  onAllDataCalculatedCb?: () => void;
  onCalculationEndedCb?: () => void;
  onInterferenceWarningCb?: () => void;
  onUnstableConditionsWarningCb?: () => void;
  onFaceOrientWarningCb?: () => void;
  onFaceSizeWarningCb?: () => void;
}

export interface UseRPPGResult {
  rppgData: RPPGData;
  ready: boolean;
  error: Error | null;
  rppgInstance?: RPPG;
  isAllDataCalculated: boolean;
  fps: Fps;
  imageQualityFlags: any;
  progressType: ProgressType;
  processing: boolean;
  checkFps: boolean;
  loadingProgress: string;
  start: () => void;
  stop: () => void;
  closeCamera: () => void;
  switchCamera: (useFrontCamera: boolean) => Promise<any>;
}

export enum ProgressType {
  START = 'START',
  CALIBRATING = 'CALIBRATING',
  CALCULATING = 'CALCULATING',
}

const defaultRppgData: RPPGData = {};
const defaultFpsValue: Fps = {
  instantFps: 0,
  averageFps: 0,
  maxFps: 0,
  unsupported: false,
};
const defaultImageQualityFlags = {};

function checkIsAllDataCalculated(measurementData?: MeasurementMeanData, hrvMetrics?: HrvMetrics) {
  const hasBasicMetrics = measurementData && 
    measurementData.bpm > 0 && 
    measurementData.oxygen > 0 && 
    measurementData.rr > 0;
  
  const hasHrvData = hrvMetrics && hrvMetrics.sdnn > 0;
  
  return {
    isAllDataCalculated: !!(hasBasicMetrics && hasHrvData),
  };
}

function normalizeBGRData(data: MeasurementMeanData): MeasurementMeanData {
  return {
    ...data,
    bpm: Math.round(data.bpm),
    oxygen: Math.round(data.oxygen),
    rr: Math.round(data.rr),
  };
}

function normalizeHRVData(data: HrvMetrics): HrvMetrics {
  return {
    ...data,
    sdnn: data.sdnn ? Math.round(data.sdnn) : 0,
    rmssd: data.rmssd ? Math.round(data.rmssd) : 0,
  };
}

function useRPPG({
  videoElement,
  canvasElement,
  useFrontCamera = true,
  userData,
  onUnsupportedDeviceCb,
  onAllDataCalculatedCb,
  onCalculationEndedCb,
  onInterferenceWarningCb,
  onUnstableConditionsWarningCb,
  onFaceOrientWarningCb,
  onFaceSizeWarningCb,
}: UseRPPG): UseRPPGResult {
  const [rppgData, setRppgData] = useReducer((
    state: RPPGData,
    updates: Partial<RPPGData>
  ) => ({ ...state, ...updates }),
    defaultRppgData
  );
  const [ready, setReady] = useState<boolean>(false);
  const [error, setError] = useState<Error | null>(null);
  const [rppgInstance, setRppgInstance] = useState<RPPG>();
  const [isAllDataCalculated, setIsAllDataCalculated] = useState(false);
  const [isCalculationEnded, setIsCalculationEnded] = useState(false);
  const [fps, setFps] = useState<Fps>(defaultFpsValue);
  const [imageQualityFlags, setImageQualityFlags] = useState(defaultImageQualityFlags);
  const [progressType, setProgressType] = useState(ProgressType.START);
  const [processing, setProcessing] = useState(false);
  const [loadingProgress, setLoadingProgress] = useState<string>('Initializing...');
  const checkDeviceDoneTimer = useRef<NodeJS.Timeout | undefined>(undefined);
  const checkDeviceTimer = useRef<NodeJS.Timeout | undefined>(undefined);
  const timeoutTimer = useRef<NodeJS.Timeout | undefined>(undefined);
  const onInterferenceWarningCbRef = useRef<(() => void) | undefined>(undefined);
  const onUnstableConditionsWarningCbRef = useRef<(() => void) | undefined>(undefined);
  const onFaceOrientWarningCbRef = useRef<(() => void) | undefined>(undefined);
  const onFaceSizeWarningCbRef = useRef<(() => void) | undefined>(undefined);

  useEffect(() => {
    let isMounted = true;
    async function initRPPG() {
      if (!videoElement.current) {
        return;
      }
      
      // Check camera permissions first
      try {
        const permissions = await navigator.permissions.query({ name: 'camera' as PermissionName });
        console.log('Camera permission status:', permissions.state);
        
        if (permissions.state === 'denied') {
          const permError = new Error('Camera permission denied. Please allow camera access in browser settings.');
          console.error(permError);
          setError(permError);
          setReady(false);
          return;
        }
      } catch (permError) {
        console.warn('Could not check camera permissions:', permError);
      }

      const onFrame = (frameData: RPPGOnFrame) => {
        if (!isMounted) {
          return;
        }
        
        // Log landmarks if available
        if (frameData.rppgTrackerData.landmarks && frameData.rppgTrackerData.landmarks.length > 0) {
          console.log('SDK Landmarks available:', frameData.rppgTrackerData.landmarks.length);
        }
        
        setFps((fps) => ({
          ...fps,
          maxFps: Math.max(frameData.instantFps, fps.maxFps),
          instantFps: frameData.instantFps,
          averageFps: frameData.averageFps,
        }));
        setImageQualityFlags(frameData.rppgTrackerData.imageQualityFlags);
        setRppgData({ frameData });
      };

      const onMeasurementMeanData = (measurementData: MeasurementMeanData) =>
        isMounted && setRppgData({ measurementData: normalizeBGRData(measurementData) });

      const onMeasurementStatus = (measurementStatus: MeasurementStatus) =>
        isMounted && setRppgData({ measurementStatus });

      const onMeasurementProgress = (measurementProgress: MeasurementProgress) =>
        isMounted && setRppgData({ measurementProgress });

      const onSignalQuality = (signalQuality: SignalQuality) =>
        isMounted && setRppgData({ signalQuality });

      const onMeasurementSignal = (measurementSignal: MeasurementSignal) =>
        isMounted && setRppgData({ measurementSignal });

      const onHrvMetrics = (hrvMetrics: HrvMetrics) =>
        isMounted && setRppgData({ hrvMetrics: normalizeHRVData(hrvMetrics) });

      const onBloodPressure = (bloodPressure: BloodPressure) =>
        isMounted && setRppgData({ bloodPressure });

      const onStressIndex = (stressIndex: StressIndex) =>
        isMounted && setRppgData({ stressIndex });

      const onInterferenceWarning = () =>
        isMounted &&
        typeof onInterferenceWarningCbRef.current === 'function' &&
        onInterferenceWarningCbRef.current();

      const onUnstableConditionsWarning = () =>
        isMounted &&
        typeof onUnstableConditionsWarningCbRef.current === 'function' &&
        onUnstableConditionsWarningCbRef.current();

      const onFaceOrientWarning = () =>
        isMounted &&
        typeof onFaceOrientWarningCbRef.current === 'function' &&
        onFaceOrientWarningCbRef.current();

      const onFaceSizeWarning = () =>
        isMounted &&
        typeof onFaceSizeWarningCbRef.current === 'function' &&
        onFaceSizeWarningCbRef.current();

      const rppg = new RPPG({
        serverless: false,
        skipSocketWhenNoFace: true,
        // camera config
        rppgCameraConfig: {
          useFrontCamera,
          videoElement: videoElement.current,
          canvasElement: canvasElement?.current,
          width: 640,
          height: 480,
          onError: (error: Error) => {
            console.error('Camera error:', error);
            setError(error);
          },
          onSuccess: (data: any) => {
            console.log('Camera initialized successfully:', data);
          },
        },

        // tracker config
        rppgTrackerConfig: {
          maxTimeBetweenBlinksSeconds: 60,
          pathToWasmData: '/assets/js/lib/',
        },

        // socket config
        rppgSocketConfig: {
          authToken: AUTH_TOKEN,
          url: SOCKET_URL,
          query: new URLSearchParams({
            authToken: AUTH_TOKEN,
            sex: userData.sex,
            age: userData.age.toString(),
            weight: userData.weight.toString(),
            height: userData.height.toString(),
          }).toString(),
          onConnect: () => {
            setReady(true);
            setError(null);
          },
          onError: () => {
            setReady(false);
            setError(new Error('Error connecting socket'));
          },
          onClose: () => {
            setReady(false);
            setError(new Error('Error connecting socket'));
          },
        },
  
        onFrame,
        onMeasurementMeanData,
        onMeasurementStatus,
        onMeasurementProgress,
        onSignalQuality,
        onMeasurementSignal,
        onHrvMetrics,
        onBloodPressure,
        onStressIndex,
        onInterferenceWarning,
        onUnstableConditionsWarning,
        onFaceOrientWarning,
        onFaceSizeWarning,
      });

      setRppgInstance(rppg);

      console.log('Initializing RPPG with user data:', userData);
      console.log('WebSocket URL:', SOCKET_URL);
      console.log('Auth token:', AUTH_TOKEN);
      setLoadingProgress('Loading WASM modules...');
      
      // Try to initialize RPPG with retry logic for camera
      let retryCount = 0;
      const maxRetries = 3;
      let initSuccess = false;
      
      while (retryCount < maxRetries && !initSuccess) {
        try {
          setLoadingProgress('Loading face detection models...');
          await rppg.init();
          initSuccess = true;
          setReady(true);
          setLoadingProgress('Ready');
          console.log('RPPG SDK initialized successfully');
        } catch (error: any) {
          retryCount++;
          console.error(`Camera initialization attempt ${retryCount} failed:`, error);
          
          // Check for specific error types
          if (error?.message?.includes('object can not be found')) {
            console.error('Camera device not found. Please check:');
            console.error('1. Camera is connected and enabled');
            console.error('2. Browser has camera permissions');
            console.error('3. No other application is using the camera');
          }
          
          if (retryCount < maxRetries) {
            console.log(`Retrying camera initialization in 2 seconds...`);
            await new Promise(resolve => setTimeout(resolve, 2000));
          } else {
            console.error('Failed to initialize camera after', maxRetries, 'attempts');
            setError(error);
            setReady(false);
          }
        }
      }
    }

    initRPPG();

    return () => {
      isMounted = false;
    };
    
  }, [videoElement, canvasElement, userData, useFrontCamera]);

  useEffect(() => {
    const {
      isAllDataCalculated,
    } = checkIsAllDataCalculated(rppgData.measurementData, rppgData.hrvMetrics);
    setIsAllDataCalculated(isAllDataCalculated);
  }, [rppgData.measurementData, rppgData.hrvMetrics]);

  const start = () => {
    if (!rppgInstance) {
      console.error('Not initialized');
      return;
    }
    setRppgData(defaultRppgData);
    setProgressType(ProgressType.CALIBRATING);
    setProcessing(true);
    setIsCalculationEnded(false);
    rppgInstance.start();
    startFPSCheckTimer();
    startTimeoutTimer();
  };

  const stop = () => {
    if (!rppgInstance) {
      console.error('Not initialized');
      return;
    }
    setProgressType(ProgressType.START);
    setProcessing(false);
    clearTimeout(checkDeviceDoneTimer.current);
    checkDeviceDoneTimer.current = undefined;
    clearTimeout(checkDeviceTimer.current);
    checkDeviceTimer.current = undefined;
    clearTimeout(timeoutTimer.current);
    timeoutTimer.current = undefined;
    rppgInstance.stop();
  };

  const closeCamera = () => rppgInstance?.closeCamera();

  const switchCamera = (useFrontCamera: boolean) =>
    rppgInstance?.switchCamera(useFrontCamera) || Promise.resolve(undefined);

  // Timers
  const startFPSCheckDoneTimer = () => {
    checkDeviceDoneTimer.current = setTimeout(() => {
      checkDeviceTimer.current = undefined;
      checkDeviceDoneTimer.current = undefined;
      setProgressType(ProgressType.CALCULATING);
    }, FPS_CHECK_DONE_TIMEOUT);
  };

  const startFPSCheckTimer = () =>
    checkDeviceTimer.current = setTimeout(() => {
      setFps((fps) => ({
        ...fps,
        unsupported: fps.maxFps < FPS_CHECK_THRESHOLD,
      }));
      startFPSCheckDoneTimer();
    }, FPS_CHECK_TIMEOUT);

  const startTimeoutTimer = () =>
    timeoutTimer.current = setTimeout(() => {
      setIsCalculationEnded(true);
      console.log('Stop - Timeout');
    }, CALCULATION_TIMEOUT);

  // callback events
  // onUnsupportedDeviceCb event
  useEffect(() => {
    if (fps.unsupported && typeof onUnsupportedDeviceCb === 'function') {
      onUnsupportedDeviceCb();
    }
  }, [fps.unsupported, onUnsupportedDeviceCb]);

  // onAllDataCalculatedCb event
  useEffect(() => {
    if (isAllDataCalculated && typeof onAllDataCalculatedCb === 'function') {
      onAllDataCalculatedCb();
    }
  }, [isAllDataCalculated, onAllDataCalculatedCb]);

  // onCalculationEndedCb event
  useEffect(() => {
    if (isCalculationEnded && typeof onCalculationEndedCb === 'function') {
      onCalculationEndedCb();
    }
  }, [isCalculationEnded, onCalculationEndedCb]);

  // onInterferenceWarningCb Event
  useEffect(() => {
    onInterferenceWarningCbRef.current = onInterferenceWarningCb;
  }, [onInterferenceWarningCb]);

  // onUnstableConditionsWarning event
  useEffect(() => {
    onUnstableConditionsWarningCbRef.current = onUnstableConditionsWarningCb;
  }, [onUnstableConditionsWarningCb]);

  // onFaceOrientWarningCb event
  useEffect(() => {
    onFaceOrientWarningCbRef.current = onFaceOrientWarningCb;
  }, [onFaceOrientWarningCb]);

  // onFaceSizeWarningCb event
  useEffect(() => {
    onFaceSizeWarningCbRef.current = onFaceSizeWarningCb;
  }, [onFaceSizeWarningCb]);

  return {
    rppgData,
    ready,
    error,
    rppgInstance,
    isAllDataCalculated,
    fps,
    imageQualityFlags,
    progressType,
    processing,
    checkFps: Boolean(checkDeviceDoneTimer.current || checkDeviceTimer.current),
    loadingProgress,
    start,
    stop,
    closeCamera,
    switchCamera,
  };
}

export default useRPPG;