function showScene(name) {
    document.querySelectorAll('.scene').forEach(s => s.classList.remove('active'));
    document.querySelectorAll('.dev-nav-btn').forEach(b => b.classList.remove('active'));
    document.getElementById('scene-' + name).classList.add('active');
    document.querySelector(`.dev-nav-btn[onclick="showScene('${name}')"]`).classList.add('active');
    if (name === 'loading') startBoot();
}
