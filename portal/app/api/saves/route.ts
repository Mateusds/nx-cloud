import { prisma } from '../../../src/lib/prisma';
import { writeFile } from 'fs/promises';
import { join } from 'path';
import { mkdir } from 'fs/promises';

export const runtime = 'nodejs';

export async function GET(request: Request) {
  try {
    const { searchParams } = new URL(request.url);
    const userId = searchParams.get('userId');

    if (!userId) {
      return Response.json({ error: 'userId ausente' }, { status: 400 });
    }

    const saves = await prisma.save.findMany({
      where: { userId },
      orderBy: { updatedAt: 'desc' },
    });

    return Response.json({ saves });
  } catch (error) {
    console.error(error);
    return Response.json({ error: 'Erro ao buscar saves' }, { status: 500 });
  }
}

export async function POST(request: Request) {
  try {
    const formData = await request.formData();
    const file = formData.get('file') as File;
    const userId = formData.get('userId') as string;
    const gameTitle = formData.get('gameTitle') as string;
    const description = formData.get('description') as string;

    if (!file || !userId) {
      return Response.json({ error: 'Arquivo ou userId ausente' }, { status: 400 });
    }

    const user = await prisma.user.findUnique({ where: { id: userId } });
    if (!user) {
      return Response.json({ error: 'Usuário não encontrado' }, { status: 404 });
    }

    const bytes = await file.arrayBuffer();
    const buffer = Buffer.from(bytes);

    const uploadDir = join(process.cwd(), 'public', 'uploads', userId);
    await mkdir(uploadDir, { recursive: true });

    const fileName = `${Date.now()}_${file.name}`;
    const filePath = join(uploadDir, fileName);
    await writeFile(filePath, buffer);

    const save = await prisma.save.create({
      data: {
        filename: file.name,
        gameTitle: gameTitle || null,
        description: description || null,
        filePath: `/uploads/${userId}/${fileName}`,
        fileSize: file.size,
        userId,
      },
    });

    return Response.json({ save });
  } catch (error) {
    console.error(error);
    return Response.json({ error: 'Erro ao fazer upload' }, { status: 500 });
  }
}
