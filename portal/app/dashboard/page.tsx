'use client';

import { useEffect, useState } from 'react';
import DashboardClient from './dashboard-client';

export default function DashboardPage() {
  const [userId, setUserId] = useState<string | null>(null);

  useEffect(() => {
    const storedUserId = localStorage.getItem('nx-cloud-user-id');
    if (storedUserId) {
      setUserId(storedUserId);
    } else {
      window.location.href = '/';
    }
  }, []);

  if (!userId) {
    return (
      <main className="dashboard-shell">
        <div className="empty-state">Verificando autenticação...</div>
      </main>
    );
  }

  return <DashboardClient userId={userId} />;
}
