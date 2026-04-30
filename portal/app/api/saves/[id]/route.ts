import { prisma } from '../../../../src/lib/prisma';
import { unlink } from 'fs/promises';
import { join } from 'path';

export const runtime = 'nodejs';

export async function DELETE(
  request: Request,
  { params }: { params: Promise<{ id: string }> }
) {
  try {
    const { id } = await params;
    const { searchParams } = new URL(request.url);
    const userId = searchParams.get('userId');

    if (!userId) {
      return Response.json({ error: 'userId ausente' }, { status: 400 });
    }

    const save = await prisma.save.findFirst({
      where: { id, userId },
    });

    if (!save) {
      return Response.json({ error: 'Save não encontrado' }, { status: 404 });
    }

    try {
      const filePath = join(process.cwd(), 'public', save.filePath);
      await unlink(filePath);
    } catch {
      // File may not exist, continue
    }

    await prisma.save.delete({
      where: { id },
    });

    return Response.json({ ok: true });
  } catch (error) {
    console.error(error);
    return Response.json({ error: 'Erro ao deletar save' }, { status: 500 });
  }
}
