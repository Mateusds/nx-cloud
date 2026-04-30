import { prisma } from '../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function GET() {
  try {
    await prisma.$queryRaw`SELECT 1`;
    return Response.json({ status: 'ok', database: 'connected' });
  } catch (error) {
    console.error(error);
    return Response.json(
      { status: 'error', message: 'Erro na conexão' },
      { status: 500 },
    );
  }
}
