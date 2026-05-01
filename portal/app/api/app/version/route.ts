export const runtime = 'nodejs';

const APP_VERSION = {
  version: '5',
  minVersion: '1.0.0',
  downloadUrl: 'https://github.com/Mateusds/nx-cloud/releases/download/v5/AppSwitch.nro',
  changelog: 'feat: status de armazenamento do Switch no portal web e tela de auth',
  forceUpdate: false,
  releasedAt: '2026-05-01',
};

export async function GET() {
  return Response.json(APP_VERSION);
}
