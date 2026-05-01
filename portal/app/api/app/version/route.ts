export const runtime = 'nodejs';

export const dynamic = 'force-dynamic';

const GITHUB_REPOSITORY = process.env.GITHUB_REPOSITORY ?? 'mateusmarquesds/nx-cloud-system';
const RELEASE_ASSET_NAME = 'AppSwitch.nro';
const FALLBACK_APP_VERSION = {
  version: '1.0.1',
  minVersion: '1.0.0',
  downloadUrl: `https://github.com/${GITHUB_REPOSITORY}/releases/latest/download/${RELEASE_ASSET_NAME}`,
  changelog: 'Lançamento oficial em produção!',
  forceUpdate: false,
  releasedAt: '2026-04-30',
};

function normalizeVersion(tagName: string | undefined) {
  if (!tagName) return FALLBACK_APP_VERSION.version;
  return tagName.startsWith('v') ? tagName.slice(1) : tagName;
}

function buildGithubReleaseUrl(assetName: string) {
  return `https://github.com/${GITHUB_REPOSITORY}/releases/latest/download/${assetName}`;
}

export async function GET() {
  try {
    const headers: Record<string, string> = {
      Accept: 'application/vnd.github+json',
      'User-Agent': 'nx-cloud-version-check',
    };

    if (process.env.GITHUB_TOKEN) {
      headers.Authorization = `Bearer ${process.env.GITHUB_TOKEN}`;
    }

    const response = await fetch(
      `https://api.github.com/repos/${GITHUB_REPOSITORY}/releases/latest`,
      {
        headers,
        cache: 'no-store',
      },
    );

    if (!response.ok) {
      throw new Error(`GitHub releases request failed with status ${response.status}`);
    }

    const release = await response.json();
    const assets = Array.isArray(release.assets) ? release.assets : [];
    const matchingAsset = assets.find((asset: { name?: string }) => asset.name === RELEASE_ASSET_NAME);

    return Response.json({
      version: normalizeVersion(release.tag_name),
      minVersion: normalizeVersion(release.tag_name),
      downloadUrl: matchingAsset?.browser_download_url ?? buildGithubReleaseUrl(RELEASE_ASSET_NAME),
      changelog: typeof release.body === 'string' && release.body.trim().length > 0
        ? release.body.trim()
        : 'Última release publicada no GitHub.',
      forceUpdate: false,
      releasedAt: typeof release.published_at === 'string'
        ? release.published_at.split('T')[0]
        : FALLBACK_APP_VERSION.releasedAt,
    });
  } catch (error) {
    console.error('Failed to load latest GitHub release:', error);
    return Response.json(FALLBACK_APP_VERSION);
  }
}
