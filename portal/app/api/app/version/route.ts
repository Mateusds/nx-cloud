export const runtime = 'nodejs';

const APP_VERSION = {
  version: '7',
  minVersion: '1.0.0',
  downloadUrl: 'https://github.com/Mateusds/nx-cloud/releases/download/v7/AppSwitch.nro',
  changelog: 'feat: login persistente, polling rápido e verificação de updates via GitHub API',
  forceUpdate: false,
  releasedAt: '2026-05-01',
};

export async function GET() {
  return Response.json(APP_VERSION);
}
