/* ===== WINDOWS DRAG & RESIZE ===== */
let resizeState = null;

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

function startResize(e, winId, direction) {
    const el = document.getElementById(winId);
    const rect = el.getBoundingClientRect();
    resizeState = {
        winId,
        direction,
        startX: e.clientX,
        startY: e.clientY,
        startWidth: rect.width,
        startHeight: rect.height,
        startLeft: rect.left,
        startTop: rect.top
    };
    el.style.zIndex = 100;
    e.preventDefault();
    e.stopPropagation();
}

document.addEventListener('mousemove', e => {
    if (resizeState) {
        const el = document.getElementById(resizeState.winId);
        const deltaX = e.clientX - resizeState.startX;
        const deltaY = e.clientY - resizeState.startY;
        const dir = resizeState.direction;

        let newWidth = resizeState.startWidth;
        let newHeight = resizeState.startHeight;
        let newLeft = resizeState.startLeft;
        let newTop = resizeState.startTop;

        if (dir.includes('e')) {
            newWidth = Math.max(300, resizeState.startWidth + deltaX);
        }
        if (dir.includes('w')) {
            newWidth = Math.max(300, resizeState.startWidth - deltaX);
            newLeft = resizeState.startLeft + deltaX;
        }
        if (dir.includes('s')) {
            newHeight = Math.max(200, resizeState.startHeight + deltaY);
        }
        if (dir.includes('n')) {
            newHeight = Math.max(200, resizeState.startHeight - deltaY);
            newTop = resizeState.startTop + deltaY;
        }

        el.style.width = newWidth + 'px';
        el.style.height = newHeight + 'px';
        el.style.left = Math.max(0, newLeft) + 'px';
        el.style.top = Math.max(0, newTop) + 'px';
        el.style.right = 'auto';
        el.style.bottom = 'auto';
    } else if (dragState) {
        const el = document.getElementById(dragState.winId);
        const x = e.clientX - dragState.startX;
        const y = e.clientY - dragState.startY;
        el.style.left = Math.max(0, x) + 'px';
        el.style.top = Math.max(0, y) + 'px';
        el.style.right = 'auto';
    }
});

document.addEventListener('mouseup', () => {
    dragState = null;
    resizeState = null;
});

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

/* ===== INITIALIZE RESIZE HANDLES ===== */
function initializeResizeHandles() {
    const resizeDirections = ['n', 's', 'e', 'w', 'ne', 'nw', 'se', 'sw'];
    const windows = document.querySelectorAll('.win10-window');

    windows.forEach(winEl => {
        const winId = winEl.id;
        resizeDirections.forEach(dir => {
            const handle = document.createElement('div');
            handle.className = `win10-resize-handle ${dir}`;
            handle.onmousedown = (e) => startResize(e, winId, dir);
            winEl.appendChild(handle);
        });
    });
}

document.addEventListener('DOMContentLoaded', initializeResizeHandles);
