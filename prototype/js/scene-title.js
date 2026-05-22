/* ===== TITLE SCENE ===== */
setTimeout(() => {
    const splash = document.querySelector('.splash-screen');
    const titleScreen = document.querySelector('.title-screen');
    const titleContent = document.querySelector('.title-content');

    if (splash) {
        splash.style.display = 'none';
    }

    if (titleScreen) {
        titleScreen.style.transition = 'opacity 0.4s ease-in-out';
        titleScreen.style.opacity = '1';
    }

    if (titleContent) {
        titleContent.style.transition = 'all 0.8s cubic-bezier(0.34, 1.56, 0.64, 1)';
        titleContent.style.opacity = '1';
        titleContent.style.transform = 'translateY(0)';
    }
}, 3200);

/* ===== PARTICLES ===== */
function createParticles() {
    const container = document.getElementById('particles-container');
    if (!container) return;

    const particleCount = 50;
    const sizes = ['small', 'medium', 'large'];

    for (let i = 0; i < particleCount; i++) {
        const particle = document.createElement('div');
        const size = sizes[Math.floor(Math.random() * sizes.length)];
        particle.className = `particle ${size}`;

        const x = Math.random() * 100;
        const y = Math.random() * 100;
        const duration = 8 + Math.random() * 6;
        const delay = Math.random() * 3;
        const moveX = -40 + Math.random() * 80;

        particle.style.left = x + '%';
        particle.style.top = y + '%';
        particle.style.setProperty('--duration', duration + 's');
        particle.style.setProperty('--delay', delay + 's');
        particle.style.setProperty('--moveX', moveX + 'px');

        container.appendChild(particle);
    }
}

const particleStyle = document.createElement('style');
particleStyle.textContent = `
    .particle {
        animation: float-particle var(--duration, 10s) ease-in-out var(--delay, 0s) infinite;
    }

    @keyframes float-particle {
        0% {
            transform: translateY(0) translateX(0);
            opacity: 0;
        }
        10% {
            opacity: 0.6;
        }
        90% {
            opacity: 0.6;
        }
        100% {
            transform: translateY(-120vh) translateX(var(--moveX, 0px));
            opacity: 0;
        }
    }
`;
document.head.appendChild(particleStyle);

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', createParticles);
} else {
    createParticles();
}
