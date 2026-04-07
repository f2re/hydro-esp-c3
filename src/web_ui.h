#pragma once
#include <Arduino.h>

const char WEB_UI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>HydroESP-C3 | Управление поливом</title>
    <style>
        :root {
            --bg-body: #0a0c10;
            --bg-card: #161b22;
            --bg-input: #0d1117;
            --accent: #238636;
            --accent-hover: #2ea043;
            --accent-glow: rgba(35, 134, 54, 0.4);
            --danger: #da3633;
            --text-main: #c9d1d9;
            --text-muted: #8b949e;
            --border: #30363d;
            --card-shadow: 0 4px 12px rgba(0,0,0,0.5);
        }
        * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
        body {
            background-color: var(--bg-body);
            color: var(--text-main);
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
            margin: 0;
            padding: 0;
            line-height: 1.5;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            width: 100%;
            flex: 1;
        }
        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 24px;
            padding-bottom: 12px;
            border-bottom: 1px solid var(--border);
        }
        h1 { font-size: 1.25rem; margin: 0; color: #fff; }
        .card {
            background: var(--bg-card);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 16px;
            box-shadow: var(--card-shadow);
        }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 12px;
            margin-bottom: 16px;
        }
        @media (max-width: 480px) {
            .status-grid { grid-template-columns: 1fr; }
        }
        .status-item { text-align: center; }
        .status-val { font-size: 1.5rem; font-weight: bold; color: #fff; display: block; }
        .status-label { font-size: 0.75rem; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.05em; }

        /* Relay Button */
        .relay-ctrl {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: 30px 0;
        }
        .btn-water {
            width: 160px;
            height: 160px;
            border-radius: 50%;
            border: none;
            background: var(--bg-input);
            border: 6px solid var(--border);
            color: var(--text-main);
            font-size: 1.1rem;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            box-shadow: 0 0 0 0 rgba(255,255,255,0);
            position: relative;
        }
        .btn-water.on {
            border-color: var(--accent);
            color: #fff;
            background: var(--accent);
            box-shadow: 0 0 25px var(--accent-glow);
            animation: pulse 2s infinite;
        }
        @keyframes pulse {
            0% { box-shadow: 0 0 0 0 var(--accent-glow); }
            70% { box-shadow: 0 0 0 20px rgba(35, 134, 54, 0); }
            100% { box-shadow: 0 0 0 0 rgba(35, 134, 54, 0); }
        }
        .btn-water svg { width: 40px; height: 40px; margin-bottom: 8px; fill: currentColor; }
        
        /* Tabs & Nav */
        .tabs-content > div { display: none; }
        .tabs-content > div.active { display: block; animation: fadeIn 0.3s ease; }
        @keyframes fadeIn { from { opacity: 0; transform: translateY(5px); } to { opacity: 1; transform: translateY(0); } }
        
        .nav-bottom {
            position: sticky;
            bottom: 0;
            background: var(--bg-card);
            border-top: 1px solid var(--border);
            display: flex;
            justify-content: space-around;
            padding: 8px 0;
            z-index: 100;
        }
        .nav-link {
            flex: 1;
            text-align: center;
            color: var(--text-muted);
            text-decoration: none;
            font-size: 0.7rem;
            display: flex;
            flex-direction: column;
            align-items: center;
            transition: 0.2s;
            cursor: pointer;
        }
        .nav-link.active { color: var(--accent); }
        .nav-link svg { width: 24px; height: 24px; margin-bottom: 4px; fill: currentColor; }

        /* Forms */
        .form-group { margin-bottom: 16px; }
        label { display: block; margin-bottom: 6px; color: var(--text-muted); font-size: 0.85rem; }
        input, select {
            width: 100%;
            padding: 10px 12px;
            background: var(--bg-input);
            border: 1px solid var(--border);
            border-radius: 6px;
            color: #fff;
            font-size: 1rem;
            outline: none;
        }
        input:focus { border-color: var(--accent); }
        .btn {
            display: inline-block;
            padding: 10px 20px;
            border-radius: 6px;
            font-weight: 600;
            text-align: center;
            cursor: pointer;
            border: none;
            transition: 0.2s;
            width: 100%;
            font-size: 1rem;
        }
        .btn-primary { background: var(--accent); color: #fff; }
        .btn-primary:hover { background: var(--accent-hover); }
        .btn-outline { background: transparent; border: 1px solid var(--border); color: var(--text-main); }
        .btn-outline:hover { background: var(--border); }
        
        /* Schedule Slots */
        .slot-item {
            display: flex;
            gap: 10px;
            align-items: center;
            background: var(--bg-input);
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 8px;
            border: 1px solid var(--border);
        }
        .slot-item input[type="time"] { flex: 2; }
        .slot-item input[type="number"] { flex: 1; }
        .btn-del { color: var(--danger); background: none; border: none; font-size: 1.5rem; cursor: pointer; padding: 0 10px; }

        /* OTA */
        .drop-area {
            border: 2px dashed var(--border);
            border-radius: 12px;
            padding: 30px;
            text-align: center;
            color: var(--text-muted);
            transition: 0.3s;
            cursor: pointer;
        }
        .drop-area:hover, .drop-area.active { border-color: var(--accent); color: var(--text-main); background: rgba(35, 134, 54, 0.05); }
        .progress-container { height: 8px; background: var(--bg-input); border-radius: 4px; margin-top: 20px; overflow: hidden; display: none; }
        .progress-bar { height: 100%; background: var(--accent); width: 0%; transition: 0.2s; }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>HydroESP-C3</h1>
            <div id="connection-status" style="font-size: 0.75rem; color: var(--accent);">● Онлайн</div>
        </header>

        <div class="tabs-content">
            <!-- ТАБ: СТАТУС -->
            <div id="tab-status" class="active">
                <div class="status-grid">
                    <div class="card status-item">
                        <span id="stat-time" class="status-val">--:--:--</span>
                        <span class="status-label">Время</span>
                    </div>
                    <div class="card status-item">
                        <span id="stat-date" class="status-val">--.--.--</span>
                        <span class="status-label">Дата</span>
                    </div>
                </div>

                <div class="card relay-ctrl">
                    <button id="btn-main-relay" class="btn-water">
                        <svg viewBox="0 0 24 24"><path d="M12 2c-4.97 0-9 4.03-9 9 0 4.17 2.84 7.67 6.69 8.69L12 22l2.31-2.31C18.16 18.67 21 15.17 21 11c0-4.97-4.03-9-9-9zm0 15c-3.31 0-6-2.69-6-6s2.69-6 6-6 6 2.69 6 6-2.69 6-6 6z"/></svg>
                        <span id="relay-text">ПОЛИВ</span>
                    </button>
                    <div id="next-info" style="margin-top: 20px; color: var(--text-muted); font-size: 0.9rem;">
                        Следующий: <span id="val-next" style="color: #fff;">--:--</span>
                    </div>
                </div>

                <div class="card">
                    <div style="display: flex; justify-content: space-between; margin-bottom: 8px;">
                        <span class="status-label">WiFi Сеть</span>
                        <span id="val-ssid">...</span>
                    </div>
                    <div style="display: flex; justify-content: space-between; margin-bottom: 8px;">
                        <span class="status-label">Сигнал</span>
                        <span id="val-rssi">0 dBm</span>
                    </div>
                    <div style="display: flex; justify-content: space-between;">
                        <span class="status-label">Работает</span>
                        <span id="val-uptime">0ч 0м</span>
                    </div>
                </div>
            </div>

            <!-- ТАБ: РАСПИСАНИЕ -->
            <div id="tab-schedule">
                <div class="card" style="margin-bottom: 12px; border-left: 4px solid var(--accent);">
                    <div style="display: flex; justify-content: space-between; align-items: center;">
                        <div>
                            <h2 style="margin:0; font-size:1.1rem;">График полива</h2>
                            <p style="margin:4px 0 0 0; font-size:0.75rem; color: var(--text-muted);">Настройте время и длительность</p>
                        </div>
                        <button class="btn-primary" style="width:auto; padding: 8px 16px; border-radius: 20px;" onclick="addSlot()">+ Добавить</button>
                    </div>
                </div>

                <div id="slots-list" style="margin-bottom: 20px;">
                    <!-- Слоты будут здесь -->
                </div>

                <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;">
                    <button class="btn btn-outline" style="border-color: #444;" onclick="resetSchedule()">Сброс (Default)</button>
                    <button class="btn btn-primary" onclick="saveSchedule()">Сохранить всё</button>
                </div>
            </div>

            <!-- ТАБ: НАСТРОЙКИ -->
            <div id="tab-settings">
                <div class="card">
                    <h2 style="margin:0 0 16px 0; font-size:1.1rem;">Связь и время</h2>
                    <div class="form-group">
                        <label>Название WiFi (SSID)</label>
                        <input type="text" id="inp-ssid" placeholder="Название сети">
                    </div>
                    <div class="form-group">
                        <label>Пароль WiFi</label>
                        <input type="password" id="inp-pass" placeholder="Пароль">
                    </div>
                    <div class="form-group">
                        <label>Часовой пояс (UTC)</label>
                        <input type="number" id="inp-tz" value="3" step="1">
                    </div>
                    <button class="btn btn-primary" onclick="saveConfig()">Сохранить и применить</button>
                </div>
                
                <div class="card" style="border-color: #442222;">
                    <h2 style="margin:0 0 16px 0; font-size:1.1rem; color: #f88;">Система</h2>
                    <p style="font-size: 0.85rem; color: var(--text-muted);">Перезагрузка устройства сбросит текущие временные состояния.</p>
                    <button class="btn btn-outline" style="color: #f88; border-color: #633;" onclick="reboot()">Перезагрузить ESP</button>
                </div>
            </div>

            <!-- ТАБ: ОБНОВЛЕНИЕ -->
            <div id="tab-ota">
                <div class="card">
                    <h2 style="margin:0 0 16px 0; font-size:1.1rem;">Обновление прошивки</h2>
                    <div id="drop-zone" class="drop-area" onclick="document.getElementById('file-ota').click()">
                        <svg viewBox="0 0 24 24" style="width:48px; height:48px; fill: var(--text-muted); margin-bottom: 12px;"><path d="M19.35 10.04C18.67 6.59 15.64 4 12 4 9.11 4 6.6 5.64 5.35 8.04 2.34 8.36 0 10.91 0 14c0 3.31 2.69 6 6 6h13c2.76 0 5-2.24 5-5 0-2.64-2.05-4.78-4.65-4.96zM14 13v4h-4v-4H7l5-5 5 5h-3z"/></svg>
                        <p>Выберите файл .bin или перетащите его сюда</p>
                        <input type="file" id="file-ota" accept=".bin" style="display:none">
                    </div>
                    <div id="ota-info" style="margin-top: 15px; text-align: center; display: none;">
                        <div id="ota-status-text">Загрузка...</div>
                        <div class="progress-container" id="ota-progress-box" style="display: block;">
                            <div class="progress-bar" id="ota-bar"></div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <nav class="nav-bottom">
        <div class="nav-link active" onclick="switchTab('status', this)">
            <svg viewBox="0 0 24 24"><path d="M10 20v-6h4v6h5v-8h3L12 3 2 12h3v8z"/></svg>
            Статус
        </div>
        <div class="nav-link" onclick="switchTab('schedule', this)">
            <svg viewBox="0 0 24 24"><path d="M19 3h-1V1h-2v2H8V1H6v2H5c-1.11 0-1.99.9-1.99 2L3 19c0 1.1.89 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm0 16H5V8h14v11z"/></svg>
            График
        </div>
        <div class="nav-link" onclick="switchTab('settings', this)">
            <svg viewBox="0 0 24 24"><path d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54c-.04-.24-.24-.41-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L3.82 7.89c-.11.2-.06.47.11.61l2.03 1.58c-.05.3-.07.62-.07.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z"/></svg>
            Опции
        </div>
        <div class="nav-link" onclick="switchTab('ota', this)">
            <svg viewBox="0 0 24 24"><path d="M5 20h14v-2H5v2zm0-10h4V3h6v7h4l-7 7-7-7z"/></svg>
            Обновление
        </div>
    </nav>

    <script>
        let statusInterval;
        
        function switchTab(tabId, el) {
            document.querySelectorAll('.tab-content > div, .tabs-content > div').forEach(d => d.classList.remove('active'));
            document.querySelectorAll('.nav-link').forEach(l => l.classList.remove('active'));
            document.getElementById('tab-' + tabId).classList.add('active');
            el.classList.add('active');
            
            if (tabId === 'schedule') loadSchedule();
            if (tabId === 'settings') loadConfig();
        }

        async function updateStatus() {
            try {
                const res = await fetch('/api/status');
                const d = await res.json();
                document.getElementById('stat-time').innerText = d.time;
                document.getElementById('stat-date').innerText = d.date;
                document.getElementById('val-ssid').innerText = d.ssid;
                document.getElementById('val-rssi').innerText = d.rssi + ' dBm';
                
                let h = Math.floor(d.uptime / 3600);
                let m = Math.floor((d.uptime % 3600) / 60);
                document.getElementById('val-uptime').innerText = `${h}ч ${m}м`;
                
                const btn = document.getElementById('btn-main-relay');
                const txt = document.getElementById('relay-text');
                if (d.relay) {
                    btn.classList.add('on');
                    txt.innerText = 'ПОЛИВ ИДЕТ';
                } else {
                    btn.classList.remove('on');
                    txt.innerText = 'ПОЛИВ';
                }
                document.getElementById('val-next').innerText = d.next;
                document.getElementById('connection-status').style.color = 'var(--accent)';
                document.getElementById('connection-status').innerText = '● Онлайн';
            } catch (e) {
                document.getElementById('connection-status').style.color = 'var(--danger)';
                document.getElementById('connection-status').innerText = '○ Офлайн';
            }
        }

        document.getElementById('btn-main-relay').onclick = async () => {
            const isOff = !document.getElementById('btn-main-relay').classList.contains('on');
            try {
                const url = isOff ? '/api/relay/on?duration=60' : '/api/relay/off';
                await fetch(url, {method: 'POST'});
                setTimeout(updateStatus, 200);
            } catch (e) {}
        };

        async function loadSchedule() {
            const res = await fetch('/api/schedule');
            const data = await res.json();
            const container = document.getElementById('slots-list');
            container.innerHTML = '';
            
            // Сортировка по времени для удобства
            data.sort((a,b) => (a.h*60+a.m) - (b.h*60+b.m));
            
            data.forEach(s => {
                addSlotElement(s.h, s.m, s.d);
            });
            
            if (data.length === 0) {
                container.innerHTML = '<div style="text-align:center; padding:20px; color:var(--text-muted);">Расписание пусто</div>';
            }
        }

        function addSlotElement(h=8, m=0, d=60) {
            const container = document.getElementById('slots-list');
            if (container.querySelector('div[style*="text-align:center"]')) {
                container.innerHTML = '';
            }
            
            const div = document.createElement('div');
            div.className = 'card slot-item';
            div.style = 'margin-bottom:8px; padding:12px; display:flex; align-items:center; gap:10px; animation: slideIn 0.3s ease;';
            div.innerHTML = `
                <div style="flex:1; display:flex; flex-direction:column; gap:4px;">
                    <div style="font-size:0.7rem; color:var(--text-muted); display:flex; justify-content:space-between;">
                        <span>ВРЕМЯ</span><span>ДЛИТ. (СЕК)</span>
                    </div>
                    <div style="display:flex; gap:8px;">
                        <input type="time" value="${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}" style="flex:2;">
                        <input type="number" value="${d}" placeholder="60" style="flex:1;">
                    </div>
                </div>
                <button class="btn-del" onclick="removeSlot(this)">×</button>
            `;
            container.appendChild(div);
        }

        function removeSlot(btn) {
            const el = btn.parentElement;
            el.style.opacity = '0';
            el.style.transform = 'translateX(20px)';
            setTimeout(() => el.remove(), 300);
        }

        function addSlot() { addSlotElement(); }

        async function saveSchedule() {
            const slots = [];
            const items = document.querySelectorAll('.slot-item');
            if (items.length > 32) { alert('Максимум 32 слота'); return; }
            
            items.forEach(el => {
                const t = el.querySelector('input[type="time"]').value.split(':');
                const d = el.querySelector('input[type="number"]').value;
                if (t.length === 2) {
                    slots.push({h: parseInt(t[0]), m: parseInt(t[1]), d: parseInt(d) || 10});
                }
            });
            
            const res = await fetch('/api/schedule', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(slots)
            });
            if (res.ok) {
                alert('Расписание успешно сохранено в памяти!');
                loadSchedule();
            }
        }

        async function resetSchedule() {
            if (confirm('Сбросить расписание к заводским настройкам? Все ваши изменения будут удалены.')) {
                await fetch('/api/schedule/reset', {method: 'POST'});
                loadSchedule();
            }
        }

        const styleSheet = document.createElement("style");
        styleSheet.innerText = `
            @keyframes slideIn { from { opacity: 0; transform: translateX(-10px); } to { opacity: 1; transform: translateX(0); } }
        `;
        document.head.appendChild(styleSheet);

        async function loadConfig() {
            const res = await fetch('/api/config');
            const d = await res.json();
            document.getElementById('inp-ssid').value = d.ssid;
            document.getElementById('inp-pass').value = d.pass;
            document.getElementById('inp-tz').value = d.tz;
        }

        async function saveConfig() {
            const cfg = {
                ssid: document.getElementById('inp-ssid').value,
                pass: document.getElementById('inp-pass').value,
                tz: parseInt(document.getElementById('inp-tz').value)
            };
            await fetch('/api/config', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(cfg)
            });
            alert('Настройки сохранены. Устройство перезагружается...');
        }

        function reboot() {
            if(confirm('Перезагрузить устройство?')) {
                fetch('/api/reboot', {method:'POST'});
            }
        }

        // OTA logic
        const fileInp = document.getElementById('file-ota');
        const dz = document.getElementById('drop-zone');
        
        dz.ondragover = e => { e.preventDefault(); dz.classList.add('active'); };
        dz.ondragleave = () => dz.classList.remove('active');
        dz.ondrop = e => {
            e.preventDefault();
            dz.classList.remove('active');
            if (e.dataTransfer.files.length) startUpload(e.dataTransfer.files[0]);
        };
        fileInp.onchange = () => { if (fileInp.files.length) startUpload(fileInp.files[0]); };

        function startUpload(file) {
            const formData = new FormData();
            formData.append('file', file);
            const xhr = new XMLHttpRequest();
            xhr.open('POST', '/ota/upload', true);
            
            document.getElementById('ota-info').style.display = 'block';
            const bar = document.getElementById('ota-bar');
            const status = document.getElementById('ota-status-text');
            
            xhr.upload.onprogress = e => {
                if (e.lengthComputable) {
                    const p = Math.round((e.loaded / e.total) * 100);
                    bar.style.width = p + '%';
                    status.innerText = 'Загрузка: ' + p + '%';
                }
            };
            xhr.onload = () => {
                if (xhr.status === 200) {
                    status.innerText = 'Успешно! Перезагрузка...';
                    setTimeout(() => window.location.reload(), 5000);
                } else {
                    status.innerText = 'Ошибка при обновлении';
                    status.style.color = 'var(--danger)';
                }
            };
            xhr.send(formData);
        }

        statusInterval = setInterval(updateStatus, 2000);
        updateStatus();
    </script>
</body>
</html>
)rawliteral";
