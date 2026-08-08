#!/usr/bin/env python3
"""One-shot materializer for Web OTA post-reboot verification; removed before merge."""

from pathlib import Path
import re

path = Path(__file__).resolve().parents[1] / "src" / "web_ui_v2.h"
text = path.read_text(encoding="utf-8")
pattern = re.compile(r"function uploadFirmware\(file\)\{.*?xhr\.send\(fd\)\}", re.S)
match = pattern.search(text)
if not match:
    raise SystemExit("Web OTA uploadFirmware function not found")
if len(pattern.findall(text)) != 1:
    raise SystemExit("Web OTA uploadFirmware function is not unique")

replacement = r'''const otaSleep=ms=>new Promise(resolve=>setTimeout(resolve,ms));
async function otaStatus(timeout=1600){const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),timeout);try{const r=await fetch('/api/status',{cache:'no-store',signal:controller.signal});if(!r.ok)return null;const data=await r.json();return data&&typeof data==='object'?data:null}catch(_){return null}finally{clearTimeout(timer)}}
async function waitForOtaReboot(before,timeout=45000){const deadline=Date.now()+timeout,beforeUptime=Number(before?.uptime),beforeBuild=String(before?.build||'');let sawOffline=false,last=null;while(Date.now()<deadline){const current=await otaStatus();if(!current){sawOffline=true;await otaSleep(700);continue}last=current;const uptime=Number(current.uptime),build=String(current.build||''),uptimeReset=Number.isFinite(beforeUptime)&&Number.isFinite(uptime)&&uptime<beforeUptime,buildChanged=!!beforeBuild&&!!build&&build!==beforeBuild;if(!before||sawOffline||uptimeReset||buildChanged)return current;await otaSleep(700)}return null}
async function uploadFirmware(file){const before=await otaStatus(2000),fd=new FormData();fd.append('file',file);const xhr=new XMLHttpRequest();xhr.open('POST','/ota/upload',true);xhr.timeout=120000;$('#ota-progress').classList.remove('hidden');$('#ota-text').textContent=`Передача: ${file.name}`;$('#ota-bar').style.width='0%';$('#ota-percent').textContent='0%';xhr.upload.onprogress=e=>{if(e.lengthComputable){const p=Math.min(99,Math.round(e.loaded/e.total*100));$('#ota-bar').style.width=p+'%';$('#ota-percent').textContent=p+'%'}};xhr.onload=async()=>{if(xhr.status===200&&xhr.responseText.trim()==='OK'){$('#ota-bar').style.width='100%';$('#ota-percent').textContent='100%';$('#ota-text').textContent='Прошивка принята. Перезагрузка…';toast('Прошивка принята','Ожидаю возвращения контроллера после перезагрузки.','ok',5000);setConnection(false);const after=await waitForOtaReboot(before);if(after){const build=after.build?` Сборка: ${after.build}.`:'';$('#ota-text').textContent='Готово. Контроллер снова доступен.';toast('Обновление завершено',`Контроллер снова доступен.${build}`,'ok',7000);setConnection(true);updateStatus()}else{$('#ota-text').textContent='Прошивка принята, но возврат контроллера не подтверждён.';toast('Нужна проверка','Прошивка была принята, но контроллер не вернулся в сеть за 45 секунд. Проверьте IP на OLED.','warn',9000)}}else{$('#ota-text').textContent='Ошибка обновления';toast('Ошибка OTA',xhr.responseText||`HTTP ${xhr.status}`,'error',7000)}};xhr.onerror=()=>{toast('Соединение прервано','Передача OTA не была подтверждена. Проверьте состояние контроллера.','error',7000);$('#ota-text').textContent='Передача не подтверждена'};xhr.ontimeout=()=>{toast('Таймаут OTA','Контроллер не подтвердил приём прошивки.','error',7000);$('#ota-text').textContent='Таймаут передачи'};xhr.send(fd)}'''

text = text[:match.start()] + replacement + text[match.end():]
path.write_text(text, encoding="utf-8")
print("materialized Web OTA post-reboot verification")
