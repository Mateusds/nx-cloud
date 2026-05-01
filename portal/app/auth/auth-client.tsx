'use client';

import dynamic from 'next/dynamic';
import { useSearchParams } from 'next/navigation';
import { useEffect, useMemo, useState } from 'react';

const GoogleOAuthProvider = dynamic(
  () => import('@react-oauth/google').then((mod) => mod.GoogleOAuthProvider),
  { ssr: false }
);

const GoogleLogin = dynamic(
  () => import('@react-oauth/google').then((mod) => mod.GoogleLogin),
  { ssr: false }
);

function AuthPanel() {
  const searchParams = useSearchParams();
  const [sessionId, setSessionId] = useState<string | null>(null);
  const [deviceInfo, setDeviceInfo] = useState<any>(null);
  const [message, setMessage] = useState<string | null>(null);
  const [googleReady, setGoogleReady] = useState(false);
  const clientId = process.env.NEXT_PUBLIC_GOOGLE_CLIENT_ID ?? '';

  useEffect(() => {
    const fromQuery = searchParams.get('sessionId');
    const fromStorage = window.sessionStorage.getItem('nx-cloud-session-id');
    const resolvedSessionId = fromQuery || fromStorage;

    if (resolvedSessionId) {
      setSessionId(resolvedSessionId);
      window.sessionStorage.setItem('nx-cloud-session-id', resolvedSessionId);
    }
  }, [searchParams]);

  useEffect(() => {
    if (sessionId) {
      fetch(`/api/session/status?sessionId=${sessionId}`)
        .then((res) => res.json())
        .then((data) => {
          if (!data.error) {
            setDeviceInfo(data);
          }
        })
        .catch(console.error);
    }
    setGoogleReady(true);
  }, [sessionId]);

  const title = useMemo(() => {
    return sessionId ? `Sessão ${sessionId}` : 'Nenhuma sessão encontrada';
  }, [sessionId]);

  const handleSuccess = async (credentialResponse: { credential?: string | null }) => {
    if (!credentialResponse.credential) {
      setMessage('O Google não retornou uma credencial válida.');
      return;
    }

    try {
      const response = await fetch('/api/auth/google', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          token: credentialResponse.credential,
          sessionId,
        }),
      });

      const data = (await response.json()) as { error?: string; userId?: string; sessionId?: string };

      if (!response.ok) {
        throw new Error(data.error ?? 'Não foi possível vincular a conta.');
      }

      if (data.userId && data.sessionId) {
        localStorage.setItem('nx-cloud-user-id', data.userId);
        localStorage.setItem('nx-cloud-session-id', data.sessionId);
      }

      setMessage('Autenticação concluída!');
      
      // Notify parent window and close popup
      setTimeout(() => {
        if (window.opener && !window.opener.closed) {
          window.opener.postMessage({ type: 'AUTH_SUCCESS', userId: data.userId }, '*');
        }
        window.close();
      }, 1000);
    } catch (caughtError) {
      const text = caughtError instanceof Error ? caughtError.message : 'Erro inesperado';
      setMessage(text);
    }
  };

  if (!clientId) {
    return (
      <main className="auth-shell">
        <section className="card auth-card">
          <span className="auth-brand">NX Cloud Auth</span>
          <h1 className="auth-title">Google Client ID ausente</h1>
          <p className="auth-text">
            Defina <strong>NEXT_PUBLIC_GOOGLE_CLIENT_ID</strong> no arquivo <code>.env.local</code> do portal para habilitar o login.
          </p>
          <div className="auth-session">
            <strong>Sessão:</strong> {title}
          </div>
        </section>
      </main>
    );
  }

  return (
    <GoogleOAuthProvider clientId={clientId}>
      <main className="auth-shell">
        <section className="card auth-card">
          <span className="auth-brand">NX Cloud Auth</span>
          <h1 className="auth-title">Conectar conta Google</h1>
          <p className="auth-text">
            Faça login para vincular a sessão do dispositivo ao seu perfil.
          </p>
          
          {deviceInfo && (
            <div className="device-info" style={{ marginBottom: '1.5rem', background: 'rgba(255,255,255,0.05)', padding: '1rem', borderRadius: '8px', border: '1px solid rgba(255,255,255,0.1)' }}>
              <h3 style={{ margin: '0 0 0.5rem 0', fontSize: '1rem', color: '#fff' }}>Aparelho Detectado</h3>
              <p style={{ margin: '0 0 0.25rem 0', fontSize: '0.875rem', color: '#ccc' }}><strong>Nome:</strong> {deviceInfo.deviceName || 'Nintendo Switch'}</p>
              <div style={{ display: 'flex', gap: '1rem', fontSize: '0.875rem', color: '#aaa', marginTop: '0.5rem' }}>
                <div>
                  <strong>Armazenamento Interno:</strong><br />
                  {deviceInfo.nandFree || 'N/A'} livre / {deviceInfo.nandTotal || 'N/A'}
                </div>
                <div>
                  <strong>Cartão SD:</strong><br />
                  {deviceInfo.sdFree || 'N/A'} livre / {deviceInfo.sdTotal || 'N/A'}
                </div>
              </div>
            </div>
          )}

          <div className="auth-session">
            <strong>Sessão:</strong> {title}
          </div>
          <div className="google-slot">
            {googleReady ? (
              <GoogleLogin
                onSuccess={handleSuccess}
                onError={() => setMessage('Falha ao iniciar o login do Google.')}
                theme="filled_blue"
                useOneTap
              />
            ) : (
              <div>Carregando...</div>
            )}
          </div>
          {!sessionId ? (
            <div className="banner">
              O sessionId não foi encontrado. Abra esta tela pela URL gerada na home.
            </div>
          ) : null}
          {message ? <div className="banner">{message}</div> : null}
        </section>
      </main>
    </GoogleOAuthProvider>
  );
}

export default function AuthClient() {
  return <AuthPanel />;
}
