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
<<<<<<< HEAD

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

    // Força o uso da URL de produção se estiver configurada no .env
    // Caso contrário, tenta detectar pelos headers ou pela URL da requisição
    const host = request.headers.get('x-forwarded-host') || request.headers.get('host');
    const protocol = request.headers.get('x-forwarded-proto') || 'https';
    
    const baseUrl = process.env.NEXT_PUBLIC_BASE_URL || 
                    (host ? `${protocol}://${host}` : new URL(request.url).origin);
    
    const authUrl = `${baseUrl}/auth?sessionId=${session.id}`;

=======
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

>>>>>>> mateus
    return Response.json({ sessionId: session.id, authUrl });
  } catch (error) {
    console.error('Session init error:', error);
    return Response.json(
      { error: 'Erro ao criar sessão' },
      { status: 500 },
    );
  }
}
