export const runtime = 'nodejs';

const APP_VERSION = {
  version: '1.0.1',
  minVersion: '1.0.0',
  downloadUrl: 'https://nx-cloud.mateusmarquesds.com/api/app/download', // Ou o link direto do GitHub
  changelog: 'Lançamento oficial em produção!',
  forceUpdate: false,
  releasedAt: '2026-04-30',
};

export async function GET() {
  return Response.json(APP_VERSION);
} 
