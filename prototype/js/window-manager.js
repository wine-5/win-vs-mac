/* ===== WINDOWS DRAG ===== */
function startDrag(e, winId) {
    if (e.target.classList.contains('win10-wctrl')) return;
    const el = document.getElementById(winId);
    const rect = el.getBoundingClientRect();
    dragState = {
        winId,
        startX: e.clientX - rect.left,
        startY: e.clientY - rect.top
    };
    el.style.zIndex = 100;
    e.preventDefault();
}

document.addEventListener('mousemove', e => {
    if (!dragState) return;
    const el = document.getElementById(dragState.winId);
    const x = e.clientX - dragState.startX;
    const y = e.clientY - dragState.startY;
    el.style.left = Math.max(0, x) + 'px';
    el.style.top = Math.max(0, y) + 'px';
    el.style.right = 'auto';
});

document.addEventListener('mouseup', () => { dragState = null; });

/* ===== WINDOW CONTROLS ===== */
const windowStates = {};

function closeWindow(id) {
    const el = document.getElementById(id);
    el.style.display = 'none';
    updateTaskbarApp(id, 'closed');
}

function minimizeWindow(id) {
    const el = document.getElementById(id);
    el.style.display = 'none';
    updateTaskbarApp(id, 'minimized');
}

function maximizeWindow(id) {
    const el = document.getElementById(id);
    if (el.dataset.maximized === 'true') {
        el.style.cssText = el.dataset.prevStyle || '';
        el.dataset.maximized = 'false';
    } else {
        el.dataset.prevStyle = el.style.cssText;
        el.style.left = '0';
        el.style.top = '0';
        el.style.width = '100%';
        el.style.height = 'calc(100% - 40px)';
        el.dataset.maximized = 'true';
    }
}

function restoreWindow(id) {
    const el = document.getElementById(id);
    el.style.display = 'flex';
    el.style.zIndex = 100;
    updateTaskbarApp(id, 'active');
}

function toggleWindow(id) {
    const el = document.getElementById(id);
    if (el.style.display === 'none') {
        restoreWindow(id);
    } else {
        minimizeWindow(id);
    }
}

function updateTaskbarApp(id, state) {
    const tb = document.getElementById('tb-' + id);
    if (!tb) return;
    if (state === 'closed') { tb.classList.remove('active'); tb.classList.add('minimized'); }
    else if (state === 'minimized') { tb.classList.remove('active'); tb.classList.add('minimized'); }
    else if (state === 'active') { tb.classList.add('active'); tb.classList.remove('minimized'); }
}
