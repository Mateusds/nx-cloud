import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function POST(request: Request) {
  try {
    let body;
    try {
      body = await request.json();
    } catch (e) {
      body = {};
    }
    
    const deviceToken = body.deviceToken || 'unknown-device';

    let session;

    if (deviceToken && deviceToken !== 'unknown-device') {
      // Para consoles reais: cria ou atualiza a sessão existente
      session = await prisma.session.upsert({
        where: { deviceToken },
        update: {
          status: 'PENDING',
          userId: null,
        },
        create: {
          status: 'PENDING',
          deviceToken,
        },
      });
    } else {
      // Para testes via navegador ou dispositivos sem token: sempre cria nova
      session = await prisma.session.create({
        data: {
          status: 'PENDING',
          deviceToken: null,
        },
      });
    }

    const authUrl = new URL(`/auth?sessionId=${session.id}`, request.url).toString();

    return Response.json({ sessionId: session.id, authUrl });
  } catch (error) {
    console.error(error);
    return Response.json(
      { error: 'Erro ao criar sessão' },
      { status: 500 },
    );
  }
}
