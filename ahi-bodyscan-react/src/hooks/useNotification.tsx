import { useState, useCallback } from 'react';

export interface NotificationMessage {
  id: string;
  text: string;
  type: 'info' | 'warning' | 'error';
  timeout?: number;
}

function useNotification() {
  const [message, setMessage] = useState<NotificationMessage | null>(null);

  const addNotification = useCallback((notification: NotificationMessage) => {
    setMessage(notification);
    
    if (notification.timeout) {
      setTimeout(() => {
        setMessage(null);
      }, notification.timeout);
    }
  }, []);

  const clearAllNotifications = useCallback(() => {
    setMessage(null);
  }, []);

  return {
    message,
    addNotification,
    clearAllNotifications,
  };
}

export default useNotification;