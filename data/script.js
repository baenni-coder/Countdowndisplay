// API Base URL
const API_BASE = '/api';

// State
let countdowns = [];

// Initialize
document.addEventListener('DOMContentLoaded', function() {
    loadStatus();
    loadCountdowns();
    loadWiFiSettings();
});

// Load system status
async function loadStatus() {
    try {
        const response = await fetch(`${API_BASE}/status`);
        const data = await response.json();

        document.getElementById('status-mode').textContent = data.apMode ? 'Access Point' : 'WiFi Client';
        document.getElementById('status-ip').textContent = data.ip;
        document.getElementById('status-ssid').textContent = data.ssid;
    } catch (error) {
        console.error('Fehler beim Laden des Status:', error);
    }
}

// Load all countdowns
async function loadCountdowns() {
    try {
        const response = await fetch(`${API_BASE}/countdowns`);
        countdowns = await response.json();
        renderCountdowns();
    } catch (error) {
        console.error('Fehler beim Laden der Countdowns:', error);
        document.getElementById('countdown-list').innerHTML = '<p class="loading">Fehler beim Laden</p>';
    }
}

// Render countdowns list
function renderCountdowns() {
    const listElement = document.getElementById('countdown-list');

    if (countdowns.length === 0) {
        listElement.innerHTML = '<p class="loading">Keine Countdowns konfiguriert</p>';
        return;
    }

    listElement.innerHTML = '';

    countdowns.forEach(countdown => {
        const daysRemaining = calculateDaysRemaining(countdown.targetDate);
        const isActive = countdown.active ? '' : 'inactive';

        const item = document.createElement('div');
        item.className = `countdown-item ${isActive}`;
        item.innerHTML = `
            <div class="countdown-info">
                <h3>${countdown.name}</h3>
                <p>📅 Datum: ${formatDate(countdown.targetDate)}</p>
                <p>🔖 UID: ${countdown.uid}</p>
                ${countdown.active ? `<p class="days-remaining">⏱️ ${daysRemaining} Tage ${daysRemaining >= 0 ? 'verbleibend' : 'vergangen'}</p>` : '<p>⏸️ Inaktiv</p>'}
            </div>
            <div class="countdown-actions">
                <button class="btn btn-secondary" onclick="editCountdown('${countdown.uid}')">Bearbeiten</button>
                <button class="btn btn-danger" onclick="deleteCountdown('${countdown.uid}')">Löschen</button>
            </div>
        `;
        listElement.appendChild(item);
    });
}

// Calculate days remaining
function calculateDaysRemaining(targetDate) {
    const target = new Date(targetDate + 'T00:00:00');
    const today = new Date();
    today.setHours(0, 0, 0, 0);

    const diffTime = target - today;
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24));

    return diffDays;
}

// Format date to German format
function formatDate(dateString) {
    const date = new Date(dateString + 'T00:00:00');
    return date.toLocaleDateString('de-DE', {
        year: 'numeric',
        month: 'long',
        day: 'numeric'
    });
}

// Show add modal
function showAddModal() {
    document.getElementById('modal-title').textContent = 'Countdown hinzufügen';
    document.getElementById('edit-mode').value = 'add';
    document.getElementById('countdown-form').reset();
    document.getElementById('card-uid').value = '';
    document.getElementById('countdown-active').checked = true;
    document.getElementById('modal').classList.add('show');
}

// Edit countdown
function editCountdown(uid) {
    const countdown = countdowns.find(c => c.uid === uid);
    if (!countdown) return;

    document.getElementById('modal-title').textContent = 'Countdown bearbeiten';
    document.getElementById('edit-mode').value = 'edit';
    document.getElementById('edit-uid').value = uid;
    document.getElementById('card-uid').value = countdown.uid;
    document.getElementById('countdown-name').value = countdown.name;
    document.getElementById('countdown-date').value = countdown.targetDate;
    document.getElementById('countdown-active').checked = countdown.active;
    document.getElementById('modal').classList.add('show');
}

// Close modal
function closeModal() {
    document.getElementById('modal').classList.remove('show');
}

