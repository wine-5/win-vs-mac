function showScene(name) {
    document.querySelectorAll('.scene').forEach(s => s.classList.remove('active'));
    document.querySelectorAll('.dev-nav-btn').forEach(b => b.classList.remove('active'));
    document.getElementById('scene-' + name).classList.add('active');
    const navBtn = document.querySelector(`.dev-nav-btn[onclick="showScene('${name}')"]`);
    if (navBtn) navBtn.classList.add('active');
    if (name === 'bios')       startBios();
    if (name === 'lockscreen') startLockScreen();
    if (name === 'loading')    startBoot();
}
