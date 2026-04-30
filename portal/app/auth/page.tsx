import { Suspense } from 'react';
import AuthClient from './auth-client';

export default function AuthPage() {
  return (
    <Suspense fallback={null}>
      <AuthClient />
    </Suspense>
  );
}
