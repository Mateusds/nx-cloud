# NX-Cloud System 🚀

O sistema definitivo de sincronização de saves e gerenciamento para Nintendo Switch.

## Estrutura do Projeto

Este repositório contém dois componentes principais:

- **[/portal](./portal):** O Backend completo feito em Next.js. Inclui a API de gerenciamento, autenticação Google OAuth, integração com banco de dados Prisma e o Dashboard Administrativo.
- **[/switch-app](./switch-app):** O aplicativo Homebrew para Nintendo Switch desenvolvido em C++ (utilizando libnx e devkitPro).

---

## Como Rodar Localmente

### 1. Portal (Backend)
```bash
cd portal
npm install
npm run dev
```

### 2. Switch App (Frontend)
Certifique-se de ter o `devkitPro` instalado.
```bash
cd switch-app
make
```

## Desenvolvedor
- **Mateusds** (Mateus Marques)

---
*Este projeto está em desenvolvimento ativo. Versão atual do ecossistema: 1.0.1*
