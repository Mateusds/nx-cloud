# NX-Cloud System

NX-Cloud System é a plataforma completa para gerenciamento de saves do Nintendo Switch, incluindo portal web, sistema de autenticação e aplicativo nativo para o console.

## Visão geral

O repositório contém:

- `portal/` - Aplicação Next.js com frontend, rotas API e acesso ao banco via Prisma
- `switch-app/` - Aplicativo nativo para Nintendo Switch (devkitPro + libnx)
- `portal/app/api/*` - Rotas API substituindo o backend Express antigo
- `portal/app/auth` - Tela de login usada no fluxo de pareamento
- `portal/app/dashboard` - Painel de gerenciamento de saves do usuário
- `portal/app/admin` - Painel administrativo para gerenciar usuários
- `portal/prisma` - Schema e configuração do Prisma 7

## Stack atual

### Portal Web
- Next.js 15
- React 19
- Prisma 7
- PostgreSQL
- Google OAuth via `@react-oauth/google`

### Switch App
- devkitPro (toolchain Nintendo Switch)
- libnx (framework oficial Switch)
- curl (requisições HTTP)
- nlohmann/json (parsing JSON)

## Estrutura principal

```text
nx-cloud-system/
├── portal/                      # Aplicação web Next.js
│   ├── app/
│   │   ├── api/                # Rotas API
│   │   ├── auth/               # Autenticação Google
│   │   ├── dashboard/          # Painel do usuário
│   │   ├── admin/              # Painel administrativo
│   │   └── _components/        # Componentes compartilhados
│   ├── prisma/                 # Schema e migrations Prisma
│   └── src/lib/prisma.ts       # Client Prisma
├── switch-app/                 # Aplicativo Nintendo Switch
│   ├── source/                 # Código C++
│   ├── include/                # Headers
│   └── Makefile                # Build configuration
```

## Fluxo de Autenticação (Web)

1. Usuário clica em "Conectar com Google" na home
2. Sistema cria sessão via `POST /api/session/init`
3. Abre popup com tela `/auth` recebendo `sessionId` pela URL
4. Login do Google chama `POST /api/auth/google`
5. Após autenticação, popup fecha e redireciona para `/dashboard`

## Fluxo de Pareamento (Nintendo Switch)

1. **App Switch** gera `deviceToken` único do console
2. **App Switch** chama `POST /api/session/init` com `deviceToken`
3. Backend cria sessão com status `PENDING` e retorna `authUrl`
4. **App Switch** exibe `authUrl` no console (ou QR code)
5. Usuário acessa a `authUrl` no navegador/celular
6. Usuário faz login com Google
7. Backend vincula sessão ao `userId` e `deviceToken`
8. **App Switch** faz polling via `GET /api/session/status?deviceToken=xxx`
9. Quando status é `CONNECTED`, Switch pode acessar saves do usuário

## Dashboard de Saves

Área exclusiva para usuários autenticados:
- Upload de saves com título do jogo e descrição
- Listagem de saves com download e exclusão
- Visualização de metadados (tamanho, data)

## Painel Administrativo

Acesso restrito a usuários com `role = ADMIN`:
- Visualização de todos os usuários cadastrados
- Estatísticas por usuário (número de saves e sessões)
- Gestão de permissões

## Como executar

### Portal Web

Entre na pasta `portal` e rode:

```bash
cd portal
npm install
npm run dev
```

Build de produção:

```bash
cd portal
npm run build
npm run start
```

### Switch App

Requisitos:
- devkitPro instalado em `/opt/devkitpro` (ou ajuste a variável no Makefile)
- Bibliotecas portlibs (curl, zlib) instaladas via pacman

Para compilar:

```bash
cd switch-app
make
```

O arquivo `AppSwitch.nro` será gerado e pode ser transferido para o Nintendo Switch.

## Variáveis de ambiente

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

O sistema utiliza **GitHub Actions** para automatizar a atualização da API sempre que uma nova versão é lançada.

1. **Desenvolva e compile:** Gere o novo `AppSwitch.nro`.
2. **Crie uma Release no GitHub:**
   - Acesse `Releases` -> `Draft a new release`.
   - Use uma tag de versão (ex: `v0.6.0`).
   - Escreva as novidades no campo **Description** (elas aparecerão no Switch como changelog).
   - Anexe o arquivo `AppSwitch.nro`.
   - Clique em **Publish release**.
3. **Automação Total:**
   - O GitHub Actions detectará a nova release.
   - O arquivo `portal/app/api/app/version/route.ts` será atualizado automaticamente com a nova versão, link de download e changelog.
   - Um commit será feito automaticamente na branch `main`.
   - Seu deploy (Vercel/Netlify) atualizará a API online.

**Vantagens:**
- Zero edição manual de código para cada release.
- Consistência garantida entre o link de download e a versão.
- Changelog sincronizado com a descrição da release.

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
