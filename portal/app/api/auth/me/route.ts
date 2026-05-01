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
      select: {
        id: true,
        email: true,
        name: true,
        role: true,
        createdAt: true,
        sessions: {
          orderBy: {
            createdAt: 'desc'
          },
          take: 1
        }
      },
    });

    if (!user) {
      return Response.json({ error: 'Usuário não encontrado' }, { status: 404 });
    }

    return Response.json({ user });
  } catch (error) {
    console.error('Auth me error:', error);
    return Response.json({ error: 'Erro ao buscar usuário' }, { status: 500 });
  }
}
