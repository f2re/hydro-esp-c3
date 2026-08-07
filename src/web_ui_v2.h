#pragma once
#include <Arduino.h>

const char WEB_UI_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0b1014" id="theme-color-meta">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="apple-mobile-web-app-title" content="HydroESP">
<link rel="manifest" href="/manifest.webmanifest">
<title>HydroESP-C3</title>
<style>
:root{
  color-scheme:dark;
  --bg:#0a0f13;--bg-glow:rgba(50,211,151,.10);--surface:#11191f;--surface2:#172229;--surface3:#1e2a32;
  --text:#f4f8f8;--muted:#91a2ab;--border:#283740;--border-strong:#3a4d58;
  --accent:#36d399;--accent2:#1db77c;--accent-text:#042217;--accent-soft:rgba(54,211,153,.13);
  --blue:#70aaff;--blue-soft:rgba(112,170,255,.10);--warn:#f5c45b;--warn-soft:rgba(245,196,91,.10);
  --danger:#ff747a;--danger-soft:rgba(255,116,122,.11);--shadow:0 18px 55px rgba(0,0,0,.28);
  --radius:18px;--radius-sm:12px;--sidebar:218px;
}
:root[data-theme="light"]{
  color-scheme:light;
  --bg:#f3f6f5;--bg-glow:rgba(26,155,108,.10);--surface:#ffffff;--surface2:#f6f9f8;--surface3:#eaf0ee;
  --text:#14201d;--muted:#66756f;--border:#dbe4e1;--border-strong:#bdcbc6;
  --accent:#159b6c;--accent2:#0e8159;--accent-text:#ffffff;--accent-soft:rgba(21,155,108,.10);
  --blue:#397bd5;--blue-soft:rgba(57,123,213,.08);--warn:#a96f08;--warn-soft:rgba(196,131,14,.09);
  --danger:#d8464e;--danger-soft:rgba(216,70,78,.09);--shadow:0 14px 40px rgba(29,55,46,.08);
}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html{background:var(--bg);scroll-behavior:smooth}
body{margin:0;min-height:100vh;background:radial-gradient(circle at 14% -10%,var(--bg-glow),transparent 28rem),var(--bg);color:var(--text);font:15px/1.45 -apple-system,BlinkMacSystemFont,"Segoe UI",Inter,Roboto,Arial,sans-serif;transition:background .25s,color .25s}
button,input,select{font:inherit}button{touch-action:manipulation}button,input,select{color:inherit}
button:focus-visible,input:focus-visible,select:focus-visible,.drop-zone:focus-visible{outline:2px solid var(--accent);outline-offset:2px}
.hidden{display:none!important}.app{min-height:100vh;display:grid;grid-template-columns:var(--sidebar) minmax(0,1fr)}
.sidebar{position:sticky;top:0;height:100vh;padding:22px 14px;border-right:1px solid var(--border);background:color-mix(in srgb,var(--bg) 86%,transparent);backdrop-filter:blur(20px);z-index:20}
.brand{display:flex;align-items:center;gap:11px;padding:0 10px 22px;font-weight:780;letter-spacing:-.025em}.brand-mark{width:35px;height:35px;border-radius:11px;display:grid;place-items:center;background:linear-gradient(145deg,#48e2aa,#16996b);color:#052218;box-shadow:0 8px 24px rgba(54,211,153,.20)}
.nav{display:grid;gap:6px}.nav-btn{border:0;background:transparent;color:var(--muted);width:100%;min-height:44px;padding:10px 12px;border-radius:12px;display:flex;align-items:center;gap:11px;text-align:left;cursor:pointer;transition:.18s ease}.nav-btn:hover{background:var(--surface2);color:var(--text)}.nav-btn.active{background:var(--accent-soft);color:var(--accent)}.nav-icon{width:23px;text-align:center;font-size:17px}
.sidebar-foot{position:absolute;left:14px;right:14px;bottom:18px;padding:0 10px;color:var(--muted);font-size:11px}.sidebar-version{display:block;color:var(--text);margin-bottom:2px}
.main{min-width:0;padding:22px 28px 52px}.topbar{max-width:1180px;margin:0 auto 20px;display:flex;justify-content:space-between;align-items:center;gap:16px}.page-title{font-size:24px;font-weight:780;letter-spacing:-.035em}.page-subtitle{color:var(--muted);font-size:13px;margin-top:2px}.top-actions{display:flex;align-items:center;gap:9px}.icon-btn{width:38px;height:38px;border-radius:12px;border:1px solid var(--border);background:var(--surface);color:var(--muted);display:grid;place-items:center;cursor:pointer;transition:.18s}.icon-btn:hover{color:var(--text);border-color:var(--border-strong)}
.connection{display:flex;align-items:center;gap:8px;background:var(--surface);border:1px solid var(--border);border-radius:999px;padding:8px 11px;color:var(--muted);font-size:12px;white-space:nowrap}.dot{width:8px;height:8px;border-radius:50%;background:var(--muted);box-shadow:0 0 0 4px color-mix(in srgb,var(--muted) 10%,transparent)}.connection.online .dot{background:var(--accent);box-shadow:0 0 0 4px var(--accent-soft)}.connection.offline .dot{background:var(--danger);box-shadow:0 0 0 4px var(--danger-soft)}
.content{max-width:1180px;margin:auto}.tab{display:none}.tab.active{display:block;animation:tabIn .22s ease both}@keyframes tabIn{from{opacity:0;transform:translateY(5px)}to{opacity:1;transform:none}}
.grid{display:grid;gap:16px}.grid.metrics{grid-template-columns:repeat(4,minmax(0,1fr))}.grid.two{grid-template-columns:minmax(0,1.45fr) minmax(310px,.85fr)}
.card{background:linear-gradient(180deg,color-mix(in srgb,var(--surface2) 82%,var(--surface)),var(--surface));border:1px solid var(--border);border-radius:var(--radius);box-shadow:var(--shadow);padding:18px}.metric{min-height:108px;display:flex;flex-direction:column;justify-content:space-between}.metric-label{font-size:12px;color:var(--muted)}.metric-value{font-size:25px;font-weight:780;letter-spacing:-.035em}.metric-note{font-size:12px;color:var(--muted);overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.section-title{font-size:18px;font-weight:740;letter-spacing:-.022em;margin:0}.section-note{font-size:13px;color:var(--muted);margin:4px 0 0}.card-head{display:flex;align-items:flex-start;justify-content:space-between;gap:12px;margin-bottom:16px}
.setup-banner{display:none;margin-bottom:16px;border:1px solid rgba(112,170,255,.28);background:linear-gradient(135deg,var(--blue-soft),var(--surface));border-radius:16px;padding:15px 16px;align-items:center;justify-content:space-between;gap:16px}.setup-banner.show{display:flex;animation:tabIn .25s ease}.setup-banner b{display:block}.setup-banner p{margin:3px 0 0;color:var(--muted);font-size:12px}
.pump-card{min-height:360px;display:grid;grid-template-columns:minmax(230px,.82fr) minmax(290px,1.18fr);align-items:center;gap:24px;overflow:hidden}.pump-visual{display:grid;place-items:center;min-height:260px}.pump-ring{--ring:0;position:relative;width:220px;height:220px;border-radius:50%;display:grid;place-items:center;background:conic-gradient(var(--accent) calc(var(--ring)*1%),var(--surface3) 0);transition:background .18s}.pump-ring:before{content:"";position:absolute;inset:10px;border-radius:50%;background:var(--surface);border:1px solid var(--border)}
.pump-button{position:relative;z-index:2;width:174px;height:174px;border-radius:50%;border:0;background:linear-gradient(145deg,var(--surface3),var(--surface));color:var(--text);cursor:pointer;box-shadow:inset 0 0 0 1px var(--border),0 16px 36px rgba(0,0,0,.18);transition:transform .16s,box-shadow .24s,background .25s}.pump-button:hover{transform:translateY(-2px)}.pump-button:active,.pump-button.holding{transform:scale(.985)}.pump-button.on{background:linear-gradient(145deg,#39d89a,#12845c);color:#041b12;box-shadow:0 0 0 10px var(--accent-soft),0 18px 44px rgba(54,211,153,.22)}.pump-button.on:after{content:"";position:absolute;inset:-12px;border:1px solid rgba(54,211,153,.30);border-radius:50%;animation:ripple 2s ease-out infinite}@keyframes ripple{0%{transform:scale(.92);opacity:.7}100%{transform:scale(1.16);opacity:0}}
.pump-icon{width:38px;height:38px;margin:auto;display:block}.pump-label{display:block;font-weight:820;margin-top:8px;font-size:14px;letter-spacing:.03em}.pump-remain{display:block;font-size:12px;opacity:.72;margin-top:4px}.control-stack{display:grid;gap:14px}.control-row{display:flex;gap:8px;flex-wrap:wrap}.duration-chip{border:1px solid var(--border);background:var(--surface2);color:var(--muted);padding:9px 11px;border-radius:10px;cursor:pointer;transition:.18s;min-width:58px}.duration-chip:hover{border-color:var(--border-strong);color:var(--text)}.duration-chip.active{border-color:color-mix(in srgb,var(--accent) 55%,var(--border));background:var(--accent-soft);color:var(--accent)}
.info-row{display:flex;justify-content:space-between;gap:16px;padding:11px 0;border-bottom:1px solid color-mix(in srgb,var(--border) 78%,transparent)}.info-row:last-child{border:0}.info-row span:first-child{color:var(--muted)}.info-row b{text-align:right;overflow-wrap:anywhere}
.toolbar{display:flex;gap:9px;align-items:center;flex-wrap:wrap}.btn{border:1px solid var(--border);background:var(--surface2);color:var(--text);border-radius:11px;min-height:42px;padding:9px 14px;cursor:pointer;font-weight:670;transition:.16s;display:inline-flex;align-items:center;justify-content:center;gap:7px}.btn:hover{transform:translateY(-1px);border-color:var(--border-strong)}.btn:active{transform:none}.btn:disabled{opacity:.42;cursor:not-allowed;transform:none}.btn-primary{background:var(--accent);border-color:var(--accent);color:var(--accent-text)}.btn-danger{color:var(--danger);border-color:color-mix(in srgb,var(--danger) 28%,var(--border));background:var(--danger-soft)}.btn-ghost{background:transparent}.btn-small{min-height:36px;padding:7px 10px;font-size:13px}.btn-block{width:100%}
.notice{padding:12px 14px;border-radius:12px;background:var(--surface);border:1px solid var(--border);font-size:12px;color:var(--muted)}.notice.warn{background:var(--warn-soft);border-color:color-mix(in srgb,var(--warn) 25%,var(--border));color:var(--text)}
.badge{display:inline-flex;align-items:center;gap:6px;padding:5px 9px;border-radius:999px;background:var(--surface2);color:var(--muted);font-size:11px}.badge.good{background:var(--accent-soft);color:var(--accent)}.badge.warn{background:var(--warn-soft);color:var(--warn)}
.schedule-summary{grid-template-columns:repeat(3,1fr);margin-bottom:14px}.mini-stat{padding:13px 15px;background:var(--surface);border:1px solid var(--border);border-radius:14px}.mini-stat b{font-size:19px;display:block}.mini-stat span{color:var(--muted);font-size:11px}.dirty-line{min-height:22px;display:flex;align-items:center;gap:7px;color:var(--warn);font-size:12px;margin-bottom:8px;opacity:0;transition:.2s}.dirty-line.show{opacity:1}
.timeline{padding:14px 8px 10px;margin-bottom:14px;background:var(--surface);border:1px solid var(--border);border-radius:14px}.timeline-track{position:relative;height:34px;margin:0 8px;border-radius:999px;background:linear-gradient(90deg,var(--surface3),color-mix(in srgb,var(--blue) 9%,var(--surface3)),var(--surface3));overflow:visible}.timeline-mark{position:absolute;top:50%;width:8px;height:20px;border-radius:5px;background:var(--accent);transform:translate(-50%,-50%);box-shadow:0 0 0 3px var(--accent-soft)}.timeline-axis{display:flex;justify-content:space-between;color:var(--muted);font-size:10px;margin-top:7px;padding:0 4px}
.schedule-list{display:grid;gap:9px}.slot{display:grid;grid-template-columns:54px minmax(120px,1fr) minmax(105px,.72fr) 40px;gap:10px;align-items:center;background:var(--surface);border:1px solid var(--border);padding:10px;border-radius:13px;animation:slotIn .18s ease both;transition:.20s}@keyframes slotIn{from{opacity:0;transform:translateY(4px)}to{opacity:1;transform:none}}.slot.removing{opacity:0;transform:translateX(14px)}.slot-index{color:var(--muted);font-size:11px;text-align:center}.slot-del{width:38px;height:38px;border-radius:10px;border:0;background:transparent;color:var(--muted);cursor:pointer;font-size:20px}.slot-del:hover{background:var(--danger-soft);color:var(--danger)}.empty{padding:34px 16px;text-align:center;color:var(--muted);border:1px dashed var(--border);border-radius:14px}
label{display:block;color:var(--muted);font-size:12px;margin:0 0 6px}.field{min-width:0}.input{width:100%;height:42px;border:1px solid var(--border);border-radius:10px;background:color-mix(in srgb,var(--bg) 62%,var(--surface));color:var(--text);padding:0 11px;transition:.16s}.input:hover{border-color:var(--border-strong)}.input:focus{border-color:var(--accent);box-shadow:0 0 0 3px var(--accent-soft);outline:0}.input.invalid{border-color:var(--danger);box-shadow:0 0 0 3px var(--danger-soft)}.input-row{display:flex;gap:8px}.input-row .input{flex:1}
.form-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:13px}.form-grid.three{grid-template-columns:repeat(3,minmax(0,1fr))}.form-section{padding-top:16px;margin-top:16px;border-top:1px solid var(--border)}
.calc-result{background:linear-gradient(145deg,var(--accent-soft),var(--blue-soft));border:1px solid color-mix(in srgb,var(--accent) 24%,var(--border));border-radius:16px;padding:16px;margin-top:16px}.result-grid{display:grid;grid-template-columns:repeat(3,1fr);gap:10px}.result-item{padding:10px;border-radius:11px;background:color-mix(in srgb,var(--surface) 70%,transparent)}.result-item b{font-size:18px;display:block}.result-item span{font-size:11px;color:var(--muted)}.formula{font:12px/1.55 ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;color:color-mix(in srgb,var(--text) 82%,var(--accent));background:color-mix(in srgb,var(--bg) 72%,var(--surface));border:1px solid var(--border);border-radius:12px;padding:12px;margin-top:12px;overflow:auto;white-space:pre-wrap}.science-note{border-left:3px solid var(--blue);padding:10px 12px;background:var(--blue-soft);border-radius:0 10px 10px 0;color:var(--text);font-size:12px;margin-top:14px}
.drop-zone{border:1.5px dashed var(--border-strong);border-radius:16px;padding:34px 18px;text-align:center;cursor:pointer;color:var(--muted);transition:.18s;background:color-mix(in srgb,var(--surface) 70%,transparent)}.drop-zone:hover,.drop-zone.drag{border-color:var(--accent);background:var(--accent-soft);color:var(--text)}.progress{height:8px;background:var(--surface3);border-radius:999px;overflow:hidden;margin-top:12px}.progress-bar{height:100%;width:0;background:linear-gradient(90deg,var(--accent),#77eac2);transition:width .16s}
.mobile-nav{display:none}.toast-stack{position:fixed;right:18px;top:18px;z-index:1000;display:grid;gap:10px;max-width:min(390px,calc(100vw - 28px));pointer-events:none}.toast{pointer-events:auto;background:var(--surface2);border:1px solid var(--border);border-radius:14px;box-shadow:0 18px 48px rgba(0,0,0,.28);padding:12px 14px;display:grid;grid-template-columns:9px 1fr auto;gap:10px;align-items:start;animation:toastIn .25s cubic-bezier(.2,.8,.2,1) both}.toast.out{animation:toastOut .20s ease forwards}.toast-mark{width:9px;height:9px;border-radius:50%;background:var(--accent);margin-top:5px}.toast.error .toast-mark{background:var(--danger)}.toast.warn .toast-mark{background:var(--warn)}.toast-title{font-weight:720;font-size:13px}.toast-text{font-size:12px;color:var(--muted);margin-top:2px}.toast-close{background:transparent;border:0;color:var(--muted);cursor:pointer;padding:0 0 0 8px;font-size:18px}@keyframes toastIn{from{opacity:0;transform:translateX(16px) scale(.985)}to{opacity:1;transform:none}}@keyframes toastOut{to{opacity:0;transform:translateX(16px) scale(.985)}}
.modal-backdrop{position:fixed;inset:0;background:rgba(0,0,0,.52);backdrop-filter:blur(5px);z-index:900;display:grid;place-items:center;padding:20px;opacity:0;pointer-events:none;transition:.18s}.modal-backdrop.open{opacity:1;pointer-events:auto}.modal{width:min(430px,100%);background:var(--surface2);border:1px solid var(--border);border-radius:18px;box-shadow:0 30px 80px rgba(0,0,0,.45);padding:20px;transform:translateY(8px) scale(.985);transition:.18s}.modal-backdrop.open .modal{transform:none}.modal h3{margin:0 0 7px;font-size:18px}.modal p{margin:0;color:var(--muted);font-size:13px}.modal-actions{display:flex;justify-content:flex-end;gap:9px;margin-top:20px}
.skeleton{position:relative;overflow:hidden;background:var(--surface2)!important;color:transparent!important;border-radius:7px}.skeleton:after{content:"";position:absolute;inset:0;transform:translateX(-100%);background:linear-gradient(90deg,transparent,color-mix(in srgb,var(--text) 5%,transparent),transparent);animation:shimmer 1.15s infinite}@keyframes shimmer{100%{transform:translateX(100%)}}
@media(max-width:1000px){.grid.metrics{grid-template-columns:repeat(2,1fr)}.grid.two{grid-template-columns:1fr}.pump-card{grid-template-columns:1fr 1fr}.main{padding:20px}}
@media(max-width:760px){body{padding-bottom:calc(76px + env(safe-area-inset-bottom))}.app{display:block}.sidebar{display:none}.main{padding:16px 14px 28px}.topbar{margin-bottom:14px}.page-title{font-size:21px}.page-subtitle{display:none}.connection{padding:7px 9px}.grid.metrics{grid-template-columns:1fr 1fr;gap:10px}.card{padding:15px;border-radius:16px}.metric{min-height:92px}.metric-value{font-size:21px}.pump-card{grid-template-columns:1fr;gap:2px}.pump-visual{min-height:236px}.pump-ring{width:198px;height:198px}.pump-button{width:156px;height:156px}.setup-banner.show{align-items:flex-start;flex-direction:column}.mobile-nav{position:fixed;left:0;right:0;bottom:0;display:grid;grid-template-columns:repeat(5,1fr);padding:7px max(6px,env(safe-area-inset-right)) calc(7px + env(safe-area-inset-bottom)) max(6px,env(safe-area-inset-left));background:color-mix(in srgb,var(--bg) 91%,transparent);border-top:1px solid var(--border);backdrop-filter:blur(18px);z-index:50}.mobile-nav .nav-btn{display:grid;place-items:center;gap:1px;padding:5px 2px;font-size:10px;border-radius:9px}.mobile-nav .nav-icon{font-size:17px;line-height:1}.schedule-summary{grid-template-columns:repeat(3,1fr);gap:8px}.mini-stat{padding:11px}.mini-stat b{font-size:17px}.slot{grid-template-columns:34px minmax(100px,1fr) minmax(82px,.75fr) 34px;gap:6px;padding:8px}.form-grid,.form-grid.three,.result-grid{grid-template-columns:1fr 1fr}.toast-stack{top:12px;right:14px;left:14px;max-width:none}.toolbar .btn{flex:1}.timeline{display:none}}
@media(max-width:430px){.metric-note{display:none}.schedule-summary{grid-template-columns:1fr}.slot{grid-template-columns:1fr 1fr 34px}.slot-index{display:none}.slot .field:nth-child(2){grid-column:1}.slot .field:nth-child(3){grid-column:2}.slot-del{grid-column:3}.form-grid,.form-grid.three,.result-grid{grid-template-columns:1fr}.mobile-nav .nav-btn{font-size:9px}.top-actions{gap:6px}.icon-btn{width:36px;height:36px}.connection{font-size:11px}}
@media(prefers-reduced-motion:reduce){*,*:before,*:after{animation-duration:.001ms!important;animation-iteration-count:1!important;transition-duration:.001ms!important;scroll-behavior:auto!important}}
</style>
</head>
<body>
<div class="app">
  <aside class="sidebar">
    <div class="brand"><div class="brand-mark">H</div><div>HydroESP-C3</div></div>
    <nav class="nav" aria-label="Разделы">
      <button class="nav-btn active" data-tab="status"><span class="nav-icon">◉</span>Обзор</button>
      <button class="nav-btn" data-tab="schedule"><span class="nav-icon">≡</span>Расписание</button>
      <button class="nav-btn" data-tab="calculator"><span class="nav-icon">∑</span>Расчёт</button>
      <button class="nav-btn" data-tab="settings"><span class="nav-icon">⚙</span>Система</button>
      <button class="nav-btn" data-tab="ota"><span class="nav-icon">⇧</span>Прошивка</button>
    </nav>
    <div class="sidebar-foot"><span class="sidebar-version" id="side-version">HydroESP-C3</span><span id="side-build">локально · без облака</span></div>
  </aside>

  <main class="main">
    <header class="topbar">
      <div><div class="page-title" id="page-title">Обзор</div><div class="page-subtitle" id="page-subtitle">Состояние установки и безопасное ручное управление</div></div>
      <div class="top-actions">
        <button class="icon-btn" id="theme-btn" title="Тема: автоматически" aria-label="Переключить тему">◐</button>
        <div class="connection" id="connection" aria-live="polite"><span class="dot"></span><span id="connection-text">Подключение…</span></div>
      </div>
    </header>

    <div class="content">
      <div class="setup-banner" id="setup-banner">
        <div><b>Контроллер в режиме первичной настройки</b><p>Подключите домашний Wi‑Fi. После сохранения устройство перезагрузится и станет доступно по hydro.local.</p></div>
        <button class="btn btn-primary" id="setup-open">Настроить сеть</button>
      </div>

      <section class="tab active" id="tab-status">
        <div class="grid metrics">
          <div class="card metric"><span class="metric-label">Время устройства</span><strong class="metric-value skeleton" id="stat-time">--:--:--</strong><span class="metric-note" id="stat-date">--.--.----</span></div>
          <div class="card metric"><span class="metric-label">Следующий полив</span><strong class="metric-value skeleton" id="stat-next">--:--</strong><span class="metric-note" id="stat-sync">ожидание времени</span></div>
          <div class="card metric"><span class="metric-label">Wi‑Fi</span><strong class="metric-value skeleton" id="stat-rssi">-- dBm</strong><span class="metric-note" id="stat-ssid">—</span></div>
          <div class="card metric"><span class="metric-label">Работает без перезапуска</span><strong class="metric-value skeleton" id="stat-uptime">—</strong><span class="metric-note" id="stat-ip">IP —</span></div>
        </div>

        <div class="grid two" style="margin-top:16px">
          <div class="card pump-card">
            <div class="pump-visual">
              <div class="pump-ring" id="pump-ring">
                <button class="pump-button" id="pump-button" aria-label="Удерживайте, чтобы запустить полив">
                  <svg class="pump-icon" viewBox="0 0 24 24" aria-hidden="true"><path fill="currentColor" d="M12 2.3S5.2 10 5.2 14.6A6.8 6.8 0 0 0 12 21.4a6.8 6.8 0 0 0 6.8-6.8C18.8 10 12 2.3 12 2.3Zm0 16.5a4.2 4.2 0 0 1-4.1-3.3c1 .7 2.3 1.1 3.7 1.1 2 0 3.8-.8 5-2.1v.1a4.6 4.6 0 0 1-4.6 4.2Z"/></svg>
                  <span class="pump-label" id="pump-label">УДЕРЖИВАЙТЕ</span><span class="pump-remain" id="pump-remain">0,7 с для запуска</span>
                </button>
              </div>
            </div>
            <div class="control-stack">
              <div><h2 class="section-title">Насос</h2><p class="section-note">Для запуска удерживайте кнопку. Остановка — одним нажатием.</p></div>
              <div><label>Длительность ручного цикла</label><div class="control-row" id="duration-chips">
                <button class="duration-chip" data-duration="30">30 с</button><button class="duration-chip active" data-duration="60">1 мин</button><button class="duration-chip" data-duration="120">2 мин</button><button class="duration-chip" data-duration="180">3 мин</button>
              </div></div>
              <div class="info-row"><span>Состояние</span><b id="pump-state">Выключен</b></div>
              <div class="info-row"><span>Следующий цикл</span><b id="pump-next">--:--</b></div>
              <div class="info-row"><span>Время</span><b id="pump-sync">не синхронизировано</b></div>
            </div>
          </div>

          <div class="card">
            <div class="card-head"><div><h2 class="section-title">Связь</h2><p class="section-note">Локальная сеть и состояние контроллера</p></div><span class="badge" id="net-badge">—</span></div>
            <div class="info-row"><span>Сеть</span><b id="info-ssid">—</b></div>
            <div class="info-row"><span>Сигнал</span><b id="info-rssi">—</b></div>
            <div class="info-row"><span>Адрес</span><b id="info-ip">—</b></div>
            <div class="info-row"><span>Режим</span><b id="info-mode">—</b></div>
            <div class="info-row"><span>Прошивка</span><b id="info-version">—</b></div>
            <div class="notice" id="time-notice" style="margin-top:14px">Расписание работает только при синхронизированном времени. Ручное управление остаётся доступно.</div>
          </div>
        </div>
      </section>

      <section class="tab" id="tab-schedule">
        <div class="card">
          <div class="card-head">
            <div><h2 class="section-title">Расписание полива</h2><p class="section-note">До 48 циклов в сутки. Дубли времени и неверные длительности подсвечиваются до сохранения.</p></div>
            <div class="toolbar"><button class="btn btn-small" id="btn-add-slot">+ Цикл</button><button class="btn btn-small btn-ghost" id="btn-reset-schedule">Исходный</button><button class="btn btn-small btn-primary" id="btn-save-schedule" disabled>Сохранить</button></div>
          </div>
          <div class="dirty-line" id="schedule-dirty"><span>●</span><span>Есть несохранённые изменения</span></div>
          <div class="grid schedule-summary">
            <div class="mini-stat"><b id="sum-count">0</b><span>циклов / сутки</span></div>
            <div class="mini-stat"><b id="sum-runtime">0 мин</b><span>работа насоса</span></div>
            <div class="mini-stat"><b id="sum-duty">0%</b><span>доля суток</span></div>
          </div>
          <div class="timeline"><div class="timeline-track" id="timeline-track"></div><div class="timeline-axis"><span>00</span><span>06</span><span>12</span><span>18</span><span>24</span></div></div>
          <div class="schedule-list" id="schedule-list"><div class="empty">Загрузка расписания…</div></div>
        </div>
      </section>

      <section class="tab" id="tab-calculator">
        <div class="grid two">
          <div class="card">
            <div class="card-head"><div><h2 class="section-title">Инженерный расчёт</h2><p class="section-note">Сначала считаем измеряемую гидравлику. Атмосферный спрос показывается отдельно и не подменяет датчики.</p></div></div>
            <div class="form-grid three">
              <div class="field"><label>Фактический расход, л/мин</label><input class="input calc" id="calc-flow" type="number" min="0.05" step="0.05" value="2.0"></div>
              <div class="field"><label>Эффективная доставка, %</label><input class="input calc" id="calc-eff" type="number" min="10" max="100" step="1" value="85"></div>
              <div class="field"><label>Количество растений</label><input class="input calc" id="calc-plants" type="number" min="1" max="500" step="1" value="20"></div>
              <div class="field"><label>Подача на растение / цикл, мл</label><input class="input calc" id="calc-dose" type="number" min="1" step="1" value="50"></div>
              <div class="field"><label>Начало периода</label><input class="input calc" id="calc-start" type="time" value="06:00"></div>
              <div class="field"><label>Конец периода</label><input class="input calc" id="calc-end" type="time" value="20:00"></div>
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
              <div class="toolbar" style="justify-content:space-between;margin-top:12px"><span class="badge" id="vpd-badge">VPD —</span><button class="btn btn-primary" id="btn-generate-schedule">Создать черновик графика</button></div>
            </div>
            <div class="science-note">VPD — диагностический показатель атмосферного спроса. Он намеренно не меняет длительность автоматически: без света/радиации, фактического расхода, состояния корневой зоны и дренажа это создало бы ложную точность.</div>
          </div>

          <div class="card">
            <h2 class="section-title">Путь к адаптивному режиму</h2><p class="section-note">Таймер остаётся fallback, а решение о поливе принимает измеряемая обратная связь.</p>
            <div class="info-row"><span>Микроклимат</span><b>T/RH → VPD</b></div>
            <div class="info-row"><span>Свет</span><b>PAR/PPFD или радиация</b></div>
            <div class="info-row"><span>Гидравлика</span><b>уровень + расход</b></div>
            <div class="info-row"><span>Корневая зона</span><b>влага/масса/дренаж</b></div>
            <div class="info-row"><span>Раствор</span><b>T, EC, pH</b></div>
            <div class="notice" style="margin-top:14px">Будущий adaptive-контур должен объяснять каждый запуск: накопленный спрос, ограничение субстрата, аварийное правило или ручная команда.</div>
          </div>
        </div>
      </section>

      <section class="tab" id="tab-settings">
        <div class="grid two">
          <div class="card">
            <div class="card-head"><div><h2 class="section-title">Сеть и время</h2><p class="section-note">Пароль никогда не выгружается обратно из контроллера.</p></div></div>
            <div class="form-grid">
              <div class="field"><label>Wi‑Fi SSID</label><input class="input" id="cfg-ssid" maxlength="32" autocomplete="off"></div>
              <div class="field"><label>Новый пароль Wi‑Fi</label><div class="input-row"><input class="input" id="cfg-pass" type="password" maxlength="63" autocomplete="new-password" placeholder="Оставьте пустым, чтобы не менять"><button class="icon-btn" id="pass-toggle" type="button" aria-label="Показать пароль">◉</button></div></div>
              <div class="field"><label>Часовой пояс UTC</label><div class="input-row"><input class="input" id="cfg-tz" type="number" min="-12" max="14" step="1"><button class="btn btn-small" id="tz-browser" type="button">Из браузера</button></div></div>
            </div>
            <div class="toolbar" style="margin-top:16px"><button class="btn btn-primary" id="btn-save-config">Сохранить и перезагрузить</button></div>
          </div>

          <div class="card">
            <div class="card-head"><div><h2 class="section-title">Диагностика</h2><p class="section-note">Ключевые показатели без подключения Serial.</p></div><button class="btn btn-small" id="btn-refresh-diag">Обновить</button></div>
            <div class="info-row"><span>Прошивка</span><b id="diag-version">—</b></div>
            <div class="info-row"><span>Свободно RAM</span><b id="diag-heap">—</b></div>
            <div class="info-row"><span>Минимум RAM</span><b id="diag-minheap">—</b></div>
            <div class="info-row"><span>Размер flash</span><b id="diag-flash">—</b></div>
            <div class="info-row"><span>Размер firmware</span><b id="diag-sketch">—</b></div>
            <div class="info-row"><span>Причина рестарта</span><b id="diag-reset">—</b></div>
          </div>

          <div class="card">
            <h2 class="section-title">Резервная копия</h2><p class="section-note">Экспортирует расписание и безопасные настройки. Пароль Wi‑Fi в backup не попадает.</p>
            <div class="toolbar" style="margin-top:16px"><button class="btn" id="btn-export">Экспорт JSON</button><button class="btn" id="btn-import">Импорт расписания</button><input id="import-file" type="file" accept="application/json,.json" hidden></div>
          </div>

          <div class="card">
            <h2 class="section-title">Сервис</h2><p class="section-note">Перезапуск не удаляет расписание и настройки из NVS.</p>
            <div class="info-row"><span>mDNS</span><b>hydro.local</b></div>
            <div class="info-row"><span>API</span><b id="diag-api">v—</b></div>
            <button class="btn btn-danger btn-block" id="btn-reboot" style="margin-top:18px">Перезагрузить контроллер</button>
          </div>
        </div>
      </section>

      <section class="tab" id="tab-ota">
        <div class="card" style="max-width:780px">
          <div class="card-head"><div><h2 class="section-title">Обновление прошивки</h2><p class="section-note">Локальный OTA из файла PlatformIO `.bin`. Питание во время записи отключать нельзя.</p></div><span class="badge" id="ota-current">текущая —</span></div>
          <div class="drop-zone" id="drop-zone" tabindex="0" role="button" aria-label="Выбрать файл прошивки"><div style="font-size:32px;margin-bottom:8px">⇧</div><b style="color:var(--text)">Перетащите .bin сюда</b><div style="margin-top:4px">или нажмите, чтобы выбрать файл</div><input id="ota-file" type="file" accept=".bin,application/octet-stream" hidden></div>
          <div id="ota-progress" class="hidden" style="margin-top:16px"><div style="display:flex;justify-content:space-between;gap:12px"><span id="ota-text">Подготовка…</span><b id="ota-percent">0%</b></div><div class="progress"><div class="progress-bar" id="ota-bar"></div></div></div>
          <div class="notice warn" style="margin-top:16px">Текущий OTA рассчитан на доверенную локальную сеть. Подписанная прошивка и web-auth вынесены в следующий security-этап.</div>
        </div>
      </section>
    </div>
  </main>
</div>

<nav class="mobile-nav" aria-label="Разделы">
  <button class="nav-btn active" data-tab="status"><span class="nav-icon">◉</span>Обзор</button>
  <button class="nav-btn" data-tab="schedule"><span class="nav-icon">≡</span>График</button>
  <button class="nav-btn" data-tab="calculator"><span class="nav-icon">∑</span>Расчёт</button>
  <button class="nav-btn" data-tab="settings"><span class="nav-icon">⚙</span>Система</button>
  <button class="nav-btn" data-tab="ota"><span class="nav-icon">⇧</span>Прошивка</button>
</nav>

<div class="toast-stack" id="toast-stack" aria-live="polite"></div>
<div class="modal-backdrop" id="modal-backdrop" aria-hidden="true">
  <div class="modal" role="dialog" aria-modal="true" aria-labelledby="modal-title"><h3 id="modal-title">Подтверждение</h3><p id="modal-text"></p><div class="modal-actions"><button class="btn" id="modal-cancel">Отмена</button><button class="btn btn-primary" id="modal-ok">Продолжить</button></div></div>
</div>

<script>
const $=s=>document.querySelector(s), $$=s=>[...document.querySelectorAll(s)];
const MAX_SLOTS=48, HOLD_MS=700;
let manualDuration=60,lastStatus=null,scheduleLoaded=false,scheduleDirty=false,modalResolve=null,holdRAF=0,holdStart=0,holdPointer=null;
const tabMeta={status:['Обзор','Состояние установки и безопасное ручное управление'],schedule:['Расписание','Циклы полива и суточная нагрузка насоса'],calculator:['Расчёт','Гидравлика цикла и инженерные оценки'],settings:['Система','Сеть, диагностика и резервное копирование'],ota:['Прошивка','Локальное OTA-обновление контроллера']};

function toast(title,text='',type='ok',timeout=3600){const el=document.createElement('div');el.className='toast '+(type==='ok'?'':type);el.innerHTML='<span class="toast-mark"></span><div><div class="toast-title"></div><div class="toast-text"></div></div><button class="toast-close" aria-label="Закрыть">×</button>';el.querySelector('.toast-title').textContent=title;el.querySelector('.toast-text').textContent=text;const close=()=>{if(el.classList.contains('out'))return;el.classList.add('out');setTimeout(()=>el.remove(),210)};el.querySelector('.toast-close').onclick=close;$('#toast-stack').appendChild(el);if(timeout)setTimeout(close,timeout)}
function ask(title,text,ok='Продолжить',danger=false){$('#modal-title').textContent=title;$('#modal-text').textContent=text;$('#modal-ok').textContent=ok;$('#modal-ok').className='btn '+(danger?'btn-danger':'btn-primary');$('#modal-backdrop').classList.add('open');$('#modal-backdrop').setAttribute('aria-hidden','false');setTimeout(()=>$('#modal-cancel').focus(),20);return new Promise(r=>modalResolve=r)}
function closeModal(value){$('#modal-backdrop').classList.remove('open');$('#modal-backdrop').setAttribute('aria-hidden','true');if(modalResolve){modalResolve(value);modalResolve=null}}
$('#modal-cancel').onclick=()=>closeModal(false);$('#modal-ok').onclick=()=>closeModal(true);$('#modal-backdrop').onclick=e=>{if(e.target.id==='modal-backdrop')closeModal(false)};document.addEventListener('keydown',e=>{if(e.key==='Escape'&&$('#modal-backdrop').classList.contains('open'))closeModal(false)});

async function api(url,opts={}){const ctl=new AbortController(),timer=setTimeout(()=>ctl.abort(),6000);try{const res=await fetch(url,{cache:'no-store',...opts,signal:ctl.signal});if(!res.ok){let msg='HTTP '+res.status;try{const j=await res.json();msg=j.error||msg}catch(_){ }throw new Error(msg)}const type=res.headers.get('content-type')||'';return type.includes('json')?await res.json():await res.text()}finally{clearTimeout(timer)}}
function setConnection(online){const c=$('#connection');c.classList.toggle('online',online);c.classList.toggle('offline',!online);$('#connection-text').textContent=online?'Онлайн':'Нет связи'}
function fmtUptime(s){s=Math.max(0,Number(s)||0);const d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60);return d?`${d} д ${h} ч`:h?`${h} ч ${m} мин`:`${m} мин`}
function bytes(v){v=Number(v)||0;if(v>=1048576)return `${(v/1048576).toFixed(1)} МБ`;if(v>=1024)return `${Math.round(v/1024)} КБ`;return `${v} Б`}
function rssiText(v){if(!Number.isFinite(+v))return '—';const n=+v,q=n>=-55?'отлично':n>=-67?'хорошо':n>=-75?'средне':'слабо';return `${n} dBm · ${q}`}
function unskeleton(){$$('.skeleton').forEach(x=>x.classList.remove('skeleton'))}

function applyTheme(mode){localStorage.setItem('hydro_theme',mode);let resolved=mode;if(mode==='auto')resolved=matchMedia('(prefers-color-scheme: light)').matches?'light':'dark';document.documentElement.dataset.theme=resolved==='light'?'light':'';$('#theme-color-meta').content=resolved==='light'?'#f3f6f5':'#0a0f13';const icon=mode==='auto'?'◐':mode==='light'?'☀':'☾';$('#theme-btn').textContent=icon;$('#theme-btn').title='Тема: '+(mode==='auto'?'автоматически':mode==='light'?'светлая':'тёмная');$('#theme-btn').dataset.mode=mode}
$('#theme-btn').onclick=()=>{const now=$('#theme-btn').dataset.mode||'auto',next=now==='auto'?'light':now==='light'?'dark':'auto';applyTheme(next)};matchMedia('(prefers-color-scheme: light)').addEventListener?.('change',()=>{if(($('#theme-btn').dataset.mode||'auto')==='auto')applyTheme('auto')});applyTheme(localStorage.getItem('hydro_theme')||'auto');

async function updateStatus(){try{const d=await api('/api/status');lastStatus=d;setConnection(true);unskeleton();$('#stat-time').textContent=d.time||'--:--:--';$('#stat-date').textContent=d.date||'—';$('#stat-next').textContent=d.next||'--:--';$('#stat-sync').textContent=d.time_synced?'время синхронизировано':'ожидание синхронизации';$('#stat-rssi').textContent=Number.isFinite(+d.rssi)?`${d.rssi} dBm`:'—';$('#stat-ssid').textContent=d.ssid||'—';$('#stat-uptime').textContent=fmtUptime(d.uptime);$('#stat-ip').textContent='IP '+(d.ip||'—');$('#info-ssid').textContent=d.ssid||'—';$('#info-rssi').textContent=rssiText(d.rssi);$('#info-ip').textContent=d.ip||'—';$('#info-mode').textContent=d.ap_mode?'Точка настройки':'Wi‑Fi клиент';$('#info-version').textContent=`${d.version||'dev'} · ${d.build||'?'}`;$('#side-version').textContent=`HydroESP-C3 ${d.version||''}`;$('#side-build').textContent=`${d.build||'local'} · без облака`;$('#ota-current').textContent='текущая '+(d.version||'dev');$('#pump-next').textContent=d.next||'--:--';$('#pump-sync').textContent=d.time_synced?'синхронизировано':'нет синхронизации';$('#time-notice').classList.toggle('warn',!d.time_synced);$('#time-notice').textContent=d.time_synced?'Расписание активно и использует синхронизированное локальное время.':'Расписание приостановлено до синхронизации времени. Ручное управление доступно.';$('#setup-banner').classList.toggle('show',!!d.ap_mode);const badge=$('#net-badge');badge.textContent=d.ap_mode?'настройка':d.time_synced?'готово':'без NTP';badge.className='badge '+(d.ap_mode||!d.time_synced?'warn':'good');const on=!!d.relay,btn=$('#pump-button'),ring=$('#pump-ring');btn.classList.toggle('on',on);const rem=Math.max(0,+d.relay_remaining||0),prog=Math.max(0,Math.min(1,+d.relay_progress||0));if(on){$('#pump-label').textContent='ОСТАНОВИТЬ';$('#pump-remain').textContent=rem?`${rem} с осталось`:'завершение…';$('#pump-state').textContent='Насос включён';ring.style.setProperty('--ring',Math.round(prog*100));btn.setAttribute('aria-label','Остановить полив')}else if(!btn.classList.contains('holding')){$('#pump-label').textContent='УДЕРЖИВАЙТЕ';$('#pump-remain').textContent='0,7 с для запуска';$('#pump-state').textContent='Выключен';ring.style.setProperty('--ring',0);btn.setAttribute('aria-label','Удерживайте, чтобы запустить полив')}}catch(_){setConnection(false)}}

function switchTab(id){$$('.tab').forEach(t=>t.classList.toggle('active',t.id==='tab-'+id));$$('.nav-btn[data-tab]').forEach(b=>b.classList.toggle('active',b.dataset.tab===id));$('#page-title').textContent=tabMeta[id][0];$('#page-subtitle').textContent=tabMeta[id][1];if(id==='schedule'&&!scheduleLoaded)loadSchedule();if(id==='settings'){loadConfig();loadDiagnostics()}if(id==='calculator')calcHydraulics();window.scrollTo({top:0,behavior:'smooth'})}
$$('.nav-btn[data-tab]').forEach(b=>b.onclick=()=>switchTab(b.dataset.tab));$('#setup-open').onclick=()=>switchTab('settings');

$$('.duration-chip').forEach(b=>b.onclick=()=>{$$('.duration-chip').forEach(x=>x.classList.remove('active'));b.classList.add('active');manualDuration=+b.dataset.duration});
async function pumpStart(){const btn=$('#pump-button');if(btn.disabled)return;btn.disabled=true;try{await api(`/api/relay/on?duration=${manualDuration}`,{method:'POST'});navigator.vibrate?.(18);toast('Полив запущен',`Таймер: ${manualDuration} с.`);await updateStatus()}catch(e){toast('Не удалось запустить насос',e.message,'error',5000)}finally{btn.disabled=false}}
async function pumpStop(){const btn=$('#pump-button');if(btn.disabled)return;btn.disabled=true;try{await api('/api/relay/off',{method:'POST'});navigator.vibrate?.(10);toast('Полив остановлен','Насос выключен.');await updateStatus()}catch(e){toast('Не удалось остановить насос',e.message,'error',5000)}finally{btn.disabled=false}}
function cancelHold(){if(holdRAF)cancelAnimationFrame(holdRAF);holdRAF=0;holdStart=0;holdPointer=null;const btn=$('#pump-button');btn.classList.remove('holding');if(!lastStatus?.relay){$('#pump-label').textContent='УДЕРЖИВАЙТЕ';$('#pump-remain').textContent='0,7 с для запуска';$('#pump-ring').style.setProperty('--ring',0)}}
function holdFrame(ts){if(!holdStart)holdStart=ts;const p=Math.min(1,(ts-holdStart)/HOLD_MS);$('#pump-ring').style.setProperty('--ring',Math.round(p*100));$('#pump-label').textContent='ЗАПУСК…';$('#pump-remain').textContent=`${Math.round(p*100)}%`;if(p>=1){cancelHold();pumpStart();return}holdRAF=requestAnimationFrame(holdFrame)}
function beginHold(e){if(lastStatus?.relay){pumpStop();return}if(holdRAF)return;if(e.pointerId!==undefined)holdPointer=e.pointerId;$('#pump-button').classList.add('holding');holdStart=0;holdRAF=requestAnimationFrame(holdFrame)}
const pump=$('#pump-button');pump.addEventListener('pointerdown',e=>{e.preventDefault();pump.setPointerCapture?.(e.pointerId);beginHold(e)});['pointerup','pointercancel','pointerleave'].forEach(name=>pump.addEventListener(name,()=>{if(!lastStatus?.relay)cancelHold()}));pump.addEventListener('keydown',e=>{if((e.key==='Enter'||e.key===' ')&&!e.repeat){e.preventDefault();beginHold(e)}});pump.addEventListener('keyup',e=>{if(e.key==='Enter'||e.key===' ')cancelHold()});

function slotRow(h=8,m=0,d=60){const row=document.createElement('div');row.className='slot';const t=`${String(h).padStart(2,'0')}:${String(m).padStart(2,'0')}`;row.innerHTML='<div class="slot-index"></div><div class="field"><label>Время</label><input class="input slot-time" type="time"></div><div class="field"><label>Длительность, с</label><input class="input slot-duration" type="number" min="1" max="3600" step="1"></div><button class="slot-del" title="Удалить" aria-label="Удалить">×</button>';row.querySelector('.slot-time').value=t;row.querySelector('.slot-duration').value=d;row.querySelector('.slot-del').onclick=()=>{row.classList.add('removing');setTimeout(()=>{row.remove();markScheduleDirty();updateScheduleSummary()},190)};row.querySelectorAll('input').forEach(i=>i.addEventListener('input',()=>{markScheduleDirty();updateScheduleSummary()}));return row}
function renumberSlots(){$$('#schedule-list .slot').forEach((r,i)=>r.querySelector('.slot-index').textContent=String(i+1).padStart(2,'0'))}
function drawTimeline(){const track=$('#timeline-track');track.innerHTML='';$$('#schedule-list .slot').forEach(r=>{const v=r.querySelector('.slot-time').value,p=v.split(':');if(p.length!==2)return;const mins=+p[0]*60 + +p[1];if(!Number.isFinite(mins))return;const mark=document.createElement('span');mark.className='timeline-mark';mark.style.left=(mins/1440*100)+'%';mark.title=`${v} · ${r.querySelector('.slot-duration').value||0} с`;track.appendChild(mark)})}
function updateScheduleSummary(){const rows=$$('#schedule-list .slot'),sec=rows.reduce((a,r)=>a+(+r.querySelector('.slot-duration').value||0),0);$('#sum-count').textContent=rows.length;$('#sum-runtime').textContent=sec<60?`${sec} с`:`${(sec/60).toFixed(sec%60?1:0)} мин`;$('#sum-duty').textContent=(sec/864).toFixed(1)+'%';renumberSlots();drawTimeline()}
function markScheduleDirty(value=true){scheduleDirty=value;$('#schedule-dirty').classList.toggle('show',value);$('#btn-save-schedule').disabled=!value}
async function loadSchedule(){try{const data=await api('/api/schedule'),box=$('#schedule-list');box.innerHTML='';data.sort((a,b)=>(a.h*60+a.m)-(b.h*60+b.m));data.forEach(s=>box.appendChild(slotRow(s.h,s.m,s.d)));if(!data.length)box.innerHTML='<div class="empty">Расписание пусто. Добавьте первый цикл.</div>';scheduleLoaded=true;markScheduleDirty(false);updateScheduleSummary()}catch(e){$('#schedule-list').innerHTML='<div class="empty">Не удалось загрузить расписание.</div>';toast('Ошибка расписания',e.message,'error',5000)}}
function addSlot(){const box=$('#schedule-list'),rows=$$('#schedule-list .slot');if(rows.length>=MAX_SLOTS){toast('Достигнут лимит',`Максимум ${MAX_SLOTS} циклов.`,'warn');return}let h=8,m=0,d=60;if(rows.length){const last=rows[rows.length-1],p=last.querySelector('.slot-time').value.split(':');let mins=(+p[0]*60 + +p[1] + 30)%1440;h=Math.floor(mins/60);m=mins%60;d=+last.querySelector('.slot-duration').value||60}if(!box.querySelector('.slot'))box.innerHTML='';box.appendChild(slotRow(h,m,d));markScheduleDirty();updateScheduleSummary();box.lastElementChild.scrollIntoView({behavior:'smooth',block:'nearest'})}
$('#btn-add-slot').onclick=addSlot;
function collectSchedule(){const rows=$$('#schedule-list .slot');if(rows.length>MAX_SLOTS)throw new Error(`Максимум ${MAX_SLOTS} циклов`);const seen=new Set(),slots=[];let bad=false;rows.forEach(r=>{const ti=r.querySelector('.slot-time'),di=r.querySelector('.slot-duration');ti.classList.remove('invalid');di.classList.remove('invalid');const p=ti.value.split(':'),d=+di.value;if(p.length!==2){ti.classList.add('invalid');bad=true;return}const h=+p[0],m=+p[1],key=`${h}:${m}`;if(seen.has(key)){ti.classList.add('invalid');bad=true}seen.add(key);if(!Number.isFinite(d)||d<1||d>3600){di.classList.add('invalid');bad=true}slots.push({h,m,d:Math.round(d)})});if(bad)throw new Error('Проверьте время, дубли и длительность 1–3600 с');return slots.sort((a,b)=>(a.h*60+a.m)-(b.h*60+b.m))}
$('#btn-save-schedule').onclick=async()=>{try{const slots=collectSchedule(),btn=$('#btn-save-schedule');btn.disabled=true;await api('/api/schedule',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(slots)});toast('Расписание сохранено',`${slots.length} циклов записано в NVS.`);await loadSchedule()}catch(e){toast('Не удалось сохранить',e.message,'error',5000);$('#btn-save-schedule').disabled=!scheduleDirty}};
$('#btn-reset-schedule').onclick=async()=>{if(!await ask('Вернуть исходный график?','Текущий график будет заменён заводским пресетом.','Вернуть',true))return;try{await api('/api/schedule/reset',{method:'POST'});toast('График восстановлен','Загружен исходный пресет.');await loadSchedule()}catch(e){toast('Сброс не выполнен',e.message,'error',5000)}};

function timeToMin(v){const p=(v||'').split(':');return p.length===2?(+p[0]*60 + +p[1]):NaN}
function calcHydraulics(){const flow=+$('#calc-flow').value,eff=+$('#calc-eff').value/100,plants=+$('#calc-plants').value,dose=+$('#calc-dose').value,interval=+$('#calc-interval').value,start=timeToMin($('#calc-start').value),end=timeToMin($('#calc-end').value),temp=+$('#calc-temp').value,rh=+$('#calc-rh').value;const valid=flow>0&&eff>0&&eff<=1&&plants>0&&dose>0&&interval>=5&&end>start;if(!valid){$('#res-duration').textContent='—';$('#res-cycles').textContent='—';$('#res-volume').textContent='—';$('#calc-formula').textContent='Проверьте исходные данные.';return null}const eventLitres=plants*dose/1000,duration=60*eventLitres/(flow*eff),cycles=Math.floor((end-start-1)/interval)+1,daily=eventLitres*cycles;$('#res-duration').textContent=duration<60?`${duration.toFixed(1)} с`:`${(duration/60).toFixed(2)} мин`;$('#res-cycles').textContent=cycles;$('#res-volume').textContent=`${daily.toFixed(1)} л`;$('#calc-formula').textContent=`Vцикла = ${plants} × ${dose} мл = ${eventLitres.toFixed(3)} л\nt = Vцикла / (${flow.toFixed(2)} л/мин × ${(eff*100).toFixed(0)}%) = ${duration.toFixed(1)} с`;if(Number.isFinite(temp)&&Number.isFinite(rh)&&rh>0&&rh<=100){const es=.6108*Math.exp(17.27*temp/(temp+237.3)),vpd=es*(1-rh/100),badge=$('#vpd-badge');badge.textContent=`VPD ${vpd.toFixed(2)} кПа`;badge.className='badge '+(vpd<.4||vpd>1.8?'warn':'good')}else $('#vpd-badge').textContent='VPD —';try{localStorage.setItem('hydro_calc',JSON.stringify({flow,eff:eff*100,plants,dose,interval,start:$('#calc-start').value,end:$('#calc-end').value,temp,rh}))}catch(_){ }return{duration,cycles,start,end,interval,daily}}
$$('.calc').forEach(i=>i.addEventListener('input',calcHydraulics));$('#btn-generate-schedule').onclick=()=>{const c=calcHydraulics();if(!c){toast('Не хватает данных','Проверьте расход, подачу, интервал и период.','warn');return}const dur=Math.max(1,Math.min(3600,Math.round(c.duration))),slots=[];for(let t=c.start;t<c.end&&slots.length<MAX_SLOTS;t+=c.interval)slots.push({h:Math.floor(t/60),m:t%60,d:dur});if(!slots.length)return;if(slots.length>=MAX_SLOTS&&c.start+c.interval*MAX_SLOTS<c.end)toast('График ограничен',`Созданы первые ${MAX_SLOTS} циклов.`,'warn',5000);switchTab('schedule');const box=$('#schedule-list');box.innerHTML='';slots.forEach(s=>box.appendChild(slotRow(s.h,s.m,s.d)));scheduleLoaded=true;markScheduleDirty();updateScheduleSummary();toast('Создан черновик','Проверьте график и нажмите «Сохранить».')};

async function loadConfig(){try{const d=await api('/api/config');$('#cfg-ssid').value=d.ssid||'';$('#cfg-tz').value=Number.isFinite(+d.tz)?d.tz:3;$('#cfg-pass').value='';$('#cfg-pass').placeholder=d.has_pass?'Пароль сохранён — пустое поле его не меняет':'Введите пароль сети'}catch(e){toast('Настройки не загружены',e.message,'error',5000)}}
$('#pass-toggle').onclick=()=>{const i=$('#cfg-pass'),show=i.type==='password';i.type=show?'text':'password';$('#pass-toggle').textContent=show?'⊘':'◉';$('#pass-toggle').setAttribute('aria-label',show?'Скрыть пароль':'Показать пароль')};$('#tz-browser').onclick=()=>{const tz=-new Date().getTimezoneOffset()/60;if(Number.isInteger(tz)&&tz>=-12&&tz<=14){$('#cfg-tz').value=tz;toast('Часовой пояс подставлен',`UTC${tz>=0?'+':''}${tz}`)}else toast('Не удалось определить','Укажите UTC вручную.','warn')};
$('#btn-save-config').onclick=async()=>{const ssid=$('#cfg-ssid').value.trim(),pass=$('#cfg-pass').value,tz=+$('#cfg-tz').value;if(!ssid||ssid.length>32){toast('Проверьте SSID','Название сети должно содержать 1–32 символа.','warn');return}if(pass.length>63||tz<-12||tz>14||!Number.isInteger(tz)){toast('Проверьте настройки','Пароль до 63 символов, UTC — целое число от −12 до +14.','warn');return}if(!await ask('Применить сетевые настройки?','Контроллер сохранит параметры и перезагрузится. Соединение временно прервётся.','Сохранить'))return;try{await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid,pass,tz})});toast('Настройки сохранены','Контроллер перезагружается.','ok',6000);setConnection(false)}catch(e){toast('Не удалось сохранить',e.message,'error',5000)}};

