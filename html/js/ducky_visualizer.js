/**
 * 🌸 DuckyScript Visualizer Module by Persephone
 * Visualizes the execution flow of DuckyScript payloads.
 */

class DuckyVisualizer {
    constructor(containerId) {
        this.container = document.getElementById(containerId);
        this.lines = [];
        this.currentLine = -1;
    }

    loadScript(scriptContent) {
        this.container.innerHTML = '';
        this.lines = scriptContent.split('\n');

        this.lines.forEach((line, index) => {
            const lineEl = document.createElement('div');
            lineEl.className = 'ds-line';
            lineEl.id = `ds-line-${index + 1}`;

            const numSpan = document.createElement('span');
            numSpan.className = 'ds-line-num';
            numSpan.innerText = index + 1;

            const codeSpan = document.createElement('span');
            codeSpan.className = 'ds-code';
            codeSpan.innerText = line;

            lineEl.appendChild(numSpan);
            lineEl.appendChild(codeSpan);
            this.container.appendChild(lineEl);
        });
    }

    highlightLine(lineNumber) {
        // Remove previous highlight
        if (this.currentLine !== -1) {
            const prev = document.getElementById(`ds-line-${this.currentLine}`);
            if (prev) prev.classList.remove('active-line');
        }

        this.currentLine = lineNumber;
        const current = document.getElementById(`ds-line-${lineNumber}`);
        if (current) {
            current.classList.add('active-line');
            current.scrollIntoView({ behavior: 'smooth', block: 'center' });
        }
    }
}

// Global instance
window.duckyVisualizer = new DuckyVisualizer('ducky-script-container');
