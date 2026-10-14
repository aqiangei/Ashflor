#!/usr/bin/env node
/**
 * Pack gardn-client into a single offline HTML (florr_clone-style MVP).
 * Usage: node Scripts/pack_offline.mjs [out.html]
 * Reads Client/public/{index.html,gardn-client.js,gardn-client.wasm}.
 * Note: multiplayer still needs a server; this only removes network fetch for assets.
 */
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(__dirname, '..');
const pub = path.join(root, 'Client', 'public');
const out = process.argv[2] || path.join(pub, 'offline.html');

const indexPath = path.join(pub, 'index.html');
const jsPath = path.join(pub, 'gardn-client.js');
const wasmPath = path.join(pub, 'gardn-client.wasm');

for (const p of [indexPath, jsPath, wasmPath]) {
  if (!fs.existsSync(p)) {
    console.error('Missing', p);
    process.exit(1);
  }
}

const js = fs.readFileSync(jsPath, 'utf8');
const wasmB64 = fs.readFileSync(wasmPath).toString('base64');
let html = fs.readFileSync(indexPath, 'utf8');

// Strip external script/wasm loads; inject inline module bootstrap
html = html.replace(/<script[^>]*gardn-client\.js[^>]*>\s*<\/script>/i, '');
html = html.replace(/script\.src\s*=\s*['"]gardn-client\.js[^'"]*['"]/g, '/* offline: inlined */');

const inject = `
<script>
window.ASHFLOR_OFFLINE = true;
window.Module = window.Module || {};
Module.instantiateWasm = function(imports, successCallback) {
  const bin = Uint8Array.from(atob(${JSON.stringify(wasmB64)}), c => c.charCodeAt(0));
  return WebAssembly.instantiate(bin, imports).then(function(pair) {
    successCallback(pair.instance, pair.module);
  });
};
</script>
<script>
${js}
</script>
`;

if (html.includes('</body>'))
  html = html.replace('</body>', inject + '\n</body>');
else
  html += inject;

fs.writeFileSync(out, html, 'utf8');
const kb = Math.round(fs.statSync(out).size / 1024);
console.log('Wrote', out, '(' + kb + ' KB)');
console.log('Open locally for asset-offline client; game still needs WS server.');
