import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function GET(request: Request) {
  try {
    const { searchParams } = new URL(request.url);
    const adminUserId = searchParams.get('adminUserId');

    if (!adminUserId) {
      return Response.json({ error: 'Admin userId ausente' }, { status: 400 });
    }

    const adminUser = await prisma.user.findUnique({
      where: { id: adminUserId },
    });

    if (!adminUser || adminUser.role !== 'ADMIN') {
      return Response.json({ error: 'Acesso negado' }, { status: 403 });
    }

    const users = await prisma.user.findMany({
      orderBy: { createdAt: 'desc' },
    });

    const usersWithCounts = await Promise.all(
      users.map(async (user: { id: string; email: string; role: string; createdAt: Date; updatedAt: Date }) => {
        const savesCount = await prisma.save.count({ where: { userId: user.id } });
        const sessionsCount = await prisma.session.count({ where: { userId: user.id } });
        return {
          ...user,
          _count: { saves: savesCount, sessions: sessionsCount }
        };
      })
    );

    return Response.json({ users: usersWithCounts });
  } catch (error) {
    console.error('Admin users error:', error);
    return Response.json({ error: 'Erro ao buscar usuários' }, { status: 500 });
  }
}
