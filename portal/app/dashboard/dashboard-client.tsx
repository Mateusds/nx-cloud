'use client';

import { useState, useCallback, useEffect } from 'react';

interface Save {
  id: string;
  filename: string;
  gameTitle: string | null;
  description: string | null;
  filePath: string;
  fileSize: number;
  createdAt: string;
  updatedAt: string;
}

interface User {
  id: string;
  email: string;
  name: string | null;
  sessions?: Array<{
    deviceName: string | null;
    sdTotal: string | null;
    sdFree: string | null;
    nandTotal: string | null;
    nandFree: string | null;
    updatedAt: string;
  }>;
}

interface DashboardClientProps {
  userId: string;
}

function formatFileSize(bytes: number): string {
  if (bytes === 0) return '0 Bytes';
  const k = 1024;
  const sizes = ['Bytes', 'KB', 'MB', 'GB'];
  const i = Math.floor(Math.log(bytes) / Math.log(k));
  return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
}

export default function DashboardClient({ userId }: DashboardClientProps) {
  const [user, setUser] = useState<User | null>(null);
  const [saves, setSaves] = useState<Save[]>([]);
  const [loading, setLoading] = useState(true);
  const [uploading, setUploading] = useState(false);
  const [gameTitle, setGameTitle] = useState('');
  const [description, setDescription] = useState('');
  const [message, setMessage] = useState<string | null>(null);

  useEffect(() => {
    const fetchData = async () => {
      try {
        const [savesRes, userRes] = await Promise.all([
          fetch(`/api/saves?userId=${userId}`),
          fetch(`/api/auth/me?userId=${userId}`)
        ]);
        
        const savesData = await savesRes.json();
        if (savesRes.ok) {
          setSaves(savesData.saves || []);
        }

        const userData = await userRes.json();
        if (userRes.ok) {
          setUser(userData.user);
        }
      } catch (err) {
        console.error('Erro ao buscar dados:', err);
      } finally {
        setLoading(false);
      }
    };
    fetchData();
  }, [userId]);

  const handleFileUpload = useCallback(async (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file || !userId) return;

    setUploading(true);
    setMessage(null);

    try {
      const formData = new FormData();
      formData.append('file', file);
      formData.append('userId', userId);
      formData.append('gameTitle', gameTitle);
      formData.append('description', description);

      const response = await fetch('/api/saves', {
        method: 'POST',
        body: formData,
      });

      const data = await response.json();

      if (!response.ok) {
        throw new Error(data.error || 'Erro ao fazer upload');
      }

      setSaves((prev) => [data.save, ...prev]);
      setMessage('Upload concluído com sucesso!');
      setGameTitle('');
      setDescription('');
      e.target.value = '';
    } catch (err) {
      setMessage(err instanceof Error ? err.message : 'Erro ao fazer upload');
    } finally {
      setUploading(false);
    }
  }, [userId, gameTitle, description]);

  const handleDelete = useCallback(async (saveId: string) => {
    if (!confirm('Tem certeza que deseja excluir este save?')) return;

    try {
      const response = await fetch(`/api/saves/${saveId}?userId=${userId}`, {
        method: 'DELETE',
      });

      if (!response.ok) {
        const data = await response.json();
        throw new Error(data.error || 'Erro ao excluir');
      }

      setSaves((prev) => prev.filter((s) => s.id !== saveId));
      setMessage('Save excluído com sucesso!');
    } catch (err) {
      setMessage(err instanceof Error ? err.message : 'Erro ao excluir');
    }
  }, [userId]);

  const handleLogout = () => {
    localStorage.removeItem('nx-cloud-user-id');
    localStorage.removeItem('nx-cloud-session-id');
    window.location.href = '/';
  };

  return (
    <main className="dashboard-shell">
      <header className="dashboard-header">
        <div className="header-content">
          <h1 className="dashboard-title">Meus Saves</h1>
          <div style={{ display: 'flex', alignItems: 'center', gap: '16px' }}>
            <span className="user-email">{user?.name || user?.email || userId}</span>
            <button
              onClick={handleLogout}
              className="btn-delete"
              style={{ padding: '8px 16px', fontSize: '0.9rem' }}
            >
              Sair
            </button>
          </div>
        </div>
      </header>

      {loading && <div className="empty-state">Carregando...</div>}

      {!loading && (() => {
        const activeSession = user?.sessions?.find(s => s.deviceName && s.nandTotal);
        if (!activeSession) return null;
        
        return (
          <section className="device-section" style={{ marginBottom: '2rem' }}>
            <div className="upload-card" style={{ borderLeft: '4px solid #00ff96' }}>
              <h2 className="section-title">Dispositivo Conectado</h2>
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(200px, 1fr))', gap: '20px' }}>
                <div>
                  <p style={{ color: '#aaa', fontSize: '0.8rem', marginBottom: '4px' }}>NOME DO APARELHO</p>
                  <p style={{ fontSize: '1.1rem', fontWeight: 'bold' }}>{activeSession.deviceName}</p>
                </div>
                <div>
                  <p style={{ color: '#aaa', fontSize: '0.8rem', marginBottom: '4px' }}>ARMAZENAMENTO INTERNO</p>
                  <p style={{ fontSize: '1.1rem', fontWeight: 'bold' }}>{activeSession.nandFree} / {activeSession.nandTotal}</p>
                </div>
                <div>
                  <p style={{ color: '#aaa', fontSize: '0.8rem', marginBottom: '4px' }}>SD CARD</p>
                  <p style={{ fontSize: '1.1rem', fontWeight: 'bold' }}>{activeSession.sdFree} / {activeSession.sdTotal}</p>
                </div>
              </div>
            </div>
          </section>
        );
      })()}

      <section className="upload-section">
        <div className="upload-card">
          <h2 className="section-title">Novo Upload</h2>
          
          <div className="form-group">
            <label htmlFor="gameTitle">Título do Jogo</label>
            <input
              id="gameTitle"
              type="text"
              value={gameTitle}
              onChange={(e) => setGameTitle(e.target.value)}
              placeholder="Ex: The Legend of Zelda"
            />
          </div>

          <div className="form-group">
            <label htmlFor="description">Descrição</label>
            <textarea
              id="description"
              value={description}
              onChange={(e) => setDescription(e.target.value)}
              placeholder="Descrição opcional do save..."
              rows={3}
            />
          </div>

          <div className="form-group">
            <label htmlFor="file" className="file-label">
              {uploading ? 'Enviando...' : 'Selecionar arquivo'}
            </label>
            <input
              id="file"
              type="file"
              onChange={handleFileUpload}
              disabled={uploading}
              className="file-input"
            />
          </div>

          {message && (
            <div className={`banner ${message.includes('sucesso') ? 'success' : 'error'}`}>
              {message}
            </div>
          )}
        </div>
      </section>

      <section className="saves-section">
        <h2 className="section-title">Saves ({saves.length})</h2>
        
        {saves.length === 0 ? (
          <div className="empty-state">
            <p>Nenhum save enviado ainda.</p>
          </div>
        ) : (
          <div className="saves-grid">
            {saves.map((save) => (
              <div key={save.id} className="save-card">
                <div className="save-header">
                  <h3 className="save-title">{save.gameTitle || 'Sem título'}</h3>
                  <span className="save-date">
                    {new Date(save.updatedAt).toLocaleDateString('pt-BR')}
                  </span>
                </div>
                
                {save.description && (
                  <p className="save-description">{save.description}</p>
                )}
                
                <div className="save-meta">
                  <span className="file-name">{save.filename}</span>
                  <span className="file-size">{formatFileSize(save.fileSize)}</span>
                </div>

                <div className="save-actions">
                  <a
                    href={save.filePath}
                    download
                    className="btn-download"
                  >
                    Download
                  </a>
                  <button
                    onClick={() => handleDelete(save.id)}
                    className="btn-delete"
                  >
                    Excluir
                  </button>
                </div>
              </div>
            ))}
          </div>
        )}
      </section>
    </main>
  );
}
