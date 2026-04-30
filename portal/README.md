# Portal NX Cloud

Aplicação Next.js do NX-Cloud System, responsável pelo portal web, pelas rotas API e pelo fluxo de autenticação.

## Stack

- Next.js 15
- React 19
- Prisma 7
- PostgreSQL
- `@react-oauth/google`

## Rodar localmente

```bash
npm install
npm run dev
```

Build e produção:

```bash
npm run build
npm run start
```

## Ambiente

Crie `portal/.env.local` com:

```env
DATABASE_URL="postgresql://user:password@localhost:5432/nx_cloud?schema=public"
NEXT_PUBLIC_GOOGLE_CLIENT_ID="seu-google-client-id"
```

## Rotas

- `/` - home do portal
- `/auth` - login Google
- `/api/health` - validação de conexão com o banco
- `/api/session/init` - criação de sessão
- `/api/auth/google` - vínculo do token com a sessão

## Prisma

O schema e as migrations ficam em `portal/prisma`.

Se precisar regenerar o client manualmente:

```bash
npx prisma generate --schema=prisma/schema.prisma
```
