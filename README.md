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

Crie `portal/.env.local` com algo como:

```env
DATABASE_URL="postgresql://user:password@localhost:5432/nx_cloud?schema=public"
NEXT_PUBLIC_GOOGLE_CLIENT_ID="seu-google-client-id"
```

## Rotas da Aplicação

### Páginas
- `/` - página inicial com controle de acesso por role
- `/auth` - tela de autenticação Google (popup)
- `/dashboard` - painel de gerenciamento de saves (usuário)
- `/admin` - painel administrativo (apenas ADMIN)

### API Routes
- `GET /api/health` - checagem de banco
- `POST /api/session/init` - criação de sessão (opcional: deviceToken para Switch)
- `GET /api/session/status?deviceToken=xxx` - status da sessão para Switch
- `POST /api/auth/google` - autenticação e criação de usuário (com name do Google)
- `GET /api/auth/me` - dados do usuário logado
- `GET /api/saves?userId=xxx` - lista saves do usuário
- `POST /api/saves` - upload de save (multipart/form-data)
- `DELETE /api/saves/[id]` - remove save
- `GET /api/admin/users` - lista todos usuários (admin only)
- `POST /api/admin/promote` - promove usuário a admin
- `GET /api/app/version` - versão atual do app Switch e URL de download

## Modelos do Banco (Prisma)

```prisma
enum UserRole {
  USER
  ADMIN
}

model User {
  id            String   @id @default(uuid())
  email         String   @unique
  name          String?  # Nome do Google
  role          UserRole @default(USER)
  googleRefresh String?
  msRefresh     String?
  createdAt     DateTime @default(now())
  updatedAt     DateTime @updatedAt
  saves         Save[]
  sessions      Session[]
}

model Session {
  id          String   @id @default(uuid())
  status      String   @default("PENDING")
  deviceToken String?  @unique  # Token único do Switch
  userId      String?
  user        User?    @relation(fields: [userId], references: [id])
  createdAt   DateTime @default(now())
}

model Save {
  id          String   @id @default(uuid())
  filename    String
  gameTitle   String?
  description String?
  filePath    String
  fileSize    Int
  userId      String
  createdAt   DateTime @default(now())
  updatedAt   DateTime @updatedAt
  user        User     @relation(fields: [userId], references: [id])
}
```

## Sistema de Atualização Automática (Switch App)

O app Switch possui sistema de atualização automática integrado:

### Como funciona:

1. Ao iniciar, o app verifica a versão atual via `GET /api/app/version`
2. Se houver nova versão disponível:
   - Exibe mensagem "NOVA VERSÃO DISPONÍVEL!"
   - Mostra changelog da atualização
   - Pergunta se deseja atualizar (A) Sim (B) Não
3. Se usuário confirmar:
   - Baixa novo `.nro` para arquivo temporário
   - Remove arquivo antigo
   - Instala nova versão
   - Solicita reinício do app

### Automação de Versão (Recomendado):

O app do Switch busca a versão mais recente diretamente da **última release publicada no GitHub**.

1. **Desenvolva e compile:** Gere o novo `AppSwitch.nro`.
2. **Crie uma Release no GitHub:**
   - Acesse `Releases` -> `Draft a new release`.
   - Use uma tag de versão (ex: `v0.6.0`).
   - Escreva as novidades no campo **Description**.
   - Anexe o arquivo `AppSwitch.nro`.
   - Clique em **Publish release**.
3. **Consumo automático:**
   - A API `GET /api/app/version` consulta a última release publicada.
   - O Switch baixa o asset `AppSwitch.nro` da release mais recente.
   - O changelog exibido vem da descrição da release.
   - Não é preciso editar o código do portal para cada nova versão.

**Vantagens:**
- Sempre usa a release mais recente sem editar arquivo manualmente.
- O download aponta para o asset publicado no GitHub.
- O changelog fica centralizado na própria release.
- O fluxo funciona sem workflow de atualização de arquivo.

## Configurando um Admin

Após primeiro login, execute no PostgreSQL:

```sql
UPDATE "User" SET role = 'ADMIN' WHERE email = 'seu-email@gmail.com';
```

## Observações

- O portal web usa Prisma 7 com `@prisma/adapter-pg`
- O backend Express antigo foi removido da execução principal
- Controle de acesso baseado em roles (USER/ADMIN)
- Uploads armazenados em `portal/public/uploads/`
- Sessões web gerenciadas via localStorage
- Switch usa `deviceToken` único para pareamento com conta Google
- Nome do usuário é extraído automaticamente do Google OAuth
- App Switch exibe QR code em ASCII escaneável
- App Switch possui sistema de atualização automática integrado
- Atualizações do app são opcionais ou forçadas via `forceUpdate`
