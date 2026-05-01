export const runtime = 'nodejs';

const APP_VERSION = {
  version: '14',
  minVersion: '1.0.0',
  downloadUrl: 'https://github.com/Mateusds/nx-cloud/releases/download/v14/AppSwitch.nro',
  changelog: 'feat: implementação completa do OAuth2 para Google Drive- Criada rota /api/drive/connect para gerar URL de consentimento com escopo drive.readonly.- Criada rota /api/drive/callback para trocar código OAuth2 por refresh token.- Auth: após login, usuário é automaticamente redirecionado para autorizar o Drive.- Drive list: atualizado para usar a mesma chave secreta e callback URL.',
  forceUpdate: false,
  releasedAt: '2026-05-01',
};

export async function GET() {
  return Response.json(APP_VERSION);
}
