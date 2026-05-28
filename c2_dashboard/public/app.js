const socket = io();

const logs = {
    keystrokes: document.getElementById('log-keystrokes'),
    honeypot: document.getElementById('log-honeypot'),
    network: document.getElementById('log-network')
};

function appendLog(target, message, isHighAlert = false) {
    const entry = document.createElement('div');
    entry.className = 'log-entry';
    
    const time = document.createElement('span');
    time.className = 'timestamp';
    const now = new Date();
    time.textContent = now.toTimeString().split(' ')[0] + '.' + now.getMilliseconds();
    
    const content = document.createElement('span');
    content.textContent = message;
    if (isHighAlert) {
        content.classList.add('alert-high');
    }
    
    entry.appendChild(time);
    entry.appendChild(content);
    
    target.appendChild(entry);
    target.scrollTop = target.scrollHeight;
}

socket.on('telemetry', (data) => {
    // USBArmyKnife JSON schema mapping
    if (data.t === 'cmd') {
        const cmd = data.c;
        const msg = data.m || '';

        if (cmd === 'keystroke') {
            appendLog(logs.keystrokes, msg);
        } 
        else if (cmd === 'honeypot_alert') {
            appendLog(logs.honeypot, `🚨 ${msg}`, true);
        }
        else if (cmd === 'network_scanner' || cmd === 'captive_portal_login') {
            appendLog(logs.network, `[${cmd}] ${msg}`);
        }
        else {
            // Generic fallback
            appendLog(logs.network, `[${cmd}] ${msg}`);
        }
    }
});
