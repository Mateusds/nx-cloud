import { google } from 'googleapis';
import { prisma } from '../../../../src/lib/prisma';

export const runtime = 'nodejs';

export async function GET(request: Request) {
  try {
    const { searchParams } = new URL(request.url);
    const userId = searchParams.get('userId');
    const folderId = searchParams.get('folderId') || 'root';

    if (!userId) return Response.json({ error: 'User ID ausente' }, { status: 400 });

    const user = await prisma.user.findUnique({
      where: { id: userId },
    });

    if (!user || !user.googleRefresh) {
      return Response.json({ error: 'Google Drive não conectado' }, { status: 401 });
    }

    const host = request.headers.get('x-forwarded-host') || request.headers.get('host');
    const protocol = request.headers.get('x-forwarded-proto') || 'https';
    const baseUrl = process.env.NEXT_PUBLIC_BASE_URL || `${protocol}://${host}`;

    const oauth2Client = new google.auth.OAuth2(
      process.env.NEXT_PUBLIC_GOOGLE_CLIENT_ID,
      process.env.NEXT_SECRET_KEY,
      `${baseUrl}/api/drive/callback`
    );

    oauth2Client.setCredentials({
      refresh_token: user.googleRefresh,
    });

    const drive = google.drive({ version: 'v3', auth: oauth2Client });
    
    // Lista arquivos e pastas
    const res = await drive.files.list({
      q: `'${folderId}' in parents and trashed = false`,
      fields: 'files(id, name, mimeType, size)',
    });

    // Lista arquivos e pastas (filtrando por ROMs, NSPs ou Pastas)
    const files = res.data.files?.map(file => {
      const fileName = file.name || '';
      const ext = fileName.split('.').pop()?.toLowerCase();
      
      const systems: { [key: string]: string } = {
        'snes': 'Nintendo_-_Super_Nintendo_Entertainment_System',
        'nes': 'Nintendo_-_Nintendo_Entertainment_System',
        'gba': 'Nintendo_-_Game_Boy_Advance',
        'gb': 'Nintendo_-_Game_Boy',
        'gbc': 'Nintendo_-_Game_Boy_Color',
        'n64': 'Nintendo_-_Nintendo_64',
        'md': 'Sega_-_Mega_Drive_-_Genesis',
        'gen': 'Sega_-_Mega_Drive_-_Genesis'
      };

      let coverUrl = null;
      const isRom = ext && systems[ext];

      if (isRom) {
        const cleanName = fileName
          .replace(/\(.*\)/g, '')
          .replace(/\[.*\]/g, '')
          .replace(`.${ext}`, '')
          .trim();
          
        coverUrl = `https://raw.githubusercontent.com/libretro/libretro-thumbnails/master/${systems[ext]}/Named_Boxarts/${encodeURIComponent(cleanName)}.png`;
      }

      return {
        ...file,
        type: isRom ? 'installable-rom' : (file.mimeType === 'application/vnd.google-apps.folder' ? 'folder' : 'file'),
        system: isRom ? systems[ext] : null,
        coverUrl
      };
    });

    return Response.json({ files });
  } catch (error) {
    console.error('Drive API Error:', error);
    return Response.json({ error: 'Erro ao acessar o Google Drive' }, { status: 500 });
  }
}
