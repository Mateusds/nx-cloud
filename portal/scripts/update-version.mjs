import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

// O script deve ser executado da raiz da pasta 'portal'
const [version, argChangelog, repository] = process.argv.slice(2);
const changelog = process.env.CHANGELOG || argChangelog;

if (!version || !changelog) {
  console.error('Uso: node scripts/update-version.mjs <version> [changelog] [repository]');
  console.error('Ou defina a variável de ambiente CHANGELOG');
  process.exit(1);
}

const filePath = path.join(process.cwd(), 'app/api/app/version/route.ts');

if (!fs.existsSync(filePath)) {
  console.error(`Erro: Arquivo não encontrado em ${filePath}`);
  process.exit(1);
}

let content = fs.readFileSync(filePath, 'utf8');

// Normaliza a versão (remove o prefixo 'v' para o campo version, mas mantém para a URL se necessário)
const cleanVersion = version.startsWith('v') ? version.substring(1) : version;
const tagVersion = version.startsWith('v') ? version : `v${version}`;

// Se o repositório não for passado, tenta manter o que já existe ou usa o padrão
const repoMatch = content.match(/github\.com\/(.*)\/releases/);
const repoName = repository || (repoMatch ? repoMatch[1] : 'mateusmarquesds/nx-cloud-system');

const downloadUrl = `https://github.com/${repoName}/releases/download/${tagVersion}/AppSwitch.nro`;
const releasedAt = new Date().toISOString().split('T')[0];

// Atualiza os campos usando Regex
content = content.replace(/version: '.*'/, `version: '${cleanVersion}'`);
content = content.replace(/downloadUrl: '.*'/, `downloadUrl: '${downloadUrl}'`);
content = content.replace(/changelog: '.*'/, `changelog: '${changelog}'`);
content = content.replace(/releasedAt: '.*'/, `releasedAt: '${releasedAt}'`);

fs.writeFileSync(filePath, content);
console.log(`✅ API atualizada com sucesso para a versão ${cleanVersion}`);
console.log(`🔗 Download URL: ${downloadUrl}`);
