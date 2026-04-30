import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

type GoogleAuthBody = {
  token?: string;
  sessionId?: string | null;
};

async function verifyGoogleToken(token: string): Promise<{ email: string; sub: string; name?: string } | null> {
  try {
    const response = await fetch(`https://oauth2.googleapis.com/tokeninfo?id_token=${token}`);
    if (!response.ok) {
      console.error('Google tokeninfo failed:', response.status, await response.text());
      return null;
    }
    const data = await response.json();
    console.log('Google token data:', { email: data.email, sub: data.sub, name: data.name });
    if (!data.email) {
      console.error('No email in Google response:', data);
      return null;
    }
    return { email: data.email, sub: data.sub, name: data.name };
  } catch (err) {
    console.error('Error verifying Google token:', err);
    return null;
  }
}

export async function POST(request: Request) {
  try {
    const url = new URL(request.url);
    const body = (await request.json()) as GoogleAuthBody;
    const token = body.token?.trim();
    const sessionId = body.sessionId?.trim() || url.searchParams.get('sessionId')?.trim();

    console.log('Auth request:', { hasToken: !!token, sessionId });

    if (!token) {
      console.error('Token ausente');
      return Response.json({ error: 'Token ausente' }, { status: 400 });
    }

    if (!sessionId) {
      console.error('sessionId ausente');
      return Response.json({ error: 'sessionId ausente' }, { status: 400 });
    }

    const googleUser = await verifyGoogleToken(token);
    if (!googleUser) {
      console.error('Token verification failed');
      return Response.json({ error: 'Token inválido' }, { status: 401 });
    }

    console.log('Verified Google user:', googleUser.email);

    const session = await prisma.session.findUnique({
      where: { id: sessionId },
    });

    if (!session) {
      console.error('Session not found:', sessionId);
      return Response.json({ error: 'Sessão não encontrada' }, { status: 404 });
    }

    console.log('Found session:', session.id);

    let user = await prisma.user.findUnique({
      where: { email: googleUser.email },
    });

    console.log('Existing user:', user ? user.id : 'not found');

    if (!user) {
      console.log('Creating new user for:', googleUser.email);
      user = await prisma.user.create({
        data: {
          email: googleUser.email,
          name: googleUser.name,
          googleRefresh: googleUser.sub,
        },
      });
      console.log('User created:', user.id);
    } else if (googleUser.name && !user.name) {
      // Update existing user with name if they don't have one
      user = await prisma.user.update({
        where: { id: user.id },
        data: { name: googleUser.name },
      });
    }

    await prisma.session.update({
      where: { id: sessionId },
      data: {
        status: 'CONNECTED',
        userId: user.id,
      },
    });

    console.log('Auth completed for user:', user.id);
    return Response.json({ 
      ok: true, 
      sessionId, 
      userId: user.id, 
      email: user.email,
      deviceToken: session.deviceToken,
    });
  } catch (error) {
    console.error('Auth error:', error);
    return Response.json(
      { error: 'Erro ao vincular conta' },
      { status: 500 },
    );
  }
}