async function loadDiagnostics(){try{const d=await api('/api/diagnostics');$('#diag-version').textContent=`${d.version||'dev'} · ${d.build||'?'}`;$('#diag-heap').textContent=bytes(d.free_heap);$('#diag-minheap').textContent=bytes(d.min_free_heap);$('#diag-flash').textContent=bytes(d.flash_size);$('#diag-sketch').textContent=bytes(d.sketch_size);$('#diag-reset').textContent=d.reset_reason_text||String(d.reset_reason??'—');$('#diag-api').textContent='v'+(d.api_version??'—')}catch(e){toast('Диагностика недоступна',e.message,'error',4200)}}$('#btn-refresh-diag').onclick=loadDiagnostics;
$('#btn-reboot').onclick=async()=>{if(!await ask('Перезагрузить контроллер?','Активный ручной полив остановится. Расписание и настройки останутся в памяти.','Перезагрузить',true))return;try{await api('/api/reboot',{method:'POST'});toast('Перезагрузка','Соединение восстановится после запуска контроллера.');setConnection(false)}catch(e){toast('Команда не отправлена',e.message,'error',5000)}};

$('#btn-export').onclick=async()=>{try{const [status,schedule,config]=await Promise.all([api('/api/status'),api('/api/schedule'),api('/api/config')]);const payload={format:'hydroesp-backup-v1',created_utc:new Date().toISOString(),device:{version:status.version,build:status.build,ssid:config.ssid,tz:config.tz},schedule};const blob=new Blob([JSON.stringify(payload,null,2)+'\n'],{type:'application/json'}),url=URL.createObjectURL(blob),a=document.createElement('a');a.href=url;a.download=`hydroesp-backup-${new Date().toISOString().slice(0,10)}.json`;a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);toast('Резервная копия создана','Пароль Wi‑Fi не экспортировался.')}catch(e){toast('Экспорт не выполнен',e.message,'error',5000)}};
$('#btn-import').onclick=()=>$('#import-file').click();$('#import-file').onchange=async()=>{const file=$('#import-file').files[0];if(!file)return;try{const data=JSON.parse(await file.text()),schedule=Array.isArray(data)?data:data.schedule;if(!Array.isArray(schedule)||schedule.length>MAX_SLOTS)throw new Error('неподдерживаемый формат или слишком много циклов');switchTab('schedule');const box=$('#schedule-list');box.innerHTML='';schedule.forEach(s=>box.appendChild(slotRow(+s.h,+s.m,+s.d)));scheduleLoaded=true;markScheduleDirty();updateScheduleSummary();toast('Расписание импортировано','Оно ещё не записано: проверьте и нажмите «Сохранить».')}catch(e){toast('Импорт не выполнен',e.message,'error',5000)}finally{$('#import-file').value=''}};

