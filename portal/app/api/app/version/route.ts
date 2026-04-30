export const runtime = 'nodejs';

const APP_VERSION = {
  version: '1.0.1',
  minVersion: '1.0.0',
  downloadUrl: 'https://github.com/Mateusds/nx-cloud/releases/download/v1.0.1/AppSwitch.nro',
  changelog: 'Fix de conexão local e melhorias no QR Code',
  forceUpdate: false,
  releasedAt: '2026-04-30',
};

export async function GET() {
  return Response.json(APP_VERSION);
}
