'use client';

import { useEffect, useState } from 'react';

interface User {
  id: string;
  email: string;
  name: string | null;
  role: string;
  createdAt: string;
  updatedAt: string;
  _count: {
    saves: number;
    sessions: number;
  };
}

export default function AdminPage() {
  const [users, setUsers] = useState<User[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [adminUserId, setAdminUserId] = useState<string | null>(null);

  useEffect(() => {
    const storedUserId = localStorage.getItem('nx-cloud-user-id');
    if (storedUserId) {
      setAdminUserId(storedUserId);
    } else {
      window.location.href = '/';
    }
  }, []);

  useEffect(() => {
    if (!adminUserId) return;

    const fetchUsers = async () => {
      try {
        const response = await fetch(`/api/admin/users?adminUserId=${adminUserId}`);
        const data = await response.json();

        if (!response.ok) {
          throw new Error(data.error || 'Erro ao carregar usuários');
        }

        setUsers(data.users);
      } catch (err) {
        setError(err instanceof Error ? err.message : 'Erro desconhecido');
      } finally {
        setLoading(false);
      }
    };

    fetchUsers();
  }, [adminUserId]);

  if (loading) {
    return (
      <main className="dashboard-shell">
        <div className="empty-state">Carregando painel de admin...</div>
      </main>
    );
  }

  if (error) {
    return (
      <main className="dashboard-shell">
        <div className="banner error">{error}</div>
        <button className="primary-button" onClick={() => window.location.href = '/'} style={{ marginTop: '20px' }}>
          Voltar para Home
        </button>
      </main>
    );
  }

  return (
    <main className="dashboard-shell">
      <header className="dashboard-header">
        <div className="header-content">
          <h1 className="dashboard-title">Painel Administrativo</h1>
          <span className="user-email">Acesso restrito</span>
        </div>
      </header>

      <section className="saves-section">
        <h2 className="section-title">Usuários ({users.length})</h2>

        {users.length === 0 ? (
          <div className="empty-state">
            <p>Nenhum usuário cadastrado.</p>
          </div>
        ) : (
          <div className="saves-grid">
            {users.map((user) => (
              <div key={user.id} className="save-card">
                <div className="save-header">
                  <h3 className="save-title">{user.name || user.email}</h3>
                  <span className={`role-badge ${user.role.toLowerCase()}`}>
                    {user.role}
                  </span>
                </div>

                <div className="save-meta">
                  <span className="stat-item">
                    <strong>Saves:</strong> {user._count.saves}
                  </span>
                  <span className="stat-item">
                    <strong>Sessões:</strong> {user._count.sessions}
                  </span>
                </div>

                <div className="save-meta">
                  <span className="file-name">ID: {user.id.slice(0, 8)}...</span>
                  <span className="file-size">
                    Cadastrado: {new Date(user.createdAt).toLocaleDateString('pt-BR')}
                  </span>
                </div>
              </div>
            ))}
          </div>
        )}
      </section>
    </main>
  );
}