// Scan RFID card
async function scanCard() {
    const button = event.target;
    button.disabled = true;
    button.textContent = 'Scanne...';

    try {
        // Warte kurz, damit Benutzer Zeit hat die Karte zu positionieren
        await new Promise(resolve => setTimeout(resolve, 500));

        const response = await fetch(`${API_BASE}/scan-card`);
        const data = await response.json();

        if (data.success) {
            document.getElementById('card-uid').value = data.uid;
            alert(`Karte gescannt: ${data.uid}`);
        } else {
            alert('Keine Karte gefunden. Bitte Karte näher an den Leser halten und erneut versuchen.');
        }
    } catch (error) {
        console.error('Fehler beim Scannen:', error);
        alert('Fehler beim Scannen der Karte');
    } finally {
        button.disabled = false;
        button.textContent = 'Karte Scannen';
    }
}

// Save countdown
async function saveCountdown(event) {
    event.preventDefault();

    const mode = document.getElementById('edit-mode').value;
    const oldUid = document.getElementById('edit-uid').value;

    const countdown = {
        uid: document.getElementById('card-uid').value.toUpperCase(),
        name: document.getElementById('countdown-name').value,
        targetDate: document.getElementById('countdown-date').value,
        active: document.getElementById('countdown-active').checked
    };

    try {
        let response;
        if (mode === 'add') {
            response = await fetch(`${API_BASE}/countdowns`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(countdown)
            });
        } else {
            response = await fetch(`${API_BASE}/countdowns/${oldUid}`, {
                method: 'PUT',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(countdown)
            });
        }

        const result = await response.json();

        if (result.success) {
            closeModal();
            loadCountdowns();
            alert('Countdown erfolgreich gespeichert!');
        } else {
            alert('Fehler: ' + (result.error || 'Unbekannter Fehler'));
        }
    } catch (error) {
        console.error('Fehler beim Speichern:', error);
        alert('Fehler beim Speichern des Countdowns');
    }
}

// Delete countdown
async function deleteCountdown(uid) {
    if (!confirm('Möchtest du diesen Countdown wirklich löschen?')) {
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/countdowns/${uid}`, {
            method: 'DELETE'
        });

        const result = await response.json();

        if (result.success) {
            loadCountdowns();
            alert('Countdown gelöscht!');
        } else {
            alert('Fehler beim Löschen: ' + (result.error || 'Unbekannter Fehler'));
        }
    } catch (error) {
        console.error('Fehler beim Löschen:', error);
        alert('Fehler beim Löschen des Countdowns');
    }
}

// Load WiFi settings
async function loadWiFiSettings() {
    try {
        const response = await fetch(`${API_BASE}/wifi`);
        const data = await response.json();

        if (data.ssid) {
            document.getElementById('wifi-ssid').value = data.ssid;
        }
    } catch (error) {
        console.error('Fehler beim Laden der WiFi Einstellungen:', error);
    }
}

// Save WiFi settings
async function saveWiFi(event) {
    event.preventDefault();

    const ssid = document.getElementById('wifi-ssid').value;
    const password = document.getElementById('wifi-password').value;

    if (!confirm('WiFi Einstellungen speichern? Das System wird neu gestartet.')) {
        return;
    }

    try {
        const response = await fetch(`${API_BASE}/wifi`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ ssid, password })
        });

        const result = await response.json();

        if (result.success) {
            alert('WiFi Einstellungen gespeichert. System wird neu gestartet...');
            // Restart system
            await fetch(`${API_BASE}/restart`, { method: 'POST' });
        } else {
            alert('Fehler: ' + (result.error || 'Unbekannter Fehler'));
        }
    } catch (error) {
        console.error('Fehler beim Speichern:', error);
        alert('Fehler beim Speichern der WiFi Einstellungen');
    }
}

// Restart system
async function restartSystem() {
    if (!confirm('System wirklich neu starten?')) {
        return;
    }

    try {
        await fetch(`${API_BASE}/restart`, { method: 'POST' });
        alert('System wird neu gestartet...');
    } catch (error) {
        console.error('Fehler beim Neustart:', error);
    }
}

// Close modal when clicking outside
window.onclick = function(event) {
    const modal = document.getElementById('modal');
    if (event.target === modal) {
        closeModal();
    }
}
