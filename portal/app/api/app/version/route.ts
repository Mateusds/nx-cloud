export const runtime = 'nodejs';

const APP_VERSION = {
  version: '10',
  minVersion: '1.0.0',
  downloadUrl: 'https://github.com/Mateusds/nx-cloud/releases/download/v10/AppSwitch.nro',
  changelog: 'fix: texto legível, botões corrigidos e acesso ao Google Drive restaurado',
  forceUpdate: false,
  releasedAt: '2026-05-01',
};

export async function GET() {
  return Response.json(APP_VERSION);
}
