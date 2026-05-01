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
      // Para consoles reais: cria ou atualiza a sessão existente (evita erro de Unique Constraint)
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
      // Para testes via navegador ou dispositivos sem token
      session = await prisma.session.create({
        data: {
          status: 'PENDING',
          deviceToken: null,
        },
      });
    }

    // Calcula a Base URL corretamente para evitar o localhost:3000 na AWS
    const host = request.headers.get('host');
    const protocol = request.headers.get('x-forwarded-proto') || 'https';
    const baseUrl = host ? `${protocol}://${host}` : new URL(request.url).origin;
    
    const authUrl = `${baseUrl}/auth?sessionId=${session.id}`;

    return Response.json({ sessionId: session.id, authUrl });
  } catch (error) {
    console.error('Session init error:', error);
    return Response.json(
      { error: 'Erro ao criar sessão' },
      { status: 500 },
    );
  }
}