const dz=$('#drop-zone'),fi=$('#ota-file');dz.onclick=()=>fi.click();dz.onkeydown=e=>{if(e.key==='Enter'||e.key===' '){e.preventDefault();fi.click()}};dz.ondragover=e=>{e.preventDefault();dz.classList.add('drag')};dz.ondragleave=()=>dz.classList.remove('drag');dz.ondrop=e=>{e.preventDefault();dz.classList.remove('drag');if(e.dataTransfer.files[0])uploadFirmware(e.dataTransfer.files[0])};fi.onchange=()=>{if(fi.files[0])uploadFirmware(fi.files[0])};
async function uploadFirmware(file){if(!file.name.toLowerCase().endsWith('.bin')){toast('Неверный файл','Нужен бинарный файл прошивки .bin.','warn');return}if(file.size<10000){toast('Файл подозрительно маленький',`${Math.round(file.size/1024)} КБ. Проверьте сборку.`,'warn',5000);return}if(!await ask('Обновить прошивку?',`${file.name}, ${Math.round(file.size/1024)} КБ. Не отключайте питание во время записи.`,'Обновить'))return;const fd=new FormData();fd.append('file',file);const xhr=new XMLHttpRequest();xhr.open('POST','/ota/upload',true);$('#ota-progress').classList.remove('hidden');$('#ota-text').textContent=file.name;$('#ota-bar').style.width='0%';$('#ota-percent').textContent='0%';xhr.upload.onprogress=e=>{if(e.lengthComputable){const p=Math.round(e.loaded/e.total*100);$('#ota-bar').style.width=p+'%';$('#ota-percent').textContent=p+'%'}};xhr.onload=()=>{if(xhr.status===200&&xhr.responseText.trim()==='OK'){toast('Прошивка записана','Контроллер перезагружается.','ok',6000);$('#ota-text').textContent='Готово. Перезагрузка…';setConnection(false)}else{toast('Ошибка OTA',xhr.responseText||`HTTP ${xhr.status}`,'error',6000);$('#ota-text').textContent='Ошибка обновления'}};xhr.onerror=()=>{toast('Соединение прервано','Не удалось завершить загрузку прошивки.','error',6000);$('#ota-text').textContent='Ошибка соединения'};xhr.send(fd)}

try{const s=JSON.parse(localStorage.getItem('hydro_calc')||'null');if(s){$('#calc-flow').value=s.flow??2;$('#calc-eff').value=s.eff??85;$('#calc-plants').value=s.plants??20;$('#calc-dose').value=s.dose??50;$('#calc-interval').value=s.interval??30;$('#calc-start').value=s.start||'06:00';$('#calc-end').value=s.end||'20:00';$('#calc-temp').value=s.temp??24;$('#calc-rh').value=s.rh??65}}catch(_){ }
calcHydraulics();updateStatus();setInterval(updateStatus,2000);
</script>
</body>
</html>
)rawliteral";
