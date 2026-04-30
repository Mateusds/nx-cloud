import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function POST(request: Request) {
  try {
    const body = await request.json();
    const { adminUserId, targetUserId, role } = body;

    if (!adminUserId || !targetUserId || !role) {
      return Response.json({ error: 'Parâmetros ausentes' }, { status: 400 });
    }

    const adminUser = await prisma.user.findUnique({
      where: { id: adminUserId },
    });

    if (!adminUser || adminUser.role !== 'ADMIN') {
      return Response.json({ error: 'Acesso negado' }, { status: 403 });
    }

    const updatedUser = await prisma.user.update({
      where: { id: targetUserId },
      data: { role },
    });

    return Response.json({ ok: true, user: updatedUser });
  } catch (error) {
    console.error('Promote user error:', error);
    return Response.json({ error: 'Erro ao atualizar usuário' }, { status: 500 });
  }
}
