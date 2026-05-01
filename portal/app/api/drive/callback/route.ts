import { google } from 'googleapis';
import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function GET(request: Request) {
  try {
    const { searchParams } = new URL(request.url);
    const code = searchParams.get('code');
    const userId = searchParams.get('state');

    if (!code || !userId) {
      return new Response('Parâmetros ausentes', { status: 400 });
    }

    const host = request.headers.get('x-forwarded-host') || request.headers.get('host');
    const protocol = request.headers.get('x-forwarded-proto') || 'https';
    const baseUrl = process.env.NEXT_PUBLIC_BASE_URL || `${protocol}://${host}`;

    const oauth2Client = new google.auth.OAuth2(
      process.env.NEXT_PUBLIC_GOOGLE_CLIENT_ID,
      process.env.NEXT_SECRET_KEY,
      `${baseUrl}/api/drive/callback`
    );

    const { tokens } = await oauth2Client.getToken(code);

    if (!tokens.refresh_token) {
      return new Response(
        '<html><body style="background:#111;color:#fff;font-family:sans-serif;text-align:center;padding-top:100px;">' +
        '<h1>Erro</h1><p>O Google não retornou um refresh token. Tente revogar o acesso em myaccount.google.com e tente novamente.</p></body></html>',
        { headers: { 'Content-Type': 'text/html' } }
      );
    }

    await prisma.user.update({
      where: { id: userId },
      data: { googleRefresh: tokens.refresh_token },
    });

    return new Response(
      '<html><body style="background:#111;color:#fff;font-family:sans-serif;text-align:center;padding-top:100px;">' +
      '<h1 style="color:#00ff96;">✅ Google Drive Conectado!</h1>' +
      '<p>Você já pode fechar esta aba e acessar seus arquivos pelo Switch.</p>' +
      '<script>setTimeout(function(){window.close()},3000);</script></body></html>',
      { headers: { 'Content-Type': 'text/html' } }
    );
  } catch (error) {
    console.error('Drive callback error:', error);
    return new Response(
      '<html><body style="background:#111;color:#fff;font-family:sans-serif;text-align:center;padding-top:100px;">' +
      '<h1 style="color:#ff5050;">Erro</h1><p>Não foi possível conectar o Google Drive. Tente novamente.</p></body></html>',
      { headers: { 'Content-Type': 'text/html' } }
    );
  }
}
