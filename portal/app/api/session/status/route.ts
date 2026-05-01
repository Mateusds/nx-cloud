import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function GET(request: Request) {
  try {
    const { searchParams } = new URL(request.url);
    const deviceToken = searchParams.get('deviceToken');
    const sessionId = searchParams.get('sessionId');

    if (!deviceToken && !sessionId) {
      return Response.json({ error: 'deviceToken ou sessionId ausente' }, { status: 400 });
    }

    let session;
    if (deviceToken) {
      session = await prisma.session.findUnique({
        where: { deviceToken },
        include: { user: true },
      });
    } else if (sessionId) {
      session = await prisma.session.findUnique({
        where: { id: sessionId },
        include: { user: true },
      });
    }

    if (!session) {
      return Response.json({ error: 'Sessão não encontrada' }, { status: 404 });
    }

    return Response.json({
      status: session.status,
      userId: session.userId,
      user: session.user ? {
        id: session.user.id,
        email: session.user.email,
        name: session.user.name,
      } : null,
    });
  } catch (error) {
    console.error('Session status error:', error);
    return Response.json({ error: 'Erro ao buscar status' }, { status: 500 });
  }
}
