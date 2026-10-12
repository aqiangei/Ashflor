// Generate files for servers that support precompressed static assets.
const fs = require('node:fs');
const path = require('node:path');
const zlib = require('node:zlib');

const directory = path.resolve(process.argv[2] || path.join(__dirname, '..', 'Client', 'public'));
for (const name of ['gardn-client.js', 'gardn-client.wasm']) {
    const source = path.join(directory, name);
    if (!fs.existsSync(source)) {
        console.error(`Missing ${source}`);
        process.exitCode = 1;
        continue;
    }
    const data = fs.readFileSync(source);
    const brotli = zlib.brotliCompressSync(data, {
        params: { [zlib.constants.BROTLI_PARAM_QUALITY]: 9 }
    });
    const gzip = zlib.gzipSync(data, { level: 9 });
    fs.writeFileSync(source + '.br', brotli);
    fs.writeFileSync(source + '.gz', gzip);
    console.log(`${name}: ${data.length} -> br ${brotli.length}, gzip ${gzip.length}`);
}
