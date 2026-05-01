import { google } from 'googleapis';

export const runtime = 'nodejs';

export async function GET(request: Request) {
  try {
    const { searchParams } = new URL(request.url);
    const userId = searchParams.get('userId');

    if (!userId) {
      return Response.json({ error: 'userId ausente' }, { status: 400 });
    }

    const host = request.headers.get('x-forwarded-host') || request.headers.get('host');
    const protocol = request.headers.get('x-forwarded-proto') || 'https';
    const baseUrl = process.env.NEXT_PUBLIC_BASE_URL || `${protocol}://${host}`;

    const oauth2Client = new google.auth.OAuth2(
      process.env.NEXT_PUBLIC_GOOGLE_CLIENT_ID,
      process.env.NEXT_SECRET_KEY,
      `${baseUrl}/api/drive/callback`
    );

    const authUrl = oauth2Client.generateAuthUrl({
      access_type: 'offline',
      prompt: 'consent',
      scope: [
        'https://www.googleapis.com/auth/drive.readonly',
      ],
      state: userId,
    });

    return Response.json({ authUrl });
  } catch (error) {
    console.error('Drive connect error:', error);
    return Response.json({ error: 'Erro ao gerar URL de autorização' }, { status: 500 });
  }
}
