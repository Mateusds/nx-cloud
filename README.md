# NX-Cloud System 🚀

O sistema definitivo de sincronização de saves e gerenciamento para Nintendo Switch.

## Estrutura do Projeto

Este repositório contém dois componentes principais:

- **[/portal](./portal):** Aplicação web Next.js 15. Inclui Dashboard do usuário, Painel Administrativo, Autenticação Google OAuth e API completa com Prisma 7.
- **[/switch-app](./switch-app):** Aplicativo nativo (Homebrew) para Nintendo Switch desenvolvido em C++ com `libnx` e `devkitPro`.

---

## Stack Tecnológica

### Portal Web
- **Next.js 15** & **React 19**
- **Prisma 7** (PostgreSQL)
- **Google OAuth** para autenticação segura.
- **Tailwind CSS** para design premium.

### Switch App
- **devkitPro** & **libnx**
- **libcurl** para requisições HTTPS (com bypass de SSL).
- **nlohmann/json** para comunicação com a API.
- **QR Code** integrado em ASCII para pareamento fácil.

---

## Fluxo de Pareamento (Switch -> Web)

1. O **App Switch** gera um `deviceToken` único.
2. O usuário escaneia o **QR Code** exibido no Switch.
3. O link leva à página de autenticação no portal web.
4. Após o login com Google, a sessão é vinculada ao console.
5. O Switch detecta a conexão automaticamente e libera o acesso aos saves.

---

## Como Executar

### 1. Portal (Backend)
```bash
cd portal
npm install
npm run dev
```

### 2. Switch App (Compilação)
Requisitos: devkitPro instalado e configurado.
```bash
cd switch-app
make
```

## Automação de Deploy (AWS)
O projeto conta com **GitHub Actions** para deploy automático na AWS EC2 sempre que houver um push na branch `main`.

## Desenvolvedor
- **Mateusds** (Mateus Marques)

---
*Versão atual do ecossistema: 1.0.1*
