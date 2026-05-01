import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function GET(request: Request) {
  try {
    const { searchParams } = new URL(request.url);
    const userId = searchParams.get('userId');

    if (!userId) {
      return Response.json({ error: 'userId ausente' }, { status: 400 });
    }

    const user = await prisma.user.findUnique({
      where: { id: userId },
      include: {
        sessions: {
          orderBy: {
            createdAt: 'desc'
          },
          take: 10
        }
      },
    });

    if (!user) {
      return Response.json({ error: 'Usuário não encontrado' }, { status: 404 });
    }

    // Filtra sessões conectadas com dados do dispositivo
    const sessions = (user.sessions as any[]).filter(
      (s) => s.status === 'CONNECTED' && s.deviceName
    );

    return Response.json({
      user: {
        id: user.id,
        email: user.email,
        name: user.name,
        role: user.role,
        createdAt: user.createdAt,
        sessions,
      }
    });
  } catch (error) {
    console.error('Auth me error:', error);
    return Response.json({ error: 'Erro ao buscar usuário' }, { status: 500 });
  }
}
