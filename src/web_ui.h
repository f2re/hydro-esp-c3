#pragma once
#include <Arduino.h>

const char WEB_UI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HydroESP-C3</title>
    <style>
        :root {
            --bg-primary: #0f1117;
            --bg-card: #1a1d27;
            --accent: #22c55e;
            --accent-warn: #f59e0b;
            --text-primary: #f1f5f9;
            --text-muted: #64748b;
            --border: #2d3148;
        }
        body {
            background: var(--bg-primary);
            color: var(--text-primary);
            font-family: -apple-system, system-ui, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
        }
        .container { padding: 20px; max-width: 600px; margin: 0 auto; width: 100%; box-sizing: border-box; }
        .card { background: var(--bg-card); border-radius: 12px; padding: 20px; margin-bottom: 20px; border: 1px solid var(--border); }
        .header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
        h1 { font-size: 1.5rem; margin: 0; color: var(--accent); }
        .btn { border: none; border-radius: 8px; padding: 10px 20px; font-weight: bold; cursor: pointer; transition: 0.3s; }
        .btn-primary { background: var(--accent); color: white; }
        .btn-warn { background: var(--accent-warn); color: white; }
        .btn-outline { background: transparent; border: 1px solid var(--border); color: var(--text-primary); }
        .relay-btn { width: 120px; height: 120px; border-radius: 50%; border: 4px solid var(--border); background: transparent; color: var(--text-primary); font-size: 1.2rem; margin: 20px auto; display: block; }
        .relay-btn.on { border-color: var(--accent); color: var(--accent); box-shadow: 0 0 20px var(--accent); animation: pulse 2s infinite; }
        @keyframes pulse { 0% { opacity: 1; } 50% { opacity: 0.6; } 100% { opacity: 1; } }
        .nav { position: fixed; bottom: 0; left: 0; right: 0; background: var(--bg-card); display: flex; justify-content: space-around; padding: 10px; border-top: 1px solid var(--border); }
        .nav-item { color: var(--text-muted); text-decoration: none; font-size: 0.8rem; display: flex; flex-direction: column; align-items: center; cursor: pointer; }
        .nav-item.active { color: var(--accent); }
        .nav-item svg { width: 24px; height: 24px; margin-bottom: 4px; fill: currentColor; }
        .tab-content { display: none; margin-bottom: 80px; }
        .tab-content.active { display: block; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; color: var(--text-muted); font-size: 0.9rem; }
        input { width: 100%; padding: 10px; border-radius: 8px; border: 1px solid var(--border); background: var(--bg-primary); color: white; box-sizing: border-box; }
        .slot { display: flex; gap: 10px; align-items: center; margin-bottom: 10px; }
        .slot input { flex: 1; }
        .slot .del { color: #ef4444; cursor: pointer; }
        #ota-progress { height: 10px; background: var(--border); border-radius: 5px; overflow: hidden; margin-top: 10px; display: none; }
        #ota-progress-bar { height: 100%; background: var(--accent); width: 0%; transition: 0.3s; }
        .drop-zone { border: 2px dashed var(--border); border-radius: 12px; padding: 40px; text-align: center; color: var(--text-muted); transition: 0.3s; }
        .drop-zone.hover { border-color: var(--accent); background: rgba(34, 197, 94, 0.1); }
    </style>
</head>
<body>
    <div class="container">
        <!-- STATUS -->
        <div id="status" class="tab-content active">
            <h1>Статус</h1>
            <div class="card">
                <div id="time" style="font-size: 2rem; font-weight: bold; text-align: center;">00:00:00</div>
                <div id="date" style="text-align: center; color: var(--text-muted);">...</div>
            </div>
            <div class="card" style="text-align: center;">
                <button id="relay-btn" class="relay-btn">OFF</button>
                <div id="next-watering" style="margin-top: 10px; color: var(--text-muted);">Следующий полив: --:--</div>
            </div>
            <div class="card">
                <div style="display: flex; justify-content: space-between;">
                    <span>Uptime:</span><span id="uptime">...</span>
                </div>
                <div style="display: flex; justify-content: space-between; margin-top: 10px;">
                    <span>WiFi:</span><span id="wifi-info">...</span>
                </div>
            </div>
        </div>

        <!-- SCHEDULE -->
        <div id="schedule" class="tab-content">
            <h1>Расписание</h1>
            <div id="slots-container"></div>
            <button class="btn btn-outline" style="width: 100%; margin-bottom: 15px;" onclick="addSlot()">+ Добавить</button>
            <button class="btn btn-primary" style="width: 100%;" onclick="saveSchedule()">Сохранить</button>
        </div>

        <!-- SETTINGS -->
        <div id="settings" class="tab-content">
            <h1>Настройки</h1>
            <div class="card">
                <div class="form-group">
                    <label>WiFi SSID</label>
                    <input type="text" id="wifi-ssid">
                </div>
                <div class="form-group">
                    <label>WiFi Password</label>
                    <input type="password" id="wifi-pass">
                </div>
                <div class="form-group">
                    <label>Timezone (UTC offset)</label>
                    <input type="number" id="tz-offset">
                </div>
                <button class="btn btn-primary" style="width: 100%;" onclick="saveConfig()">Сохранить и перезагрузить</button>
            </div>
        </div>

        <!-- OTA -->
        <div id="ota" class="tab-content">
            <h1>Обновление</h1>
            <div class="card">
                <div id="drop-zone" class="drop-zone">
                    <p>Перетащите файл .bin сюда или нажмите для выбора</p>
                    <input type="file" id="ota-file" accept=".bin" style="display: none;">
                    <button class="btn btn-outline" onclick="document.getElementById('ota-file').click()">Выбрать файл</button>
                </div>
                <div id="ota-status" style="margin-top: 15px; text-align: center;"></div>
                <div id="ota-progress">
                    <div id="ota-progress-bar"></div>
                </div>
            </div>
        </div>
    </div>

    <div class="nav">
        <div class="nav-item active" onclick="showTab('status')">
            <svg viewBox="0 0 24 24"><path d="M10 20v-6h4v6h5v-8h3L12 3 2 12h3v8z"/></svg>
            Статус
        </div>
        <div class="nav-item" onclick="showTab('schedule')">
            <svg viewBox="0 0 24 24"><path d="M19 3h-1V1h-2v2H8V1H6v2H5c-1.11 0-1.99.9-1.99 2L3 19c0 1.1.89 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm0 16H5V8h14v11z"/></svg>
            График
        </div>
        <div class="nav-item" onclick="showTab('settings')">
            <svg viewBox="0 0 24 24"><path d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54c-.04-.24-.24-.41-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L3.82 7.89c-.11.2-.06.47.11.61l2.03 1.58c-.05.3-.07.62-.07.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z"/></svg>
            Настройки
        </div>
        <div class="nav-item" onclick="showTab('ota')">
            <svg viewBox="0 0 24 24"><path d="M5 20h14v-2H5v2zm0-10h4V3h6v7h4l-7 7-7-7z"/></svg>
            OTA
        </div>
    </div>

    <script>
        function showTab(tabId) {
            document.querySelectorAll('.tab-content').forEach(el => el.classList.remove('active'));
            document.querySelectorAll('.nav-item').forEach(el => el.classList.remove('active'));
            document.getElementById(tabId).classList.add('active');
            event.currentTarget.classList.add('active');
            if (tabId === 'schedule') loadSchedule();
            if (tabId === 'settings') loadConfig();
        }

        async function updateStatus() {
            try {
                const r = await fetch('/api/status');
                const d = await r.json();
                document.getElementById('time').innerText = d.time;
                document.getElementById('date').innerText = d.date;
                document.getElementById('uptime').innerText = d.uptime;
                document.getElementById('wifi-info').innerText = `${d.ssid} (${d.rssi} dBm)`;
                const btn = document.getElementById('relay-btn');
                btn.innerText = d.relay ? 'ON' : 'OFF';
                btn.className = 'relay-btn ' + (d.relay ? 'on' : '');
                document.getElementById('next-watering').innerText = 'Следующий полив: ' + d.next;
            } catch (e) {}
        }
        setInterval(updateStatus, 1000);

        document.getElementById('relay-btn').onclick = async () => {
            const on = !document.getElementById('relay-btn').classList.contains('on');
            await fetch('/api/relay/' + (on ? 'on' : 'off'), {method: 'POST'});
            updateStatus();
        };

        async function loadSchedule() {
            const r = await fetch('/api/schedule');
            const d = await r.json();
            const container = document.getElementById('slots-container');
            container.innerHTML = '';
            d.forEach((s, i) => {
                const div = document.createElement('div');
                div.className = 'card slot';
                div.innerHTML = `
                    <input type="time" value="${String(s.h).padStart(2,'0')}:${String(s.m).padStart(2,'0')}">
                    <input type="number" value="${s.d}" placeholder="сек">
                    <span class="del" onclick="this.parentElement.remove()">✕</span>
                `;
                container.appendChild(div);
            });
        }

        function addSlot() {
            const div = document.createElement('div');
            div.className = 'card slot';
            div.innerHTML = `<input type="time" value="08:00"><input type="number" value="300" placeholder="сек"><span class="del" onclick="this.parentElement.remove()">✕</span>`;
            document.getElementById('slots-container').appendChild(div);
        }

        async function saveSchedule() {
            const slots = [];
            document.querySelectorAll('.slot').forEach(el => {
                const [h, m] = el.querySelectorAll('input')[0].value.split(':');
                const d = el.querySelectorAll('input')[1].value;
                slots.push({h: parseInt(h), m: parseInt(m), d: parseInt(d)});
            });
            await fetch('/api/schedule', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(slots)
            });
            alert('Сохранено');
        }

        async function loadConfig() {
            const r = await fetch('/api/config');
            const d = await r.json();
            document.getElementById('wifi-ssid').value = d.ssid;
            document.getElementById('wifi-pass').value = d.pass;
            document.getElementById('tz-offset').value = d.tz;
        }

        async function saveConfig() {
            const config = {
                ssid: document.getElementById('wifi-ssid').value,
                pass: document.getElementById('wifi-pass').value,
                tz: parseInt(document.getElementById('tz-offset').value)
            };
            await fetch('/api/config', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(config)
            });
            alert('Конфигурация сохранена. ESP перезагрузится.');
        }

        // OTA
        const dz = document.getElementById('drop-zone');
        dz.ondragover = e => { e.preventDefault(); dz.classList.add('hover'); };
        dz.ondragleave = () => dz.classList.remove('hover');
        dz.ondrop = e => {
            e.preventDefault();
            dz.classList.remove('hover');
            if (e.dataTransfer.files.length) handleUpload(e.dataTransfer.files[0]);
        };
        document.getElementById('ota-file').onchange = e => {
            if (e.target.files.length) handleUpload(e.target.files[0]);
        };

        function handleUpload(file) {
            if (!file.name.endsWith('.bin')) { alert('Нужен .bin файл'); return; }
            const formData = new FormData();
            formData.append('file', file);
            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/ota/upload', true);
            xhr.upload.onprogress = e => {
                if (e.lengthComputable) {
                    const p = Math.round((e.loaded / e.total) * 100);
                    document.getElementById('ota-progress').style.display = 'block';
                    document.getElementById('ota-progress-bar').style.width = p + '%';
                    document.getElementById('ota-status').innerText = 'Загрузка: ' + p + '%';
                }
            };
            xhr.onload = () => {
                if (xhr.status === 200) {
                    document.getElementById('ota-status').innerText = 'Успешно! Перезагрузка...';
                    let count = 3;
                    setInterval(() => {
                        if (count > 0) document.getElementById('ota-status').innerText = 'Успешно! Перезагрузка через ' + count-- + '...';
                        else window.location.href = '/';
                    }, 1000);
                } else {
                    document.getElementById('ota-status').innerText = 'Ошибка при загрузке';
                }
            };
            xhr.send(formData);
        }
    </script>
</body>
</html>
)rawliteral";
