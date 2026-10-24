/**
 * Node real-disk file IO for the WASM server.
 * Linked with: -s --js-library=node_fs_library.js
 *
 * Avoids EM_ASM comma/preprocessor pitfalls and MEMFS.
 */
addToLibrary({
  node_fs_read_alloc__deps: ['$UTF8ToString', '$HEAPU8', 'malloc'],
  node_fs_read_alloc: function (pathPtr) {
    var fs = require('fs');
    var p = UTF8ToString(pathPtr);
    try {
      if (!fs.existsSync(p)) return 0;
      var buf = fs.readFileSync(p);
      var mem = _malloc(buf.length + 1);
      if (!mem) return 0;
      HEAPU8.set(buf, mem);
      HEAPU8[mem + buf.length] = 0;
      return mem;
    } catch (e) {
      console.error('[node_fs_read_alloc]', p, e);
      return 0;
    }
  },

  node_fs_write_atomic__deps: ['$UTF8ToString', '$HEAPU8'],
  node_fs_write_atomic: function (pathPtr, dataPtr, len) {
    var fs = require('fs');
    var p = UTF8ToString(pathPtr);
    try {
      var tmp = p + '.tmp';
      var bytes = Buffer.from(HEAPU8.subarray(dataPtr, dataPtr + len));
      // Atomic replace only — no .bak copy on the tick hot path.
      // Periodic node_fs_snapshot() already keeps timestamped backups/.
      fs.writeFileSync(tmp, bytes);
      fs.renameSync(tmp, p);
      return 1;
    } catch (e) {
      console.error('[node_fs_write_atomic]', p, e);
      return 0;
    }
  },

  node_fs_write_durable_atomic__deps: ['$UTF8ToString', '$HEAPU8'],
  node_fs_write_durable_atomic: function (pathPtr, dataPtr, len) {
    var fs = require('fs'), path = require('path'), fd;
    var p = UTF8ToString(pathPtr), tmp = p + '.tmp';
    try {
      fd = fs.openSync(tmp, 'w', 384);
      fs.writeFileSync(fd, Buffer.from(HEAPU8.subarray(dataPtr, dataPtr + len)));
      fs.fsyncSync(fd); fs.closeSync(fd); fd = undefined;
      fs.renameSync(tmp, p);
      var dir = fs.openSync(path.dirname(p), 'r');
      try { fs.fsyncSync(dir); } finally { fs.closeSync(dir); }
      return 1;
    } catch (e) {
      if (fd !== undefined) try { fs.closeSync(fd); } catch (_) {}
      console.error('[feedback durable write]', p, e.message); return 0;
    }
  },

  node_fs_snapshot__deps: ['$UTF8ToString'],
  node_fs_snapshot: function (pathPtr) {
    var fs = require('fs');
    var pathMod = require('path');
    var p = UTF8ToString(pathPtr);
    try {
      if (!fs.existsSync(p)) return;
      var dir = pathMod.join(pathMod.dirname(p) || '.', 'backups');
      if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });
      var stamp = new Date().toISOString().replace(/[:.]/g, '-');
      var base = pathMod.basename(p).replace(/\.dat$/, '');
      fs['copyFileSync'](p, pathMod.join(dir, base + '-' + stamp + '.dat'));
      var files = fs.readdirSync(dir).filter(function (f) {
        return f.indexOf(base + '-') === 0 && f.slice(-4) === '.dat';
      }).sort();
      while (files.length > 40) {
        fs.unlinkSync(pathMod.join(dir, files.shift()));
      }
      console.log('[node_fs_snapshot]', p);
    } catch (e) {
      console.error('[node_fs_snapshot]', p, e);
    }
  }
});
