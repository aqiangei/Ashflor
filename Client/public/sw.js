/* Cache only versioned client assets; the browser makes the initial request. */
const VERSION = new URL(self.location.href).searchParams.get("v") || "unversioned";
const CACHE_PREFIX = "ashflor-client-";
const CACHE = CACHE_PREFIX + VERSION;

self.addEventListener("install", function (event) {
    self.skipWaiting();
});

self.addEventListener("activate", function (event) {
    event.waitUntil((async function () {
        const keys = await caches.keys();
        await Promise.all(keys.filter(function (key) {
            return (key.startsWith(CACHE_PREFIX) && key !== CACHE)
                || key.startsWith("ashflor-assets-");
        }).map(function (key) { return caches.delete(key); }));
        await self.clients.claim();
    })());
});

self.addEventListener("fetch", function (event) {
    const url = new URL(event.request.url);
    if (url.pathname !== "/gardn-client.js" && url.pathname !== "/gardn-client.wasm")
        return;
    if (url.origin !== self.location.origin || url.searchParams.get("v") !== VERSION)
        return;
    event.respondWith((async function () {
        const cache = await caches.open(CACHE);
        const cached = await cache.match(event.request);
        if (cached) return cached;
        const res = await fetch(event.request);
        if (res && res.ok)
            event.waitUntil(cache.put(event.request, res.clone()).catch(function () {}));
        return res;
    })());
});
