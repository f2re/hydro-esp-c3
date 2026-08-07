#pragma once
#include <Arduino.h>

const char WEB_UI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0b1014">
<title>HydroESP-C3</title>
<style>
:root{
  color-scheme:dark;
  --bg:#0b1014;--surface:#11181e;--surface2:#172129;--surface3:#1d2932;
  --text:#f3f7f8;--muted:#91a0aa;--border:#26343e;
  --accent:#36d399;--accent2:#22b980;--accent-soft:rgba(54,211,153,.13);
  --blue:#69a7ff;--warn:#f6c453;--danger:#ff6b72;--danger-soft:rgba(255,107,114,.12);
  --shadow:0 18px 48px rgba(0,0,0,.28);--radius:18px;--radius-sm:12px;
  --sidebar:216px;
}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html{background:var(--bg);scroll-behavior:smooth}
body{margin:0;min-height:100vh;background:
 radial-gradient(circle at 15% -10%,rgba(54,211,153,.10),transparent 28rem),
 var(--bg);color:var(--text);font:15px/1.45 -apple-system,BlinkMacSystemFont,"Segoe UI",Inter,Roboto,Arial,sans-serif}
button,input,select{font:inherit}
button{touch-action:manipulation}
button:focus-visible,input:focus-visible,select:focus-visible,.drop-zone:focus-visible{outline:2px solid var(--accent);outline-offset:2px}
.hidden{display:none!important}
.app{min-height:100vh;display:grid;grid-template-columns:var(--sidebar) minmax(0,1fr)}
.sidebar{position:sticky;top:0;height:100vh;padding:22px 14px;border-right:1px solid var(--border);background:rgba(11,16,20,.82);backdrop-filter:blur(20px);z-index:20}
.brand{display:flex;align-items:center;gap:11px;padding:0 10px 22px;font-weight:750;letter-spacing:-.02em}
.brand-mark{width:34px;height:34px;border-radius:11px;display:grid;place-items:center;background:linear-gradient(145deg,#42e1a5,#1d9c6d);color:#062419;box-shadow:0 8px 22px rgba(54,211,153,.2)}
.nav{display:grid;gap:6px}
.nav-btn{border:0;background:transparent;color:var(--muted);width:100%;padding:11px 12px;border-radius:12px;display:flex;align-items:center;gap:11px;text-align:left;cursor:pointer;transition:.2s ease}
.nav-btn:hover{background:var(--surface2);color:var(--text)}
.nav-btn.active{background:var(--accent-soft);color:var(--accent)}
.nav-icon{width:23px;text-align:center;font-size:17px}
.sidebar-foot{position:absolute;left:14px;right:14px;bottom:18px;color:var(--muted);font-size:12px;padding:0 10px}
.main{min-width:0;padding:22px 28px 50px}
.topbar{max-width:1160px;margin:0 auto 20px;display:flex;justify-content:space-between;align-items:center;gap:16px}
.page-title{font-size:24px;font-weight:760;letter-spacing:-.035em}
.page-subtitle{color:var(--muted);font-size:13px;margin-top:2px}
.connection{display:flex;align-items:center;gap:8px;background:var(--surface);border:1px solid var(--border);border-radius:999px;padding:8px 11px;color:var(--muted);font-size:12px;white-space:nowrap}
.dot{width:8px;height:8px;border-radius:50%;background:var(--muted);box-shadow:0 0 0 4px rgba(145,160,170,.08)}
.connection.online .dot{background:var(--accent);box-shadow:0 0 0 4px var(--accent-soft)}
.connection.offline .dot{background:var(--danger);box-shadow:0 0 0 4px var(--danger-soft)}
.content{max-width:1160px;margin:auto}
.tab{display:none}.tab.active{display:block;animation:tabIn .25s ease both}
@keyframes tabIn{from{opacity:0;transform:translateY(6px)}to{opacity:1;transform:none}}
.grid{display:grid;gap:16px}.grid.metrics{grid-template-columns:repeat(4,minmax(0,1fr))}.grid.two{grid-template-columns:minmax(0,1.4fr) minmax(300px,.8fr)}
.card{background:linear-gradient(180deg,rgba(23,33,41,.96),rgba(17,24,30,.96));border:1px solid var(--border);border-radius:var(--radius);box-shadow:var(--shadow);padding:18px}
.metric{min-height:108px;display:flex;flex-direction:column;justify-content:space-between}
.metric-label{font-size:12px;color:var(--muted)}.metric-value{font-size:25px;font-weight:760;letter-spacing:-.035em}.metric-note{font-size:12px;color:var(--muted)}
.section-title{font-size:18px;font-weight:720;letter-spacing:-.02em;margin:0}.section-note{font-size:13px;color:var(--muted);margin:4px 0 0}
.card-head{display:flex;align-items:flex-start;justify-content:space-between;gap:12px;margin-bottom:16px}
.pump-card{min-height:356px;display:grid;grid-template-columns:minmax(230px,.8fr) minmax(280px,1.2fr);align-items:center;gap:24px;overflow:hidden;position:relative}
.pump-visual{display:grid;place-items:center;position:relative;min-height:260px}
.pump-ring{--p:0;position:relative;width:220px;height:220px;border-radius:50%;display:grid;place-items:center;background:conic-gradient(var(--accent) calc(var(--p)*1%),var(--surface3) 0);transition:background .35s ease}
.pump-ring:before{content:"";position:absolute;inset:10px;border-radius:50%;background:var(--surface);border:1px solid var(--border)}
.pump-button{position:relative;z-index:2;width:174px;height:174px;border-radius:50%;border:0;background:linear-gradient(145deg,#18232b,#0e151a);color:var(--text);cursor:pointer;box-shadow:inset 0 0 0 1px var(--border),0 16px 36px rgba(0,0,0,.32);transition:transform .18s ease,box-shadow .25s ease,background .3s ease}
.pump-button:hover{transform:translateY(-2px)}.pump-button:active{transform:scale(.98)}
.pump-button.on{background:linear-gradient(145deg,#2bd18f,#16885f);color:#041a11;box-shadow:0 0 0 10px rgba(54,211,153,.08),0 18px 44px rgba(54,211,153,.25)}
.pump-button.on:after{content:"";position:absolute;inset:-12px;border:1px solid rgba(54,211,153,.32);border-radius:50%;animation:ripple 2s ease-out infinite}
@keyframes ripple{0%{transform:scale(.9);opacity:.7}100%{transform:scale(1.18);opacity:0}}
.pump-icon{font-size:38px;display:block;line-height:1}.pump-label{display:block;font-weight:800;margin-top:8px;font-size:15px;letter-spacing:.02em}.pump-remain{display:block;font-size:12px;opacity:.75;margin-top:4px}
.control-stack{display:grid;gap:14px}.control-row{display:flex;gap:8px;flex-wrap:wrap}.duration-chip{border:1px solid var(--border);background:var(--surface2);color:var(--muted);padding:9px 11px;border-radius:10px;cursor:pointer;transition:.2s}.duration-chip.active{border-color:rgba(54,211,153,.55);background:var(--accent-soft);color:var(--accent)}
.info-row{display:flex;justify-content:space-between;gap:16px;padding:11px 0;border-bottom:1px solid rgba(38,52,62,.8)}.info-row:last-child{border:0}.info-row span:first-child{color:var(--muted)}
.toolbar{display:flex;gap:10px;align-items:center;flex-wrap:wrap}.btn{border:1px solid var(--border);background:var(--surface2);color:var(--text);border-radius:11px;padding:10px 14px;cursor:pointer;font-weight:650;transition:.18s ease;display:inline-flex;align-items:center;justify-content:center;gap:7px}.btn:hover{transform:translateY(-1px);border-color:#3c4d59}.btn:active{transform:none}.btn:disabled{opacity:.45;cursor:not-allowed;transform:none}.btn-primary{background:var(--accent);border-color:var(--accent);color:#062318}.btn-danger{color:var(--danger);border-color:rgba(255,107,114,.28);background:var(--danger-soft)}.btn-ghost{background:transparent}.btn-small{padding:8px 10px;font-size:13px}.btn-block{width:100%}
.schedule-summary{grid-template-columns:repeat(3,1fr);margin-bottom:16px}.mini-stat{padding:14px 16px;background:var(--surface);border:1px solid var(--border);border-radius:14px}.mini-stat b{font-size:20px;display:block}.mini-stat span{color:var(--muted);font-size:12px}
.schedule-list{display:grid;gap:9px}.slot{display:grid;grid-template-columns:70px minmax(110px,1fr) minmax(90px,.7fr) 40px;gap:10px;align-items:center;background:var(--surface);border:1px solid var(--border);padding:10px;border-radius:13px;animation:slotIn .22s ease both;transition:.22s ease}
@keyframes slotIn{from{opacity:0;transform:translateY(5px)}to{opacity:1;transform:none}}.slot.removing{opacity:0;transform:translateX(16px)}.slot-index{color:var(--muted);font-size:12px;text-align:center}.slot-del{width:38px;height:38px;border-radius:10px;border:0;background:transparent;color:var(--muted);cursor:pointer;font-size:20px}.slot-del:hover{background:var(--danger-soft);color:var(--danger)}
.empty{padding:36px 16px;text-align:center;color:var(--muted);border:1px dashed var(--border);border-radius:14px}
label{display:block;color:var(--muted);font-size:12px;margin:0 0 6px}.field{min-width:0}.input{width:100%;height:42px;border:1px solid var(--border);border-radius:10px;background:#0e151a;color:var(--text);padding:0 11px;transition:.18s}.input:hover{border-color:#394c58}.input:focus{border-color:var(--accent);box-shadow:0 0 0 3px var(--accent-soft);outline:0}.input.invalid{border-color:var(--danger);box-shadow:0 0 0 3px var(--danger-soft)}
.form-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:13px}.form-grid.three{grid-template-columns:repeat(3,minmax(0,1fr))}.form-section{padding-top:16px;margin-top:16px;border-top:1px solid var(--border)}
.calc-result{background:linear-gradient(145deg,rgba(54,211,153,.10),rgba(105,167,255,.06));border:1px solid rgba(54,211,153,.25);border-radius:16px;padding:16px;margin-top:16px}.result-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}.result-item{padding:10px;border-radius:11px;background:rgba(11,16,20,.46)}.result-item b{font-size:18px;display:block}.result-item span{font-size:11px;color:var(--muted)}
.formula{font:12px/1.55 ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;color:#bad1c8;background:#0b1115;border:1px solid var(--border);border-radius:12px;padding:12px;margin-top:12px;overflow:auto}
.science-note{border-left:3px solid var(--blue);padding:10px 12px;background:rgba(105,167,255,.07);border-radius:0 10px 10px 0;color:#c8d9ef;font-size:12px;margin-top:14px}
.vpd-badge{display:inline-flex;padding:5px 9px;border-radius:999px;background:var(--surface2);font-size:12px;color:var(--muted)}
.drop-zone{border:1.5px dashed #3a4b57;border-radius:16px;padding:34px 18px;text-align:center;cursor:pointer;color:var(--muted);transition:.2s ease;background:rgba(11,16,20,.35)}.drop-zone:hover,.drop-zone.drag{border-color:var(--accent);background:var(--accent-soft);color:var(--text)}
.progress{height:8px;background:#0b1115;border-radius:999px;overflow:hidden;margin-top:12px}.progress-bar{height:100%;width:0;background:linear-gradient(90deg,var(--accent),#75ecc3);transition:width .18s ease}
.notice{padding:12px 14px;border-radius:12px;background:var(--surface);border:1px solid var(--border);font-size:12px;color:var(--muted)}
.mobile-nav{display:none}.toast-stack{position:fixed;right:18px;top:18px;z-index:1000;display:grid;gap:10px;max-width:min(390px,calc(100vw - 28px));pointer-events:none}.toast{pointer-events:auto;background:#162129;border:1px solid var(--border);border-radius:14px;box-shadow:0 18px 48px rgba(0,0,0,.42);padding:12px 14px;display:grid;grid-template-columns:9px 1fr auto;gap:10px;align-items:start;animation:toastIn .28s cubic-bezier(.2,.8,.2,1) both}.toast.out{animation:toastOut .22s ease forwards}.toast-mark{width:9px;height:9px;border-radius:50%;background:var(--accent);margin-top:5px}.toast.error .toast-mark{background:var(--danger)}.toast.warn .toast-mark{background:var(--warn)}.toast-title{font-weight:700;font-size:13px}.toast-text{font-size:12px;color:var(--muted);margin-top:2px}.toast-close{background:transparent;border:0;color:var(--muted);cursor:pointer;padding:0 0 0 8px;font-size:18px}
@keyframes toastIn{from{opacity:0;transform:translateX(18px) scale(.98)}to{opacity:1;transform:none}}@keyframes toastOut{to{opacity:0;transform:translateX(18px) scale(.98)}}
.modal-backdrop{position:fixed;inset:0;background:rgba(0,0,0,.58);backdrop-filter:blur(5px);z-index:900;display:grid;place-items:center;padding:20px;opacity:0;pointer-events:none;transition:.2s}.modal-backdrop.open{opacity:1;pointer-events:auto}.modal{width:min(430px,100%);background:var(--surface2);border:1px solid var(--border);border-radius:18px;box-shadow:0 30px 80px rgba(0,0,0,.55);padding:20px;transform:translateY(8px) scale(.98);transition:.2s}.modal-backdrop.open .modal{transform:none}.modal h3{margin:0 0 7px;font-size:18px}.modal p{margin:0;color:var(--muted);font-size:13px}.modal-actions{display:flex;justify-content:flex-end;gap:9px;margin-top:20px}
.skeleton{position:relative;overflow:hidden;background:var(--surface2)!important;color:transparent!important;border-radius:7px}.skeleton:after{content:"";position:absolute;inset:0;transform:translateX(-100%);background:linear-gradient(90deg,transparent,rgba(255,255,255,.05),transparent);animation:shimmer 1.2s infinite}@keyframes shimmer{100%{transform:translateX(100%)}}
@media(max-width:980px){.grid.metrics{grid-template-columns:repeat(2,1fr)}.grid.two{grid-template-columns:1fr}.pump-card{grid-template-columns:1fr 1fr}.main{padding:20px}}
@media(max-width:760px){body{padding-bottom:76px}.app{display:block}.sidebar{display:none}.main{padding:16px 14px 28px}.topbar{margin-bottom:14px}.page-title{font-size:21px}.page-subtitle{display:none}.connection{padding:7px 9px}.grid.metrics{grid-template-columns:1fr 1fr;gap:10px}.card{padding:15px;border-radius:16px}.metric{min-height:92px}.metric-value{font-size:21px}.pump-card{grid-template-columns:1fr;gap:2px}.pump-visual{min-height:236px}.pump-ring{width:198px;height:198px}.pump-button{width:156px;height:156px}.mobile-nav{position:fixed;left:0;right:0;bottom:0;display:grid;grid-template-columns:repeat(5,1fr);padding:7px max(6px,env(safe-area-inset-right)) calc(7px + env(safe-area-inset-bottom)) max(6px,env(safe-area-inset-left));background:rgba(15,22,27,.93);border-top:1px solid var(--border);backdrop-filter:blur(18px);z-index:50}.mobile-nav .nav-btn{display:grid;place-items:center;gap:1px;padding:5px 2px;font-size:10px;border-radius:9px}.mobile-nav .nav-icon{font-size:17px;line-height:1}.schedule-summary{grid-template-columns:repeat(3,1fr);gap:8px}.mini-stat{padding:11px}.mini-stat b{font-size:17px}.slot{grid-template-columns:34px minmax(100px,1fr) minmax(82px,.75fr) 34px;gap:6px;padding:8px}.form-grid,.form-grid.three,.result-grid{grid-template-columns:1fr 1fr}.toast-stack{top:12px;right:14px;left:14px;max-width:none}.toolbar .btn{flex:1}}
@media(max-width:430px){.grid.metrics{grid-template-columns:1fr 1fr}.metric-note{display:none}.schedule-summary{grid-template-columns:1fr}.slot{grid-template-columns:1fr 1fr 34px}.slot-index{display:none}.slot .field:nth-child(2){grid-column:1}.slot .field:nth-child(3){grid-column:2}.slot-del{grid-column:3}.form-grid,.form-grid.three,.result-grid{grid-template-columns:1fr}.mobile-nav .nav-btn{font-size:9px}}
@media(prefers-reduced-motion:reduce){*,*:before,*:after{animation-duration:.001ms!important;animation-iteration-count:1!important;transition-duration:.001ms!important;scroll-behavior:auto!important}}
</style>
</head>
<body>
<div class="app">
  <aside class="sidebar">
    <div class="brand"><div class="brand-mark">H</div><div>HydroESP-C3</div></div>
    <nav class="nav">
      <button class="nav-btn active" data-tab="status"><span class="nav-icon">◉</span>Обзор</button>
      <button class="nav-btn" data-tab="schedule"><span class="nav-icon">≡</span>Расписание</button>
      <button class="nav-btn" data-tab="calculator"><span class="nav-icon">∑</span>Расчёт</button>
      <button class="nav-btn" data-tab="settings"><span class="nav-icon">⚙</span>Настройки</button>
      <button class="nav-btn" data-tab="ota"><span class="nav-icon">⇧</span>Прошивка</button>
    </nav>
    <div class="sidebar-foot">Локальное управление · без облака</div>
  </aside>

  <main class="main">
    <header class="topbar">
      <div><div class="page-title" id="page-title">Обзор</div><div class="page-subtitle" id="page-subtitle">Состояние установки и ручное управление</div></div>
      <div class="connection" id="connection"><span class="dot"></span><span id="connection-text">Подключение…</span></div>
    </header>

    <div class="content">
      <section class="tab active" id="tab-status">
        <div class="grid metrics">
          <div class="card metric"><span class="metric-label">Время устройства</span><strong class="metric-value skeleton" id="stat-time">--:--:--</strong><span class="metric-note" id="stat-date">--.--.----</span></div>
          <div class="card metric"><span class="metric-label">Следующий полив</span><strong class="metric-value skeleton" id="stat-next">--:--</strong><span class="metric-note">по сохранённому графику</span></div>
          <div class="card metric"><span class="metric-label">Wi‑Fi</span><strong class="metric-value skeleton" id="stat-rssi">-- dBm</strong><span class="metric-note" id="stat-ssid">—</span></div>
          <div class="card metric"><span class="metric-label">Работает без перезапуска</span><strong class="metric-value skeleton" id="stat-uptime">—</strong><span class="metric-note" id="stat-ip">IP —</span></div>
        </div>

        <div class="grid two" style="margin-top:16px">
          <div class="card pump-card">
            <div class="pump-visual">
              <div class="pump-ring" id="pump-ring">
                <button class="pump-button" id="pump-button" aria-label="Запустить полив">
                  <span class="pump-icon">💧</span><span class="pump-label" id="pump-label">ЗАПУСТИТЬ</span><span class="pump-remain" id="pump-remain">ручной цикл</span>
                </button>
              </div>
            </div>
            <div class="control-stack">
              <div><h2 class="section-title">Насос</h2><p class="section-note">Запуск ограничен таймером. Повторное нажатие останавливает цикл.</p></div>
              <div><label>Длительность ручного запуска</label><div class="control-row" id="duration-chips">
                <button class="duration-chip" data-duration="30">30 с</button><button class="duration-chip active" data-duration="60">1 мин</button><button class="duration-chip" data-duration="120">2 мин</button><button class="duration-chip" data-duration="180">3 мин</button>
              </div></div>
              <div class="info-row"><span>Состояние</span><b id="pump-state">Выключен</b></div>
              <div class="info-row"><span>Следующий цикл</span><b id="pump-next">--:--</b></div>
            </div>
          </div>

          <div class="card">
            <div class="card-head"><div><h2 class="section-title">Связь</h2><p class="section-note">Локальная сеть и режим контроллера</p></div></div>
            <div class="info-row"><span>Сеть</span><b id="info-ssid">—</b></div>
            <div class="info-row"><span>Сигнал</span><b id="info-rssi">—</b></div>
            <div class="info-row"><span>Адрес</span><b id="info-ip">—</b></div>
            <div class="info-row"><span>Режим</span><b id="info-mode">—</b></div>
            <div class="notice" style="margin-top:14px">Если связь пропадёт, расписание продолжит работать локально. Для запуска по расписанию требуется синхронизированное время.</div>
          </div>
        </div>
      </section>

      <section class="tab" id="tab-schedule">
        <div class="card">
          <div class="card-head">
            <div><h2 class="section-title">Расписание полива</h2><p class="section-note">До 48 точек в сутки. Одинаковое время у двух циклов запрещено.</p></div>
            <div class="toolbar"><button class="btn btn-small" id="btn-add-slot">+ Цикл</button><button class="btn btn-small btn-ghost" id="btn-reset-schedule">Исходный график</button><button class="btn btn-small btn-primary" id="btn-save-schedule">Сохранить</button></div>
          </div>
          <div class="grid schedule-summary">
            <div class="mini-stat"><b id="sum-count">0</b><span>циклов / сутки</span></div>
            <div class="mini-stat"><b id="sum-runtime">0 мин</b><span>работа насоса</span></div>
            <div class="mini-stat"><b id="sum-duty">0%</b><span>доля времени</span></div>
          </div>
          <div class="schedule-list" id="schedule-list"><div class="empty">Загрузка расписания…</div></div>
        </div>
      </section>

      <section class="tab" id="tab-calculator">
        <div class="grid two">
          <div class="card">
            <div class="card-head"><div><h2 class="section-title">Гидравлический расчёт</h2><p class="section-note">Расчёт длительности цикла по измеренному расходу насоса и целевой подаче.</p></div></div>
            <div class="form-grid three">
              <div class="field"><label>Расход насоса, л/мин</label><input class="input calc" id="calc-flow" type="number" min="0.05" step="0.05" value="2.0"></div>
              <div class="field"><label>Эффективная подача, %</label><input class="input calc" id="calc-eff" type="number" min="10" max="100" step="1" value="85"></div>
              <div class="field"><label>Количество растений</label><input class="input calc" id="calc-plants" type="number" min="1" max="500" step="1" value="20"></div>
              <div class="field"><label>Подача на растение за цикл, мл</label><input class="input calc" id="calc-dose" type="number" min="1" step="1" value="50"></div>
              <div class="field"><label>Начало светового периода</label><input class="input calc" id="calc-start" type="time" value="06:00"></div>
              <div class="field"><label>Конец светового периода</label><input class="input calc" id="calc-end" type="time" value="20:00"></div>
              <div class="field"><label>Интервал, мин</label><input class="input calc" id="calc-interval" type="number" min="5" max="720" step="5" value="30"></div>
              <div class="field"><label>Температура воздуха, °C</label><input class="input calc" id="calc-temp" type="number" min="5" max="45" step="0.1" value="24"></div>
              <div class="field"><label>Относительная влажность, %</label><input class="input calc" id="calc-rh" type="number" min="10" max="100" step="1" value="65"></div>
            </div>
            <div class="calc-result">
              <div class="result-grid">
                <div class="result-item"><b id="res-duration">—</b><span>длительность цикла</span></div>
                <div class="result-item"><b id="res-cycles">—</b><span>циклов за период</span></div>
                <div class="result-item"><b id="res-volume">—</b><span>расчётная подача / сутки</span></div>
              </div>
              <div class="formula" id="calc-formula">t = V / (Q × η)</div>
              <div style="display:flex;justify-content:space-between;gap:10px;align-items:center;margin-top:12px"><span class="vpd-badge" id="vpd-badge">VPD —</span><button class="btn btn-primary" id="btn-generate-schedule">Создать график</button></div>
            </div>
            <div class="science-note">Этот блок считает гидравлику, а не «потребность растения по таймеру». VPD показан как индикатор атмосферного спроса и не меняет график автоматически: без датчиков радиации/освещённости, влажности корневой зоны и фактического дренажа такая автокоррекция была бы ложной точностью.</div>
          </div>

          <div class="card">
            <h2 class="section-title">Как сделать управление действительно адаптивным</h2><p class="section-note">Следующий уровень — закрытая обратная связь, а не ещё один фиксированный пресет.</p>
            <div class="info-row"><span>1. Микроклимат</span><b>SHT31 / SHT4x</b></div>
            <div class="info-row"><span>2. Свет</span><b>PAR/PPFD или радиация</b></div>
            <div class="info-row"><span>3. Раствор</span><b>температура, уровень</b></div>
            <div class="info-row"><span>4. Питание</span><b>EC + pH</b></div>
            <div class="info-row"><span>5. Корневая зона</span><b>влага/масса/дренаж</b></div>
            <div class="notice" style="margin-top:14px">После добавления сенсоров алгоритм можно строить по транспирационному спросу (радиация + VPD) с ограничениями по влажности субстрата/корневой зоны и аварийными порогами.</div>
          </div>
        </div>
      </section>

      <section class="tab" id="tab-settings">
        <div class="grid two">
          <div class="card">
            <div class="card-head"><div><h2 class="section-title">Сеть и время</h2><p class="section-note">Пароль устройства больше не выгружается обратно в браузер.</p></div></div>
            <div class="form-grid">
              <div class="field"><label>Wi‑Fi SSID</label><input class="input" id="cfg-ssid" maxlength="32" autocomplete="off"></div>
              <div class="field"><label>Новый пароль Wi‑Fi</label><input class="input" id="cfg-pass" type="password" maxlength="63" autocomplete="new-password" placeholder="Оставьте пустым, чтобы не менять"></div>
              <div class="field"><label>Часовой пояс UTC</label><input class="input" id="cfg-tz" type="number" min="-12" max="14" step="1"></div>
            </div>
            <div class="toolbar" style="margin-top:16px"><button class="btn btn-primary" id="btn-save-config">Сохранить и перезагрузить</button></div>
          </div>
          <div class="card">
            <h2 class="section-title">Система</h2><p class="section-note">Сервисные действия контроллера.</p>
            <div class="info-row"><span>mDNS</span><b>hydro.local</b></div>
            <div class="info-row"><span>Веб-интерфейс</span><b>локальный HTTP</b></div>
            <button class="btn btn-danger btn-block" id="btn-reboot" style="margin-top:18px">Перезагрузить контроллер</button>
          </div>
        </div>
      </section>

      <section class="tab" id="tab-ota">
        <div class="card" style="max-width:760px">
          <div class="card-head"><div><h2 class="section-title">Обновление прошивки</h2><p class="section-note">Файл PlatformIO `.bin`. Не выключайте питание во время записи.</p></div></div>
          <div class="drop-zone" id="drop-zone" tabindex="0"><div style="font-size:32px;margin-bottom:8px">⇧</div><b style="color:var(--text)">Перетащите .bin сюда</b><div style="margin-top:4px">или нажмите, чтобы выбрать файл</div><input id="ota-file" type="file" accept=".bin,application/octet-stream" hidden></div>
          <div id="ota-progress" class="hidden" style="margin-top:16px"><div style="display:flex;justify-content:space-between;gap:12px"><span id="ota-text">Подготовка…</span><b id="ota-percent">0%</b></div><div class="progress"><div class="progress-bar" id="ota-bar"></div></div></div>
          <div class="notice" style="margin-top:16px">OTA сейчас доступна в локальной сети без аутентификации и криптографической проверки подписи. Для эксплуатации в недоверенной сети это нужно закрыть отдельным механизмом доступа/подписанной прошивкой.</div>
        </div>
      </section>
    </div>
  </main>
</div>

<nav class="mobile-nav">
  <button class="nav-btn active" data-tab="status"><span class="nav-icon">◉</span>Обзор</button>
  <button class="nav-btn" data-tab="schedule"><span class="nav-icon">≡</span>График</button>
  <button class="nav-btn" data-tab="calculator"><span class="nav-icon">∑</span>Расчёт</button>
  <button class="nav-btn" data-tab="settings"><span class="nav-icon">⚙</span>Настр.</button>
  <button class="nav-btn" data-tab="ota"><span class="nav-icon">⇧</span>Прошивка</button>
</nav>

<div class="toast-stack" id="toast-stack" aria-live="polite"></div>
<div class="modal-backdrop" id="modal-backdrop" aria-hidden="true">
  <div class="modal" role="dialog" aria-modal="true" aria-labelledby="modal-title"><h3 id="modal-title">Подтверждение</h3><p id="modal-text"></p><div class="modal-actions"><button class="btn" id="modal-cancel">Отмена</button><button class="btn btn-primary" id="modal-ok">Продолжить</button></div></div>
</div>

<script>
const $=s=>document.querySelector(s), $$=s=>[...document.querySelectorAll(s)];
const MAX_SLOTS=48;
let manualDuration=60, lastStatus=null, scheduleLoaded=false, modalResolve=null;
const tabMeta={
  status:['Обзор','Состояние установки и ручное управление'],
  schedule:['Расписание','Редактирование циклов полива'],
  calculator:['Расчёт','Гидравлика цикла и инженерные оценки'],
  settings:['Настройки','Сеть, время и системные действия'],
  ota:['Прошивка','Локальное OTA-обновление контроллера']
};

function toast(title,text='',type='ok',timeout=3600){
  const el=document.createElement('div'); el.className='toast '+(type==='ok'?'':type);
  el.innerHTML=`<span class="toast-mark"></span><div><div class="toast-title"></div><div class="toast-text"></div></div><button class="toast-close" aria-label="Закрыть">×</button>`;
  el.querySelector('.toast-title').textContent=title; el.querySelector('.toast-text').textContent=text;
  const close=()=>{if(el.classList.contains('out'))return;el.classList.add('out');setTimeout(()=>el.remove(),230)};
  el.querySelector('.toast-close').onclick=close; $('#toast-stack').appendChild(el); if(timeout)setTimeout(close,timeout);
}
function ask(title,text,ok='Продолжить',danger=false){
  $('#modal-title').textContent=title; $('#modal-text').textContent=text; $('#modal-ok').textContent=ok;
  $('#modal-ok').className='btn '+(danger?'btn-danger':'btn-primary'); $('#modal-backdrop').classList.add('open'); $('#modal-backdrop').setAttribute('aria-hidden','false');
  return new Promise(r=>modalResolve=r);
}
function closeModal(value){$('#modal-backdrop').classList.remove('open');$('#modal-backdrop').setAttribute('aria-hidden','true');if(modalResolve){modalResolve(value);modalResolve=null}}
$('#modal-cancel').onclick=()=>closeModal(false); $('#modal-ok').onclick=()=>closeModal(true); $('#modal-backdrop').onclick=e=>{if(e.target.id==='modal-backdrop')closeModal(false)};

async function api(url,opts={}){
  const ctl=new AbortController(), timer=setTimeout(()=>ctl.abort(),5000);
  try{
    const res=await fetch(url,{cache:'no-store',...opts,signal:ctl.signal});
    if(!res.ok){let msg='HTTP '+res.status;try{const j=await res.json();msg=j.error||msg}catch(_){ }throw new Error(msg)}
    const type=res.headers.get('content-type')||''; return type.includes('json')?await res.json():await res.text();
  }finally{clearTimeout(timer)}
}
function setConnection(online){const c=$('#connection');c.classList.toggle('online',online);c.classList.toggle('offline',!online);$('#connection-text').textContent=online?'Онлайн':'Нет связи'}
function fmtUptime(s){s=Math.max(0,Number(s)||0);const d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60);return d?`${d} д ${h} ч`:h?`${h} ч ${m} мин`:`${m} мин`}
function rssiText(v){if(!Number.isFinite(+v))return '—';const n=+v, q=n>=-55?'отлично':n>=-67?'хорошо':n>=-75?'средне':'слабо';return `${n} dBm · ${q}`}
function unskeleton(){ $$('.skeleton').forEach(x=>x.classList.remove('skeleton')) }

async function updateStatus(){
  try{
    const d=await api('/api/status'); lastStatus=d; setConnection(true); unskeleton();
    $('#stat-time').textContent=d.time||'--:--:--'; $('#stat-date').textContent=d.date||'—'; $('#stat-next').textContent=d.next||'--:--';
    $('#stat-rssi').textContent=Number.isFinite(+d.rssi)?`${d.rssi} dBm`:'—'; $('#stat-ssid').textContent=d.ssid||'—'; $('#stat-uptime').textContent=fmtUptime(d.uptime); $('#stat-ip').textContent='IP '+(d.ip||'—');
    $('#info-ssid').textContent=d.ssid||'—'; $('#info-rssi').textContent=rssiText(d.rssi); $('#info-ip').textContent=d.ip||'—'; $('#info-mode').textContent=d.ap_mode?'Точка доступа':'Wi‑Fi клиент';
    $('#pump-next').textContent=d.next||'--:--';
    const on=!!d.relay, btn=$('#pump-button'), ring=$('#pump-ring'); btn.classList.toggle('on',on); btn.setAttribute('aria-label',on?'Остановить полив':'Запустить полив');
    $('#pump-label').textContent=on?'ПОЛИВ ИДЁТ':'ЗАПУСТИТЬ'; $('#pump-state').textContent=on?'Насос включён':'Выключен';
    const rem=Math.max(0,+d.relay_remaining||0), prog=Math.max(0,Math.min(1,+d.relay_progress||0));
    $('#pump-remain').textContent=on?(rem?`${rem} с осталось`:'завершение…'):'ручной цикл'; ring.style.setProperty('--p',on?Math.round(prog*100):0);
  }catch(e){setConnection(false)}
}

function switchTab(id){
  $$('.tab').forEach(t=>t.classList.toggle('active',t.id==='tab-'+id)); $$('.nav-btn[data-tab]').forEach(b=>b.classList.toggle('active',b.dataset.tab===id));
  $('#page-title').textContent=tabMeta[id][0]; $('#page-subtitle').textContent=tabMeta[id][1];
  if(id==='schedule'&&!scheduleLoaded)loadSchedule(); if(id==='settings')loadConfig(); if(id==='calculator')calcHydraulics();
  window.scrollTo({top:0,behavior:'smooth'});
}
$$('.nav-btn[data-tab]').forEach(b=>b.onclick=()=>switchTab(b.dataset.tab));

$$('.duration-chip').forEach(b=>b.onclick=()=>{$$('.duration-chip').forEach(x=>x.classList.remove('active'));b.classList.add('active');manualDuration=+b.dataset.duration});
$('#pump-button').onclick=async()=>{
  const on=!!lastStatus?.relay; $('#pump-button').disabled=true;
  try{if(on){await api('/api/relay/off',{method:'POST'});toast('Полив остановлен','Насос выключен.')}else{await api(`/api/relay/on?duration=${manualDuration}`,{method:'POST'});toast('Полив запущен',`Таймер: ${manualDuration} с.`)}await updateStatus()}catch(e){toast('Не удалось управлять насосом',e.message,'error',5000)}finally{$('#pump-button').disabled=false}
};

function slotRow(h=8,m=0,d=60){
  const row=document.createElement('div');row.className='slot';
  const t=`${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}`;
  row.innerHTML=`<div class="slot-index"></div><div class="field"><label>Время</label><input class="input slot-time" type="time"></div><div class="field"><label>Длительность, с</label><input class="input slot-duration" type="number" min="1" max="3600" step="1"></div><button class="slot-del" title="Удалить" aria-label="Удалить">×</button>`;
  row.querySelector('.slot-time').value=t;row.querySelector('.slot-duration').value=d;
  row.querySelector('.slot-del').onclick=()=>{row.classList.add('removing');setTimeout(()=>{row.remove();renumberSlots();updateScheduleSummary()},220)};
  row.querySelectorAll('input').forEach(i=>i.addEventListener('input',updateScheduleSummary));return row;
}
function renumberSlots(){ $$('#schedule-list .slot').forEach((r,i)=>r.querySelector('.slot-index').textContent=String(i+1).padStart(2,'0')) }
function updateScheduleSummary(){
  const rows=$$('#schedule-list .slot'), sec=rows.reduce((a,r)=>a+(+r.querySelector('.slot-duration').value||0),0); $('#sum-count').textContent=rows.length; $('#sum-runtime').textContent=sec<60?`${sec} с`:`${(sec/60).toFixed(sec%60?1:0)} мин`; $('#sum-duty').textContent=(sec/864).toFixed(1)+'%';renumberSlots();
}
async function loadSchedule(){
  try{const data=await api('/api/schedule');const box=$('#schedule-list');box.innerHTML='';data.sort((a,b)=>(a.h*60+a.m)-(b.h*60+b.m));data.forEach(s=>box.appendChild(slotRow(s.h,s.m,s.d)));if(!data.length)box.innerHTML='<div class="empty">Расписание пусто. Добавьте первый цикл.</div>';scheduleLoaded=true;updateScheduleSummary()}catch(e){$('#schedule-list').innerHTML='<div class="empty">Не удалось загрузить расписание.</div>';toast('Ошибка расписания',e.message,'error',5000)}
}
function addSlot(h=8,m=0,d=60){const box=$('#schedule-list');if($$('.slot').length>=MAX_SLOTS){toast('Достигнут лимит',`Максимум ${MAX_SLOTS} циклов.`,'warn');return}if(!box.querySelector('.slot'))box.innerHTML='';box.appendChild(slotRow(h,m,d));updateScheduleSummary();box.lastElementChild.scrollIntoView({behavior:'smooth',block:'nearest'})}
$('#btn-add-slot').onclick=()=>addSlot();
function collectSchedule(){
  const rows=$$('#schedule-list .slot'); if(rows.length>MAX_SLOTS)throw new Error(`Максимум ${MAX_SLOTS} циклов`); const seen=new Set(),slots=[];let bad=false;
  rows.forEach(r=>{const ti=r.querySelector('.slot-time'),di=r.querySelector('.slot-duration');ti.classList.remove('invalid');di.classList.remove('invalid');const p=ti.value.split(':'),d=+di.value;if(p.length!==2){ti.classList.add('invalid');bad=true;return}const h=+p[0],m=+p[1],key=`${h}:${m}`;if(seen.has(key)){ti.classList.add('invalid');bad=true}seen.add(key);if(!Number.isFinite(d)||d<1||d>3600){di.classList.add('invalid');bad=true}slots.push({h,m,d:Math.round(d)})});
  if(bad)throw new Error('Проверьте время, дубли и длительность 1–3600 с');return slots.sort((a,b)=>(a.h*60+a.m)-(b.h*60+b.m));
}
$('#btn-save-schedule').onclick=async()=>{try{const slots=collectSchedule(),btn=$('#btn-save-schedule');btn.disabled=true;await api('/api/schedule',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(slots)});toast('Расписание сохранено',`${slots.length} циклов записано в память.`);await loadSchedule();btn.disabled=false}catch(e){toast('Не удалось сохранить',e.message,'error',5000);$('#btn-save-schedule').disabled=false}};
$('#btn-reset-schedule').onclick=async()=>{if(!await ask('Вернуть исходный график?','Текущие изменения расписания будут удалены.','Вернуть',true))return;try{await api('/api/schedule/reset',{method:'POST'});toast('График восстановлен','Загружены исходные значения.');await loadSchedule()}catch(e){toast('Сброс не выполнен',e.message,'error',5000)}};

function timeToMin(v){const p=(v||'').split(':');return p.length===2?(+p[0]*60+ +p[1]):NaN}
function calcHydraulics(){
  const flow=+$('#calc-flow').value,eff=+$('#calc-eff').value/100,plants=+$('#calc-plants').value,dose=+$('#calc-dose').value,interval=+$('#calc-interval').value,start=timeToMin($('#calc-start').value),end=timeToMin($('#calc-end').value),temp=+$('#calc-temp').value,rh=+$('#calc-rh').value;
  const valid=flow>0&&eff>0&&eff<=1&&plants>0&&dose>0&&interval>=5&&end>start; if(!valid){$('#res-duration').textContent='—';$('#res-cycles').textContent='—';$('#res-volume').textContent='—';$('#calc-formula').textContent='Проверьте исходные данные.';return null}
  const eventLitres=plants*dose/1000, duration=60*eventLitres/(flow*eff), cycles=Math.floor((end-start-1)/interval)+1, daily=eventLitres*cycles;
  $('#res-duration').textContent=duration<60?`${duration.toFixed(1)} с`:`${(duration/60).toFixed(2)} мин`;$('#res-cycles').textContent=cycles;$('#res-volume').textContent=`${daily.toFixed(1)} л`;
  $('#calc-formula').textContent=`Vцикла = ${plants} × ${dose} мл = ${eventLitres.toFixed(3)} л\nt = Vцикла / (${flow.toFixed(2)} л/мин × ${(eff*100).toFixed(0)}%) = ${duration.toFixed(1)} с`;
  if(Number.isFinite(temp)&&Number.isFinite(rh)&&rh>0&&rh<=100){const es=.6108*Math.exp(17.27*temp/(temp+237.3)),vpd=es*(1-rh/100);$('#vpd-badge').textContent=`VPD ${vpd.toFixed(2)} кПа`;$('#vpd-badge').style.color=vpd<.4||vpd>1.8?'var(--warn)':'var(--accent)'}else $('#vpd-badge').textContent='VPD —';
  try{localStorage.setItem('hydro_calc',JSON.stringify({flow,eff:eff*100,plants,dose,interval,start:$('#calc-start').value,end:$('#calc-end').value,temp,rh}))}catch(_){ }
  return{duration,cycles,start,end,interval,daily};
}
$$('.calc').forEach(i=>i.addEventListener('input',calcHydraulics));
$('#btn-generate-schedule').onclick=async()=>{const c=calcHydraulics();if(!c){toast('Не хватает данных','Проверьте расход, подачу, интервал и световой период.','warn');return}const dur=Math.max(1,Math.min(3600,Math.round(c.duration))),slots=[];for(let t=c.start;t<c.end&&slots.length<MAX_SLOTS;t+=c.interval)slots.push({h:Math.floor(t/60),m:t%60,d:dur});if(!slots.length)return;if(slots.length>=MAX_SLOTS&&c.start+c.interval*MAX_SLOTS<c.end)toast('График ограничен',`Созданы первые ${MAX_SLOTS} циклов.`,'warn',5000);switchTab('schedule');const box=$('#schedule-list');box.innerHTML='';slots.forEach(s=>box.appendChild(slotRow(s.h,s.m,s.d)));updateScheduleSummary();toast('График рассчитан','Проверьте его и нажмите «Сохранить».')};

async function loadConfig(){try{const d=await api('/api/config');$('#cfg-ssid').value=d.ssid||'';$('#cfg-tz').value=Number.isFinite(+d.tz)?d.tz:3;$('#cfg-pass').value='';$('#cfg-pass').placeholder=d.has_pass?'Пароль сохранён — оставьте пустым':'Введите пароль сети'}catch(e){toast('Настройки не загружены',e.message,'error',5000)}}
$('#btn-save-config').onclick=async()=>{const ssid=$('#cfg-ssid').value.trim(),pass=$('#cfg-pass').value,tz=+$('#cfg-tz').value;if(!ssid||ssid.length>32){toast('Проверьте SSID','Название сети должно содержать 1–32 символа.','warn');return}if(pass.length>63||tz<-12||tz>14){toast('Проверьте настройки','Пароль до 63 символов, UTC от −12 до +14.','warn');return}if(!await ask('Применить сетевые настройки?','Контроллер сохранит параметры и перезагрузится. Текущее соединение временно прервётся.','Сохранить'))return;try{await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,pass,tz})});toast('Настройки сохранены','Контроллер перезагружается.', 'ok',6000);setConnection(false)}catch(e){toast('Не удалось сохранить',e.message,'error',5000)}};
$('#btn-reboot').onclick=async()=>{if(!await ask('Перезагрузить контроллер?','Полив, запущенный вручную, будет остановлен. Расписание останется в памяти.','Перезагрузить',true))return;try{await api('/api/reboot',{method:'POST'});toast('Перезагрузка','Соединение восстановится после запуска контроллера.');setConnection(false)}catch(e){toast('Команда не отправлена',e.message,'error',5000)}};

const dz=$('#drop-zone'),fi=$('#ota-file');dz.onclick=()=>fi.click();dz.onkeydown=e=>{if(e.key==='Enter'||e.key===' '){e.preventDefault();fi.click()}};dz.ondragover=e=>{e.preventDefault();dz.classList.add('drag')};dz.ondragleave=()=>dz.classList.remove('drag');dz.ondrop=e=>{e.preventDefault();dz.classList.remove('drag');if(e.dataTransfer.files[0])uploadFirmware(e.dataTransfer.files[0])};fi.onchange=()=>{if(fi.files[0])uploadFirmware(fi.files[0])};
function uploadFirmware(file){
  if(!file.name.toLowerCase().endsWith('.bin')){toast('Неверный файл','Нужен бинарный файл прошивки .bin.','warn');return}if(file.size<10000){toast('Файл выглядит подозрительно маленьким',`${Math.round(file.size/1024)} КБ. Проверьте сборку.`,'warn',5000);return}
  const fd=new FormData();fd.append('file',file);const xhr=new XMLHttpRequest();xhr.open('POST','/ota/upload',true);$('#ota-progress').classList.remove('hidden');$('#ota-text').textContent=file.name;$('#ota-bar').style.width='0%';$('#ota-percent').textContent='0%';
  xhr.upload.onprogress=e=>{if(e.lengthComputable){const p=Math.round(e.loaded/e.total*100);$('#ota-bar').style.width=p+'%';$('#ota-percent').textContent=p+'%'}};
  xhr.onload=()=>{if(xhr.status===200&&xhr.responseText.trim()==='OK'){toast('Прошивка записана','Контроллер перезагружается.','ok',6000);$('#ota-text').textContent='Готово. Перезагрузка…';setConnection(false)}else{toast('Ошибка OTA',xhr.responseText||`HTTP ${xhr.status}`,'error',6000);$('#ota-text').textContent='Ошибка обновления'}};
  xhr.onerror=()=>{toast('Соединение прервано','Не удалось завершить загрузку прошивки.','error',6000);$('#ota-text').textContent='Ошибка соединения'};xhr.send(fd);
}

try{const s=JSON.parse(localStorage.getItem('hydro_calc')||'null');if(s){$('#calc-flow').value=s.flow??2;$('#calc-eff').value=s.eff??85;$('#calc-plants').value=s.plants??20;$('#calc-dose').value=s.dose??50;$('#calc-interval').value=s.interval??30;$('#calc-start').value=s.start||'06:00';$('#calc-end').value=s.end||'20:00';$('#calc-temp').value=s.temp??24;$('#calc-rh').value=s.rh??65}}catch(_){ }
calcHydraulics(); updateStatus(); setInterval(updateStatus,2000);
</script>
</body>
</html>
)rawliteral";
