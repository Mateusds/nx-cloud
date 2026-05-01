import { NextResponse } from 'next/server';

export const runtime = 'nodejs';

// Esta rota redireciona para o arquivo NRO mais recente
export async function GET() {
  const GITHUB_RELEASE_URL = 'https://github.com/Mateusds/nx-cloud/releases/latest/download/AppSwitch.nro';
  
  return NextResponse.redirect(GITHUB_RELEASE_URL);
}
