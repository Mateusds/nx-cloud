import SessionLauncher from './_components/session-launcher';

export default function HomePage() {
  return (
    <main>
      <div className="shell">
        <section className="hero-grid">
          <div className="card">
            <div className="card-inner">
              <span className="kicker">NX Cloud console</span>
              <h1 className="title">Sessões e login123 agora vivem inteiramente no Next.</h1>
              <p className="lead">
                A antiga divisão entre Vite e Express foi removida. A criação de sessão,
                a página de autenticação e as rotas de backend agora estão todas no mesmo app.
              </p>

              <div className="feature-list">
                <div className="feature">
                  <strong>Frontend Next</strong>
                  <span>Páginas server e client components no App Router.</span>
                </div>
                <div className="feature">
                  <strong>Backend Next</strong>
                  <span>Rotas API substituindo o servidor Express separado.</span>
                </div>
                <div className="feature">
                  <strong>Prisma v7</strong>
                  <span>Client com adapter PostgreSQL e schema no projeto Next.</span>
                </div>
              </div>
            </div>
          </div>

          <SessionLauncher />
        </section>
      </div>
    </main>
  );
}
