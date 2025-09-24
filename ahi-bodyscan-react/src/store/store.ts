import { configureStore } from '@reduxjs/toolkit';

// Placeholder reducers - to be implemented
const authReducer = (state = { isAuthenticated: false }, action: any) => state;
const assessmentReducer = (state = { assessments: [] }, action: any) => state;

export const store = configureStore({
  reducer: {
    auth: authReducer,
    assessment: assessmentReducer,
  },
});

export type RootState = ReturnType<typeof store.getState>;
export type AppDispatch = typeof store.dispatch;