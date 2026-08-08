#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/web_ui_v2.h"

MOCK = {
    "/api/status": {
        "api_version": 3,
        "version": "v1.3-preview",
        "build": "9c5b0f1a",
        "time": "08:42:16",
        "date": "08.08.2026",
        "time_synced": True,
        "uptime": 186420,
        "relay": False,
        "relay_remaining": 0,
        "relay_progress": -1,
        "pump_source": "none",
        "ssid": "Greenhouse",
        "rssi": -56,
        "ip": "192.168.1.42",
        "ap_mode": False,
        "next": "09:00",
        "schedule_count": 18,
        "automation_enabled": True,
        "pump_flow_lpm": 1.742,
        "delivery_efficiency_pct": 86,
        "hydraulics_calibrated": True,
        "calibration_sample_count": 3,
        "calibration_cv_pct": 2.7,
        "calibration_protocol_version": 1,
        "calibration_epoch": 1786174936,
        "calibrated_at": "2026-08-08 08:22:16",
        "event_count": 6,
    },
    "/api/schedule": [
        {"h": 6, "m": 0, "d": 120}, {"h": 6, "m": 40, "d": 90},
        {"h": 7, "m": 20, "d": 90}, {"h": 8, "m": 0, "d": 90},
        {"h": 9, "m": 0, "d": 105}, {"h": 10, "m": 0, "d": 105},
        {"h": 11, "m": 0, "d": 105}, {"h": 12, "m": 0, "d": 105},
        {"h": 13, "m": 0, "d": 105}, {"h": 14, "m": 0, "d": 105},
        {"h": 15, "m": 0, "d": 105}, {"h": 16, "m": 0, "d": 105},
        {"h": 17, "m": 0, "d": 105}, {"h": 18, "m": 0, "d": 105},
        {"h": 18, "m": 40, "d": 90}, {"h": 19, "m": 20, "d": 90},
        {"h": 20, "m": 0, "d": 90}, {"h": 20, "m": 40, "d": 75},
    ],
    "/api/config": {"ssid": "Greenhouse", "has_pass": True, "tz": 3},
    "/api/hydraulics": {
        "flow_lpm": 1.742,
        "efficiency_pct": 86,
        "calibrated": True,
        "sample_count": 3,
        "cv_pct": 2.7,
        "protocol_version": 1,
        "calibration_epoch": 1786174936,
        "calibrated_at": "2026-08-08 08:22:16",
    },
    "/api/diagnostics": {
        "api_version": 3,
        "version": "v1.3-preview",
        "build": "9c5b0f1a",
        "free_heap": 192144,
        "min_free_heap": 174632,
        "flash_size": 4194304,
        "sketch_size": 1032848,
        "free_sketch_space": 1015808,
        "reset_reason_text": "питание включено",
        "event_log_capacity": 32,
        "event_log_session_only": True,
    },
    "/api/events": {
        "session_only": True,
        "events": [
            {"sequence": 6, "uptime": 1702, "timestamp": "2026-08-08 08:42:02", "type": "pump_stop", "source": "schedule", "reason": "timeout", "value": 0},
            {"sequence": 5, "uptime": 1597, "timestamp": "2026-08-08 08:40:17", "type": "pump_start", "source": "schedule", "reason": "none", "value": 105},
            {"sequence": 4, "uptime": 840, "timestamp": "2026-08-08 08:27:40", "type": "hydraulics_saved", "source": "none", "reason": "none", "value": 1742},
            {"sequence": 3, "uptime": 620, "timestamp": "2026-08-08 08:24:00", "type": "automation_enabled", "source": "none", "reason": "none", "value": 0},
            {"sequence": 2, "uptime": 410, "timestamp": "2026-08-08 08:20:30", "type": "automation_paused", "source": "none", "reason": "none", "value": 0},
            {"sequence": 1, "uptime": 1, "timestamp": "2026-08-08 08:13:41", "type": "boot", "source": "none", "reason": "none", "value": 1},
        ],
    },
}


def extract_html() -> str:
    text = SOURCE.read_text(encoding="utf-8")
    match = re.search(r'R"rawliteral\((.*)\)rawliteral";', text, flags=re.S)
    if not match:
        raise SystemExit("embedded UI literal not found")
    return match.group(1)


def mock_script() -> str:
    payload = json.dumps(MOCK, ensure_ascii=False)
    return f"""<script>
const __HYDRO_MOCK__={payload};
const __realFetch=window.fetch.bind(window);
window.fetch=async function(input,options={{}}){{
  const raw=typeof input==='string'?input:input.url;
  const url=new URL(raw,location.href);
  const path=url.pathname;
  if(Object.prototype.hasOwnProperty.call(__HYDRO_MOCK__,path)){{
    const body=JSON.stringify(__HYDRO_MOCK__[path]);
    return new Response(body,{{status:200,headers:{{'Content-Type':'application/json'}}}});
  }}
  if((options.method||'GET').toUpperCase()==='POST'){{
    return new Response('{{\"status\":\"ok\"}}',{{status:200,headers:{{'Content-Type':'application/json'}}}});
  }}
  return __realFetch(input,options);
}};
window.addEventListener('load',()=>{{
  const tab=new URLSearchParams(location.search).get('tab');
  if(!tab)return;
  setTimeout(()=>document.querySelector(`[data-tab="${{tab}}"]`)?.click(),250);
}});
</script>"""


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("output", nargs="?", default="ui-preview.html")
    parser.add_argument("--mock", action="store_true")
    args = parser.parse_args()

    html = extract_html()
    if args.mock:
        html = html.replace("<script>", mock_script() + "\n<script>", 1)
    target = Path(args.output)
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(html, encoding="utf-8")
    print(f"wrote {target} ({len(html)} bytes)")


if __name__ == "__main__":
    main()
