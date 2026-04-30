import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function POST(request: Request) {
  try {
    const body = await request.json();
    const deviceToken = body.deviceToken || null;

    const newSession = await prisma.session.create({
      data: { 
        status: 'PENDING',
        deviceToken,
      },
    });

    const authUrl = new URL(`/auth?sessionId=${newSession.id}`, request.url).toString();

    return Response.json({ sessionId: newSession.id, authUrl });
  } catch (error) {
    console.error(error);
    return Response.json(
      { error: 'Erro ao criar sessão' },
      { status: 500 },
    );
  }
}
