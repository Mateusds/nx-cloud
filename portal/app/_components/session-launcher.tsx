'use client';

import { useEffect, useState } from 'react';

export default function SessionLauncher() {
  const [loading, setLoading] = useState(false);
  const [sessionId, setSessionId] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [userRole, setUserRole] = useState<string | null>(null);

  useEffect(() => {
    const checkAuth = async () => {
      const storedUserId = localStorage.getItem('nx-cloud-user-id');
      if (storedUserId) {
        try {
          const response = await fetch(`/api/auth/me?userId=${storedUserId}`);
          if (response.ok) {
            const data = await response.json();
            setUserRole(data.user?.role || null);
          }
        } catch {
          // Ignore error
        }
      }
    };
    checkAuth();

    // Listen for auth success from popup
    const handleMessage = (event: MessageEvent) => {
      if (event.data?.type === 'AUTH_SUCCESS') {
        window.location.href = '/dashboard';
      }
    };
    window.addEventListener('message', handleMessage);
    return () => window.removeEventListener('message', handleMessage);
  }, []);

  const handleAuth = async () => {
    setLoading(true);
    setError(null);

    try {
      const response = await fetch('/api/session/init', {
        method: 'POST',
      });

      const data = (await response.json()) as {
        sessionId?: string;
        authUrl?: string;
        error?: string;
      };

      if (!response.ok) {
        throw new Error(data.error ?? 'Não foi possível criar a sessão.');
      }

      const newSessionId = data.sessionId;
      const newAuthUrl = data.authUrl;

      if (newSessionId) {
        setSessionId(newSessionId);
      }

      if (newAuthUrl) {
        const width = 500;
        const height = 600;
        const left = window.screenX + (window.outerWidth - width) / 2;
        const top = window.screenY + (window.outerHeight - height) / 2;
        window.open(
          newAuthUrl,
          'googleAuth',
          `width=${width},height=${height},left=${left},top=${top},resizable=yes,scrollbars=yes`
        );
      } else {
        throw new Error('URL de autenticação não retornada.');
      }
    } catch (caughtError) {
      const message = caughtError instanceof Error ? caughtError.message : 'Erro inesperado';
      setError(message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <aside className="card">
      <div className="card-inner">
        <h2 className="panel-title">
          {userRole === 'ADMIN' ? 'Painel Admin' : userRole === 'USER' ? 'Bem-vindo' : 'Fluxo de autenticação'}
        </h2>
        <p className="panel-copy">
          {userRole === 'ADMIN' 
            ? 'Você tem acesso administrativo ao portal.' 
            : userRole === 'USER'
            ? 'Acesse seus saves ou o painel administrativo.'
            : 'Clique para criar uma sessão e abrir a página de login do Google automaticamente.'}
        </p>

        {!userRole && (
          <>
            <button
              className="primary-button"
              onClick={handleAuth}
              disabled={loading}
              style={{ width: '100%' }}
            >
              {loading ? 'Criando sessão...' : 'Conectar com Google'}
            </button>
            {sessionId && (
              <div className="status-box" style={{ marginTop: '1rem' }}>
                <span className="status-label">Sessão criada</span>
                <div className="status-value">{sessionId}</div>
              </div>
            )}
            {error ? <div className="banner" style={{ marginTop: '1rem' }}>{error}</div> : null}
            <p className="inline-note" style={{ marginTop: '1rem' }}>
              Uma janela popup será aberta para autenticação. Após o login, você será redirecionado.
            </p>
          </>
        )}

        {userRole && (
          <>
            <button
              className="primary-button"
              onClick={() => window.location.href = '/dashboard'}
              style={{ width: '100%', marginBottom: '12px' }}
            >
              Meus Saves
            </button>
            {userRole === 'ADMIN' && (
              <button
                className="secondary-button"
                onClick={() => window.location.href = '/admin'}
                style={{ width: '100%' }}
              >
                Painel Administrativo
              </button>
            )}
          </>
        )}
      </div>
    </aside>
  );
}
