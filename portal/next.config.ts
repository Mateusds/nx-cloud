import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  /* config options here */
  typescript: {
    // Pula a checagem de tipos para economizar memória na EC2
    ignoreBuildErrors: true,
  },
  eslint: {
    // Pula o linting para economizar memória na EC2
    ignoreDuringBuilds: true,
  },
};

export default nextConfig;
