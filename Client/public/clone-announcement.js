(() => {
    'use strict';
    const notice = document.getElementById('clone-announcement');
    const entry = document.getElementById('clone-entry');
    const entryLink = document.getElementById('clone-entry-link');
    const close = document.getElementById('clone-announcement-close');
    const reopen = document.getElementById('clone-announcement-reopen');
    if (!notice || !entry || !entryLink || !close || !reopen) return;
    let returnFocus = null;

    function showNotice() {
        if (notice.hasAttribute('open')) return;
        returnFocus = document.activeElement;
        document.documentElement.classList.add('clone-notice-open');
        if (typeof notice.showModal === 'function') notice.showModal();
        else notice.setAttribute('open', '');
        close.focus({ preventScroll: true });
    }
    function dismissNotice() {
        if (!notice.hasAttribute('open')) return;
        if (typeof notice.close === 'function') notice.close();
        else notice.removeAttribute('open');
        document.documentElement.classList.remove('clone-notice-open');
        if (returnFocus && returnFocus !== document.body && returnFocus.isConnected) {
            returnFocus.focus({ preventScroll: true });
        } else {
            const canvas = document.getElementById('canvas');
            if (canvas) { canvas.tabIndex = -1; canvas.focus({ preventScroll: true }); }
        }
    }

    // These controls are outside the game canvas. Their input must never leak
    // into the window-level WASM handlers as movement, zoom or a spawn request.
    for (const type of ['pointerdown', 'pointerup', 'mousedown', 'mouseup', 'mousemove',
        'touchstart', 'touchmove', 'touchend', 'touchcancel', 'wheel', 'click', 'keyup']) {
        for (const surface of [notice, entry]) {
            surface.addEventListener(type, event => event.stopPropagation(), { passive: true });
        }
    }
    notice.addEventListener('keydown', event => {
        event.stopPropagation();
        if (event.key === 'Escape') { event.preventDefault(); dismissNotice(); }
    });
    entry.addEventListener('keydown', event => event.stopPropagation());
    notice.addEventListener('cancel', event => { event.preventDefault(); dismissNotice(); });
    close.addEventListener('click', dismissNotice);
    reopen.addEventListener('click', showNotice);
    notice.querySelectorAll('a').forEach(link => link.addEventListener('click', dismissNotice));

    // Keep the prominent shortcut on the title screen; combat controls own the
    // same corner in a match. One small poll avoids adding work to every frame.
    function syncEntry() {
        entry.hidden = !!(window.Module && window.Module.ashflorPlaying);
        const orientation = document.getElementById('landscape-btn');
        document.documentElement.classList.toggle('clone-entry-orientation',
            !entry.hidden && !!orientation && orientation.style.display === 'block');
    }
    syncEntry();
    setInterval(syncEntry, 250);
    showNotice(); // Every page visit, immediately; closing never waits on a timer.
})();
