export const runtime = 'nodejs';

const APP_VERSION = {
  version: '15',
  minVersion: '1.0.0',
  downloadUrl: 'https://github.com/Mateusds/nx-cloud/releases/download/v15/AppSwitch.nro',
  changelog: 'v15',
  forceUpdate: false,
  releasedAt: '2026-05-01',
};

export async function GET() {
  return Response.json(APP_VERSION);
}
