#pragma once
#include <Arduino.h>

const char WEB_UI_CSS[] PROGMEM = R"rawliteral(
<style>
    :root {
        --bg-body: #0a0c12; --bg-card: #161b22; --bg-input: #0d1117; --bg-modal: rgba(10, 12, 18, 0.95);
        --accent: #238636; --accent-hover: #2ea043; --accent-glow: rgba(35, 134, 54, 0.4);
        --danger: #da3633; --warning: #d29922; --text-main: #c9d1d9; --text-muted: #8b949e;
        --border: #30363d; --card-shadow: 0 8px 24px rgba(0,0,0,0.6); --transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    }
    * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; outline: none; }
    body {
        background-color: var(--bg-body); color: var(--text-main); font-family: -apple-system, BlinkMacSystemFont, "Inter", sans-serif;
        margin: 0; padding: 0; line-height: 1.6; min-height: 100vh; display: flex; flex-direction: column; overflow-x: hidden;
    }
    .container { max-width: 900px; margin: 0 auto; padding: 20px; width: 100%; flex: 1; padding-bottom: 100px; }
    header {
        display: flex; justify-content: space-between; align-items: center; margin-bottom: 30px; padding: 15px 0;
        border-bottom: 1px solid var(--border); position: sticky; top: 0; background: var(--bg-body); z-index: 90;
    }
    h1 { font-size: 1.4rem; margin: 0; font-weight: 700; letter-spacing: -0.02em; }
    .card { background: var(--bg-card); border: 1px solid var(--border); border-radius: 16px; padding: 24px; margin-bottom: 20px; box-shadow: var(--card-shadow); }
    .status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); gap: 16px; margin-bottom: 20px; }
    .status-item { text-align: center; padding: 16px; display: flex; flex-direction: column; justify-content: center; }
    .status-val { font-size: 1.6rem; font-weight: 800; color: #fff; display: block; margin-bottom: 4px; }
    .status-label { font-size: 0.7rem; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.1em; font-weight: 700; }
    .relay-ctrl { display: flex; flex-direction: column; align-items: center; padding: 40px 0; }
    .btn-water {
        width: 180px; height: 180px; border-radius: 50%; border: 8px solid var(--border); background: var(--bg-input);
        color: var(--text-main); font-size: 1.2rem; font-weight: 800; cursor: pointer; transition: var(--transition);
        display: flex; flex-direction: column; align-items: center; justify-content: center; position: relative;
    }
    .btn-water.on { border-color: var(--accent); color: #fff; background: var(--accent); box-shadow: 0 0 40px var(--accent-glow); transform: scale(1.05); }
    .btn-water svg { width: 48px; height: 48px; margin-bottom: 10px; fill: currentColor; }
    .nav-bottom {
        position: fixed; bottom: 20px; left: 50%; transform: translateX(-50%); background: rgba(22, 27, 34, 0.85);
        backdrop-filter: blur(12px); border: 1px solid var(--border); border-radius: 30px; display: flex; padding: 6px;
        z-index: 1000; width: calc(100% - 40px); max-width: 500px; box-shadow: 0 10px 30px rgba(0,0,0,0.5);
    }
    .nav-link {
        flex: 1; text-align: center; color: var(--text-muted); font-size: 0.75rem; padding: 10px 0; border-radius: 25px;
        cursor: pointer; transition: var(--transition); display: flex; flex-direction: column; align-items: center; font-weight: 600;
    }
    .nav-link.active { color: #fff; background: var(--accent); }
    .nav-link svg { width: 22px; height: 22px; margin-bottom: 2px; fill: currentColor; }
    .tabs-content > div { display: none; }
    .tabs-content > div.active { display: block; animation: slideUp 0.4s ease; }
    @keyframes slideUp { from { opacity: 0; transform: translateY(20px); } to { opacity: 1; transform: translateY(0); } }
    .form-group { margin-bottom: 20px; }
    label { display: block; margin-bottom: 8px; color: var(--text-muted); font-size: 0.85rem; font-weight: 600; }
    input { width: 100%; padding: 12px 16px; background: var(--bg-input); border: 1px solid var(--border); border-radius: 10px; color: #fff; font-size: 1rem; }
    .btn { padding: 12px 24px; border-radius: 10px; font-weight: 700; cursor: pointer; border: none; transition: var(--transition); display: flex; align-items: center; justify-content: center; gap: 8px; }
    .btn-primary { background: var(--accent); color: #fff; }
    .btn-outline { background: transparent; border: 1.5px solid var(--border); color: var(--text-main); }
    .btn-danger { background: rgba(218, 54, 51, 0.1); border: 1.5px solid var(--danger); color: #ff7b72; }
    .slots-container { display: grid; grid-template-columns: repeat(auto-fill, minmax(280px, 1fr)); gap: 12px; }
    .slot-item { display: flex; align-items: center; background: var(--bg-input); padding: 16px; border-radius: 12px; border: 1px solid var(--border); }
    #toast-container { position: fixed; top: 20px; right: 20px; z-index: 9999; display: flex; flex-direction: column; gap: 10px; pointer-events: none; }
    .toast { padding: 16px 24px; background: var(--bg-card); border-left: 4px solid var(--accent); border-radius: 8px; box-shadow: var(--card-shadow); color: #fff; transform: translateX(120%); transition: var(--transition); pointer-events: auto; display: flex; align-items: center; gap: 12px; }
    .toast.show { transform: translateX(0); }
    .modal-overlay { position: fixed; top: 0; left: 0; right: 0; bottom: 0; background: rgba(0,0,0,0.8); backdrop-filter: blur(8px); z-index: 2000; display: none; align-items: center; justify-content: center; padding: 20px; }
    .modal { background: var(--bg-card); border: 1px solid var(--border); border-radius: 20px; width: 100%; max-width: 450px; padding: 30px; transform: scale(0.9); opacity: 0; transition: var(--transition); }
    .modal-overlay.show { display: flex; }
    .modal-overlay.show .modal { transform: scale(1); opacity: 1; }
    .progress-container { height: 8px; background: var(--bg-input); border-radius: 10px; overflow: hidden; }
    .progress-bar { height: 100%; background: var(--accent); width: 0%; transition: width 0.4s ease; }
    .tip-box { font-size: 0.8rem; color: var(--text-muted); background: rgba(35, 134, 54, 0.05); border-radius: 12px; padding: 16px; display: flex; gap: 12px; margin-top: 20px; }
</style>
)rawliteral";

const char WEB_UI_BODY[] PROGMEM = R"rawliteral(
<body>
    <div id="toast-container"></div>
    <div id="modal-confirm" class="modal-overlay">
        <div class="modal">
            <h3 id="modal-title">Подтверждение</h3>
            <p id="modal-msg" style="color: var(--text-muted); margin-bottom: 24px;"></p>
            <div style="display: flex; gap: 12px;">
                <button class="btn btn-outline" style="flex:1" onclick="closeModal()">Отмена</button>
                <button id="modal-ok" class="btn btn-primary" style="flex:1">Да</button>
            </div>
        </div>
    </div>
    <div class="container">
        <header>
            <h1>Hydro<span style="color:var(--accent)">ESP</span></h1>
            <div id="connection-status" style="font-size: 0.8rem; display: flex; align-items: center; gap: 6px;">
                <span style="width:8px; height:8px; background:var(--accent); border-radius:50%"></span> Онлайн
            </div>
        </header>
        <div class="tabs-content">
            <div id="tab-status" class="active">
                <div class="status-grid">
                    <div class="card status-item"><span id="stat-time" class="status-val">--:--</span><span class="status-label">Время</span></div>
                    <div class="card status-item"><span id="stat-sunrise" class="status-val">--:--</span><span class="status-label">Рассвет</span></div>
                    <div class="card status-item"><span id="stat-sunset" class="status-val">--:--</span><span class="status-label">Закат</span></div>
                </div>
                <div class="card relay-ctrl">
                    <button id="btn-main-relay" class="btn-water">
                        <svg viewBox="0 0 24 24"><path d="M12 2c-4.97 0-9 4.03-9 9 0 4.17 2.84 7.67 6.69 8.69L12 22l2.31-2.31C18.16 18.67 21 15.17 21 11c0-4.97-4.03-9-9-9zm0 15c-3.31 0-6-2.69-6-6s2.69-6 6-6 6 2.69 6 6-2.69 6-6 6z"/></svg>
                        <span id="relay-text">START</span>
                    </button>
                    <p style="margin-top: 25px; color: var(--text-muted); font-weight: 600;">Next: <span id="val-next" style="color: #fff;">--:--</span></p>
                </div>
                <div class="card">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 15px;"><span class="status-label">Раствор</span><span id="val-solution-days" style="font-weight: 700;">-- дн.</span></div>
                    <div class="progress-container" style="margin-bottom: 20px;"><div class="progress-bar" id="solution-bar"></div></div>
                    <button class="btn btn-outline" style="width:100%" onclick="confirmResetSolution()">🔄 Смена раствора</button>
                    <div class="tip-box"><span>💡</span><span>Рекомендуется менять раствор каждые 7-14 дней. 18-22°C.</span></div>
                </div>
            </div>
            <div id="tab-schedule">
                <div class="card">
                    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px;">
                        <div><h2>График</h2></div>
                        <div style="display: flex; gap: 8px;"><button class="btn btn-outline" onclick="toggleGenerator()">⚡</button><button class="btn btn-primary" onclick="addSlot()">+ Добавить</button></div>
                    </div>
                    <div id="generator-box" style="display: none; padding: 20px; background: var(--bg-input); border-radius: 12px; margin-bottom: 20px; border: 1px solid var(--accent);">
                        <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 15px;">
                            <div class="form-group"><label>Старт</label><input type="time" id="gen-start" value="06:00"></div>
                            <div class="form-group"><label>Стоп</label><input type="time" id="gen-stop" value="20:00"></div>
                            <div class="form-group"><label>Интервал</label><input type="number" id="gen-interval" value="120"></div>
                            <div class="form-group"><label>Полив</label><input type="number" id="gen-duration" value="120"></div>
                        </div>
                        <div style="display: flex; gap: 8px; margin-bottom: 15px;"><button class="btn btn-outline" style="flex:1" onclick="setGenTimes('sunrise')">Рассвет</button><button class="btn btn-outline" style="flex:1" onclick="setGenTimes('sunset')">Закат</button></div>
                        <button class="btn btn-primary" style="width:100%" onclick="generateSchedule()">Сгенерировать</button>
                    </div>
                    <div id="slots-list" class="slots-container" style="margin-bottom: 24px;"></div>
                    <div style="display: grid; grid-template-columns: 1fr 2fr; gap: 12px;"><button class="btn btn-outline" onclick="confirmResetSchedule()">Сброс</button><button class="btn btn-primary" onclick="saveSchedule()">Сохранить</button></div>
                </div>
            </div>
            <div id="tab-settings">
                <div class="card">
                    <h2>Конфигурация</h2>
                    <div class="form-group"><label>WiFi SSID / Pass</label><div style="display: flex; gap: 8px;"><input type="text" id="inp-ssid" style="flex:1"><input type="password" id="inp-pass" style="flex:1"></div></div>
                    <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 12px;"><div class="form-group"><label>Lat</label><input type="number" id="inp-lat" step="0.0001"></div><div class="form-group"><label>Lon</label><input type="number" id="inp-lon" step="0.0001"></div></div>
                    <div class="form-group"><label>TZ (UTC)</label><input type="number" id="inp-tz" step="1"></div>
                    <button class="btn btn-primary" style="width:100%" onclick="saveConfig()">Сохранить</button>
                </div>
                <div class="card" style="border-color: #442222;"><button class="btn btn-danger" style="width:100%" onclick="confirmReboot()">Перезагрузить ESP</button></div>
            </div>
            <div id="tab-ota">
                <div class="card">
                    <h2>Прошивка</h2>
                    <div id="drop-zone" style="border: 2px dashed var(--border); border-radius: 16px; padding: 40px; text-align: center; cursor: pointer;" onclick="document.getElementById('file-ota').click()">
                        <p>Нажмите для выбора .bin файла</p><input type="file" id="file-ota" accept=".bin" style="display:none">
                    </div>
                    <div id="ota-info" style="margin-top: 25px; display: none;"><div id="ota-status-text" style="text-align: center;">Подготовка...</div><div class="progress-container"><div class="progress-bar" id="ota-bar"></div></div></div>
                </div>
            </div>
        </div>
    </div>
    <nav class="nav-bottom">
        <div class="nav-link active" onclick="switchTab('status', this)"><svg viewBox="0 0 24 24"><path d="M10 20v-6h4v6h5v-8h3L12 3 2 12h3v8z"/></svg>Дом</div>
        <div class="nav-link" onclick="switchTab('schedule', this)"><svg viewBox="0 0 24 24"><path d="M19 3h-1V1h-2v2H8V1H6v2H5c-1.11 0-1.99.9-1.99 2L3 19c0 1.1.89 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm0 16H5V8h14v11z"/></svg>График</div>
        <div class="nav-link" onclick="switchTab('settings', this)"><svg viewBox="0 0 24 24"><path d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54c-.04-.24-.24-.41-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L3.82 7.89c-.11.2-.06.47.11.61l2.03 1.58c-.05.3-.07.62-.07.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z"/></svg>Опции</div>
        <div class="nav-link" onclick="switchTab('ota', this)"><svg viewBox="0 0 24 24"><path d="M5 20h14v-2H5v2zm0-10h4V3h6v7h4l-7 7-7-7z"/></svg>OTA</div>
    </nav>
)rawliteral";

const char WEB_UI_JS[] PROGMEM = R"rawliteral(
<script>
    let statusInterval, lastStatus = null;
    function showToast(m, t='success'){
        const c=document.getElementById('toast-container'), e=document.createElement('div');
        e.className=`toast ${t}`; e.innerHTML=`<span>${t==='success'?'✅':'❌'}</span> ${m}`;
        c.appendChild(e); setTimeout(()=>e.classList.add('show'),10);
        setTimeout(()=>{ e.classList.remove('show'); setTimeout(()=>e.remove(),400); },4000);
    }
    function openModal(t, m, f){
        document.getElementById('modal-title').innerText=t;
        document.getElementById('modal-msg').innerText=m;
        const o=document.getElementById('modal-confirm'); o.classList.add('show');
        document.getElementById('modal-ok').onclick=()=>{ f(); closeModal(); };
    }
    function closeModal(){ document.getElementById('modal-confirm').classList.remove('show'); }
    function switchTab(id, el){
        document.querySelectorAll('.tabs-content > div').forEach(d=>d.classList.remove('active'));
        document.querySelectorAll('.nav-link').forEach(l=>l.classList.remove('active'));
        document.getElementById('tab-'+id).classList.add('active'); el.classList.add('active');
        if(id==='schedule')loadSchedule(); if(id==='settings')loadConfig();
    }
    async function updateStatus(){
        try {
            const r=await fetch('/api/status'), d=await r.json(); lastStatus=d;
            document.getElementById('stat-time').innerText=d.time.substring(0,5);
            document.getElementById('stat-sunrise').innerText=d.sunrise;
            document.getElementById('stat-sunset').innerText=d.sunset;
            const b=document.getElementById('btn-main-relay'), t=document.getElementById('relay-text');
            if(d.relay){ b.classList.add('on'); t.innerText='STOP'; } else { b.classList.remove('on'); t.innerText='START'; }
            document.getElementById('val-next').innerText=d.next;
            if(d.last_sol>0 && d.now_epoch>0){
                const diff=Math.floor((d.now_epoch-d.last_sol)/86400);
                document.getElementById('val-solution-days').innerText=diff+' дн.';
                const p=Math.min(100,(diff/14)*100), bar=document.getElementById('solution-bar');
                bar.style.width=p+'%'; bar.style.background=diff>10?'var(--danger)':'var(--accent)';
            }
        } catch(e){}
    }
    document.getElementById('btn-main-relay').onclick=async()=>{
        const on=!document.getElementById('btn-main-relay').classList.contains('on');
        await fetch(on?'/api/relay/on?duration=60':'/api/relay/off',{method:'POST'});
        setTimeout(updateStatus,300);
    };
    function setGenTimes(t){ if(!lastStatus)return; document.getElementById('gen-start').value=(t==='sunrise'?lastStatus.sunrise:document.getElementById('gen-start').value); document.getElementById('gen-stop').value=(t==='sunset'?lastStatus.sunset:document.getElementById('gen-stop').value); }
    async function loadSchedule(){
        const r=await fetch('/api/schedule'), d=await r.json();
        const c=document.getElementById('slots-list'); c.innerHTML='';
        d.sort((a,b)=>(a.h*60+a.m)-(b.h*60+b.m)).forEach(s=>addSlotElement(s.h,s.m,s.d));
    }
    function addSlotElement(h=8,m=0,d=60){
        const c=document.getElementById('slots-list'), e=document.createElement('div');
        e.className='slot-item'; e.innerHTML=`<div style="flex:1;display:flex;gap:12px;"><input type="time" value="${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}" style="width:110px"><input type="number" value="${d}" style="width:80px"></div><button class="btn" style="color:var(--danger)" onclick="this.parentElement.remove()">×</button>`;
        c.appendChild(e);
    }
    function addSlot(){ addSlotElement(); }
    async function saveSchedule(){
        const s=[]; document.querySelectorAll('.slot-item').forEach(e=>{
            const t=e.querySelector('input[type="time"]').value.split(':'), d=e.querySelector('input[type="number"]').value;
            if(t.length===2)s.push({h:parseInt(t[0]),m:parseInt(t[1]),d:parseInt(d)||10});
        });
        if(await fetch('/api/schedule',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(s)})) showToast('Сохранено');
    }
    function confirmResetSchedule(){ openModal('Сброс','Вернуть стандартный график?',async()=>{ await fetch('/api/schedule/reset',{method:'POST'}); loadSchedule(); showToast('Сброшено'); }); }
    function confirmResetSolution(){ openModal('Смена раствора','Сбросить таймер?',async()=>{ await fetch('/api/solution/reset',{method:'POST'}); updateStatus(); showToast('Обновлено'); }); }
    function generateSchedule(){
        const start=document.getElementById('gen-start').value.split(':').map(Number), stop=document.getElementById('gen-stop').value.split(':').map(Number), interval=parseInt(document.getElementById('gen-interval').value), duration=parseInt(document.getElementById('gen-duration').value);
        let cur=start[0]*60+start[1], end=stop[0]*60+stop[1];
        document.getElementById('slots-list').innerHTML='';
        while(cur<=end){ addSlotElement(Math.floor(cur/60),cur%60,duration); cur+=interval; }
        document.getElementById('generator-box').style.display='none'; showToast('Сгенерировано');
    }
    function toggleGenerator(){ const b=document.getElementById('generator-box'); b.style.display=b.style.display==='none'?'block':'none'; }
    async function loadConfig(){
        const r=await fetch('/api/config'), d=await r.json();
        document.getElementById('inp-ssid').value=d.ssid; document.getElementById('inp-tz').value=d.tz;
        document.getElementById('inp-lat').value=d.lat; document.getElementById('inp-lon').value=d.lon;
    }
    async function saveConfig(){
        const c={ssid:document.getElementById('inp-ssid').value,pass:document.getElementById('inp-pass').value,tz:parseInt(document.getElementById('inp-tz').value),lat:parseFloat(document.getElementById('inp-lat').value),lon:parseFloat(document.getElementById('inp-lon').value)};
        await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(c)});
        showToast('Сохранено');
    }
    function confirmReboot(){ openModal('Перезагрузка','Перезагрузить ESP?',()=>{ fetch('/api/reboot',{method:'POST'}); showToast('Перезагрузка...','warning'); }); }
    const fi=document.getElementById('file-ota'); fi.onchange=()=>{ if(fi.files.length)startUpload(fi.files[0]); };
    function startUpload(f){
        const fd=new FormData(); fd.append('file',f); const x=new XMLHttpRequest(); x.open('POST','/ota/upload',true);
        document.getElementById('ota-info').style.display='block';
        const b=document.getElementById('ota-bar'), s=document.getElementById('ota-status-text');
        x.upload.onprogress=e=>{ if(e.lengthComputable){ const p=Math.round((e.loaded/e.total)*100); b.style.width=p+'%'; s.innerText='Прошивка: '+p+'%'; } };
        x.onload=()=>{ if(x.status===200){ s.innerText='Успешно!'; setTimeout(()=>window.location.reload(),5000); } else { s.innerText='Ошибка'; } };
        x.send(fd);
    }
    setInterval(updateStatus,3000); updateStatus();
</script>
</body>
</html>
)rawliteral";
