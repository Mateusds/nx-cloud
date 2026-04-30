import type { Metadata } from 'next';
import './globals.css';

export const metadata: Metadata = {
  title: 'NX Cloud',
  description: 'Autenticação e sessões no Next.js com Prisma.',
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="pt-BR">
      <body>{children}</body>
    </html>
  );
}
